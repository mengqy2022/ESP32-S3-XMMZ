/*
 * apps.h — 应用注册表
 */
#pragma once

#include "esp_err.h"

/* 应用入口: 运行后返回 (用户按 KEY3/KEY1 退出) */
typedef esp_err_t (*app_func_t)(void);

typedef struct {
    const char *name;
    uint16_t color;      /* 菜单色点 */
    app_func_t run;
} menu_item_t;

/* 各应用入口 */
esp_err_t app_ebook_run(void);
esp_err_t app_games_run(void);
esp_err_t app_sysinfo_run(void);
esp_err_t app_wifi_run(void);
esp_err_t app_keytest_run(void);
esp_err_t app_led_run(void);
esp_err_t app_ble_run(void);
esp_err_t app_music_run(void);
esp_err_t app_settings_run(void);
esp_err_t app_gallery_run(void);
esp_err_t app_nes_run(void);
