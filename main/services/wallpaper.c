/*
 * wallpaper.c — 壁纸模块 (屏保用)
 * 来源优先级: SD 卡 /sdcard/wallpaper 目录的 jpg/jpeg/png/bmp (LVGL FS_POSIX 路径, 自动解码)
 *            > 内置程序生成 (渐变/星空/波纹/几何/网格 兜底)
 */
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "lvgl.h"
#include "wallpaper.h"
#include "safe_string.h"

static const char *TAG = "wp";

static uint8_t *s_buf = NULL;              /* PSRAM 缓冲 (内置 RGB565) */
static lv_image_dsc_t s_dsc = {0};

/* SD 卡壁纸文件名列表 */
static char s_sd_files[WP_SD_MAX][40];
static int s_sd_count = 0;

/* ---------- 内置壁纸生成 (兜底) ---------- */

static void wp_gradient(uint16_t *px, uint32_t c_top, uint32_t c_bot)
{
    int r0 = (c_top >> 16) & 0xFF, g0 = (c_top >> 8) & 0xFF, b0 = c_top & 0xFF;
    int r1 = (c_bot >> 16) & 0xFF, g1 = (c_bot >> 8) & 0xFF, b1 = c_bot & 0xFF;
    for (int y = 0; y < WP_H; y++) {
        float t = (float)y / (WP_H - 1);
        int r = r0 + (int)((r1 - r0) * t);
        int g = g0 + (int)((g1 - g0) * t);
        int b = b0 + (int)((b1 - b0) * t);
        uint16_t c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        for (int x = 0; x < WP_W; x++) px[y * WP_W + x] = c;
    }
}

static void wp_stars(uint16_t *px, uint32_t seed)
{
    for (int i = 0; i < WP_W * WP_H; i++) px[i] = 0x0000;
    srand(seed);
    int n = 260 + rand() % 120;
    for (int i = 0; i < n; i++) {
        int x = rand() % WP_W, y = rand() % WP_H;
        int b = 180 + rand() % 76;
        uint16_t c = ((b >> 3) << 11) | ((b >> 2) << 5) | (b >> 3);
        px[y * WP_W + x] = c;
    }
}

static void wp_waves(uint16_t *px)
{
    for (int y = 0; y < WP_H; y++)
        for (int x = 0; x < WP_W; x++) {
            float v = sinf(x * 0.03f) * 0.5f + sinf(x * 0.011f + y * 0.02f) * 0.5f;
            v = (v + 1.0f) * 0.5f;
            int r = (int)(40 + v * 180), g = (int)(20 + v * 60), b = (int)(90 + v * 150);
            px[y * WP_W + x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        }
}

static void wp_geometric(uint16_t *px)
{
    for (int y = 0; y < WP_H; y++)
        for (int x = 0; x < WP_W; x++) {
            int cx = x - 160, cy = y - 120;
            float d = sqrtf(cx * cx + cy * cy);
            int ring = ((int)(d / 18) % 2) == 0;
            int r = ring ? 200 : 30, g = ring ? 60 : 120, b = ring ? 30 : 180;
            px[y * WP_W + x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        }
}

static void wp_grid(uint16_t *px)
{
    for (int y = 0; y < WP_H; y++)
        for (int x = 0; x < WP_W; x++) {
            bool line = (x % 40 < 2) || (y % 40 < 2);
            bool dot = ((x % 40) == 20 && (y % 40) == 20);
            int r, g, b;
            if (dot) { r = 255; g = 220; b = 80; }
            else if (line) { r = 60; g = 160; b = 255; }
            else { r = 16; g = 30; b = 46; }
            px[y * WP_W + x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        }
}

static void (*const s_gen[])(uint16_t *) = { NULL, NULL, wp_waves, wp_geometric, wp_grid };

/* ---------- SD 卡扫描 ---------- */

static int sd_scan(void)
{
    DIR *d = opendir("/sdcard/wallpaper");
    if (!d) {
        ESP_LOGW(TAG, "no /sdcard/wallpaper (SD 卡未格式化或目录不存在)");
        return 0;
    }
    struct dirent *e;
    int n = 0;
    while ((e = readdir(d)) && n < WP_SD_MAX) {
        const char *name = e->d_name;
        const char *dot = strrchr(name, '.');
        if (!dot) continue;
        if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0 ||
            strcasecmp(dot, ".png") == 0 || strcasecmp(dot, ".bmp") == 0) {
            xm_strlcpy(s_sd_files[n], name, sizeof(s_sd_files[n]));
            n++;
        }
    }
    closedir(d);
    /* 按文件名排序, 保证顺序稳定 */
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (strcmp(s_sd_files[i], s_sd_files[j]) > 0) {
                char t[40];
                strcpy(t, s_sd_files[i]);
                strcpy(s_sd_files[i], s_sd_files[j]);
                strcpy(s_sd_files[j], t);
            }
    ESP_LOGI(TAG, "SD wallpapers: %d", n);
    return n;
}

/* ---------- 公开 API ---------- */

int wallpaper_sd_count(void) { return s_sd_count; }

int wallpaper_count(void)
{
    if (s_sd_count > 0) return s_sd_count;              /* SD 图优先 */
    return WP_BUILTIN_COUNT;                            /* 否则内置兜底 */
}

int wallpaper_init(void)
{
    s_sd_count = sd_scan();
    if (!s_buf) {
        s_buf = heap_caps_malloc(WP_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!s_buf) {
            ESP_LOGE(TAG, "no PSRAM for wallpaper");
            return -1;
        }
        s_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
        s_dsc.header.w = WP_W;
        s_dsc.header.h = WP_H;
        s_dsc.header.stride = WP_W * 2;
        s_dsc.data_size = WP_SIZE;
        s_dsc.data = s_buf;
    }
    ESP_LOGI(TAG, "wallpaper init: sd=%d builtin=%d", s_sd_count, WP_BUILTIN_COUNT);
    return 0;
}

int wallpaper_load(int idx, char *src_path, int path_len)
{
    /* 1) SD 卡壁纸: 返回 LVGL 文件路径 (P: -> /sdcard), 由 LVGL 解码显示 */
    if (s_sd_count > 0 && idx < s_sd_count) {
        snprintf(src_path, path_len, "P:/wallpaper/%s", s_sd_files[idx]);
        /* TJPGD 对扩展名大小写敏感, 与图库一致: 统一转小写 */
        char *dot = strrchr(src_path, '.');
        if (dot) {
            for (char *p = dot; *p; p++) *p = (char)tolower((unsigned char)*p);
        }
        return 1;
    }
    /* 2) 内置兜底 */
    if (!s_buf) return -1;
    uint16_t *px = (uint16_t *)s_buf;
    int bi = idx % WP_BUILTIN_COUNT;
    if (bi == 0) {
        wp_gradient(px, 0x0A2A5A, 0x2E86DE);
    } else if (bi == 1) {
        wp_stars(px, 20260817);
    } else {
        s_gen[bi](px);
    }
    return 0;
}

lv_image_dsc_t *wallpaper_dsc(void)
{
    return s_buf ? &s_dsc : NULL;
}
