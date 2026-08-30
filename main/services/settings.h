/*
 * settings.h — 系统设置 (NVS)
 */
#pragma once

#include <stdint.h>

/* 屏保空闲超时 (秒), 0 = 关闭屏保 */
uint32_t settings_saver_timeout_sec(void);
void settings_set_saver_timeout_sec(uint32_t sec);
