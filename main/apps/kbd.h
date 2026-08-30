/*
 * kbd.h — 虚拟键盘 (中英文输入: 拼音 / ABC / abc / 123 / 符号)
 * 操作: 五向=移动, KEY4=按键, KEY3=退格, KEY1=完成, KEY5=下一种模式, BOOT=上一种模式
 */
#pragma once

#include <stdint.h>

typedef void (*kbd_done_cb_t)(const char *text, void *ctx);

/* 阻塞运行: 打开键盘编辑 buf (最长 maxlen 字节), 完成或取消时调用 cb 并返回 */
void kbd_show(const char *title, const char *initial,
              char *buf, uint32_t maxlen, kbd_done_cb_t cb, void *ctx);
