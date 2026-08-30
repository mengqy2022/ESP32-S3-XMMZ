/*
 * board_config.h — ESP32-S3 游戏掌机 引脚定义
 * 依据原理图 SCH_Schematic1_2026-08-15.pdf（嘉立创EDA V1.0）
 * 芯片: ESP32-S3-WROOM-1-N16R8 (16MB Flash + 8MB OPI PSRAM)
 */
#pragma once

#include "sdkconfig.h"
#include "hal/gpio_types.h"
#include "driver/spi_common.h"

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
#error "xiaomeng-mizhixitong requires ESP32-S3; clean build/sdkconfig and configure target esp32s3"
#endif

/* ==================== TFT 显示屏 ST7789 320x240 横屏 (SPI2) ==================== */
#define TFT_SPI_HOST       SPI2_HOST
#define TFT_PIN_CLK        GPIO_NUM_48
#define TFT_PIN_MOSI       GPIO_NUM_12
#define TFT_PIN_CS         GPIO_NUM_14
#define TFT_PIN_DC         GPIO_NUM_47
#define TFT_PIN_RST        GPIO_NUM_3
#define TFT_PIN_BL         GPIO_NUM_39     /* 背光, LEDC PWM */
#define TFT_PIN_MISO       GPIO_NUM_NC     /* 只写 */
#define TFT_WIDTH          320
#define TFT_HEIGHT         240
#define TFT_SPI_CLK_HZ     80 * 1000 * 1000   /* 80MHz: 翻页/刷新流畅 (ST7789 支持) */

/* ==================== SD 卡 (SPI3, 独立总线) ==================== */
#define SD_SPI_HOST        SPI3_HOST
#define SD_PIN_CLK         GPIO_NUM_13
#define SD_PIN_CMD         GPIO_NUM_11     /* SPI MOSI */
#define SD_PIN_DATA        GPIO_NUM_9      /* SPI MISO */
#define SD_PIN_CD          GPIO_NUM_10     /* 卡检测 (低=有卡) */
/* SD CS: 原理图未引出 CS 网络, 运行时探测 (候选列表见 sd_card.c) */
#define SD_PIN_CS_PROBE_A  GPIO_NUM_45
#define SD_PIN_CS_PROBE_B  GPIO_NUM_10

/* ==================== 功放 MAX98357A (I2S 标准模式) ==================== */
#define SPK_PIN_BCLK       GPIO_NUM_41
#define SPK_PIN_LRCK       GPIO_NUM_42
#define SPK_PIN_DOUT       GPIO_NUM_40
/* SD_MODE# 经 100kΩ 接地 → 右声道 (RIGHT slot), 常开 */

/* ==================== 按键 (低电平有效, 外部上拉) ==================== */
#define BTN_PIN_BOOT       GPIO_NUM_0
#define BTN_PIN_MENU       GPIO_NUM_18
#define BTN_PIN_OPTION     GPIO_NUM_8
#define BTN_PIN_SELECT     GPIO_NUM_16
#define BTN_PIN_START      GPIO_NUM_17
#define BTN_PIN_A          GPIO_NUM_15
#define BTN_PIN_B          GPIO_NUM_5
#define BTN_PIN_LEFT       GPIO_NUM_19
#define BTN_PIN_RIGHT      GPIO_NUM_6
#define BTN_PIN_UP         GPIO_NUM_7
#define BTN_PIN_DOWN       GPIO_NUM_20

/* ==================== 状态灯 WS2812B ==================== */
#define LED_PIN_WS2812     GPIO_NUM_38

/* ==================== 电池电压采样 (ADC1_CH3) ==================== */
#define BAT_PIN_ADC        GPIO_NUM_4
/* 分压比 150k/(680k+150k)=0.1807, 可校准 */
#define BAT_DIVIDER_RATIO  0.1807f
#define BAT_FULL_V         4.20f
#define BAT_EMPTY_V        3.30f
