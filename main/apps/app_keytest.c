/*
 * app_keytest.c — 按键测试 (实时显示 11 键状态, 用于硬件诊断)
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "apps.h"

typedef struct {
    const char *name;
    key_id_t key;
} kt_item_t;

static const kt_item_t s_keys[] = {
    { "KEY1 主界面", KEY_MENU },
    { "KEY2 设置",   KEY_OPTION },
    { "KEY3 返回",   KEY_SELECT },
    { "KEY4 确定",   KEY_START },
    { "KEY5 (GPIO15)", KEY_A },
    { "KEY6 (GPIO5)",  KEY_B },
    { "五向-上(上翻)", KEY_UP },
    { "五向-下(下翻)", KEY_DOWN },
    { "五向-左",     KEY_LEFT },
    { "五向-右",     KEY_RIGHT },
    { "BOOT",        KEY_BOOT },
};
#define KT_COUNT (sizeof(s_keys) / sizeof(s_keys[0]))

esp_err_t app_keytest_run(void)
{
    lv_obj_t *scr = ui_screen_new("按键测试");
    lv_obj_t *labels[KT_COUNT];

    for (int i = 0; i < (int)KT_COUNT; i++) {
        int col = i % 2;
        int row = i / 2;
        lv_obj_t *l = lv_label_create(scr);
        lv_label_set_text_fmt(l, "%s: -", s_keys[i].name);
        lv_obj_set_style_text_color(l, UI_THEME_TEXT, 0);
        lv_obj_set_pos(l, 12 + col * 150, 40 + row * 22);
        labels[i] = l;
    }
    ui_screen_show(scr);

    while (1) {
        lv_timer_handler();
        for (int i = 0; i < (int)KT_COUNT; i++) {
            bool st = buttons_get_state(s_keys[i].key);
            lv_label_set_text_fmt(labels[i], "%s: %s", s_keys[i].name, st ? "按下" : "释放");
            lv_obj_set_style_text_color(labels[i], st ? UI_THEME_ACCENT : UI_THEME_DIM, 0);
        }
        key_event_t evt;
        if (buttons_wait_event(&evt, 50)) {
            if (evt.evt == KEY_EVT_PRESS &&
                (evt.key == KEY_BACK || evt.key == KEY_HOME)) break;
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }
    return ESP_OK;
}
