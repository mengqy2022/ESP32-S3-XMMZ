/*
 * led_ctrl.h — WS2812B 灯管控制器
 * 支持: 颜色 / 亮度 / 点亮方式 (常亮/呼吸/闪烁/彩虹/关闭), 设置存入 NVS
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"

typedef enum {
    LED_MODE_OFF = 0,      /* 关闭 */
    LED_MODE_ON = 1,       /* 常亮 */
    LED_MODE_BREATH = 2,   /* 呼吸 */
    LED_MODE_BLINK = 3,    /* 闪烁 */
    LED_MODE_RAINBOW = 4,  /* 彩虹循环 */
    LED_MODE_MAX
} led_mode_t;

/* 预设颜色 (R,G,B) */
#define LED_COLOR_COUNT 5
typedef struct { uint8_t r, g, b; const char *name; } led_color_t;
extern const led_color_t led_colors[LED_COLOR_COUNT];

esp_err_t led_ctrl_init(void);                       /* 载入 NVS 设置并启动后台任务 */
esp_err_t led_ctrl_set(uint8_t color_idx, uint8_t bright_pct, led_mode_t mode);  /* 应用+保存 */

uint8_t led_ctrl_get_color(void);
uint8_t led_ctrl_get_bright(void);
led_mode_t led_ctrl_get_mode(void);

/* 模式中文名 (供 UI 显示) */
const char *led_ctrl_mode_name(led_mode_t m);
