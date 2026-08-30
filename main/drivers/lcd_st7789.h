/*
 * lcd_st7789.h — ST7789 240x320 显示屏驱动 (esp_lcd + SPI2)
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

esp_err_t lcd_init(void);
esp_err_t lcd_set_backlight(uint8_t percent);   /* 0-100 */
esp_err_t lcd_fill(uint16_t color);             /* 整屏填充 */
esp_err_t lcd_draw_bitmap(int x, int y, int w, int h, const uint16_t *data);

/* 供 LVGL port 使用 */
void *lcd_get_panel_handle(void);
void *lcd_get_io_handle(void);

/* 屏幕尺寸 (逻辑横屏 320x240) */
#define LCD_W 320
#define LCD_H 240
