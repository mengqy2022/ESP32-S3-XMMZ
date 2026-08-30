/*
 * led_ws2812.c — WS2812B 状态灯, RMT 发送 (800kHz, GRB)
 * 原理图: LED2 XL-5050RGBC-2812B-S, DI <- GPIO38
 */
#include "freertos/FreeRTOS.h"
#include "driver/rmt_tx.h"
#include "esp_check.h"
#include "esp_log.h"
#include "led_ws2812.h"
#include "board_config.h"

static const char *TAG = "ws2812";

static rmt_channel_handle_t s_tx = NULL;
static rmt_encoder_handle_t s_encoder = NULL;

esp_err_t ws2812_init(void)
{
    rmt_tx_channel_config_t cfg = {
        .gpio_num = LED_PIN_WS2812,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,   /* 10MHz → 1 tick = 100ns */
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&cfg, &s_tx), TAG, "rmt new tx channel");

    rmt_bytes_encoder_config_t ecfg = {
        .bit0 = { .duration0 = 4, .level0 = 1, .duration1 = 8, .level1 = 0 },  /* 0: 400ns高+800ns低 */
        .bit1 = { .duration0 = 8, .level0 = 1, .duration1 = 4, .level1 = 0 },  /* 1: 800ns高+400ns低 */
        .flags.msb_first = 1,
    };
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&ecfg, &s_encoder), TAG, "new bytes encoder");
    ESP_RETURN_ON_ERROR(rmt_enable(s_tx), TAG, "rmt enable");

    ws2812_off();
    ESP_LOGI(TAG, "WS2812B init OK (GPIO%d)", LED_PIN_WS2812);
    return ESP_OK;
}

esp_err_t ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t grb[3] = { g, r, b };   /* WS2812B 字节序: GRB */
    rmt_transmit_config_t tcfg = {
        .loop_count = 0,
        .flags.eot_level = 0,
    };
    ESP_RETURN_ON_ERROR(rmt_transmit(s_tx, s_encoder, grb, sizeof(grb), &tcfg), TAG, "transmit");
    ESP_RETURN_ON_ERROR(rmt_tx_wait_all_done(s_tx, pdMS_TO_TICKS(50)), TAG, "wait done");
    return ESP_OK;
}

esp_err_t ws2812_off(void)
{
    return ws2812_set_rgb(0, 0, 0);
}
