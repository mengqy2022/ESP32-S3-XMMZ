/*
 * app_games.c — 游戏大厅 (游戏专属卡片风格, 记住上次选择)
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "apps.h"
#include "audio_sfx.h"

LV_FONT_DECLARE(ui_font_lvgl_10);
LV_FONT_DECLARE(book_font_lvgl);   /* 全 GB2312 字库, 游戏名不乱码 */

typedef struct {
    const char *name;
    const char *desc;
    uint32_t color;
    int (*run)(void);
} game_entry_t;

extern int game_snake_run(void);
extern int game_tetris_run(void);
extern int game_brick_run(void);
extern int game_shoot_run(void);
extern int game_invader_run(void);

static const game_entry_t s_games[] = {
    { "太空入侵者", "经典街机 整排敌机压境",  0xFF2266, game_invader_run },
    { "贪吃蛇",   "经典 无尽 极速 三种模式", 0x00FF88, game_snake_run },
    { "俄罗斯方块", "消行挑战 越玩越快",      0x9B59B6, game_tetris_run },
    { "打砖块",   "66 块砖 三命通关",       0xE74C3C, game_brick_run },
    { "飞机大战", "星际射击 躲避敌机",       0x00BFFF, game_shoot_run },
};
#define GAME_COUNT (sizeof(s_games) / sizeof(s_games[0]))

static int s_last_game = 0;   /* 记住上次选中的游戏 */

esp_err_t app_games_run(void)
{
    for (;;) {
        lv_obj_t *scr = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(scr, UI_THEME_BG, 0);
        lv_obj_set_style_text_font(scr, &ui_font_lvgl, 0);
        lv_obj_set_style_pad_all(scr, 0, 0);

        /* 顶部游戏专属标题栏 */
        lv_obj_t *bar = lv_obj_create(scr);
        lv_obj_set_size(bar, 320, 34);
        lv_obj_set_pos(bar, 0, 0);
        lv_obj_set_style_bg_color(bar, UI_THEME_BAR, 0);
        lv_obj_set_style_border_width(bar, 0, 0);
        lv_obj_set_style_radius(bar, 0, 0);
        lv_obj_set_style_pad_all(bar, 0, 0);
        lv_obj_t *t = lv_label_create(bar);
        lv_label_set_text(t, "游戏厅");
        lv_obj_set_style_text_color(t, UI_THEME_TEXT, 0);
        lv_obj_align(t, LV_ALIGN_LEFT_MID, 12, 0);
        lv_obj_t *tv = lv_label_create(bar);
        lv_label_set_text(tv, "KEY3 返回");
        lv_obj_set_style_text_color(tv, UI_THEME_DIM, 0);
        lv_obj_set_style_text_font(tv, &ui_font_lvgl_10, 0);
        lv_obj_align(tv, LV_ALIGN_RIGHT_MID, -12, 0);

        /* 游戏卡片 */
        lv_obj_t *game_btns[GAME_COUNT] = {0};
        for (int i = 0; i < (int)GAME_COUNT; i++) {
            lv_obj_t *b = lv_button_create(scr);
            lv_obj_set_size(b, 300, 30);
            lv_obj_set_pos(b, 10, 44 + i * 34);
            ui_style_list_button(b);
            lv_obj_set_user_data(b, (void *)(intptr_t)i);
            lv_group_add_obj(lv_group_get_default(), b);
            game_btns[i] = b;

            /* 左侧色块 */
            lv_obj_t *dot = lv_obj_create(b);
            lv_obj_set_size(dot, 10, 16);
            lv_obj_set_style_bg_color(dot, lv_color_hex(s_games[i].color), 0);
            lv_obj_set_style_radius(dot, 4, 0);
            lv_obj_set_style_border_width(dot, 0, 0);
            lv_obj_align(dot, LV_ALIGN_LEFT_MID, 10, 0);

            /* 游戏名 + 描述 */
            lv_obj_t *n = lv_label_create(b);
            lv_label_set_text(n, s_games[i].name);
            lv_obj_set_style_text_color(n, UI_THEME_TEXT, 0);
            lv_obj_set_style_text_font(n, &book_font_lvgl, 0);
            lv_obj_align(n, LV_ALIGN_LEFT_MID, 30, 0);

            /* 开始箭头 */
            lv_obj_t *arr = lv_label_create(b);
            lv_label_set_text(arr, ">");
            lv_obj_set_style_text_color(arr, lv_color_hex(s_games[i].color), 0);
            lv_obj_set_style_text_color(arr, UI_THEME_ACCENT, LV_STATE_FOCUSED);
            lv_obj_align(arr, LV_ALIGN_RIGHT_MID, -14, 0);
        }

        lv_obj_t *hint = lv_label_create(scr);
        lv_label_set_text(hint, "KEY4 开始  KEY3 返回");
        lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
        lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

        lv_obj_t *old = lv_scr_act();
        if (old && old != scr) ui_group_cleanup(old);
        lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);

        /* 聚焦上次选中的游戏。直接使用按钮句柄，避免 user_data=0 与标题栏默认 NULL 混淆。 */
        if (s_last_game < 0 || s_last_game >= (int)GAME_COUNT) s_last_game = 0;
        if (game_btns[s_last_game]) lv_group_focus_obj(game_btns[s_last_game]);

        int launch = -1;
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
                    lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                    launch = f ? (int)(intptr_t)lv_obj_get_user_data(f) : -1;
                    break;
                }
                case KEY_BACK:
                case KEY_HOME:
                    return ESP_OK;
                default:
                    break;
                }
            }
            if (launch >= 0 && launch < (int)GAME_COUNT) {
                int idx = launch;
                launch = -1;
                s_last_game = idx;              /* 记住选中, 返回后仍选中它 */
                sfx_play(SFX_CLICK);
                s_games[idx].run();
                break;                          /* 回到大厅 (重建) */
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}
