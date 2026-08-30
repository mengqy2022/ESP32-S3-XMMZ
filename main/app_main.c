/*
 * app_main.c — ESP32-S3 游戏掌机 主程序
 * 启动流程: 硬件自检 -> LVGL 初始化 -> 主菜单 (常驻)
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "board_config.h"
#include "lcd_st7789.h"
#include "buttons.h"
#include "led_ws2812.h"
#include "battery.h"
#include "audio_beep.h"
#include "sd_card.h"
#include "lvgl_port.h"
#include "services/led_ctrl.h"
#include "services/ble.h"
#include "services/wifi_mgr.h"
#include "services/file_proto.h"
#include "apps/lvgl_menu.h"
#include "apps/apps.h"

static const char *TAG = "main";

static void hw_selftest(void)
{
    /* 显示 / 按键 / 灯 / 电池 / 功放 / SD 全部初始化, 失败不阻塞 */
    lcd_init();
    buttons_init();
    ws2812_init();
    battery_init();
    audio_init();

    esp_err_t sd_err = sd_card_init();
    ESP_LOGI(TAG, "SD init result: %s", esp_err_to_name(sd_err));

    /* 串口文件传输协议 (PC 工具经 CH340 直接读写 SD 卡, 无需拔卡) */
    file_proto_start();

    /* 开机提示音: 验证喇叭/功放通路 (应听到一声短"嘀") */
    audio_beep(880, 80, 50);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== 小孟蜜汁系统 启动 ===");

    /* 第一时间强制灭背光: 上电瞬间 ST7789 显存随机, 若背光先亮会露花屏
     * (lcd_init 里会先清屏再重新点亮) */
    gpio_config_t bl_cfg = {
        .pin_bit_mask = (1ULL << TFT_PIN_BL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&bl_cfg);
    gpio_set_level(TFT_PIN_BL, 0);

    esp_err_t nvs = nvs_flash_init();
    if (nvs == ESP_ERR_NVS_NO_FREE_PAGES || nvs == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_chip_info_t chip;
    esp_chip_info(&chip);
    uint32_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);
    ESP_LOGI(TAG, "chip: %d cores, flash %luMB, features 0x%x",
             chip.cores, (unsigned long)(flash_size / 1024 / 1024), (unsigned)chip.features);

    hw_selftest();

    esp_err_t lerr = lvgl_sys_init();
    if (lerr != ESP_OK) {
        ESP_LOGE(TAG, "LVGL init failed, halt");
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    led_ctrl_init();   /* 状态灯: 载入保存的颜色/亮度/模式并驱动 */
    /* 蓝牙: 开机初始化 (BT 钉在核心1, 不与 SPI 渲染争抢); 广播默认关闭 */
    ble_start();
    wifi_mgr_init();   /* WiFi: 有保存配置则自动连接, 连上后 SNTP 同步时间 */

    ESP_LOGI(TAG, "enter LVGL main menu");
    lvgl_menu_run();     /* 常驻 (含屏保) */
}
