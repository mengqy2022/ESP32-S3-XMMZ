/*
 * game_snake.c — 贪吃蛇 (全屏, 三种模式, 直接像素渲染)
 * 模式: 经典(撞墙/撞自己结束) / 无尽(穿墙不死) / 极速(穿墙+更快)
 * 渲染: lv_image 直接显示 PSRAM 像素缓冲
 */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "audio_sfx.h"

#define CELL  16
#define GW    20
#define GH    15
#define MAX_LEN 300

typedef struct { int x, y; } pt_t;

#define MODE_CLASSIC 0
#define MODE_ENDLESS 1
#define MODE_SPEED   2

LV_FONT_DECLARE(ui_font_lvgl_10);

static uint16_t *s_fb = NULL;
static lv_image_dsc_t s_dsc = { 0 };
static lv_obj_t *s_img = NULL;

static inline void fb_px(int x, int y, uint16_t c)
{
    if (x < 0 || x >= 320 || y < 0 || y >= 240) return;
    s_fb[y * 320 + x] = c;
}

static uint16_t rgb565(uint32_t rgb)
{
    return ((rgb >> 8 & 0xFF) >> 3 << 11) | ((rgb >> 0 & 0xFF) >> 2 << 5) | (rgb >> 16 & 0xFF) >> 3;
}

static void fb_rect(int x, int y, int w, int h, uint32_t rgb)
{
    uint16_t c = rgb565(rgb);
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            fb_px(xx, yy, c);
}

/* ---------- 模式选择 ---------- */
static int mode_select(void)
{
    static const char *names[3] = { "经典", "无尽", "极速" };
    static const char *descs[3] = { "撞墙/撞自己结束", "穿墙不死 一直吃", "穿墙+超快节奏" };
    static const uint32_t cols[3] = { 0xE74C3C, 0x00FF88, 0xFFD700 };

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x060A12), 0);
    lv_obj_set_style_text_font(scr, &ui_font_lvgl, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "贪吃蛇");
    lv_obj_set_style_text_color(title, lv_color_hex(0x44FFAA), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text(sub, "选择模式");
    lv_obj_set_style_text_color(sub, lv_color_hex(0x667788), 0);
    lv_obj_set_style_text_font(sub, &ui_font_lvgl_10, 0);
    lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 56);

    for (int i = 0; i < 3; i++) {
        lv_obj_t *b = lv_button_create(scr);
        lv_obj_set_size(b, 280, 40);
        lv_obj_set_pos(b, 20, 84 + i * 46);
        lv_obj_set_style_bg_color(b, lv_color_hex(0x0D1626), 0);
        lv_obj_set_style_bg_color(b, lv_color_hex(0x16263A), LV_STATE_FOCUSED);
        lv_obj_set_style_shadow_width(b, 0, 0);
        lv_obj_set_style_radius(b, 8, 0);
        lv_obj_set_style_border_width(b, 0, 0);
        lv_obj_set_user_data(b, (void *)(intptr_t)i);
        lv_group_add_obj(lv_group_get_default(), b);
        lv_obj_t *dot = lv_obj_create(b);
        lv_obj_set_size(dot, 8, 8);
        lv_obj_set_style_bg_color(dot, lv_color_hex(cols[i]), 0);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_align(dot, LV_ALIGN_LEFT_MID, 14, 0);
        lv_obj_t *n = lv_label_create(b);
        lv_label_set_text(n, names[i]);
        lv_obj_set_style_text_color(n, UI_THEME_TEXT, 0);
        lv_obj_align(n, LV_ALIGN_LEFT_MID, 32, -7);
        lv_obj_t *d = lv_label_create(b);
        lv_label_set_text(d, descs[i]);
        lv_obj_set_style_text_color(d, lv_color_hex(0x667788), 0);
        lv_obj_set_style_text_font(d, &ui_font_lvgl_10, 0);
        lv_obj_align(d, LV_ALIGN_LEFT_MID, 32, 8);
    }
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "KEY4 开始  KEY3 退出");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x556677), 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);

    lv_obj_t *first = lv_obj_get_child(scr, 4);
    if (first) lv_group_focus_obj(first);

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
                int m = f ? (int)(intptr_t)lv_obj_get_user_data(f) : 0;
                return m;
            }
            case KEY_BACK:
            case KEY_HOME:
                return -1;
            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/* ---------- 游戏 ---------- */
int game_snake_run(void)
{
    int mode = mode_select();
    if (mode < 0) return 0;

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(scr, &ui_font_lvgl, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    s_fb = heap_caps_malloc(320 * 240 * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!s_fb) return ESP_FAIL;
    s_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
    s_dsc.header.w = 320;
    s_dsc.header.h = 240;
    s_dsc.header.stride = 320 * 2;
    s_dsc.data_size = 320 * 240 * 2;
    s_dsc.data = (const uint8_t *)s_fb;
    s_img = lv_image_create(scr);
    lv_obj_set_size(s_img, 320, 240);
    lv_obj_set_pos(s_img, 0, 0);
    lv_image_set_src(s_img, &s_dsc);

    lv_obj_t *score_label = lv_label_create(scr);
    lv_obj_set_style_text_color(score_label, lv_color_hex(0x66D9FF), 0);
    lv_obj_set_pos(score_label, 8, 4);
    lv_obj_t *mode_label = lv_label_create(scr);
    lv_obj_set_style_text_color(mode_label, lv_color_hex(0xFFAA33), 0);
    lv_obj_set_style_text_font(mode_label, &ui_font_lvgl_10, 0);
    lv_obj_align(mode_label, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_t *msg = lv_label_create(scr);
    lv_obj_set_style_text_color(msg, lv_color_hex(0xFFAA33), 0);
    lv_obj_set_style_text_font(msg, &ui_font_lvgl_10, 0);
    lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "五向=方向  KEY4=暂停  KEY3=退出");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x667788), 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);

    static pt_t snake[MAX_LEN];
    int len = 3, dir = 0, fx = 12, fy = 7, score = 0;
    bool paused = false, over = false;
    uint32_t frame = 0;

    const char *mode_name = (mode == MODE_CLASSIC) ? "经典" : (mode == MODE_ENDLESS) ? "无尽" : "极速";
    lv_label_set_text(mode_label, mode_name);

    void place_food(void)
    {
        bool on;
        do {
            on = false;
            fx = rand() % GW; fy = rand() % GH;
            for (int i = 0; i < len; i++)
                if (snake[i].x == fx && snake[i].y == fy) { on = true; break; }
        } while (on);
    }

    void reset(void)
    {
        len = 3; dir = 0; score = 0; paused = false; over = false;
        snake[0] = (pt_t){ 10, 7 }; snake[1] = (pt_t){ 9, 7 }; snake[2] = (pt_t){ 8, 7 };
        place_food();
        lv_label_set_text_fmt(score_label, "得分 %d", score);
        lv_label_set_text(msg, "");
    }

    void draw(void)
    {
        /* 背景渐变 */
        for (int y = 0; y < 240; y++) {
            uint16_t c = rgb565(0x060A14);
            if (y > 200) c = rgb565(0x0A1220);
            for (int x = 0; x < 320; x++) s_fb[y * 320 + x] = c;
        }
        /* 网格 */
        for (int gx = 0; gx <= GW; gx++)
            for (int y = 0; y < 240; y += 2) fb_px(gx * CELL, y, rgb565(0x0D1830));
        for (int gy = 0; gy <= GH; gy++)
            for (int x = 0; x < 320; x += 2) fb_px(x, gy * CELL, rgb565(0x0D1830));
        /* 食物: 闪烁 */
        if ((frame / 6) % 2) {
            fb_rect(fx * CELL + 1, fy * CELL + 1, CELL - 2, CELL - 2, 0xFF3322);
            fb_rect(fx * CELL + 4, fy * CELL + 4, 4, 4, 0xFFBBAA);
        } else {
            fb_rect(fx * CELL + 1, fy * CELL + 1, CELL - 2, CELL - 2, 0xFF6644);
        }
        /* 蛇: 头部亮绿, 身体渐变 */
        for (int i = len - 1; i >= 0; i--) {
            int x = snake[i].x * CELL, y = snake[i].y * CELL;
            uint32_t c;
            if (i == 0) c = 0x55FFBB;
            else if (i < 8) c = 0x33CC77;
            else c = 0x1A8A4E - ((i / 8) % 5) * 0x002000;
            fb_rect(x + 1, y + 1, CELL - 2, CELL - 2, c);
        }
        /* 蛇头: 眼睛 */
        int hx = snake[0].x * CELL, hy = snake[0].y * CELL;
        fb_px(hx + 4, hy + 4, rgb565(0x000000));
        fb_px(hx + 5, hy + 4, rgb565(0x000000));
        fb_px(hx + 10, hy + 4, rgb565(0x000000));
        fb_px(hx + 11, hy + 4, rgb565(0x000000));
        lv_obj_invalidate(s_img);
    }

    void step(void)
    {
        int hx = snake[0].x, hy = snake[0].y;
        switch (dir) {
        case 0: hx++; break;
        case 1: hy++; break;
        case 2: hx--; break;
        case 3: hy--; break;
        }
        /* 经典模式: 撞墙/撞自己结束 */
        if (mode == MODE_CLASSIC) {
            if (hx < 0 || hx >= GW || hy < 0 || hy >= GH) { over = true; return; }
            for (int i = 0; i < len - 1; i++)
                if (snake[i].x == hx && snake[i].y == hy) { over = true; return; }
        } else {
            /* 无尽/极速: 穿墙 */
            if (hx < 0) hx = GW - 1;
            if (hx >= GW) hx = 0;
            if (hy < 0) hy = GH - 1;
            if (hy >= GH) hy = 0;
        }
        bool ate = (hx == fx && hy == fy);
        if (ate) {
            if (len < MAX_LEN - 1) len++;
            score += 10;
            sfx_play(SFX_EAT);
            lv_label_set_text_fmt(score_label, "得分 %d", score);
            place_food();
        }
        for (int i = len - 1; i > 0; i--) snake[i] = snake[i - 1];
        snake[0] = (pt_t){ hx, hy };
        frame++;
    }

    reset();
    draw();
    uint32_t last = (uint32_t)(esp_timer_get_time() / 1000);

    for (;;) {
        lv_timer_handler();
        key_event_t evt;
        while (buttons_wait_event(&evt, 0)) {
            if (evt.evt != KEY_EVT_PRESS) continue;
            switch (evt.key) {
            case KEY_UP:    if (dir != 1) { dir = 3; sfx_play(SFX_MOVE); } break;
            case KEY_DOWN:  if (dir != 3) { dir = 1; sfx_play(SFX_MOVE); } break;
            case KEY_LEFT:  if (dir != 0) { dir = 2; sfx_play(SFX_MOVE); } break;
            case KEY_RIGHT: if (dir != 2) { dir = 0; sfx_play(SFX_MOVE); } break;
            case KEY_CONFIRM:
                if (over) {
                    reset();          /* 重开 (同模式) */
                } else {
                    paused = !paused;
                }
                break;
            case KEY_BACK:
            case KEY_HOME:
                heap_caps_free(s_fb);
                s_fb = NULL;
                return 0;
            default: break;
            }
        }
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        /* 速度: 基础(经典180/无尽180/极速110), 每 100 分减 5, 下限 60 */
        int base = (mode == MODE_SPEED) ? 110 : 180;
        int speed = base - (score / 100) * 5;
        if (speed < 60) speed = 60;
        if (!paused && !over && (now - last >= (uint32_t)speed)) {
            last = now;
            step();
            if (over) {
                sfx_play(SFX_GAMEOVER);
                lv_label_set_text_fmt(msg, "游戏结束 得分 %d  KEY4 重开", score);
            }
            draw();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
