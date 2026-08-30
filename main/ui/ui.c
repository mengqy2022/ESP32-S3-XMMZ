/*
 * ui.c — 轻量 UI 引擎
 * 帧缓冲放 PSRAM (240x320x2 = 150KB), 整屏刷新
 */
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "ui.h"
#include "lcd_st7789.h"
#include "font.h"

static const char *TAG = "ui";

static uint16_t *s_fb = NULL;

/* 字库指针 (由 font.c 提供) */
static const uint8_t *s_ascii = NULL;
static const uint8_t *s_gb = NULL;

esp_err_t ui_init(void)
{
    if (s_fb) return ESP_OK;
    s_fb = heap_caps_malloc(UI_W * UI_H * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!s_fb) {
        ESP_LOGE(TAG, "framebuffer alloc failed");
        return ESP_ERR_NO_MEM;
    }
    ui_fill(UI_BLACK);
    ui_flush();
    ESP_LOGI(TAG, "UI init OK (fb @psram)");
    return ESP_OK;
}

void ui_flush(void)
{
    if (s_fb) lcd_draw_bitmap(0, 0, UI_W, UI_H, s_fb);
}

uint16_t *ui_fb(void) { return s_fb; }

void ui_fill(uint16_t color)
{
    if (!s_fb) return;
    for (int i = 0; i < UI_W * UI_H; i++) s_fb[i] = color;
}

void ui_pixel(int x, int y, uint16_t c)
{
    if (!s_fb || x < 0 || y < 0 || x >= UI_W || y >= UI_H) return;
    s_fb[y * UI_W + x] = c;
}

void ui_hline(int x, int y, int w, uint16_t c)
{
    if (!s_fb || y < 0 || y >= UI_H) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > UI_W) w = UI_W - x;
    if (w <= 0) return;
    uint16_t *p = &s_fb[y * UI_W + x];
    for (int i = 0; i < w; i++) p[i] = c;
}

void ui_vline(int x, int y, int h, uint16_t c)
{
    if (!s_fb || x < 0 || x >= UI_W) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > UI_H) h = UI_H - y;
    if (h <= 0) return;
    for (int i = 0; i < h; i++) s_fb[(y + i) * UI_W + x] = c;
}

void ui_rect(int x, int y, int w, int h, uint16_t c)
{
    ui_hline(x, y, w, c);
    ui_hline(x, y + h - 1, w, c);
    ui_vline(x, y, h, c);
    ui_vline(x + w - 1, y, h, c);
}

void ui_rect_fill(int x, int y, int w, int h, uint16_t c)
{
    for (int i = 0; i < h; i++) ui_hline(x, y + i, w, c);
}

esp_err_t ui_font_init(void)
{
    esp_err_t err = font_load(&s_ascii, &s_gb);
    if (err != ESP_OK) ESP_LOGW(TAG, "embedded font not available");
    else ESP_LOGI(TAG, "font loaded: ascii=%p gb2312=%p", s_ascii, s_gb);
    return err;
}

/* UTF-8 解码: 返回码点, 推进指针 */
static uint32_t utf8_next(const char **sp)
{
    const uint8_t *p = (const uint8_t *)*sp;
    if (p[0] < 0x80) { *sp += 1; return p[0]; }
    int n;
    uint32_t v;
    if ((p[0] & 0xE0) == 0xC0)      { n = 1; v = p[0] & 0x1F; }
    else if ((p[0] & 0xF0) == 0xE0) { n = 2; v = p[0] & 0x0F; }
    else if ((p[0] & 0xF8) == 0xF0) { n = 3; v = p[0] & 0x07; }
    else { *sp += 1; return 0xFFFD; }
    for (int i = 1; i <= n; i++) {
        if (!p[i]) { *sp += 1; return 0xFFFD; }
        v = (v << 6) | (p[i] & 0x3F);
    }
    *sp += n + 1;
    return v;
}

int ui_text(int x, int y, const char *s, uint16_t color)
{
    if (!s_fb || !s) return 0;
    int cx = x;
    while (*s) {
        const char *sp = s;
        uint32_t cp = utf8_next(&sp);
        if (cp == '\n') {
            cx = x; y += 16;
        } else if (cp < 0x80) {
            if (s_ascii && cp >= 0x20 && cp < 0x7F) {
                const uint8_t *g = &s_ascii[(cp - 0x20) * 16];
                for (int r = 0; r < 16; r++) {
                    uint8_t bits = g[r];
                    for (int b = 0; b < 8; b++) {
                        if (bits & (0x80 >> b)) ui_pixel(cx + b, y + r, color);
                    }
                }
                cx += 8;
            } else {
                cx += 8;   /* 不可见 ASCII 跳过 */
            }
        } else {
            /* 中文/符号: Unicode -> 字模索引 */
            int idx = font_cp_index(cp);
            if (idx >= 0 && s_gb) {
                const uint8_t *g = &s_gb[idx * 32];
                for (int r = 0; r < 16; r++) {
                    uint8_t b0 = g[r * 2], b1 = g[r * 2 + 1];
                    for (int b = 0; b < 8; b++) {
                        if (b0 & (0x80 >> b)) ui_pixel(cx + b, y + r, color);
                        if (b1 & (0x80 >> b)) ui_pixel(cx + 8 + b, y + r, color);
                    }
                }
                cx += 16;
            } else {
                cx += 16;   /* 未收录字符占位 */
            }
        }
        s = sp;
    }
    return y;
}

int ui_text_w(const char *s)
{
    int w = 0;
    while (*s) {
        const char *sp = s;
        uint32_t cp = utf8_next(&sp);
        if (cp < 0x80) w += 8;
        else w += 16;
        s = sp;
    }
    return w;
}

/* GB2312 区位码 -> 字库索引: 由 font.c 提供 */

void ui_statusbar(const char *left, const char *right, uint16_t bg, uint16_t fg)
{
    ui_rect_fill(0, 0, UI_W, 18, bg);
    ui_hline(0, 18, UI_W, UI_DGRAY);
    ui_text(2, 1, left, fg);
    if (right) {
        int w = ui_text_w(right);
        ui_text(UI_W - 2 - w, 1, right, fg);
    }
}

void ui_progress(int x, int y, int w, int h, uint8_t pct, uint16_t fg, uint16_t bg)
{
    ui_rect_fill(x, y, w, h, bg);
    if (pct > 100) pct = 100;
    int fw = w * pct / 100;
    if (fw > 0) ui_rect_fill(x, y, fw, h, fg);
}
