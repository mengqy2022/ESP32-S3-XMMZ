/*
 * font.h — 内嵌字库访问 (GB2312 16x16 + ASCII 8x16)
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"

/* 返回内嵌字库指针: ascii(96*16B), gb2312(94*94*32B) */
esp_err_t font_load(const uint8_t **ascii_out, const uint8_t **gb_out);

/* Unicode 码点 -> 16x16 字模索引 (二分查找), 未收录返回 -1 */
int font_cp_index(uint32_t cp);

/* GB2312 双字节 -> 94x94 网格索引, 未定义返回 -1 (兼容旧接口) */
int font_gb2312_index(unsigned char hi, unsigned char lo);
