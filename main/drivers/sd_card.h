/*
 * sd_card.h — SD 卡驱动 (sdspi, SPI3)
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "esp_vfs_fat.h"

/* 挂载点 */
#define SD_MOUNT_POINT "/sdcard"

esp_err_t sd_card_init(void);       /* 挂载 SD, 返回 ESP_OK 表示成功 */
esp_err_t sd_card_remount(void);    /* 运行中重新探测挂载 (插拔/按压卡后调用) */
bool sd_card_present(void);         /* 卡检测脚状态 (GPIO10) */
void sd_card_unmount(void);
