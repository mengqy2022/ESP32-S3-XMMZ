/*
 * app_wifi.c — WiFi 功能栏
 * 操作: 五向=导航, KEY5=扫描/断开, KEY4=选择AP/连接, KEY3=返回
 * 连接: 开放AP直接连; 加密AP弹虚拟键盘输密码 -> 保存并连接
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "services/wifi_mgr.h"
#include "kbd.h"
#include "apps.h"

LV_FONT_DECLARE(ui_font_lvgl_10);

static const char *TAG = "wifi_app";

#define MAX_AP_SHOW 6

static wifi_ap_t s_aps[WIFI_SCAN_MAX_AP];
static int s_ap_count = 0;

static lv_obj_t *s_ap_labels[MAX_AP_SHOW];
static char s_pass_buf[64];


/* 密码输入完成回调: 连接 */
static void pass_done_cb(const char *text, void *ctx)
{
    int apidx = (int)(intptr_t)ctx;
    if (apidx >= 0 && apidx < s_ap_count) {
        ESP_LOGI(TAG, "connect %s", s_aps[apidx].ssid);
        wifi_mgr_connect(s_aps[apidx].ssid, text);
    }
}

static void refresh_status(lv_obj_t *status)
{
    char buf[128];
    if (wifi_mgr_connected()) {
        snprintf(buf, sizeof(buf), "已连接 %.32s  IP:%.15s%s",
                 wifi_mgr_ssid(), wifi_mgr_ip(),
                 wifi_mgr_time_synced() ? "  时间已同步" : "");
    } else {
        snprintf(buf, sizeof(buf), "未连接  时间%s同步", wifi_mgr_time_synced() ? "已" : "未");
    }
    lv_label_set_text(status, buf);
}

static void refresh_ap_list(void)
{
    for (int i = 0; i < MAX_AP_SHOW; i++) {
        if (!s_ap_labels[i]) continue;
        if (i < s_ap_count) {
            const wifi_ap_t *a = &s_aps[i];
            char buf[64];
            /* 802.11 SSID 最长 32 byte；显式精度上限避免 GCC 14
             * -Wformat-truncation，同时防止异常扫描结果撑爆 UI 缓冲。 */
            snprintf(buf, sizeof(buf), "%.32s %s %ddBm", a->ssid,
                     a->authmode == 0 ? "开放" : "加密", (int)a->rssi);
            lv_label_set_text(s_ap_labels[i], buf);
            lv_obj_remove_flag(s_ap_labels[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(s_ap_labels[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/* 动态重建 AP 按钮列表 */
static void build_ap_buttons(lv_obj_t *scr)
{
    lv_group_t *g = lv_group_get_default();

    /* 先清空标签句柄，再从后向前删除旧 AP 按钮。
     * 旧实现只检查 child(0)，在标题栏位于第一个 child 时可能删不到动态按钮，
     * 多次扫描会持续累积 LVGL 对象，最终造成堆内存下降甚至崩溃。 */
    for (int i = 0; i < MAX_AP_SHOW; i++) s_ap_labels[i] = NULL;
    int32_t child_count = (int32_t)lv_obj_get_child_count(scr);
    for (int32_t i = child_count - 1; i >= 0; i--) {
        lv_obj_t *ch = lv_obj_get_child(scr, (uint32_t)i);
        intptr_t v = (intptr_t)lv_obj_get_user_data(ch);
        if (v >= 0x200 && v < 0x200 + MAX_AP_SHOW) {
            if (g) lv_group_remove_obj(ch);
            lv_obj_delete(ch);
        }
    }

    lv_obj_t *first_ap = NULL;
    for (int i = 0; i < s_ap_count && i < MAX_AP_SHOW; i++) {
        lv_obj_t *b = lv_button_create(scr);
        lv_obj_set_size(b, 300, 16);
        lv_obj_set_pos(b, 10, 116 + i * 17);
        ui_style_compact_button(b);
        lv_obj_set_user_data(b, (void *)(intptr_t)(0x200 + i));
        if (g) lv_group_add_obj(g, b);
        if (!first_ap) first_ap = b;

        lv_obj_t *lb = lv_label_create(b);
        lv_obj_set_style_text_color(lb, UI_THEME_TEXT, 0);
        lv_obj_set_style_text_font(lb, &ui_font_lvgl_10, 0);
        lv_obj_align(lb, LV_ALIGN_LEFT_MID, 8, 0);
        s_ap_labels[i] = lb;
    }
    if (first_ap) lv_group_focus_obj(first_ap);
}

/* AP 选择/连接 (由应用循环队列驱动, 不在 LVGL 回调里阻塞) */
static void ap_select(int idx)
{
    if (idx < 0 || idx >= s_ap_count) return;
    wifi_ap_t *a = &s_aps[idx];
    ESP_LOGI(TAG, "select AP[%d] %s auth=%d", idx, a->ssid, a->authmode);
    if (a->authmode == 0) {
        wifi_mgr_connect(a->ssid, "");
    } else {
        s_pass_buf[0] = 0;
        kbd_show("WiFi密码", "", s_pass_buf, sizeof(s_pass_buf), pass_done_cb, (void *)(intptr_t)idx);
    }
}

esp_err_t app_wifi_run(void)
{
    wifi_mgr_init();   /* 确保已初始化 (含自动连接保存的配置) */

    lv_obj_t *scr = ui_screen_new_ex("WiFi", false);

    lv_obj_t *status = lv_label_create(scr);
    lv_obj_set_style_text_color(status, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(status, &ui_font_lvgl_10, 0);
    lv_obj_set_pos(status, 10, 42);
    lv_obj_set_width(status, 300);
    lv_label_set_long_mode(status, LV_LABEL_LONG_WRAP);

    /* 扫描按钮 */
    lv_obj_t *bscan = lv_button_create(scr);
    lv_obj_set_size(bscan, 140, 28);
    lv_obj_set_pos(bscan, 10, 84);
    ui_style_list_button(bscan);
    lv_obj_set_user_data(bscan, (void *)(intptr_t)1);
    lv_group_add_obj(lv_group_get_default(), bscan);
    lv_obj_t *lb1 = lv_label_create(bscan);
    lv_label_set_text(lb1, "扫描");
    lv_obj_set_style_text_color(lb1, UI_THEME_TEXT, 0);
    lv_obj_center(lb1);

    /* 断开按钮 */
    lv_obj_t *bdis = lv_button_create(scr);
    lv_obj_set_size(bdis, 140, 28);
    lv_obj_set_pos(bdis, 170, 84);
    ui_style_list_button(bdis);
    lv_obj_set_user_data(bdis, (void *)(intptr_t)2);
    lv_group_add_obj(lv_group_get_default(), bdis);
    lv_obj_t *lb2 = lv_label_create(bdis);
    lv_label_set_text(lb2, "断开");
    lv_obj_set_style_text_color(lb2, UI_THEME_TEXT, 0);
    lv_obj_center(lb2);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "KEY5 扫描/断开  选择AP后KEY4连接");
    lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_group_focus_obj(bscan);
    ui_screen_show(scr);

    uint32_t last = 0;
    for (;;) {
        lv_timer_handler();
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (now - last > 500) {
            last = now;
            refresh_status(status);
            refresh_ap_list();
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
            case KEY_A: {   /* KEY5: 焦点行对应动作 */
                lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                int row = f ? (int)(intptr_t)lv_obj_get_user_data(f) : 0;
                if (row == 1) {   /* 扫描 */
                    s_ap_count = 0;
                    refresh_ap_list();
                    wifi_mgr_scan(s_aps, WIFI_SCAN_MAX_AP, &s_ap_count);
                    build_ap_buttons(scr);
                    refresh_ap_list();
                    ESP_LOGI(TAG, "scan done: %d", s_ap_count);
                } else if (row == 2) {   /* 断开 */
                    wifi_mgr_disconnect();
                }
                break;
            }
            case KEY_CONFIRM: {   /* KEY4: 选择 AP 连接 (队列驱动, 避免 LVGL 重入) */
                lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                if (f) {
                    intptr_t v = (intptr_t)lv_obj_get_user_data(f);
                    if (v >= 0x200) ap_select((int)(v - 0x200));
                }
                break;
            }
            case KEY_BACK:
            case KEY_HOME:
                return ESP_OK;
            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
