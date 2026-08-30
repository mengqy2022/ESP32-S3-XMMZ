/*
 * game_invader.c — 太空入侵者 (经典街机)
 * 五向-左/右 = 移动炮台  KEY5/KEY6 也可移动
 * KEY4/上 = 开炮    五向-上 = 暂停
 * 敌机整排左右移动逐步下压, 击毁敌机得分, 敌机触底或撞炮台扣命, KEY3 退出
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
LV_FONT_DECLARE(book_font_lvgl);   /* 全 GB2312 */

#define COLS       8
#define ROWS       5
#define MAX_INV    (COLS * ROWS)
#define MAX_PB     4   /* 玩家子弹 */
#define MAX_EB     3   /* 敌机子弹 */

typedef struct { float x, y; bool alive; } ent_t;

/* 敌机编队 */
static float s_grid_x, s_grid_y;          /* 编队左上角 */
static bool  s_inv[MAX_INV];              /* 每格是否存活 */
static int   s_dir = 1;                   /* 左右移动方向 1/-1 */
static int   s_score, s_lives;
static bool  s_over, s_paused;

/* 子弹 */
static ent_t s_pb[MAX_PB];   /* 玩家子弹 */
static ent_t s_eb[MAX_EB];   /* 敌机子弹 */

#define INV_COL0 0x66FF66
#define INV_COL1 0xFFAA22

int game_invader_run(void)
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
    lv_obj_set_style_text_font(score_label, &book_font_lvgl, 0);
    lv_obj_set_pos(score_label, 8, 6);
    lv_obj_t *msg = lv_label_create(scr);
    lv_obj_set_style_text_color(msg, lv_color_hex(0xFFAA33), 0);
    lv_obj_set_style_text_font(msg, &book_font_lvgl, 0);
    lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 28);
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "左右移动 KEY4开炮 KEY1暂停");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x667788), 0);
    lv_obj_set_style_text_font(hint, &book_font_lvgl, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -2);

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);

    /* 玩家炮台 x (整数) */
    int gx = 150;
    uint32_t last = (uint32_t)(esp_timer_get_time() / 1000);

    void reset(void)
    {
        gx = 150; s_score = 0; s_lives = 3; s_over = false; s_paused = false;
        s_grid_x = 30; s_grid_y = 52; s_dir = 1;
        for (int i = 0; i < MAX_INV; i++) s_inv[i] = true;
        for (int i = 0; i < MAX_PB; i++) s_pb[i].alive = false;
        for (int i = 0; i < MAX_EB; i++) s_eb[i].alive = false;
        lv_label_set_text_fmt(score_label, "得分 %d  命 %d", s_score, s_lives);
        lv_label_set_text(msg, "");
    }

    /* 画一个敌机 (用矩形拼出简单造型) */
    void draw_invader(int x, int y, int alt)
    {
        /* 身体: 上半 + 下半, 两种颜色渐进 */
        fb_rect(fb, x + 4, y, 8, 6, alt ? INV_COL0 : INV_COL1);
        fb_rect(fb, x + 1, y + 4, 15, 6, alt ? INV_COL1 : INV_COL0);
        fb_rect(fb, x, y + 8, 17, 4, 0xFF3344);
        fb_rect(fb, x + 3, y + 10, 11, 5, 0xFF5566);
        fb_rect(fb, x + 6, y + 14, 2, 3, 0xFF8800);   /* 触角/腿 */
        fb_rect(fb, x + 10, y + 14, 2, 3, 0xFF8800);
    }

    void draw(void)
    {
        for (int y = 0; y < FB_H; y++) {
            uint16_t c = fb_rgb565(0x000208);
            if (y > 214) c = fb_rgb565(0x04060E);
            for (int x = 0; x < FB_W; x++) fb[y * FB_W + x] = c;
        }
        /* 敌机 */
        int alt = (int)(esp_timer_get_time() / 500000) & 1;
        for (int r = 0; r < ROWS; r++)
            for (int c = 0; c < COLS; c++) {
                int idx = r * COLS + c;
                if (s_inv[idx]) {
                    int x = (int)s_grid_x + c * 28, y = (int)s_grid_y + r * 22;
                    /* 行越高颜色越偏蓝(近到远) 或 按难度: 最低行分值高 */
                    draw_invader(x, y, alt);
                }
            }
        /* 玩家炮台 */
        fb_rect(fb, gx - 2, FB_H - 24, 22, 4, 0xFFAA33);      /* 底座 */
        fb_rect(fb, gx + 8, FB_H - 28, 4, 6, 0xFFEE99);       /* 炮管 */
        fb_rect(fb, gx + 9, FB_H - 6, 2, 4, 0xFFFFFF);
        /* 玩家子弹 */
        for (int i = 0; i < MAX_PB; i++)
            if (s_pb[i].alive) fb_rect(fb, (int)s_pb[i].x, (int)s_pb[i].y, 3, 10, 0xFFFFFF);
        /* 敌机子弹 */
        for (int i = 0; i < MAX_EB; i++)
            if (s_eb[i].alive) fb_rect(fb, (int)s_eb[i].x, (int)s_eb[i].y, 3, 8, 0xFF2211);
        fb_show(img);
    }

    void fire(void)
    {
        for (int i = 0; i < MAX_PB; i++)
            if (!s_pb[i].alive) { s_pb[i].x = gx + 9; s_pb[i].y = FB_H - 30; s_pb[i].alive = true; break; }
    }

    /* 统计存活敌机数, 并找射击者 (最低行存活敌机) */
    int enemy_fire_lowest_row(void)
    {
        for (int r = ROWS - 1; r >= 0; r--)
            for (int c = 0; c < COLS; c++)
                if (s_inv[r * COLS + c]) return r;
        return -1;
    }

    void step(void)
    {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        /* 编队左右移动 + 下压 (速度随存活数略增) */
        int alive = 0;
        for (int i = 0; i < MAX_INV; i++) if (s_inv[i]) alive++;
        float speed = 30.0f + (MAX_INV - alive) * 0.6f;
        s_grid_x += s_dir * speed * 0.02f;
        if (s_grid_x < 6) { s_grid_x = 6; s_dir = 1; s_grid_y += 10; }
        else if (s_grid_x > FB_W - 6 - 8 * 28) { s_grid_x = FB_W - 6 - 8 * 28; s_dir = -1; s_grid_y += 10; }

        /* 玩家子弹 */
        for (int i = 0; i < MAX_PB; i++) {
            if (!s_pb[i].alive) continue;
            s_pb[i].y -= 5;
            if (s_pb[i].y < 0) { s_pb[i].alive = false; continue; }
            /* 命中敌机 */
            for (int r = 0; r < ROWS; r++)
                for (int c = 0; c < COLS; c++) {
                    int idx = r * COLS + c;
                    if (!s_inv[idx]) continue;
                    float ix = s_grid_x + c * 28, iy = s_grid_y + r * 22;
                    if (s_pb[i].x > ix - 2 && s_pb[i].x < ix + 20 &&
                        s_pb[i].y > iy && s_pb[i].y < iy + 18) {
                        s_inv[idx] = false;
                        s_pb[i].alive = false;
                        s_score += 100 - r * 20;   /* 越低的行分越高 */
                        sfx_play(SFX_HIT);
                        lv_label_set_text_fmt(score_label, "得分 %d  命 %d", s_score, s_lives);
                        break;
                    }
                }
        }
        /* 敌机子弹 (随机发射) */
        if (alive > 0 && (rand() % 70) == 0) {
            int r = enemy_fire_lowest_row();
            if (r >= 0) {
                int c = rand() % COLS;
                for (int k = c; k < COLS; k++) if (s_inv[r * COLS + k]) { c = k; break; }
                float ex = s_grid_x + c * 28 + 9, ey = s_grid_y + r * 22 + 18;
                for (int k = 0; k < MAX_EB; k++)
                    if (!s_eb[k].alive) { s_eb[k].x = ex; s_eb[k].y = ey; s_eb[k].alive = true; break; }
            }
        }
        for (int i = 0; i < MAX_EB; i++) {
            if (!s_eb[i].alive) continue;
            s_eb[i].y += 3;
            if (s_eb[i].y > FB_H) { s_eb[i].alive = false; continue; }
            /* 命中玩家炮台 */
            if (s_eb[i].x > gx - 2 && s_eb[i].x < gx + 20 && s_eb[i].y > FB_H - 28) {
                s_eb[i].alive = false;
                s_lives--;
                sfx_play(SFX_DIE);
                lv_label_set_text_fmt(score_label, "得分 %d  命 %d", s_score, s_lives);
                if (s_lives <= 0) { s_over = true; sfx_play(SFX_GAMEOVER); return; }
            }
        }
        /* 敌机触底 或 撞到炮台 */
        if (s_grid_y + (ROWS - 1) * 22 + 18 >= FB_H - 20) {
            s_over = true; sfx_play(SFX_GAMEOVER); return;
        }
        if (alive == 0) { s_over = true; s_score += 500; sfx_play(SFX_GAMEOVER);
            lv_label_set_text_fmt(score_label, "得分 %d  命 %d", s_score, s_lives); return; }
        (void)now;
    }

    reset();
    draw();
    for (;;) {
        lv_timer_handler();
        key_event_t evt;
        while (buttons_wait_event(&evt, 0)) {
            if (evt.evt != KEY_EVT_PRESS) continue;
            if (s_over) {
                if (evt.key == KEY_CONFIRM) { reset(); draw(); }
                if (evt.key == KEY_BACK || evt.key == KEY_HOME) { fb_free(&fb); return 0; }
                continue;
            }
            switch (evt.key) {
            case KEY_CONFIRM:
            case KEY_UP:
                fire();
                sfx_play(SFX_SHOOT);
                break;
            case KEY_LEFT:  case KEY_RIGHT:  break;  /* 按住持续移动 */
            case KEY_MENU:   /* KEY1: 暂停/继续 */
                s_paused = !s_paused;
                break;
            case KEY_BACK:   /* KEY3: 退出 */
                fb_free(&fb);
                return 0;
            default: break;
            }
        }
        if (!s_over && !s_paused) {
            bool moved = false;
            if (buttons_get_state(KEY_LEFT) || buttons_get_state(KEY_A)) { if (gx > 0) gx -= 3; moved = true; }
            if (buttons_get_state(KEY_RIGHT) || buttons_get_state(KEY_B)) { if (gx < FB_W - 22) gx += 3; moved = true; }
            if (moved) draw();
        }
        uint32_t nowms = (uint32_t)(esp_timer_get_time() / 1000);
        if (!s_over && !s_paused && (nowms - last >= 16)) {
            last = nowms;
            step();
            if (s_over) {
                lv_label_set_text_fmt(msg, "游戏结束 %d 分  KEY4 重开", s_score);
            }
            draw();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
