/*
 * app_sysinfo.c — 系统信息 (LVGL)
 * KEY5: 重新检测 SD 卡 (插拔/按压卡后无需重启)
 *
 * 2026 UI: 信息区与底部提示严格分离，避免 320x240 屏幕末行重叠；
 * 同时低频刷新堆/PSRAM/电池状态，方便观察长期运行的内存健康度。
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_mac.h"
#include "esp_psram.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "buttons.h"
#include "battery.h"
#include "sd_card.h"
#include "services/ble.h"
#include "lvgl_ui.h"
#include "apps.h"

LV_FONT_DECLARE(ui_font_lvgl_10);
LV_FONT_DECLARE(book_font_lvgl);

static lv_obj_t *s_psram_label = NULL;
static lv_obj_t *s_heap_label = NULL;
static lv_obj_t *s_sd_label = NULL;
static lv_obj_t *s_bat_label = NULL;
static lv_obj_t *s_ble_label = NULL;

static lv_obj_t *info_row(lv_obj_t *parent, const char *text, int y)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_color(l, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(l, &book_font_lvgl, 0);
    lv_obj_set_width(l, 296);
    lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_pos(l, 12, y);
    return l;
}

static void refresh_dynamic_info(void)
{
    if (s_psram_label) {
        lv_label_set_text_fmt(s_psram_label, "PSRAM: %lu/%lu KB 可用",
                              (unsigned long)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024),
                              (unsigned long)(esp_psram_get_size() / 1024));
    }
    if (s_heap_label) {
        lv_label_set_text_fmt(s_heap_label, "空闲堆: %lu KB  最低: %lu KB",
                              (unsigned long)(esp_get_free_heap_size() / 1024),
                              (unsigned long)(esp_get_minimum_free_heap_size() / 1024));
    }
    if (s_bat_label) {
        lv_label_set_text_fmt(s_bat_label, "电池: %d%%  %.2fV", battery_percent(), battery_voltage());
    }
    if (s_sd_label) {
        lv_label_set_text(s_sd_label, sd_card_present() ? "SD卡: 已挂载" : "SD卡: 未挂载");
    }
    if (s_ble_label) {
        lv_label_set_text(s_ble_label,
            ble_inited() ? (ble_is_active() ? "蓝牙: 已连接" : "蓝牙: 广播中(XiaoMeng-SYS)") : "蓝牙: 未开启");
    }
}

esp_err_t app_sysinfo_run(void)
{
    /* 不使用 ui_screen_new() 自带的底部提示，避免与第 9 行信息叠在一起。 */
    lv_obj_t *scr = ui_screen_new_ex("系统信息", false);

    lv_obj_t *content = lv_obj_create(scr);
    lv_obj_set_pos(content, 0, UI_TOPBAR_H);
    lv_obj_set_size(content, 320, 179);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_radius(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    esp_chip_info_t chip;
    esp_chip_info(&chip);
    uint32_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);

    char line[96];
    int y = 3;
#define ROW_GAP 19
    info_row(content, "芯片: ESP32-S3", y); y += ROW_GAP;
    snprintf(line, sizeof(line), "核心: %d  Rev.%d", chip.cores, (int)chip.revision);
    info_row(content, line, y); y += ROW_GAP;
    snprintf(line, sizeof(line), "Flash: %lu MB", (unsigned long)(flash_size / 1024 / 1024));
    info_row(content, line, y); y += ROW_GAP;

    s_psram_label = info_row(content, "", y); y += ROW_GAP;
    s_heap_label = info_row(content, "", y); y += ROW_GAP;

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(line, sizeof(line), "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    info_row(content, line, y); y += ROW_GAP;

    s_bat_label = info_row(content, "", y); y += ROW_GAP;
    s_sd_label = info_row(content, "", y); y += ROW_GAP;
    s_ble_label = info_row(content, "", y);

    /* 固定 footer：正文最多到约 213px，提示位于 224~236px，互不覆盖。 */
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "KEY5 重新检测 SD 卡   KEY3 返回");
    lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -3);

    refresh_dynamic_info();
    ui_screen_show(scr);

    uint32_t last_refresh = (uint32_t)(esp_timer_get_time() / 1000);
    for (;;) {
        lv_timer_handler();
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (now - last_refresh >= 1000) {
            last_refresh = now;
            refresh_dynamic_info();
        }

        key_event_t evt;
        if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
            switch (evt.key) {
            case KEY_A: {   /* KEY5: 重新检测 SD */
                (void)sd_card_remount();
                refresh_dynamic_info();
                break;
            }
            case KEY_BACK:
            case KEY_HOME:
                s_psram_label = NULL;
                s_heap_label = NULL;
                s_sd_label = NULL;
                s_bat_label = NULL;
                s_ble_label = NULL;
                return ESP_OK;
            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
