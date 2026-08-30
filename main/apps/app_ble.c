/*
 * app_ble.c — 蓝牙功能栏
 * 行1: 广播(开始/暂停)  行2: 扫描(开始/停止)  下方: 扫描到的设备列表
 * 操作: 五向-上/下=切换行, KEY5/KEY6=切换数值, KEY3=返回
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "services/ble.h"
#include "apps.h"

LV_FONT_DECLARE(ui_font_lvgl_10);

#define MAX_DEV_SHOW 8

static lv_obj_t *s_dev_labels[MAX_DEV_SHOW];
static lv_obj_t *s_adv_val = NULL;
static lv_obj_t *s_scan_val = NULL;
static lv_obj_t *s_status = NULL;

static void refresh_top(void)
{
    if (!ble_inited()) {
        if (s_adv_val) lv_label_set_text(s_adv_val, "关闭");
        if (s_scan_val) lv_label_set_text(s_scan_val, "停止");
        return;
    }
    if (s_adv_val) lv_label_set_text(s_adv_val, ble_adv_on() ? "开启" : "关闭");
    if (s_scan_val) lv_label_set_text(s_scan_val, ble_scanning() ? "扫描中..." : "停止");
}

static void refresh_devices(void)
{
    int n = ble_scan_count();
    for (int i = 0; i < MAX_DEV_SHOW; i++) {
        if (!s_dev_labels[i]) continue;
        if (i < n) {
            const ble_dev_t *d = ble_scan_get(i);
            char buf[40];
            snprintf(buf, sizeof(buf), "%.19s  %ddBm", d->name, (int)d->rssi);
            lv_label_set_text(s_dev_labels[i], buf);
            lv_obj_remove_flag(s_dev_labels[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(s_dev_labels[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

esp_err_t app_ble_run(void)
{
    lv_obj_t *scr = ui_screen_new_ex("蓝牙", false);

    /* 行1: 广播 */
    lv_obj_t *b1 = lv_button_create(scr);
    lv_obj_set_size(b1, 300, 32);
    lv_obj_set_pos(b1, 10, 42);
    ui_style_list_button(b1);
    lv_obj_set_user_data(b1, (void *)0);
    lv_group_add_obj(lv_group_get_default(), b1);
    lv_obj_t *n1 = lv_label_create(b1);
    lv_label_set_text(n1, "广播");
    lv_obj_set_style_text_color(n1, UI_THEME_TEXT, 0);
    lv_obj_align(n1, LV_ALIGN_LEFT_MID, 14, 0);
    s_adv_val = lv_label_create(b1);
    lv_obj_set_style_text_color(s_adv_val, UI_THEME_ACCENT, 0);
    lv_obj_align(s_adv_val, LV_ALIGN_RIGHT_MID, -14, 0);

    /* 行2: 扫描 */
    lv_obj_t *b2 = lv_button_create(scr);
    lv_obj_set_size(b2, 300, 32);
    lv_obj_set_pos(b2, 10, 80);
    ui_style_list_button(b2);
    lv_obj_set_user_data(b2, (void *)1);
    lv_group_add_obj(lv_group_get_default(), b2);
    lv_obj_t *n2 = lv_label_create(b2);
    lv_label_set_text(n2, "扫描");
    lv_obj_set_style_text_color(n2, UI_THEME_TEXT, 0);
    lv_obj_align(n2, LV_ALIGN_LEFT_MID, 14, 0);
    s_scan_val = lv_label_create(b2);
    lv_obj_set_style_text_color(s_scan_val, UI_THEME_ACCENT, 0);
    lv_obj_align(s_scan_val, LV_ALIGN_RIGHT_MID, -14, 0);

    /* 状态行 */
    s_status = lv_label_create(scr);
    lv_obj_set_style_text_color(s_status, UI_THEME_DIM, 0);
    lv_obj_set_pos(s_status, 12, 118);
    lv_obj_set_style_text_font(s_status, &ui_font_lvgl_10, 0);

    /* 设备列表 */
    for (int i = 0; i < MAX_DEV_SHOW; i++) {
        lv_obj_t *l = lv_label_create(scr);
        lv_obj_set_style_text_color(l, UI_THEME_TEXT, 0);
        lv_obj_set_pos(l, 16, 134 + i * 11);
        lv_obj_set_style_text_font(l, &ui_font_lvgl_10, 0);
        lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
        s_dev_labels[i] = l;
    }

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "KEY5/KEY6 切换");
    lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_t *first = lv_obj_get_child(scr, 1);
    if (first) lv_group_focus_obj(first);
    ui_screen_show(scr);
    refresh_top();

    uint32_t last_refresh = 0;
    for (;;) {
        lv_timer_handler();

        /* 扫描期间定期刷新列表 */
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (now - last_refresh > 400) {
            last_refresh = now;
            refresh_top();
            refresh_devices();
            if (s_status) {
                lv_label_set_text(s_status, ble_is_active() ? "已连接设备" : "等待连接...");
            }
        }

        key_event_t evt;
        if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
            switch (evt.key) {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
                ui_nav_key(evt.key);
                break;
            case KEY_A:
            case KEY_B: {   /* KEY5/KEY6: 切换广播/扫描 */
                lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                int row = f ? (int)(intptr_t)lv_obj_get_user_data(f) : 0;
                if (row == 0) {
                    /* 默认关闭: 未初始化时按 KEY5 即开启并初始化 */
                    if (!ble_inited() || !ble_adv_on()) ble_set_adv(true);
                    else ble_set_adv(false);
                } else if (row == 1) {
                    if (ble_scanning()) ble_scan_stop();
                    else ble_scan_start(10000);   /* 扫描 10 秒 */
                }
                refresh_top();
                break;
            }
            case KEY_BACK:
            case KEY_HOME:
                if (ble_scanning()) ble_scan_stop();
                return ESP_OK;
            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
