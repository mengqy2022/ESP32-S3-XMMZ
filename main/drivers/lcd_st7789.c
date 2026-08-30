/*
 * lcd_st7789.c — ST7789 240x320 显示屏驱动
 * 接线 (原理图): MOSI=GPIO12, CLK=GPIO48, CS=GPIO14, DC=GPIO47, RST=GPIO3, BL=GPIO39
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_st7789.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "lcd_st7789.h"
#include "board_config.h"

static const char *TAG = "lcd";

static esp_lcd_panel_handle_t s_panel = NULL;
static esp_lcd_panel_io_handle_t s_io = NULL;

esp_err_t lcd_init(void)
{
    ESP_LOGI(TAG, "init SPI2 bus: clk=GPIO%d mosi=GPIO%d", TFT_PIN_CLK, TFT_PIN_MOSI);
    spi_bus_config_t buscfg = {
        .sclk_io_num = TFT_PIN_CLK,
        .mosi_io_num = TFT_PIN_MOSI,
        .miso_io_num = TFT_PIN_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_W * LCD_H * 2 + 8,
    };
    esp_err_t err = spi_bus_initialize(TFT_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num = TFT_PIN_DC,
        .cs_gpio_num = TFT_PIN_CS,
        .pclk_hz = TFT_SPI_CLK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TFT_SPI_HOST, &io_cfg, &s_io);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "new panel io failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = TFT_PIN_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
        .flags.reset_active_high = false,
    };
    err = esp_lcd_new_panel_st7789(s_io, &panel_cfg, &s_panel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "new panel failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_lcd_panel_reset(s_panel);
    esp_lcd_panel_init(s_panel);
    /* 方向/镜像由 LVGL port 统一管理 (见 lvgl_port.c), 这里只做颜色反相 */
    esp_lcd_panel_invert_color(s_panel, true);
    esp_lcd_panel_disp_on_off(s_panel, true);

    /* 关键: 清屏前先设横屏方向!
     * ST7789 默认竖屏(物理 240x320), 若按逻辑 320x240 填充窗口会超出物理宽度,
     * 只清掉部分屏幕 → 上电瞬间残留随机内容(花屏/分区).
     * 先 swap_xy+mirror 与 LVGL 最终方向一致, 填充窗口才能覆盖整屏. */
    esp_lcd_panel_swap_xy(s_panel, true);
    esp_lcd_panel_mirror(s_panel, false, true);

    /* 背光 LEDC PWM */
    ledc_timer_config_t ltim = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ltim);
    ledc_channel_config_t lch = {
        .gpio_num = TFT_PIN_BL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,      /* 先灭背光: 上电瞬间 ST7789 显存是随机的, 亮了会露花屏 */
        .hpoint = 0,
    };
    ledc_channel_config(&lch);

    /* 先清屏 (背光灭, 用户看不到未初始化内容), 再亮背光 → 上电无花屏 */
    lcd_fill(0x0000);
    lcd_set_backlight(80);
    ESP_LOGI(TAG, "ST7789 init OK, %dx%d", LCD_W, LCD_H);
    return ESP_OK;
}

esp_err_t lcd_set_backlight(uint8_t percent)
{
    if (percent > 100) percent = 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, percent * 255 / 100);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    return ESP_OK;
}

esp_err_t lcd_fill(uint16_t color)
{
    /* 关键: 不能用整屏 PSRAM 缓冲! esp_lcd SPI panel io 不支持 PSRAM 直达
     * DMA (不置 SPI_TRANS_DMA_USE_PSRAM), 整屏 153KB 需要一次性私有 DMA
     * 拷贝 → 内部 DMA 内存不够 → 失败 (日志: setup_dma_priv_buffer Failed).
     * 用小块内部 DMA 缓冲分带填充: 直达 DMA, 零拷贝, 永不失败. */
    const size_t BAND_PX = 4096;   /* 8KB 内部 DMA 缓冲 */
    static uint16_t *buf = NULL;
    if (!buf) {
        buf = heap_caps_malloc(BAND_PX * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (!buf) {
            ESP_LOGW(TAG, "lcd_fill: no internal DMA buffer, skip fill");
            return ESP_ERR_NO_MEM;
        }
    }
    for (size_t i = 0; i < BAND_PX; i++) buf[i] = color;
    for (int y = 0; y < LCD_H; y += BAND_PX / LCD_W) {
        int rows = BAND_PX / LCD_W;
        if (y + rows > LCD_H) rows = LCD_H - y;
        esp_err_t err = lcd_draw_bitmap(0, y, LCD_W, rows, buf);
        if (err != ESP_OK) return err;
    }
    return ESP_OK;
}

esp_err_t lcd_draw_bitmap(int x, int y, int w, int h, const uint16_t *data)
{
    if (!s_panel) return ESP_ERR_INVALID_STATE;
    return esp_lcd_panel_draw_bitmap(s_panel, x, y, x + w, y + h, data);
}

void *lcd_get_panel_handle(void)
{
    return s_panel;
}

void *lcd_get_io_handle(void)
{
    return s_io;
}
