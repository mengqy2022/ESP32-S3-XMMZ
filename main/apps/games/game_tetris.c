/*
 * game_tetris.c — 俄罗斯方块 (全屏, 直接像素渲染)
 * 五向-左/右=移动, 上=旋转, 下=加速下落, KEY4=暂停, KEY3=退出
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "audio_sfx.h"
#include "game_common.h"

#define COLS  10
#define ROWS  20
#define CS    12
#define TW    (COLS * CS)      /* 120 */
#define TH    (ROWS * CS)      /* 240 */
#define GX    16
#define PX    158

LV_FONT_DECLARE(ui_font_lvgl_10);
LV_FONT_DECLARE(ui_font_lvgl);

static const uint8_t T_SHAPES[7][16] = {
    { 0,0,0,0,  1,1,1,1,  0,0,0,0,  0,0,0,0 },
    { 1,1,0,0,  1,1,0,0,  0,0,0,0,  0,0,0,0 },
    { 0,1,0,0,  1,1,1,0,  0,0,0,0,  0,0,0,0 },
    { 1,0,0,0,  1,1,1,0,  0,0,0,0,  0,0,0,0 },
    { 0,0,1,0,  1,1,1,0,  0,0,0,0,  0,0,0,0 },
    { 1,1,0,0,  0,1,1,0,  0,0,0,0,  0,0,0,0 },
    { 0,1,1,0,  1,1,0,0,  0,0,0,0,  0,0,0,0 },
};
static const uint32_t T_COLORS[7] = {
    0x00BFFF, 0xFFD700, 0x9B59B6, 0x3498DB, 0xE67E22, 0x2ECC71, 0xE74C3C,
};

int game_tetris_run(void)
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
    lv_obj_set_pos(score_label, PX, 130);
    lv_obj_t *next_label = lv_label_create(scr);
    lv_obj_set_style_text_color(next_label, lv_color_hex(0x66D9FF), 0);
    lv_obj_set_style_text_font(next_label, &ui_font_lvgl_10, 0);
    lv_obj_set_pos(next_label, PX, 22);
    lv_obj_t *msg = lv_label_create(scr);
    lv_obj_set_style_text_color(msg, lv_color_hex(0xFFAA33), 0);
    lv_obj_set_style_text_font(msg, &ui_font_lvgl_10, 0);
    lv_obj_set_pos(msg, PX, 170);
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "左右=移动 上=旋转 下=加速");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x667788), 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_set_pos(hint, PX, 216);

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);

    static uint8_t board[ROWS][COLS];
    int cur_type = 0, cx = 3, cy = 0, rot = 0, score = 0, next_type = 0;
    bool over = false, paused = false;

    void shape_get(int type, int r, int out[16])
    {
        const uint8_t *base = T_SHAPES[type];
        int tmp[16];
        for (int i = 0; i < 16; i++) tmp[i] = base[i];
        for (int k = 0; k < r; k++) {
            int o2[16] = { 0 };
            for (int yy = 0; yy < 4; yy++)
                for (int xx = 0; xx < 4; xx++)
                    o2[xx * 4 + (3 - yy)] = tmp[yy * 4 + xx];
            for (int i = 0; i < 16; i++) tmp[i] = o2[i];
        }
        for (int i = 0; i < 16; i++) out[i] = tmp[i];
    }

    bool collides(int type, int r, int x, int y)
    {
        int s[16];
        shape_get(type, r, s);
        for (int yy = 0; yy < 4; yy++)
            for (int xx = 0; xx < 4; xx++) {
                if (!s[yy * 4 + xx]) continue;
                int bx = x + xx, by = y + yy;
                if (bx < 0 || bx >= COLS || by >= ROWS) return true;
                if (by >= 0 && board[by][bx]) return true;
            }
        return false;
    }

    void spawn(void)
    {
        cur_type = next_type;
        next_type = rand() % 7;
        cx = 3;
        cy = -1;
        if (collides(cur_type, 0, cx, cy)) over = true;
    }

    void fix_and_clear(void)
    {
        int s[16];
        shape_get(cur_type, rot, s);
        for (int yy = 0; yy < 4; yy++)
            for (int xx = 0; xx < 4; xx++)
                if (s[yy * 4 + xx]) {
                    int by = cy + yy, bx = cx + xx;
                    if (by >= 0 && by < ROWS && bx >= 0 && bx < COLS) board[by][bx] = 1;
                }
        int lines = 0;
        for (int by = ROWS - 1; by >= 0; by--) {
            bool full = true;
            for (int bx = 0; bx < COLS; bx++) if (!board[by][bx]) { full = false; break; }
            if (full) {
                lines++;
                for (int r = by; r > 0; r--)
                    for (int bx = 0; bx < COLS; bx++) board[r][bx] = board[r - 1][bx];
                for (int bx = 0; bx < COLS; bx++) board[0][bx] = 0;
                by++;
            }
        }
        score += lines * 100;
        if (lines > 0) sfx_play(SFX_CLEAR);
        lv_label_set_text_fmt(score_label, "得分 %d", score);
        lv_label_set_text_fmt(next_label, "下一块: %d", next_type + 1);
    }

    void draw(void)
    {
        /* 背景 */
        for (int y = 0; y < FB_H; y++) {
            uint16_t c = fb_rgb565(0x05080F);
            if (y > 210) c = fb_rgb565(0x081020);
            for (int x = 0; x < FB_W; x++) fb[y * FB_W + x] = c;
        }
        /* 游戏区背景 + 边框 */
        fb_rect(fb, GX, 0, TW, TH, 0x0A1420);
        fb_frame(fb, GX, 0, TW, TH, 2, 0x1E3A5A);
        /* 网格 */
        for (int gx = 1; gx < COLS; gx++)
            for (int y = 0; y < TH; y += 3) fb_px(fb, GX + gx * CS, y, 0x101E30);
        for (int gy = 1; gy < ROWS; gy++)
            for (int x = 0; x < TW; x += 3) fb_px(fb, GX + x, gy * CS, 0x101E30);
        /* 已固定方块: 带高光 */
        for (int by = 0; by < ROWS; by++)
            for (int bx = 0; bx < COLS; bx++)
                if (board[by][bx]) {
                    int x = GX + bx * CS, y = by * CS;
                    fb_rect(fb, x, y, CS - 1, CS - 1, 0x5D8AA8);
                    fb_rect(fb, x + 1, y + 1, CS - 3, 2, 0x7FA8C8);
                }
        /* 当前块 */
        int s[16];
        shape_get(cur_type, rot, s);
        for (int yy = 0; yy < 4; yy++)
            for (int xx = 0; xx < 4; xx++) {
                if (!s[yy * 4 + xx]) continue;
                int by = cy + yy, bx = cx + xx;
                if (by >= 0 && by < ROWS && bx >= 0 && bx < COLS) {
                    int x = GX + bx * CS, y = by * CS;
                    fb_rect(fb, x, y, CS - 1, CS - 1, T_COLORS[cur_type]);
                    fb_rect(fb, x + 1, y + 1, CS - 3, 2, 0xFFFFFF80);
                }
            }
        /* 右侧面板: 下一块预览 */
        fb_rect(fb, PX, 40, 118, 62, 0x0A1420);
        fb_frame(fb, PX, 40, 118, 62, 2, 0x1E3A5A);
        {
            int ns[16];
            shape_get(next_type, 0, ns);
            for (int yy = 0; yy < 4; yy++)
                for (int xx = 0; xx < 4; xx++)
                    if (ns[yy * 4 + xx])
                        fb_rect(fb, PX + 40 + xx * 10, 48 + yy * 10, 9, 9, T_COLORS[next_type]);
        }
        fb_show(img);
    }

reset:
    memset(board, 0, sizeof(board));
    score = 0;
    over = false;
    paused = false;
    next_type = rand() % 7;
    spawn();
    lv_label_set_text_fmt(score_label, "得分 %d", score);
    lv_label_set_text_fmt(next_label, "下一块: %d", next_type + 1);
    lv_label_set_text(msg, "");
    draw();

    uint32_t last = (uint32_t)(esp_timer_get_time() / 1000);
    int fall_ms = 600;

    for (;;) {
        lv_timer_handler();
        key_event_t evt;
        while (buttons_wait_event(&evt, 0)) {
            if (evt.evt != KEY_EVT_PRESS) continue;
            if (!over && !paused) {
                switch (evt.key) {
                case KEY_LEFT:
                    if (!collides(cur_type, rot, cx - 1, cy)) { cx--; sfx_play(SFX_MOVE); }
                    break;
                case KEY_RIGHT:
                    if (!collides(cur_type, rot, cx + 1, cy)) cx++;
                    break;
                case KEY_UP: {
                    int nr = (rot + 1) % 4;
                    if (!collides(cur_type, nr, cx, cy)) { rot = nr; sfx_play(SFX_MOVE); }
                    break;
                }
                case KEY_DOWN:
                    if (!collides(cur_type, rot, cx, cy + 1)) { cy++; last = (uint32_t)(esp_timer_get_time() / 1000); }
                    else { fix_and_clear(); spawn(); draw(); }
                    break;
                case KEY_CONFIRM:
                    paused = true;
                    break;
                case KEY_BACK:
                case KEY_HOME:
                    fb_free(&fb);
                    return 0;
                default: break;
                }
                draw();
            } else if (over) {
                if (evt.key == KEY_CONFIRM) goto reset;
                if (evt.key == KEY_BACK || evt.key == KEY_HOME) { fb_free(&fb); return 0; }
            } else {
                if (evt.key == KEY_CONFIRM) paused = false;
                if (evt.key == KEY_BACK || evt.key == KEY_HOME) { fb_free(&fb); return 0; }
            }
        }

        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (!paused && !over && (now - last >= (uint32_t)fall_ms)) {
            last = now;
            if (!collides(cur_type, rot, cx, cy + 1)) {
                cy++;
            } else {
                fix_and_clear();
                spawn();
                draw();
                int nf = 600 - (score / 500) * 50;
                fall_ms = (nf < 200) ? 200 : nf;
            }
            if (over) { sfx_play(SFX_GAMEOVER); lv_label_set_text_fmt(msg, "游戏结束 %d 分 KEY4 重开", score); }
            draw();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
