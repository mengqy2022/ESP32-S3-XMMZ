/*
 * lvgl_ui.c — LVGL 应用公共辅助实现 (v2 主题)
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "version.h"

static void ui_disable_outline(lv_obj_t *obj)
{
    lv_obj_set_style_outline_width(obj, 0, 0);
    lv_obj_set_style_outline_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_width(obj, 0, LV_STATE_FOCUSED);
    lv_obj_set_style_outline_opa(obj, LV_OPA_TRANSP, LV_STATE_FOCUSED);
}

void ui_style_card(lv_obj_t *obj)
{
    if (!obj) return;
    lv_obj_set_style_bg_color(obj, UI_THEME_CARD, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, UI_THEME_SEPARATOR, 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_70, 0);
    lv_obj_set_style_radius(obj, UI_CARD_RADIUS, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

void ui_style_list_button(lv_obj_t *btn)
{
    if (!btn) return;
    ui_style_card(btn);
    /* 聚焦: 强调色描边 + 卡片轻微提亮, 作为干净利落的焦点环 */
    lv_obj_set_style_bg_color(btn, UI_THEME_CARD_FOC, LV_STATE_FOCUSED);
    lv_obj_set_style_border_color(btn, UI_THEME_ACCENT, LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btn, 2, LV_STATE_FOCUSED);
    ui_disable_outline(btn);
}

void ui_style_compact_button(lv_obj_t *btn)
{
    if (!btn) return;
    ui_style_list_button(btn);
    lv_obj_set_style_radius(btn, 6, 0);
}

lv_obj_t *ui_app_icon(lv_obj_t *parent, uint32_t color, const char *glyph, lv_coord_t size)
{
    lv_obj_t *tile = lv_obj_create(parent);
    lv_obj_set_size(tile, size, size);
    /* 颜色向黑压暗 30%, 让图标更有层次, 同时保证白字在浅色系上依然清晰 */
    lv_obj_set_style_bg_color(tile, lv_color_mix(lv_color_hex(color), lv_color_black(), 30), 0);
    lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(tile, size * 3 / 10, 0);
    lv_obj_set_style_border_width(tile, 0, 0);
    lv_obj_set_style_pad_all(tile, 0, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *g = lv_label_create(tile);
    lv_label_set_text(g, glyph);
    lv_obj_set_style_text_color(g, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(g, &ui_font_lvgl, 0);
    lv_obj_center(g);
    return tile;
}

lv_obj_t *ui_screen_new_ex(const char *title, bool show_hint)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, UI_THEME_BG, 0);
    lv_obj_set_style_text_font(scr, &ui_font_lvgl, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    /* 保持 screen 默认滚动能力，兼容电子书/NES 等动态长列表。 */

    /* 顶栏: 轻量纯色 + 底部分隔线，避免阴影/渐变产生额外重绘。 */
    lv_obj_t *bar = lv_obj_create(scr);
    lv_obj_set_size(bar, LV_HOR_RES, UI_TOPBAR_H);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, UI_THEME_BAR, 0);
    lv_obj_set_style_border_color(bar, UI_THEME_SEPARATOR, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *t = lv_label_create(bar);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, UI_THEME_TEXT, 0);
    lv_obj_align(t, LV_ALIGN_LEFT_MID, 12, 0);

    lv_obj_t *ver = lv_label_create(bar);
    lv_label_set_text(ver, SYS_UI_TAG_STR);
    lv_obj_set_style_text_color(ver, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(ver, &book_font_lvgl, 0);
    lv_obj_align(ver, LV_ALIGN_RIGHT_MID, -12, 0);

    /* 底部返回提示 (可选) */
    if (show_hint) {
        lv_obj_t *hint = lv_label_create(scr);
        lv_label_set_text(hint, "KEY3 返回");
        lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
        lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);
    }

    return scr;
}

lv_obj_t *ui_screen_new(const char *title)
{
    return ui_screen_new_ex(title, true);
}

void ui_group_cleanup(lv_obj_t *obj)
{
    if (!obj) return;
    lv_group_t *g = lv_group_get_default();
    if (g) lv_group_remove_obj(obj);
    uint32_t n = lv_obj_get_child_count(obj);
    for (uint32_t i = 0; i < n; i++) {
        ui_group_cleanup(lv_obj_get_child(obj, i));
    }
}

void ui_screen_show(lv_obj_t *scr)
{
    /* 先把当前屏所有对象移出分组, 再带淡入加载新屏并自动删除旧屏, 避免悬空指针 */
    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_FADE_ON, UI_TRANSITION_MS, 0, true);
}

void ui_nav_key(int key)
{
    lv_group_t *g = lv_group_get_default();
    if (!g) return;
    if (key == KEY_UP || key == KEY_LEFT) lv_group_focus_prev(g);
    else if (key == KEY_DOWN || key == KEY_RIGHT) lv_group_focus_next(g);
}

lv_obj_t *ui_label(lv_obj_t *parent, const char *text, int x, int y)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_color(l, UI_THEME_TEXT, 0);
    lv_obj_set_pos(l, x, y);
    return l;
}

int ui_app_loop_wait(void)
{
    key_event_t evt;
    while (1) {
        lv_timer_handler();
        /* 5ms 事件等待 + 1 tick 让出 CPU，兼顾响应速度和 WiFi/BLE 后台任务。 */
        if (buttons_wait_event(&evt, 5)) {
            if (evt.evt != KEY_EVT_PRESS) continue;
            if (evt.key == KEY_BACK || evt.key == KEY_HOME) return 1;
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}