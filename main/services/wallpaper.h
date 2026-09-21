/*
 * wallpaper.h — 壁纸模块 (屏保用)
 * 图片来源优先级: SD 卡 /sdcard/wallpaper 目录下的 jpg/jpeg/png/bmp > 内置程序生成
 */
#pragma once

#include <stdint.h>
#include "lvgl.h"

#define WP_W        320
#define WP_H        240
#define WP_SIZE     (WP_W * WP_H * 2)     /* RGB565 原始数据 */

#define WP_BUILTIN_COUNT  5
#define WP_SD_MAX         10   /* SD 卡最多 10 张 */

/* 初始化 (扫描 SD 卡 + 分配 PSRAM 缓冲) */
int wallpaper_init(void);

/* 壁纸总数 (SD > 网络 > 内置, 按可用来源) */
int wallpaper_count(void);

/* 加载第 idx 张: 返回 0 = 用 wallpaper_dsc() 的原始缓冲;
 *                返回 1 = 用 src_path 的 LVGL 图片路径 (SD jpg, LVGL 自行解码) */
int wallpaper_load(int idx, char *src_path, int path_len);

/* 当前壁纸的 LVGL 图像描述符 (内置/网络 RGB565 数据) */
lv_image_dsc_t *wallpaper_dsc(void);

/* 是否成功加载过 SD 壁纸 */
int wallpaper_sd_count(void);
