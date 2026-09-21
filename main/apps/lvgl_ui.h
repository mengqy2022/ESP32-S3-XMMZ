/*
 * lvgl_ui.h — LVGL 应用公共辅助 / 小孟蜜汁系统统一视觉主题 (v2 "星夜暗色")
 *
 * 设计原则:
 * - 深空近黑背景 + 分层卡片 + 高对比白字 + 克制的青蓝强调色, 视觉更高级
 * - 只用纯色 / 细描边 / 超短过渡动画, 不用大阴影、透明模糊与全屏渐变,
 *   保住 ESP32-S3 + ST7789 的渲染帧率, 让整机更流畅
 * - 统一 10px 页面边距、10px 卡片圆角、胶囊圆角; 焦点用强调色描边 + 轻微提亮
 *
 * 字体约定 (仅使用已内嵌的 3 套子集/全量字库):
 * - ui_font_lvgl    14px  界面字库(子集, 见 tools/ui_chars.txt)
 * - ui_font_lvgl_10 10px  小标签/提示
 * - book_font_lvgl  16px  全量 GB2312(标题/正文/品牌名)
 */
#pragma once

#include "lvgl.h"

/* ============================ 调色板 (Dark) ============================ */
#define UI_THEME_BG          lv_color_hex(0x0A0A0E)   /* 全局背景 */
#define UI_THEME_BAR         lv_color_hex(0x121218)   /* 顶栏/底栏 */
#define UI_THEME_CARD        lv_color_hex(0x1B1B23)   /* 卡片/按钮 */
#define UI_THEME_CARD_FOC    lv_color_hex(0x262633)   /* 聚焦卡片 */
#define UI_THEME_ACCENT      lv_color_hex(0x3DA5FF)   /* 系统强调色 */
#define UI_THEME_ACCENT_DIM  lv_color_hex(0x17406B)   /* 强调色深底(状态胶囊) */
#define UI_THEME_TEXT        lv_color_hex(0xF7F7FA)   /* 主文字 */
#define UI_THEME_DIM         lv_color_hex(0x8E8E9A)   /* 次要文字 */
#define UI_THEME_SEPARATOR   lv_color_hex(0x2A2A34)   /* 分隔线 */
#define UI_THEME_SUCCESS     lv_color_hex(0x34D05A)
#define UI_THEME_WARNING     lv_color_hex(0xFFB020)
#define UI_THEME_DANGER      lv_color_hex(0xFF4D42)

/* ============================ 尺寸 / 动效 ============================ */
#define UI_PAGE_MARGIN       10
#define UI_TOPBAR_H          38
#define UI_FOOTER_H          20
#define UI_CARD_RADIUS       10
#define UI_TRANSITION_MS     150    /* 屏幕切换淡入时长 */
#define UI_SCROLL_MS         180    /* 主页翻页动画时长 */

/* ============================ 字体声明 ============================ */
LV_FONT_DECLARE(ui_font_lvgl);
LV_FONT_DECLARE(ui_font_lvgl_10);
LV_FONT_DECLARE(book_font_lvgl);

/* ============================ 公共接口 ============================ */

/* 创建应用根屏: 统一背景 + 顶部标题栏, 返回 screen (未加载) */
lv_obj_t *ui_screen_new(const char *title);

/* 同上, 但可控制底部 "KEY3 返回" 提示是否显示 (阅读器等全屏场景用 false) */
lv_obj_t *ui_screen_new_ex(const char *title, bool show_hint);

/* 应用统一的卡片按钮样式。只使用纯色/细边框, 避免阴影造成额外重绘。 */
void ui_style_list_button(lv_obj_t *btn);

/* 小尺寸键盘/紧凑按钮样式。 */
void ui_style_compact_button(lv_obj_t *btn);

/* 普通卡片/面板样式 (不可聚焦)。 */
void ui_style_card(lv_obj_t *obj);

/* 应用/菜单图标徽标: 圆角方块(颜色自动压暗一点) + 居中单字/字母, 返回 tile 对象。 */
lv_obj_t *ui_app_icon(lv_obj_t *parent, uint32_t color, const char *glyph, lv_coord_t size);

/* 加载新屏并自动删除旧屏 (带淡入过渡)。LVGL9 的 lv_screen_load 不删旧屏, 必须用这个避免泄漏 */
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