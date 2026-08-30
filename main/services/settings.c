/*
 * settings.c — 系统设置 (NVS namespace "sys")
 */
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "settings.h"

static const char *TAG = "settings";

#define SAVER_DEFAULT_SEC  0   /* 默认关闭屏保 (壁纸解码太吃内部 RAM, 自动屏保已取消) */

uint32_t settings_saver_timeout_sec(void)
{
    uint32_t v = SAVER_DEFAULT_SEC;
    nvs_handle_t h;
    if (nvs_open("sys", NVS_READONLY, &h) == ESP_OK) {
        nvs_get_u32(h, "sa_to", &v);
        nvs_close(h);
    }
    return v;
}

void settings_set_saver_timeout_sec(uint32_t sec)
{
    nvs_handle_t h;
    if (nvs_open("sys", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u32(h, "sa_to", sec);
        nvs_commit(h);
        nvs_close(h);
    }
    ESP_LOGI(TAG, "saver timeout = %lu s", (unsigned long)sec);
}
