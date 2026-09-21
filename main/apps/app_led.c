/*
 * app_led.c — 灯光设置 (GPIO38 WS2812B)
 * 行: 模式/颜色/亮度; 五向-上/下=切换行, KEY5/KEY6=调节数值, 实时生效并保存
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "services/led_ctrl.h"
#include "apps.h"

LV_FONT_DECLARE(ui_font_lvgl_10);

typedef struct {
    const char *name;
    const char *glyph;       /* 图标徽标单字 */
    uint32_t color;
    const char *(*value_str)(void);
    void (*change)(int dir);   /* dir: +1/-1 */
    lv_obj_t *value_label;
} led_row_t;

static const char *mode_names[LED_MODE_MAX] = { "关闭", "常亮", "呼吸", "闪烁", "彩虹" };

static const char *mode_val(void)
{
    return mode_names[led_ctrl_get_mode()];
}

static void mode_change(int dir)
{
    int m = (int)led_ctrl_get_mode() + dir;
    if (m < 0) m = LED_MODE_MAX - 1;
    if (m >= LED_MODE_MAX) m = 0;
    led_ctrl_set(led_ctrl_get_color(), led_ctrl_get_bright(), (led_mode_t)m);
}

static const char *color_val(void)
{
    return led_colors[led_ctrl_get_color()].name;
}

static void color_change(int dir)
{
    int c = (int)led_ctrl_get_color() + dir;
    if (c < 0) c = LED_COLOR_COUNT - 1;
    if (c >= LED_COLOR_COUNT) c = 0;
    led_ctrl_set((uint8_t)c, led_ctrl_get_bright(), led_ctrl_get_mode());
}

static char s_bright_buf[8];

static const char *bright_val(void)
{
    snprintf(s_bright_buf, sizeof(s_bright_buf), "%d%%", led_ctrl_get_bright());
    return s_bright_buf;
}

static void bright_change(int dir)
{
    int b = (int)led_ctrl_get_bright() + dir * 10;
    if (b < 0) b = 0;
    if (b > 100) b = 100;
    led_ctrl_set(led_ctrl_get_color(), (uint8_t)b, led_ctrl_get_mode());
}

static led_row_t s_rows[3] = {
    { "模式", "模", 0xFF9F0A, mode_val, mode_change, NULL },
    { "颜色", "色", 0x64D2FF, color_val, color_change, NULL },
    { "亮度", "亮", 0xFFD700, bright_val, bright_change, NULL },
};
#define ROW_COUNT (sizeof(s_rows) / sizeof(s_rows[0]))

static void refresh_values(void)
{
    for (int i = 0; i < (int)ROW_COUNT; i++) {
        if (s_rows[i].value_label) lv_label_set_text(s_rows[i].value_label, s_rows[i].value_str());
    }
}

esp_err_t app_led_run(void)
{
    lv_obj_t *scr = ui_screen_new_ex("灯光设置", false);

    for (int i = 0; i < (int)ROW_COUNT; i++) {
        lv_obj_t *btn = lv_button_create(scr);
        lv_obj_set_size(btn, 300, 34);
        lv_obj_set_pos(btn, 10, 44 + i * 40);
        ui_style_list_button(btn);
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        lv_group_add_obj(lv_group_get_default(), btn);

        lv_obj_t *icon = ui_app_icon(btn, s_rows[i].color, s_rows[i].glyph, 22);
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 12, 0);

        lv_obj_t *name = lv_label_create(btn);
        lv_label_set_text(name, s_rows[i].name);
        lv_obj_set_style_text_color(name, UI_THEME_TEXT, 0);
        lv_obj_align(name, LV_ALIGN_LEFT_MID, 44, 0);

        lv_obj_t *val = lv_label_create(btn);
        lv_obj_set_style_text_color(val, UI_THEME_ACCENT, 0);
        lv_obj_align(val, LV_ALIGN_RIGHT_MID, -14, 0);
        s_rows[i].value_label = val;
    }
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "KEY5/KEY6 调节  五向切换行");
    lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_t *first = lv_obj_get_child(scr, 1);
    if (first) lv_group_focus_obj(first);
    ui_screen_show(scr);
    refresh_values();

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
            case KEY_A:   /* KEY5: 值 +1 */
            case KEY_B: { /* KEY6: 值 -1 */
                lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                int row = f ? (int)(intptr_t)lv_obj_get_user_data(f) : 0;
                if (row >= 0 && row < (int)ROW_COUNT) {
                    s_rows[row].change(evt.key == KEY_A ? 1 : -1);
                    refresh_values();
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
