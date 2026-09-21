/*
 * app_settings.c — 系统设置 (二级菜单)
 * 导航: 主菜单 KEY4 进入本菜单(画面1); 本菜单 KEY4 进入子功能(画面11);
 *       子功能按 KEY3 返回本菜单(画面1); 本菜单按 KEY3 返回主菜单.
 * 子功能: 灯光 / 蓝牙 / WiFi / 系统信息 / 按键测试
 * (原"设置"里的屏保时间已随屏保功能一并删除)
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "apps.h"

static const char *TAG = "sysmenu";

LV_FONT_DECLARE(ui_font_lvgl_10);

typedef struct {
    const char *name;
    const char *glyph;   /* 图标徽标单字/字母 */
    uint32_t color;
    app_func_t run;
} sys_item_t;

static const sys_item_t s_items[] = {
    { "灯光",     "灯", 0xFFD700, app_led_run },
    { "蓝牙",     "蓝", 0x4FB3FF, app_ble_run },
    { "WiFi",     "W",  0x9B7BFF, app_wifi_run },
    { "系统信息", "信", 0xFFB547, app_sysinfo_run },
    { "按键测试", "测", 0x6C7A89, app_keytest_run },
};
#define SYS_COUNT (sizeof(s_items) / sizeof(s_items[0]))

/* 构建系统设置列表 (返回后重建, 保持"子功能 KEY3 → 回到本菜单") */
static void sys_build(void)
{
    lv_obj_t *scr = ui_screen_new_ex("系统设置", false);

    for (int i = 0; i < (int)SYS_COUNT; i++) {
        lv_obj_t *btn = lv_button_create(scr);
        lv_obj_set_size(btn, 300, 30);
        lv_obj_set_pos(btn, 10, 42 + i * 34);
        ui_style_list_button(btn);
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        lv_group_add_obj(lv_group_get_default(), btn);

        lv_obj_t *icon = ui_app_icon(btn, s_items[i].color, s_items[i].glyph, 22);
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 12, 0);

        lv_obj_t *nm = lv_label_create(btn);
        lv_label_set_text(nm, s_items[i].name);
        lv_obj_set_style_text_color(nm, UI_THEME_TEXT, 0);
        lv_obj_align(nm, LV_ALIGN_LEFT_MID, 44, 0);

        lv_obj_t *arrow = lv_label_create(btn);
        lv_label_set_text(arrow, ">");
        lv_obj_set_style_text_color(arrow, UI_THEME_DIM, 0);
        lv_obj_set_style_text_color(arrow, UI_THEME_ACCENT, LV_STATE_FOCUSED);
        lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -14, 0);
    }

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "KEY4 进入  KEY3 返回主菜单");
    lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_t *first = lv_obj_get_child(scr, 1);
    if (first) lv_group_focus_obj(first);
    ui_screen_show(scr);
}

esp_err_t app_settings_run(void)
{
    for (;;) {
        sys_build();            /* 每次从子功能返回都重建 (回到画面1) */
        bool launch = false;
        for (;;) {
            lv_timer_handler();
            key_event_t evt;
            if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
                switch (evt.key) {
                case KEY_UP:
                case KEY_DOWN:
                case KEY_LEFT:
                case KEY_RIGHT:
                    ui_nav_key(evt.key);
                    break;
                case KEY_CONFIRM: {
                    lv_group_t *g = lv_group_get_default();
                    lv_obj_t *f = g ? lv_group_get_focused(g) : NULL;
                    if (f) {
                        int idx = (int)(intptr_t)lv_obj_get_user_data(f);
                        if (idx >= 0 && idx < (int)SYS_COUNT) {
                            ESP_LOGI(TAG, "launch %s", s_items[idx].name);
                            s_items[idx].run();   /* 子功能: KEY3 返回后重建本菜单 */
                            launch = true;
                        }
                    }
                    break;
                }
                case KEY_BACK:
                case KEY_HOME:
                    return ESP_OK;   /* 返回主菜单 */
                default:
                    break;
                }
                if (launch) break;   /* 子功能已返回 → 外层重建列表 */
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}
