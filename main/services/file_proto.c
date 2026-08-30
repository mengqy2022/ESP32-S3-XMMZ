/*
 * file_proto.c — 串口文件传输协议服务
 * 通过标准输入/输出 (控制台 UART0 = CH340 = PC 的 COM 口) 与 PC 工具
 * (tools/inventory_tool.py, smoke_test.py) 通信, 在 PC 与设备 SD 卡之间
 * 直接传文件, 无需拔卡. 协议见 file_proto.h.
 * 注意:
 *   - 读取用非阻塞 fcntl(O_NONBLOCK), 不阻塞控制台
 *   - GET 发原始数据前全局关日志, 防止日志行混入文件数据
 *   - 路径强制限 /sdcard 前缀, 防越权
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/uart.h"
#include "driver/uart_vfs.h"
#include "sd_card.h"
#include "file_proto.h"

static const char *TAG = "file_proto";

/* 传输块缓冲: 必须静态 (任务栈仅 4096, 局部 4KB 数组会栈溢出 → panic 重启) */
static char s_fp_chunk[4096];

/* 传输进行中标志: 主菜单据此跳过屏保 (壁纸加载会抢占 SD/内部 DMA 内存, 干扰传输) */
static volatile bool s_busy = false;
bool file_proto_active(void) { return s_busy; }

#define FP_LINE_MAX   300
#define FP_MAX_SIZE   (64 * 1024 * 1024)   /* 单文件上限 64MB */
#define FP_RX_TIMEOUT_MS 30000             /* 接收原始数据超时 */

/* 发送一行文本并立即刷出 */
static void fp_send(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    fflush(stdout);
}

/* 路径安全: 必须 /sdcard 前缀, 且不含 ".." */
static bool fp_path_ok(const char *p)
{
    if (!p || strncmp(p, "/sdcard", 7) != 0) return false;
    if (strstr(p, "..")) return false;
    return true;
}

/* 确保父目录存在 (mkdir -p 语义), 失败不致命 */
static void fp_mkdirs(const char *path)
{
    char tmp[FP_LINE_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    char *p = tmp + 1;   /* 跳过开头 '/' */
    while ((p = strchr(p, '/')) != NULL) {
        *p = 0;
        mkdir(tmp, 0755);
        *p = '/';
        p++;
    }
}

/* 非阻塞读 1 字节: 控制台无 UART 驱动, VFS 直接读寄存器;
 * 必须先 fcntl(O_NONBLOCK) 否则 read 会永久忙等 */
static int fp_read_char(char *c)
{
    return read(0, c, 1) == 1;
}

/* 读一行 (直到 \n 或 \r); 等新行或空闲超时才返回, 避免分片行被当完整命令处理.
 * 返回长度; 无数据且超时返回 0. */
static int fp_read_line(char *buf, int cap)
{
    int i = 0;
    uint32_t last = xTaskGetTickCount();
    for (;;) {
        char c;
        if (fp_read_char(&c)) {
            if (c == '\n' || c == '\r') { buf[i] = 0; return i; }
            if (i < cap - 1) buf[i++] = c;
            last = xTaskGetTickCount();
        } else {
            uint32_t idle = xTaskGetTickCount() - last;
            if (i > 0 && idle > pdMS_TO_TICKS(100)) break;   /* 行内容空闲 100ms → 行结束 */
            if (i == 0 && idle > pdMS_TO_TICKS(100)) return 0;
            vTaskDelay(pdMS_TO_TICKS(2));
        }
    }
    buf[i] = 0;
    return i;
}

/* 读 n 字节原始数据 (跨多次到达) */
static int fp_read_raw(char *buf, int n, int timeout_ms)
{
    int got = 0;
    uint32_t start = xTaskGetTickCount();
    while (got < n) {
        ssize_t r = read(0, buf + got, n - got);
        if (r > 0) { got += (int)r; continue; }
        if ((xTaskGetTickCount() - start) > pdMS_TO_TICKS(timeout_ms)) break;
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    return got;
}

/* 发送原始字节 (GET 数据); 用 fwrite 走 stdout (与 printf 同路径, 可靠;
 * 勿用 write(1,...) — 控制台 VFS 无驱动模式下实测会返回 -1 卡死);
 * 传输期间全局关日志防混入 */
static void fp_send_raw(const char *buf, int n)
{
    esp_log_level_set("*", ESP_LOG_NONE);
    size_t off = 0;
    while (off < (size_t)n) {
        size_t w = fwrite(buf + off, 1, (size_t)n - off, stdout);
        if (w > 0) off += w;
        else vTaskDelay(pdMS_TO_TICKS(1));
    }
    fflush(stdout);
    esp_log_level_set("*", ESP_LOG_INFO);
}

/* ---------- 命令处理 ---------- */

static void fp_cmd_list(const char *path)
{
    if (!fp_path_ok(path)) { fp_send("FILE:ERR|BAD PATH"); return; }
    DIR *d = opendir(path);
    if (!d) { fp_send("FILE:ERR|NO DIR"); return; }
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;
        char full[FP_LINE_MAX];
        snprintf(full, sizeof(full), "%s/%s", path, e->d_name);
        struct stat st;
        long size = 0;
        const char *type = "file";
        if (stat(full, &st) == 0) {
            size = (long)st.st_size;
            if (S_ISDIR(st.st_mode)) type = "dir";
        }
        /* 名称/路径中不允许 '|', 否则 PC 端解析错位 */
        if (strchr(e->d_name, '|')) continue;
        fp_send("FILE:ENTRY|%s|%ld|%s", e->d_name, size, type);
    }
    closedir(d);
    fp_send("FILE:DONE");
}

static void fp_cmd_put(char *args)   /* args: <path>|<size> */
{
    if (!args) { fp_send("FILE:ERR|BAD CMD"); return; }
    char *sep = strchr(args, '|');
    if (!sep) { fp_send("FILE:ERR|BAD CMD"); return; }
    *sep = 0;
    const char *path = args;
    long size = atol(sep + 1);
    if (!fp_path_ok(path)) { fp_send("FILE:ERR|BAD PATH"); return; }
    if (size <= 0 || size > FP_MAX_SIZE) { fp_send("FILE:ERR|BAD SIZE"); return; }
    if (!sd_card_present()) { fp_send("FILE:ERR|NO SD"); return; }

    fp_send("FILE:READY");
    fp_mkdirs(path);
    ESP_LOGI(TAG, "PUT mkdirs ok");          /* 临时调试: 定位卡点 */
    FILE *f = fopen(path, "wb");
    ESP_LOGI(TAG, "PUT fopen ok f=%p", (void *)f);   /* 临时调试 */
    if (!f) { fp_send("FILE:ERR|OPEN FAIL"); return; }

    /* 流式接收: 4KB 一块读 UART 并写盘, 不用大 malloc, 大文件不会中途超时.
     * 每块超时 5s (115200 下 4KB≈0.36s), 卡死则报 SHORT 并删半截文件.
     * 注意: chunk 是指针, 必须用 sizeof(s_fp_chunk) 取数组大小, 不能 sizeof(chunk)! */
    long remaining = size;
    char *chunk = s_fp_chunk;
    ESP_LOGI(TAG, "PUT start %s size=%ld", path, size);
    while (remaining > 0) {
        int want = remaining > (long)sizeof(s_fp_chunk) ? (int)sizeof(s_fp_chunk) : (int)remaining;
        int got = fp_read_raw(chunk, want, 5000);
        if (got != want) {
            fclose(f);
            remove(path);
            fp_send("FILE:ERR|SHORT %ld/%ld", (long)(size - remaining + got), size);
            return;
        }
        if (fwrite(chunk, 1, (size_t)got, f) != (size_t)got) {
            fclose(f);
            remove(path);
            fp_send("FILE:ERR|WRITE FAIL");
            return;
        }
        remaining -= got;
    }
    fclose(f);
    ESP_LOGI(TAG, "PUT done, sending OK");
    fp_send("FILE:OK");
}

static void fp_cmd_get(const char *path)
{
    if (!fp_path_ok(path)) { fp_send("FILE:ERR|BAD PATH"); return; }
    FILE *f = fopen(path, "rb");
    if (!f) { fp_send("FILE:ERR|NO FILE"); return; }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0 || size > FP_MAX_SIZE) { fclose(f); fp_send("FILE:ERR|BAD SIZE"); return; }

    printf("FILE:READY|%ld\n", size);
    fflush(stdout);

    /* 流式发送: 4KB 一块读文件送 UART, 不用大 malloc */
    long remaining = size;
    char *chunk = s_fp_chunk;
    while (remaining > 0) {
        int want = remaining > (long)sizeof(s_fp_chunk) ? (int)sizeof(s_fp_chunk) : (int)remaining;
        size_t rd = fread(chunk, 1, (size_t)want, f);
        if (rd == 0) break;
        fp_send_raw(chunk, (int)rd);
        remaining -= (long)rd;
    }
    fclose(f);
    fp_send("FILE:DONE");
}

static void fp_cmd_del(const char *path)
{
    if (!fp_path_ok(path)) { fp_send("FILE:ERR|BAD PATH"); return; }
    if (remove(path) == 0) fp_send("FILE:OK");
    else fp_send("FILE:ERR|DEL FAIL");
}

/* FILE:PEEK|<path>|<offset>|<len> -> FILE:READY|<实际长度> + 原始字节 + FILE:DONE
 * 诊断用: 读文件任意范围 (如检查 mp3 头部), 避免整文件下载 */
static void fp_cmd_peek(char *args)
{
    if (!args) { fp_send("FILE:ERR|BAD CMD"); return; }
    char *sep1 = strchr(args, '|');
    if (!sep1) { fp_send("FILE:ERR|BAD CMD"); return; }
    *sep1 = 0;
    const char *path = args;
    char *sep2 = strchr(sep1 + 1, '|');
    if (!sep2) { fp_send("FILE:ERR|BAD CMD"); return; }
    *sep2 = 0;
    long off = atol(sep1 + 1);
    long len = atol(sep2 + 1);
    if (!fp_path_ok(path)) { fp_send("FILE:ERR|BAD PATH"); return; }
    if (off < 0 || len <= 0 || len > (long)sizeof(s_fp_chunk)) { fp_send("FILE:ERR|BAD RANGE"); return; }
    FILE *f = fopen(path, "rb");
    if (!f) { fp_send("FILE:ERR|NO FILE"); return; }
    if (fseek(f, off, SEEK_SET) != 0) { fclose(f); fp_send("FILE:ERR|BAD OFF"); return; }
    size_t rd = fread(s_fp_chunk, 1, (size_t)len, f);
    fclose(f);
    printf("FILE:READY|%d\n", (int)rd);
    fflush(stdout);
    fp_send_raw(s_fp_chunk, (int)rd);
    fp_send("FILE:DONE");
}

/* FILE:MOVE|<src>|<dst> -> FILE:OK / FILE:ERR|<原因> (SD 卡上重命名/移动) */
static void fp_cmd_move(char *args)
{
    if (!args) { fp_send("FILE:ERR|BAD CMD"); return; }
    char *sep = strchr(args, '|');
    if (!sep) { fp_send("FILE:ERR|BAD CMD"); return; }
    *sep = 0;
    const char *src = args;
    const char *dst = sep + 1;
    if (!fp_path_ok(src) || !fp_path_ok(dst)) { fp_send("FILE:ERR|BAD PATH"); return; }
    if (rename(src, dst) == 0) fp_send("FILE:OK");
    else fp_send("FILE:ERR|RENAME FAIL");
}

/* FILE:MEM -> 报告空闲内部 RAM, 并测试各尺寸内部块分配 (模拟器可行性依据) */
static void fp_cmd_mem(void)
{
    size_t free_int = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    size_t free_ext = heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    printf("FILE:MEM|free_internal=%d B|free_psram=%d B\n",
           (int)free_int, (int)free_ext);
    /* 逐尺寸测内部连续分配: 2,4,6,...,20KB (模拟器状态/任务栈需要) */
    static void *s_probe[10];
    for (int i = 0; i < 10; i++) {
        size_t req = (i + 1) * 2 * 1024;
        void *p = heap_caps_malloc(req, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        s_probe[i] = p;
        int ok = (p != NULL);
        if (ok) memset(p, 0, 64);
        printf("FILE:MEM|alloc_%dKB=%s\n", (int)(req / 1024), ok ? "OK" : "FAIL");
    }
    for (int i = 0; i < 10; i++) if (s_probe[i]) heap_caps_free(s_probe[i]);
}

static void fp_task(void *arg)
{
    /* 标准输入置非阻塞: 控制台无 UART 驱动, VFS 直接读寄存器,
     * 不置 O_NONBLOCK 时 read 会永久忙等无数据 */
    int fl = fcntl(0, F_GETFL, 0);
    if (fl >= 0) fcntl(0, F_SETFL, fl | O_NONBLOCK);
    else ESP_LOGW(TAG, "fcntl failed: %d", fl);

    ESP_LOGI(TAG, "file protocol ready (115200, FILE:* commands)");
    char line[FP_LINE_MAX];
    for (;;) {
        int n = fp_read_line(line, sizeof(line));
        if (n > 0 && strncmp(line, "FILE:", 5) == 0) {
            char *cmd = line + 5;
            char *args = strchr(cmd, '|');
            if (args) { *args = 0; args++; }
            if (strcmp(cmd, "PING") == 0) fp_send("FILE:PONG");
            else if (strcmp(cmd, "LIST") == 0) fp_cmd_list(args ? args : "/sdcard");
            else if (strcmp(cmd, "PUT") == 0 || strcmp(cmd, "GET") == 0) {
                /* 传输期间置忙标志: 主菜单跳过屏保, 避免壁纸抢占 SD/内部 DMA 内存 */
                s_busy = true;
                if (strcmp(cmd, "PUT") == 0) fp_cmd_put(args);
                else fp_cmd_get(args ? args : "");
                s_busy = false;
            } else if (strcmp(cmd, "DEL") == 0) fp_cmd_del(args ? args : "");
            else if (strcmp(cmd, "PEEK") == 0) fp_cmd_peek(args);
            else if (strcmp(cmd, "MOVE") == 0) fp_cmd_move(args);
            else if (strcmp(cmd, "MEM") == 0) fp_cmd_mem();
            else fp_send("FILE:ERR|UNKNOWN");
        } else {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

esp_err_t file_proto_start(void)
{
    /* 文件传输要求字节透明: 关闭控制台 UART 的行结束转换 (\n<->\r\n),
     * 否则二进制文件中的 0x0A/0x0D 会被改写 */
    uart_vfs_dev_port_set_rx_line_endings(UART_NUM_0, ESP_LINE_ENDINGS_LF);
    uart_vfs_dev_port_set_tx_line_endings(UART_NUM_0, ESP_LINE_ENDINGS_LF);
    /* 注意: 不安装 UART 驱动 (esp_vfs_console 默认无驱动直读寄存器)。
     * 装驱动虽能防大文件 FIFO 溢出, 但会占用内部 DMA 内存 (~4KB), 实测把
     * sdmmc 的 DMA 缓冲挤爆 (sdmmc_read_sectors: not enough mem) → 连 LIST 都失败。
     * 本板内部 DMA RAM 仅 ~190KB, 大文件直传不可行 (用户改用手动拷入 SD)。 */

    xTaskCreatePinnedToCore(fp_task, "file_proto", 4096, NULL, 5, NULL, 1);
    return ESP_OK;
}
