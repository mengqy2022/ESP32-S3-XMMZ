/*
 * app_login.c — 开机登录界面
 * 用虚拟键盘输入密码, 正确后进入主页; 密码错误可重试
 * 默认密码: 8888 (改 LOGIN_PASSWORD 后重新编译)
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lvgl_ui.h"
#include "kbd.h"
#include "apps.h"
#include "version.h"

#define LOGIN_PASSWORD "8888"

static const char *TAG = "login";

LV_FONT_DECLARE(ui_font_lvgl_10);
LV_FONT_DECLARE(book_font_lvgl);

static volatile bool s_ok = false;

static void pwd_cb(const char *text, void *ctx)
{
    (void)ctx;
    if (text && strcmp(text, LOGIN_PASSWORD) == 0) {
        s_ok = true;
        ESP_LOGI(TAG, "login OK");
    } else {
        ESP_LOGW(TAG, "wrong password");
    }
}

esp_err_t app_login_run(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, UI_THEME_BG, 0);
    lv_obj_set_style_text_font(scr, &ui_font_lvgl, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* 品牌区 */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, SYS_NAME_STR);
    lv_obj_set_style_text_color(title, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(title, &book_font_lvgl, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text_fmt(sub, "%s  |  ESP32-S3", SYS_VERSION_STR);
    lv_obj_set_style_text_color(sub, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(sub, &ui_font_lvgl_10, 0);
    lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 52);

    /* 登录卡片 */
    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_set_size(card, 280, 82);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 4);
    ui_style_card(card);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *hint = lv_label_create(card);
    lv_label_set_text(hint, "请输入密码");
    lv_obj_set_style_text_color(hint, UI_THEME_TEXT, 0);
    lv_obj_align(hint, LV_ALIGN_TOP_LEFT, 14, 12);

    lv_obj_t *desc = lv_label_create(card);
    lv_label_set_text(desc, "按 KEY4 打开键盘");
    lv_obj_set_style_text_color(desc, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(desc, &ui_font_lvgl_10, 0);
    lv_obj_align(desc, LV_ALIGN_TOP_LEFT, 14, 39);

    lv_obj_t *err = lv_label_create(card);
    lv_label_set_text(err, "密码错误, 请重试");
    lv_obj_set_style_text_color(err, UI_THEME_DANGER, 0);
    lv_obj_set_style_text_font(err, &ui_font_lvgl_10, 0);
    lv_obj_align(err, LV_ALIGN_BOTTOM_LEFT, 14, -8);
    lv_obj_add_flag(err, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *tips = lv_label_create(scr);
    lv_label_set_text(tips, "KEY4输入  KEY1完成  KEY3退格");
    lv_obj_set_style_text_color(tips, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(tips, &ui_font_lvgl_10, 0);
    lv_obj_align(tips, LV_ALIGN_BOTTOM_MID, 0, -14);

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load(scr);

    for (;;) {
        s_ok = false;
        char pwd[16] = {0};
        kbd_show("请输入密码", "", pwd, sizeof(pwd), pwd_cb, NULL);
        if (s_ok) break;

        lv_obj_clear_flag(err, LV_OBJ_FLAG_HIDDEN);
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(900));
        lv_obj_add_flag(err, LV_OBJ_FLAG_HIDDEN);
    }

    ESP_LOGI(TAG, "login success, enter home");
    return ESP_OK;
}
