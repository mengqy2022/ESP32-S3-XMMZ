/*
 * game_brick.c — 打砖块 (全屏, 直接像素渲染)
 * 五向-左/右=移动挡板, KEY4=发球/暂停, 球落底扣命, 清完砖赢, KEY3=退出
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

#define BRC 11
#define BRR 6
#define BRW 26
#define BRH 12
#define BRX0 6
#define BRY0 40

int game_brick_run(void)
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
    lv_obj_set_style_text_color(score_label, lv_color_hex(0xFFD700), 0);
    lv_obj_set_pos(score_label, 8, 6);
    lv_obj_t *msg = lv_label_create(scr);
    lv_obj_set_style_text_color(msg, lv_color_hex(0xFFAA33), 0);
    lv_obj_set_style_text_font(msg, &ui_font_lvgl_10, 0);
    lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "左右=移动  KEY4=发球/暂停  KEY3=退出");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x667788), 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);

    static uint8_t bricks[BRR][BRC];
    int paddle_x = 130, score = 0, lives = 3;
    float bx = 160, by = 170, vx = 2.0f, vy = -2.4f;
    bool running = false, over = false, paused = false, win = false;

    void reset(void)
    {
        for (int r = 0; r < BRR; r++)
            for (int c = 0; c < BRC; c++) bricks[r][c] = 1;
        paddle_x = 130;
        bx = 160; by = 170; vx = 2.0f; vy = -2.4f;
        score = 0; lives = 3;
        running = false; over = false; win = false; paused = false;
        lv_label_set_text_fmt(score_label, "得分 %d  命 %d", score, lives);
        lv_label_set_text(msg, "KEY4 发球");
    }

    void draw(void)
    {
        /* 背景 */
        for (int y = 0; y < FB_H; y++) {
            uint16_t c = fb_rgb565(0x05080F);
            if (y > 210) c = fb_rgb565(0x081020);
            for (int x = 0; x < FB_W; x++) fb[y * FB_W + x] = c;
        }
        /* 砖块: 按行渐变色 + 高光 */
        for (int r = 0; r < BRR; r++)
            for (int c = 0; c < BRC; c++)
                if (bricks[r][c]) {
                    int x = BRX0 + c * (BRW + 2), y = BRY0 + r * (BRH + 2);
                    uint32_t col = (r < 2) ? 0xE74C3C : (r < 4) ? 0xE67E22 : 0x3498DB;
                    fb_rect(fb, x, y, BRW, BRH, col);
                    fb_rect(fb, x + 1, y + 1, BRW - 2, 2, 0xFFFFFF60);
                }
        /* 挡板: 渐变发光 */
        fb_rect(fb, paddle_x, FB_H - 16, 60, 9, 0x33FF88);
        fb_rect(fb, paddle_x + 6, FB_H - 17, 48, 2, 0xAAFFCC);
        /* 球: 白色 + 光晕 */
        int bx0 = (int)bx, by0 = (int)by;
        fb_rect(fb, bx0 - 5, by0 - 5, 10, 10, 0x88AAFF);
        fb_rect(fb, bx0 - 3, by0 - 3, 6, 6, 0xFFFFFF);
        fb_show(img);
    }

    void step(void)
    {
        bx += vx; by += vy;
        if (bx < 4 || bx > 316) vx = -vx;
        if (by < 4) vy = -vy;
        if (by > 216 && by < 226 && bx > paddle_x - 4 && bx < paddle_x + 64) {
            vy = -fabsf(vy);
            vx = (bx - (paddle_x + 30)) / 30.0f * 4.0f;
            if (vx > 4) vx = 4;
            if (vx < -4) vx = -4;
        }
        int cr = (int)((by - BRY0) / (BRH + 2));
        int cc = (int)((bx - BRX0) / (BRW + 2));
        if (cr >= 0 && cr < BRR && cc >= 0 && cc < BRC && bricks[cr][cc]) {
            bricks[cr][cc] = 0;
            sfx_play(SFX_CLICK);   /* 打砖清脆短音 */
            score += 10;
            lv_label_set_text_fmt(score_label, "得分 %d  命 %d", score, lives);
            vy = -vy;
            bool all = true;
            for (int r = 0; r < BRR; r++)
                for (int c = 0; c < BRC; c++) if (bricks[r][c]) all = false;
            if (all) { win = true; over = true; sfx_play(SFX_WIN); }
        }
        if (by > 236) {
            lives--;
            lv_label_set_text_fmt(score_label, "得分 %d  命 %d", score, lives);
            if (lives <= 0) {
                over = true;
                sfx_play(SFX_GAMEOVER);
            } else {
                sfx_play(SFX_DIE);
                bx = 160; by = 170; vx = 2.0f; vy = -2.4f; running = false; lv_label_set_text(msg, "KEY4 发球");
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
            case KEY_CONFIRM:
                if (!running) {
                    running = true;
                    sfx_play(SFX_SHOOT);
                    lv_label_set_text(msg, "");
                } else {
                    paused = !paused;
                }
                break;
            case KEY_BACK:
            case KEY_HOME:
                fb_free(&fb);
                return 0;
            default: break;
            }
        }
        /* 按住持续移动挡板 (每帧 2px, 流畅跟手) */
        if (!over) {
            bool moved = false;
            if (buttons_get_state(KEY_LEFT)) { if (paddle_x > 0) paddle_x -= 2; moved = true; }
            if (buttons_get_state(KEY_RIGHT)) { if (paddle_x < 260) paddle_x += 2; moved = true; }
            if (moved) draw();
        }
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (running && !paused && !over && (now - last >= 16)) {
            last = now;
            step();
            if (over) { lv_label_set_text_fmt(msg, win ? "胜利! %d 分 KEY4 重开" : "游戏结束 %d 分 KEY4 重开", score); }
            draw();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
