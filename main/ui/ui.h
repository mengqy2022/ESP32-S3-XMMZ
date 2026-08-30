/*
 * ui.h — 轻量 UI 引擎 (PSRAM 帧缓冲, RGB565)
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#define UI_W 240
#define UI_H 320

/* ---------- 帧缓冲 ---------- */
esp_err_t ui_init(void);
void ui_flush(void);                        /* 整屏刷到 LCD */
uint16_t *ui_fb(void);
void ui_fill(uint16_t color);

/* ---------- 绘制原语 ---------- */
void ui_pixel(int x, int y, uint16_t c);
void ui_rect(int x, int y, int w, int h, uint16_t c);
void ui_rect_fill(int x, int y, int w, int h, uint16_t c);
void ui_hline(int x, int y, int w, uint16_t c);
void ui_vline(int x, int y, int h, uint16_t c);

/* ---------- 文本 ---------- */
/* 中文字库: 16x16 GB2312 (内置嵌入), ASCII: 16x8 */
esp_err_t ui_font_init(void);               /* 加载/确认内嵌字库 */
int ui_text(int x, int y, const char *s, uint16_t color);   /* 返回下一行 y 偏移 */
int ui_text_w(const char *s);               /* 字符串像素宽度 */

/* ---------- 常用控件 ---------- */
void ui_statusbar(const char *left, const char *right, uint16_t bg, uint16_t fg);
void ui_progress(int x, int y, int w, int h, uint8_t pct, uint16_t fg, uint16_t bg);

/* 常用颜色 */
#define UI_BLACK   0x0000
#define UI_WHITE   0xFFFF
#define UI_RED     0xF800
#define UI_GREEN   0x07E0
#define UI_BLUE    0x001F
#define UI_YELLOW  0xFFE0
#define UI_CYAN    0x07FF
#define UI_MAGENTA 0xF81F
#define UI_GRAY    0x8410
#define UI_DGRAY   0x4208
#define UI_ORANGE  0xFD20
