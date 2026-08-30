#pragma once
/* nes_core.h — app_nes.c (C) 与 infoNES 内核 (C++) 的桥接接口
 * app_nes.c 只通过这里跟模拟器交互. */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 载入 ROM (指向 PSRAM 里的 .nes 原始数据含 16 字节头). 返回 0 成功. */
int nes_core_load_rom(const uint8_t *rom, uint32_t len);

/* 运行模拟: 阻塞直到用户长按 MENU 退出. 每帧通过回调处理渲染/手柄/音频. */
void nes_core_run(void);

/* ---- app_nes.c 提供的回调 (被模拟器每帧调用) ---- */
void nes_host_render(void);          /* 把当前帧 RGB565 缩放到 320x240 并渲染到屏幕 */
uint32_t nes_host_pad(int *pquit);   /* 读标准 NES pad1; MENU 长按满足退出条件时 *pquit=1 */

/* 读取当前手柄输入 (由 app 读按键后设置, 供 InfoNES_PadState 用) */
void nes_core_set_pad(uint32_t pad1);

/* 释放 */
void nes_core_release(void);

/* 获取当前 RGB565 256x240 帧 (blit 后由 app 缩放显示) */
uint16_t *nes_core_get_frame(void);

#ifdef __cplusplus
}
#endif
