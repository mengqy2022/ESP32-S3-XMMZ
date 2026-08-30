/*
 * lvgl_ui.h — LVGL 应用公共辅助 / 小孟蜜汁系统统一视觉主题
 *
 * 设计原则:
 * - Apple Dark 风格: 深色背景、分层卡片、高对比文字、克制蓝色强调色
 * - 只使用纯色/细边框/短动画，不使用大阴影与透明模糊，降低 ESP32-S3 渲染负担
 * - 统一 10px 页面边距与 8px 圆角，避免 320x240 小屏内容互相挤压
 */
#pragma once

#include "lvgl.h"

/* 统一主题色 (接近 iOS Dark 调色板) */
#define UI_THEME_BG          lv_color_hex(0x0B0B0F)
#define UI_THEME_BAR         lv_color_hex(0x111114)
#define UI_THEME_CARD        lv_color_hex(0x1C1C1E)
#define UI_THEME_CARD_FOC    lv_color_hex(0x2C2C2E)
#define UI_THEME_ACCENT      lv_color_hex(0x0A84FF)
#define UI_THEME_TEXT        lv_color_hex(0xF5F5F7)
#define UI_THEME_DIM         lv_color_hex(0x98989D)
#define UI_THEME_SEPARATOR   lv_color_hex(0x38383A)
#define UI_THEME_SUCCESS     lv_color_hex(0x30D158)
#define UI_THEME_WARNING     lv_color_hex(0xFF9F0A)
#define UI_THEME_DANGER      lv_color_hex(0xFF453A)

#define UI_PAGE_MARGIN       10
#define UI_TOPBAR_H          38
#define UI_FOOTER_H          20
#define UI_CARD_RADIUS       8

/* 中文字体 (lv_font_conv 生成, 内嵌) */
LV_FONT_DECLARE(ui_font_lvgl);

/* 创建应用根屏: 统一背景 + 顶部标题栏, 返回 screen (未加载) */
lv_obj_t *ui_screen_new(const char *title);

/* 同上, 但可控制底部 "KEY3 返回" 提示是否显示 (阅读器等全屏场景用 false) */
lv_obj_t *ui_screen_new_ex(const char *title, bool show_hint);

/* 应用统一的轻量卡片按钮样式。只使用纯色/细边框，避免阴影造成额外重绘。 */
void ui_style_list_button(lv_obj_t *btn);

/* 小尺寸键盘/紧凑按钮样式。 */
void ui_style_compact_button(lv_obj_t *btn);

/* 普通卡片/面板样式（不可聚焦）。 */
void ui_style_card(lv_obj_t *obj);

/* 加载新屏并自动删除旧屏 (LVGL9 的 lv_screen_load 不会删旧屏, 必须用这个避免泄漏) */
void ui_screen_show(lv_obj_t *scr);

/* 方向键导航 (队列驱动; LVGL9 keypad 不会自动用方向键移动焦点) */
void ui_nav_key(int key);

/* 递归把对象及其子对象移出默认分组 (删除屏幕前必须先调用) */
void ui_group_cleanup(lv_obj_t *obj);

/* 在当前屏上等待用户按键, 返回: 1=退出(KEY3/KEY1), 0=其他键(KEY5刷新等) */
/* 内部驱动 lv_timer_handler, 供应用自己跑事件循环 */
int ui_app_loop_wait(void);

/* 创建一行文字 (label 左对齐) */
lv_obj_t *ui_label(lv_obj_t *parent, const char *text, int x, int y);
