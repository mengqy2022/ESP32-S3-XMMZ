/*
 * game_shoot.c — 飞机大战 (全屏, 直接像素渲染)
 * 五向-左/右=移动, 上=射击, KEY4=暂停, 敌机撞击扣命, KEY3=退出
 */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "audio_sfx.h"
#include "game_common.h"

LV_FONT_DECLARE(ui_font_lvgl_10);
LV_FONT_DECLARE(ui_font_lvgl);

#define MAX_ENEMY 8
#define MAX_BULLET 12

typedef struct { float x, y; bool alive; } ent_t;

int game_shoot_run(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(scr, &ui_font_lvgl, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    uint16_t *fb = NULL;
    lv_image_dsc_t dsc = { 0 };
    lv_obj_t *img = fb_create(scr, &fb, &dsc);
    if (!img) return ESP_FAIL;

    lv_obj_t *score_label = lv_label_create(scr);
    lv_obj_set_style_text_color(score_label, lv_color_hex(0x66D9FF), 0);
    lv_obj_set_pos(score_label, 8, 6);
    lv_obj_t *msg = lv_label_create(scr);
    lv_obj_set_style_text_color(msg, lv_color_hex(0xFFAA33), 0);
    lv_obj_set_style_text_font(msg, &ui_font_lvgl_10, 0);
    lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "左右=移动  上=射击  KEY4=暂停  KEY3=退出");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x667788), 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);

    float px = 150;
    int score = 0, lives = 3;
    bool over = false, paused = false;
    static ent_t enemies[MAX_ENEMY];
    static ent_t bullets[MAX_BULLET];
    static int star_y[30];   /* 动态星空 */
    static bool star_init = false;

    void reset(void)
    {
        px = 150; score = 0; lives = 3; over = false; paused = false;
        for (int i = 0; i < MAX_ENEMY; i++) { enemies[i].alive = false; bullets[i].alive = false; }
        lv_label_set_text_fmt(score_label, "得分 %d  命 %d", score, lives);
        lv_label_set_text(msg, "");
    }

    void spawn_enemy(void)
    {
        for (int i = 0; i < MAX_ENEMY; i++) {
            if (!enemies[i].alive) {
                enemies[i].x = 8 + rand() % (FB_W - 32);
                enemies[i].y = -14;
                enemies[i].alive = true;
                break;
            }
        }
    }

    void fire(void)
    {
        for (int i = 0; i < MAX_BULLET; i++) {
            if (!bullets[i].alive) {
                bullets[i].x = px + 8;
                bullets[i].y = FB_H - 26;
                bullets[i].alive = true;
                break;
            }
        }
    }

    void draw(void)
    {
        /* 背景 */
        for (int y = 0; y < FB_H; y++) {
            uint16_t c = fb_rgb565(0x030509);
            if (y > 210) c = fb_rgb565(0x060A14);
            for (int x = 0; x < FB_W; x++) fb[y * FB_W + x] = c;
        }
        /* 动态星空: 星星缓缓下移 */
        if (!star_init) {
            for (int i = 0; i < 30; i++) star_y[i] = (i * 41) % FB_H;
            star_init = true;
        }
        for (int i = 0; i < 30; i++) {
            star_y[i] += 1;
            if (star_y[i] >= FB_H) star_y[i] = 0;
            fb_px(fb, (i * 67 + 13) % FB_W, star_y[i], 0x3A4A5A);
            fb_px(fb, (i * 67 + 13) % FB_W, (star_y[i] + FB_H - 2) % FB_H, 0x1A2A3A);
        }
        /* 玩家飞机 */
        int pxi = (int)px;
        fb_rect(fb, pxi + 6, FB_H - 32, 8, 8, 0x66D9FF);           /* 座舱 */
        fb_rect(fb, pxi + 2, FB_H - 26, 16, 18, 0x00BFFF);         /* 机身 */
        fb_rect(fb, pxi - 2, FB_H - 22, 6, 12, 0xFF8800);          /* 左翼焰 */
        fb_rect(fb, pxi + 16, FB_H - 22, 6, 12, 0xFF8800);         /* 右翼焰 */
        fb_rect(fb, pxi + 8, FB_H - 6, 4, 6, 0xFFFFFF);            /* 喷口 */
        /* 敌机 */
        for (int i = 0; i < MAX_ENEMY; i++)
            if (enemies[i].alive) {
                int ex = (int)enemies[i].x, ey = (int)enemies[i].y;
                fb_rect(fb, ex + 4, ey, 12, 10, 0xFF4444);
                fb_rect(fb, ex, ey + 4, 20, 10, 0xFF5544);
                fb_rect(fb, ex + 6, ey + 14, 8, 6, 0xFF8800);      /* 尾焰 */
            }
        /* 子弹: 金色光柱 */
        for (int i = 0; i < MAX_BULLET; i++)
            if (bullets[i].alive) {
                int bxi = (int)bullets[i].x, byi = (int)bullets[i].y;
                fb_rect(fb, bxi, byi, 4, 10, 0xFFAA00);
                fb_rect(fb, bxi + 1, byi + 10, 2, 4, 0xFFE080);
            }
        fb_show(img);
    }

    void step(void)
    {
        for (int i = 0; i < MAX_ENEMY; i++) {
            if (enemies[i].alive) {
                enemies[i].y += 1.4f;
                if (enemies[i].y > FB_H) enemies[i].alive = false;
                if (enemies[i].y > FB_H - 34 && enemies[i].y < FB_H - 4 &&
                    enemies[i].x + 20 > px && enemies[i].x < px + 20) {
                    enemies[i].alive = false;
                    lives--;
                    lv_label_set_text_fmt(score_label, "得分 %d  命 %d", score, lives);
                    if (lives <= 0) { over = true; sfx_play(SFX_GAMEOVER); return; }
                    sfx_play(SFX_DIE);
                }
            }
        }
        if ((rand() % 26) == 0) spawn_enemy();
        for (int i = 0; i < MAX_BULLET; i++) {
            if (!bullets[i].alive) continue;
            bullets[i].y -= 4.0f;
            if (bullets[i].y < -10) { bullets[i].alive = false; continue; }
            for (int e = 0; e < MAX_ENEMY; e++) {
                if (!enemies[e].alive) continue;
                if (bullets[i].x > enemies[e].x && bullets[i].x < enemies[e].x + 20 &&
                    bullets[i].y > enemies[e].y && bullets[i].y < enemies[e].y + 22) {
                    enemies[e].alive = false;
                    bullets[i].alive = false;
                    score += 10;
                    sfx_play(SFX_HIT);
                    lv_label_set_text_fmt(score_label, "得分 %d  命 %d", score, lives);
                    break;
                }
            }
        }
    }

    reset();
    draw();
    uint32_t last = (uint32_t)(esp_timer_get_time() / 1000);

    for (;;) {
        lv_timer_handler();
        key_event_t evt;
        while (buttons_wait_event(&evt, 0)) {
            if (evt.evt != KEY_EVT_PRESS) continue;
            if (over) {
                if (evt.key == KEY_CONFIRM) { reset(); draw(); }
                if (evt.key == KEY_BACK || evt.key == KEY_HOME) { fb_free(&fb); return 0; }
                continue;
            }
            switch (evt.key) {
            case KEY_UP:
                fire();
                sfx_play(SFX_SHOOT);
                break;
            case KEY_CONFIRM:
                paused = !paused;
                break;
            case KEY_BACK:
            case KEY_HOME:
                fb_free(&fb);
                return 0;
            default: break;
            }
        }
        /* 按住持续移动玩家飞机 (每帧 3px) */
        if (!over && !paused) {
            bool moved = false;
            if (buttons_get_state(KEY_LEFT)) { if (px > 0) px -= 3; moved = true; }
            if (buttons_get_state(KEY_RIGHT)) { if (px < FB_W - 20) px += 3; moved = true; }
            if (moved) draw();
        }
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (!paused && !over && (now - last >= 30)) {
            last = now;
            step();
            if (over) { lv_label_set_text_fmt(msg, "游戏结束 %d 分 KEY4 重开", score); }
            draw();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
