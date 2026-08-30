/*
 * wifi_mgr.c — WiFi 管理 (STA)
 * 配置存 NVS (namespace "wifi"): ssid/pass; 上电自动连接; 连上后 SNTP 同步时间
 */
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_sntp.h"
#include "wifi_mgr.h"

static const char *TAG = "wifi";

#define WIFI_EVT_CONNECTED BIT0
#define WIFI_EVT_GOT_IP    BIT1

static EventGroupHandle_t s_evt = NULL;
static char s_ssid[33] = {0};
static char s_pass[65] = {0};
static char s_ip[16] = {0};
static bool s_connected = false;
static bool s_time_synced = false;
static bool s_inited = false;
static int s_retry = 0;   /* 连续失败次数 */
static volatile bool s_suspended = false; /* NES 等场景主动停 WiFi 时禁止后台重连 */

/* GCC 14 + -Werror=all 会把 strncpy(dst, src, size-1) 的潜在截断
 * 诊断为 stringop-truncation。这里不用关闭警告，而是明确实现两种语义：
 *
 * 1) copy_cstr(): 普通 C 字符串，始终保证末尾 '\0'；
 * 2) copy_wifi_field(): ESP-IDF wifi_config_t 中的定长字节字段。
 *    SSID 字段本身只有 32 byte，合法 SSID 可以刚好占满 32 byte，
 *    因此不能强制为 '\0' 额外牺牲一个字符。先清零，再最多拷满字段。 */
static void copy_cstr(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";

    size_t n = strnlen(src, dst_size - 1);
    if (n > 0) memcpy(dst, src, n);
    dst[n] = '\0';
}

static void copy_wifi_field(uint8_t *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0) return;
    memset(dst, 0, dst_size);
    if (!src) return;

    size_t n = strnlen(src, dst_size);
    if (n > 0) memcpy(dst, src, n);
}

/* ---------------- NVS 配置 ---------------- */
static void load_cfg(void)
{
    nvs_handle_t h;
    if (nvs_open("wifi", NVS_READONLY, &h) == ESP_OK) {
        size_t sz = sizeof(s_ssid);
        if (nvs_get_str(h, "ssid", s_ssid, &sz) != ESP_OK) s_ssid[0] = 0;
        sz = sizeof(s_pass);
        if (nvs_get_str(h, "pass", s_pass, &sz) != ESP_OK) s_pass[0] = 0;
        nvs_close(h);
    }
    ESP_LOGI(TAG, "saved cfg: %s", s_ssid[0] ? s_ssid : "(无)");
}

static void save_cfg(void)
{
    nvs_handle_t h;
    if (nvs_open("wifi", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_str(h, "ssid", s_ssid);
        nvs_set_str(h, "pass", s_pass);
        nvs_commit(h);
        nvs_close(h);
    }
}

/* ---------------- 事件 ---------------- */
static void sntp_start(void)
{
    /* esp_wifi_stop() does not stop lwIP SNTP. After NES resumes WiFi, GOT_IP can fire
     * again while the existing SNTP client is still running; calling setoperatingmode
     * in that state asserts in ESP-IDF 5.5. */
    if (esp_sntp_enabled()) {
        ESP_LOGI(TAG, "SNTP already running");
        return;
    }
    /* 本地时区: 北京时间 UTC+8 (不设 TZ 则 localtime_r 按 UTC 显示, 差 8 小时!) */
    setenv("TZ", "CST-8", 1);
    tzset();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "ntp.aliyun.com");
    esp_sntp_setservername(1, "pool.ntp.org");
    esp_sntp_init();
    ESP_LOGI(TAG, "SNTP started");
}

static void event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        /* 自动连接已保存配置；主动 suspend 期间绝不重连。 */
        if (!s_suspended && s_ssid[0]) {
            ESP_LOGI(TAG, "auto connect %s", s_ssid);
            wifi_config_t cfg = {0};
            copy_wifi_field(cfg.sta.ssid, sizeof(cfg.sta.ssid), s_ssid);
            copy_wifi_field(cfg.sta.password, sizeof(cfg.sta.password), s_pass);
            esp_wifi_set_config(WIFI_IF_STA, &cfg);
            esp_wifi_connect();
        }
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        s_connected = false;
        s_ip[0] = 0;
        if (s_suspended) {
            ESP_LOGI(TAG, "disconnected (intentional suspend)");
            return;
        }
        s_retry++;
        ESP_LOGW(TAG, "disconnected (retry %d/5)", s_retry);
        if (s_retry < 5 && s_ssid[0]) esp_wifi_connect();   /* 自动重连, 限 5 次 */
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;
        snprintf(s_ip, sizeof(s_ip), IPSTR, IP2STR(&e->ip_info.ip));
        s_connected = true;
        s_retry = 0;
        ESP_LOGI(TAG, "got ip: %s", s_ip);
        xEventGroupSetBits(s_evt, WIFI_EVT_CONNECTED | WIFI_EVT_GOT_IP);
        if (!s_time_synced) sntp_start();
    }
}

/* 周期性自动重连: 连接失败 5 次后不永久放弃,
 * 每 30 秒重试一次, 网络恢复后自动连上并 SNTP 同步时间 */
static void reconnect_task(void *arg)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        if (!s_suspended && !s_connected && s_ssid[0]) {
            ESP_LOGI(TAG, "periodic reconnect %s", s_ssid);
            s_retry = 0;
            esp_wifi_connect();
        }
    }
}


/* 停止 WiFi 驱动: esp_wifi_stop 释放驱动内部缓冲区 (可省大量内部 RAM).
 * 进内存吃紧的应用(如 NES 模拟器)前调用; 之后可 wifi_mgr_start 恢复. */
void wifi_mgr_stop(void)
{
    if (!s_inited) return;
    s_suspended = true;
    if (s_connected) s_connected = false;
    esp_wifi_stop();
    ESP_LOGI(TAG, "wifi stopped (freed internal RAM)");
}

void wifi_mgr_start(void)
{
    if (!s_inited) return;
    s_suspended = false;
    s_retry = 0;
    esp_wifi_start();
    ESP_LOGI(TAG, "wifi started");
    /* WIFI_EVENT_STA_START 会统一执行自动连接，不再这里重复 esp_wifi_connect()，
     * 避免日志中的 "sta is connecting, return error"。 */
}

/* ---------------- 公开 API ---------------- */
esp_err_t wifi_mgr_init(void)
{
    if (s_inited) return ESP_OK;
    s_evt = xEventGroupCreate();

    load_cfg();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    xTaskCreate(reconnect_task, "wifi_rec", 3072, NULL, 2, NULL);

    s_inited = true;
    ESP_LOGI(TAG, "wifi mgr init OK");
    return ESP_OK;
}

esp_err_t wifi_mgr_scan(wifi_ap_t *aps, int max, int *count)
{
    if (max > WIFI_SCAN_MAX_AP) max = WIFI_SCAN_MAX_AP;
    wifi_scan_config_t scan = {0};
    esp_err_t err = esp_wifi_scan_start(&scan, true);   /* 阻塞扫描 */
    if (err != ESP_OK) return err;
    uint16_t num = 0;
    esp_wifi_scan_get_ap_num(&num);
    if (num > max) num = max;
    wifi_ap_record_t *rec = calloc(num, sizeof(wifi_ap_record_t));
    if (!rec) return ESP_ERR_NO_MEM;
    esp_wifi_scan_get_ap_records(&num, rec);
    for (int i = 0; i < num; i++) {
        copy_cstr(aps[i].ssid, sizeof(aps[i].ssid), (const char *)rec[i].ssid);
        aps[i].rssi = rec[i].rssi;
        aps[i].authmode = rec[i].authmode;
    }
    *count = num;
    free(rec);
    ESP_LOGI(TAG, "scan: %d AP", num);
    return ESP_OK;
}

esp_err_t wifi_mgr_connect(const char *ssid, const char *pass)
{
    copy_cstr(s_ssid, sizeof(s_ssid), ssid);
    copy_cstr(s_pass, sizeof(s_pass), pass ? pass : "");
    save_cfg();

    wifi_config_t cfg = {0};
    copy_wifi_field(cfg.sta.ssid, sizeof(cfg.sta.ssid), s_ssid);
    copy_wifi_field(cfg.sta.password, sizeof(cfg.sta.password), s_pass);
    cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
    esp_wifi_set_config(WIFI_IF_STA, &cfg);
    s_retry = 0;
    esp_wifi_connect();
    ESP_LOGI(TAG, "connecting %s", s_ssid);
    return ESP_OK;
}

esp_err_t wifi_mgr_disconnect(void)
{
    esp_wifi_disconnect();
    return ESP_OK;
}

void wifi_mgr_forget(void)
{
    s_ssid[0] = 0;
    s_pass[0] = 0;
    nvs_handle_t h;
    if (nvs_open("wifi", NVS_READWRITE, &h) == ESP_OK) {
        nvs_erase_all(h);
        nvs_commit(h);
        nvs_close(h);
    }
    esp_wifi_disconnect();
}

bool wifi_mgr_connected(void) { return s_connected; }
const char *wifi_mgr_ssid(void) { return s_ssid; }
const char *wifi_mgr_ip(void) { return s_ip; }

bool wifi_mgr_time_synced(void)
{
    /* SNTP 同步状态 */
    if (esp_sntp_enabled() && esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
        if (!s_time_synced) {
            s_time_synced = true;
            /* 打印一次北京时间, 便于验证 time_t/时区转换 (time_t 是 64 位!) */
            time_t now = time(NULL);
            struct tm ti;
            localtime_r(&now, &ti);
            ESP_LOGI(TAG, "SNTP time synced: %04d-%02d-%02d %02d:%02d:%02d (Beijing)",
                     ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
                     ti.tm_hour, ti.tm_min, ti.tm_sec);
        }
    }
    return s_time_synced;
}

void wifi_mgr_set_power_save(bool enable)
{
    /* 省电模式(modem sleep)在弱信号下易 bcn_timeout 断连;
     * 播放流媒体(音乐)时关掉, 保证实时收包 */
    esp_wifi_set_ps(enable ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE);
    ESP_LOGI(TAG, "power save %s", enable ? "ON" : "OFF");
}
