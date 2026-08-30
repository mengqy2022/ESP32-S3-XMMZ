/*
 * game_common.h — 游戏共享工具 (直接像素渲染: lv_image 显示 PSRAM 帧缓冲)
 */
#pragma once

#include <string.h>
#include "lvgl.h"
#include "esp_heap_caps.h"

#define FB_W  320
#define FB_H  240

/* 分配帧缓冲 (PSRAM) 并创建 lv_image, 返回 image 对象 */
static inline lv_obj_t *fb_create(lv_obj_t *parent, uint16_t **fb, lv_image_dsc_t *dsc)
{
    *fb = heap_caps_malloc(FB_W * FB_H * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!*fb) return NULL;
    memset(*fb, 0, FB_W * FB_H * sizeof(uint16_t));
    dsc->header.cf = LV_COLOR_FORMAT_RGB565;
    dsc->header.w = FB_W;
    dsc->header.h = FB_H;
    dsc->header.stride = FB_W * 2;
    dsc->data_size = FB_W * FB_H * 2;
    dsc->data = (const uint8_t *)*fb;
    lv_obj_t *img = lv_image_create(parent);
    lv_obj_set_size(img, FB_W, FB_H);
    lv_obj_set_pos(img, 0, 0);
    lv_image_set_src(img, dsc);
    return img;
}

static inline void fb_free(uint16_t **fb)
{
    if (*fb) { heap_caps_free(*fb); *fb = NULL; }
}

static inline uint16_t fb_rgb565(uint32_t rgb)
{
    return ((rgb >> 8 & 0xFF) >> 3 << 11) | ((rgb >> 0 & 0xFF) >> 2 << 5) | (rgb >> 16 & 0xFF) >> 3;
}

static inline void fb_px(uint16_t *fb, int x, int y, uint32_t rgb)
{
    if (x < 0 || x >= FB_W || y < 0 || y >= FB_H) return;
    fb[y * FB_W + x] = fb_rgb565(rgb);
}

static inline void fb_rect(uint16_t *fb, int x, int y, int w, int h, uint32_t rgb)
{
    uint16_t c = fb_rgb565(rgb);
    for (int yy = y; yy < y + h; yy++) {
        if (yy < 0 || yy >= FB_H) continue;
        for (int xx = x; xx < x + w; xx++) {
            if (xx < 0 || xx >= FB_W) continue;
            fb[yy * FB_W + xx] = c;
        }
    }
}

/* 画空心矩形边框 */
static inline void fb_frame(uint16_t *fb, int x, int y, int w, int h, int t, uint32_t rgb)
{
    fb_rect(fb, x, y, w, t, rgb);
    fb_rect(fb, x, y + h - t, w, t, rgb);
    fb_rect(fb, x, y, t, h, rgb);
    fb_rect(fb, x + w - t, y, t, h, rgb);
}

static inline void fb_show(lv_obj_t *img)
{
    lv_obj_invalidate(img);
}
