/*
 * ble.h — BLE GATT 服务 + 扫描 (功能栏)
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

esp_err_t ble_start(void);
bool ble_inited(void);             /* 是否已初始化 (默认关闭) */
bool ble_is_active(void);          /* 是否已连接 */

/* 广播开关 (开始/暂停) */
esp_err_t ble_set_adv(bool on);
bool ble_adv_on(void);

/* 扫描 */
#define BLE_SCAN_MAX_DEV 12
typedef struct {
    char name[20];
    uint8_t addr[6];
    int8_t rssi;
    bool has_name;
} ble_dev_t;

void ble_scan_start(uint32_t duration_ms);   /* 异步扫描, 结果累积在内部数组 */
void ble_scan_stop(void);
bool ble_scanning(void);
int ble_scan_count(void);
const ble_dev_t *ble_scan_get(int idx);
