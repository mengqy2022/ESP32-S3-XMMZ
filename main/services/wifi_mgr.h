/*
 * wifi_mgr.h — WiFi 管理 (STA: 扫描/连接/自动重连/时间同步)
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

#define WIFI_SCAN_MAX_AP 12

typedef struct {
    char ssid[33];
    int8_t rssi;
    uint8_t authmode;      /* 0=开放 */
} wifi_ap_t;

esp_err_t wifi_mgr_init(void);                  /* 初始化 STA + 自动连接已保存配置 */
esp_err_t wifi_mgr_scan(wifi_ap_t *aps, int max, int *count);
esp_err_t wifi_mgr_connect(const char *ssid, const char *pass);  /* 保存+连接 */
esp_err_t wifi_mgr_disconnect(void);
void wifi_mgr_forget(void);
void wifi_mgr_stop(void);    /* 停止 WiFi 驱动, 释放内部 RAM (进 NES 前调) */
void wifi_mgr_start(void);   /* 重新启动 WiFi 驱动 + 自动重连 */                     /* 清除保存的配置 */

bool wifi_mgr_connected(void);
const char *wifi_mgr_ssid(void);
const char *wifi_mgr_ip(void);
bool wifi_mgr_time_synced(void);                /* NTP 时间是否已同步 */
void wifi_mgr_set_power_save(bool enable);      /* 省电(默认开); 流媒体播放时应关, 避免断流 */
