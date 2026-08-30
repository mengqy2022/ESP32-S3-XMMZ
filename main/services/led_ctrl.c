/*
 * led_ctrl.c — WS2812B 灯管控制器
 * 后台任务按模式驱动 GPIO38 灯珠; 设置持久化到 NVS (namespace "led")
 */
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "led_ws2812.h"
#include "led_ctrl.h"

static const char *TAG = "led_ctrl";

const led_color_t led_colors[LED_COLOR_COUNT] = {
    { 255,   0,   0, "红色" },
    {   0, 255,   0, "绿色" },
    {   0,   0, 255, "蓝色" },
    { 255, 255,   0, "黄色" },
    { 255, 255, 255, "白色" },
};

const char *led_ctrl_mode_name(led_mode_t m)
{
    static const char *names[LED_MODE_MAX] = { "关闭", "常亮", "呼吸", "闪烁", "彩虹" };
    if ((int)m < 0 || m >= LED_MODE_MAX) return "?";
    return names[m];
}

static uint8_t s_color = 0;       /* 颜色索引 */
static uint8_t s_bright = 50;     /* 亮度 0-100 */
static led_mode_t s_mode = LED_MODE_ON;

static void load_settings(void)
{
    nvs_handle_t h;
    if (nvs_open("led", NVS_READONLY, &h) == ESP_OK) {
        uint8_t v = 0;
        if (nvs_get_u8(h, "color", &v) == ESP_OK && v < LED_COLOR_COUNT) s_color = v;
        if (nvs_get_u8(h, "bright", &v) == ESP_OK) s_bright = v;
        if (nvs_get_u8(h, "mode", &v) == ESP_OK && v < LED_MODE_MAX) s_mode = (led_mode_t)v;
        nvs_close(h);
    }
}

static void save_settings(void)
{
    nvs_handle_t h;
    if (nvs_open("led", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u8(h, "color", s_color);
        nvs_set_u8(h, "bright", s_bright);
        nvs_set_u8(h, "mode", (uint8_t)s_mode);
        nvs_commit(h);
        nvs_close(h);
    }
}

esp_err_t led_ctrl_set(uint8_t color_idx, uint8_t bright_pct, led_mode_t mode)
{
    if (color_idx >= LED_COLOR_COUNT) color_idx = 0;
    if (bright_pct > 100) bright_pct = 100;
    if (mode >= LED_MODE_MAX) mode = LED_MODE_ON;
    s_color = color_idx;
    s_bright = bright_pct;
    s_mode = mode;
    save_settings();
    ESP_LOGI(TAG, "led set: color=%d bright=%d mode=%d", s_color, s_bright, s_mode);
    return ESP_OK;
}

uint8_t led_ctrl_get_color(void) { return s_color; }
uint8_t led_ctrl_get_bright(void) { return s_bright; }
led_mode_t led_ctrl_get_mode(void) { return s_mode; }

/* HSV -> RGB (彩虹用) */
static void hsv2rgb(uint8_t h, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t region = h / 43;
    uint8_t rem = (h - region * 43) * 6;
    uint8_t q = 255 - rem;
    switch (region) {
    case 0: *r = 255; *g = rem; *b = 0; break;
    case 1: *r = q;   *g = 255; *b = 0; break;
    case 2: *r = 0;   *g = 255; *b = rem; break;
    case 3: *r = 0;   *g = q;   *b = 255; break;
    case 4: *r = rem; *g = 0;   *b = 255; break;
    default:*r = 255; *g = 0;   *b = q; break;
    }
}

static void led_task(void *arg)
{
    uint32_t tick = 0;
    for (;;) {
        uint8_t r = 0, g = 0, b = 0;
        float scale = 1.0f;

        switch (s_mode) {
        case LED_MODE_OFF:
            break;
        case LED_MODE_ON:
            r = led_colors[s_color].r; g = led_colors[s_color].g; b = led_colors[s_color].b;
            break;
        case LED_MODE_BREATH: {
            float ph = (float)(tick % 200) / 200.0f * 2.0f * (float)M_PI;
            scale = 0.2f + 0.8f * (0.5f + 0.5f * sinf(ph));
            r = led_colors[s_color].r; g = led_colors[s_color].g; b = led_colors[s_color].b;
            break;
        }
        case LED_MODE_BLINK:
            if ((tick / 10) % 2 == 0) {
                r = led_colors[s_color].r; g = led_colors[s_color].g; b = led_colors[s_color].b;
            }
            break;
        case LED_MODE_RAINBOW:
            hsv2rgb((uint8_t)(tick * 2), &r, &g, &b);
            break;
        default:
            break;
        }

        /* 亮度缩放 */
        float br = s_bright / 100.0f * scale;
        ws2812_set_rgb((uint8_t)(r * br), (uint8_t)(g * br), (uint8_t)(b * br));
        tick++;
        vTaskDelay(pdMS_TO_TICKS(25));   /* 呼吸/彩虹平滑 */
    }
}

esp_err_t led_ctrl_init(void)
{
    load_settings();
    xTaskCreate(led_task, "led_ctrl", 3072, NULL, 4, NULL);
    ESP_LOGI(TAG, "led_ctrl init: color=%d bright=%d mode=%d", s_color, s_bright, s_mode);
    return ESP_OK;
}
