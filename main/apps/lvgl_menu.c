/*
 * lvgl_menu.c — 小孟蜜汁系统主页 (v2 星夜暗色)
 *
 * 布局:
 * - 顶部 44px 品牌/状态区 (品牌、时间、电量互不重叠)
 * - 中部 4 项/页的应用卡片列表 (彩色图标徽标 + 标题 + 箭头)
 * - 底部固定操作提示 + 页指示
 *
 * 性能策略: 纯色、细边框、无阴影; 翻页只做 180ms 缓动位移;
 * 屏切换用 150ms 淡入过渡, 不做大面积复杂重绘。
 */
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "lvgl.h"
#include "buttons.h"
#include "battery.h"
#include "lvgl_ui.h"
#include "audio_beep.h"
#include "apps.h"
#include "services/wifi_mgr.h"
#include "services/led_ctrl.h"
#include "services/screensaver.h"
#include "version.h"
#include "safe_string.h"

static const char *TAG = "menu";

typedef struct {
    const char *name;
    const char *glyph;   /* 图标徽标单字/字母 */
    uint32_t color;
    app_func_t run;
} menu_entry_t;

static const menu_entry_t s_items[] = {
    { "电子书",   "书", 0x0A84FF, app_ebook_run },
    { "图库",     "图", 0x30D158, app_gallery_run },
    { "NES",      "N",  0xFF453A, app_nes_run },
    { "游戏",     "游", 0x64D2FF, app_games_run },
    { "音乐",     "音", 0xFF375F, app_music_run },
    { "系统设置", "设", 0xFF9F0A, app_settings_run },
};
#define ITEM_COUNT (sizeof(s_items) / sizeof(s_items[0]))

#define PAGE_SIZE    4
#define PAGE_COUNT   ((ITEM_COUNT + PAGE_SIZE - 1) / PAGE_SIZE)
#define TOP_H        44
#define BOT_H        22
#define CONT_H       (240 - TOP_H - BOT_H)
#define ITEM_H       36
#define ITEM_DY      42
#define ITEM_Y_PAD   6
#define CONT_TOTAL   (PAGE_COUNT * CONT_H)
#define CONT_MAX     ((PAGE_COUNT - 1) * CONT_H)
#define SCREENSAVER_IDLE_S   30   /* 无按键多少秒后进入屏保 */

static lv_obj_t *s_container = NULL;
static lv_obj_t *s_dots[PAGE_COUNT];
static lv_obj_t *s_time_label = NULL;
static lv_obj_t *s_bat_label = NULL;
static lv_obj_t *s_sysname_label = NULL;
static lv_obj_t *s_toast = NULL;
static lv_obj_t *s_toast_label = NULL;
static uint32_t s_toast_until = 0;
static volatile int s_launch = -1;
static int s_cur_page = 0;
static int s_focus_idx = 0;

static void item_click_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    s_launch = (int)(intptr_t)lv_obj_get_user_data(btn);
}

static void show_toast(const char *text)
{
    if (s_toast_label) lv_label_set_text(s_toast_label, text);
    if (s_toast) lv_obj_clear_flag(s_toast, LV_OBJ_FLAG_HIDDEN);
    s_toast_until = (uint32_t)(esp_timer_get_time() / 1000000) + 2;
}

static void show_toast_inline(uint8_t vol)
{
    char buf[20];
    snprintf(buf, sizeof(buf), "音量 %u%%", (unsigned)vol);
    show_toast(buf);
}

static void show_toast_led(led_mode_t m)
{
    char buf[20];
    snprintf(buf, sizeof(buf), "灯光: %s", led_ctrl_mode_name(m));
    show_toast(buf);
}

static lv_obj_t *create_item(lv_obj_t *parent, int idx)
{
    const menu_entry_t *it = &s_items[idx];
    int page = idx / PAGE_SIZE;
    int row = idx % PAGE_SIZE;
    int y = page * CONT_H + ITEM_Y_PAD + row * ITEM_DY;

    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 300, ITEM_H);
    lv_obj_set_pos(btn, 10, y);
    ui_style_list_button(btn);
    lv_obj_add_event_cb(btn, item_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_user_data(btn, (void *)(intptr_t)idx);
    lv_group_add_obj(lv_group_get_default(), btn);

    /* 左侧应用图标徽标 */
    lv_obj_t *icon = ui_app_icon(btn, it->color, it->glyph, 26);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 12, 0);

    lv_obj_t *name = lv_label_create(btn);
    lv_label_set_text(name, it->name);
    lv_obj_set_style_text_color(name, UI_THEME_TEXT, 0);
    lv_obj_align(name, LV_ALIGN_LEFT_MID, 50, 0);

    lv_obj_t *arrow = lv_label_create(btn);
    lv_label_set_text(arrow, ">");
    lv_obj_set_style_text_color(arrow, UI_THEME_DIM, 0);
    lv_obj_set_style_text_color(arrow, UI_THEME_ACCENT, LV_STATE_FOCUSED);
    lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -13, 0);

    return btn;
}

static lv_obj_t *find_btn(int idx)
{
    if (!s_container) return NULL;
    uint32_t n = lv_obj_get_child_count(s_container);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *c = lv_obj_get_child(s_container, i);
        if ((intptr_t)lv_obj_get_user_data(c) == idx) return c;
    }
    return NULL;
}

static void scroll_exec_cb(void *var, int32_t v)
{
    lv_obj_scroll_to_y((lv_obj_t *)var, v, LV_ANIM_OFF);
}

static void scroll_complete_cb(lv_anim_t *a)
{
    (void)a;
    lv_obj_t *btn = find_btn(s_focus_idx);
    if (btn) lv_group_focus_obj(btn);
}

static void page_scroll(int page, bool animate)
{
    if (page < 0 || page >= PAGE_COUNT) return;
    s_cur_page = page;
    int target = page * CONT_H;
    if (target > CONT_MAX) target = CONT_MAX;

    for (int p = 0; p < PAGE_COUNT; p++) {
        if (!s_dots[p]) continue;
        lv_obj_set_size(s_dots[p], p == page ? 14 : 6, 6);
        lv_obj_set_style_bg_color(s_dots[p],
            p == page ? UI_THEME_ACCENT : UI_THEME_SEPARATOR, 0);
    }

    if (animate && s_container) {
        lv_anim_delete(s_container, scroll_exec_cb);  /* 防止连续按键叠加动画 */
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, s_container);
        lv_anim_set_exec_cb(&a, scroll_exec_cb);
        lv_anim_set_values(&a, lv_obj_get_scroll_y(s_container), target);
        lv_anim_set_time(&a, UI_SCROLL_MS);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_completed_cb(&a, scroll_complete_cb);
        lv_anim_start(&a);
    } else {
        if (s_container) lv_obj_scroll_to_y(s_container, target, LV_ANIM_OFF);
        lv_obj_t *btn = find_btn(s_focus_idx);
        if (btn) lv_group_focus_obj(btn);
    }
}

static void menu_build(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, UI_THEME_BG, 0);
    lv_obj_set_style_text_font(scr, &ui_font_lvgl, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* 顶部品牌状态区 */
    lv_obj_t *bar = lv_obj_create(scr);
    lv_obj_set_size(bar, LV_HOR_RES, TOP_H);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, UI_THEME_BAR, 0);
    lv_obj_set_style_border_color(bar, UI_THEME_SEPARATOR, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    s_sysname_label = lv_label_create(bar);
    lv_label_set_text(s_sysname_label, SYS_NAME_STR);
    lv_obj_set_style_text_color(s_sysname_label, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(s_sysname_label, &book_font_lvgl, 0);
    lv_obj_set_pos(s_sysname_label, 12, 4);

    lv_obj_t *sub = lv_label_create(bar);
    lv_label_set_text(sub, "应用中心");
    lv_obj_set_style_text_color(sub, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(sub, &ui_font_lvgl_10, 0);
    lv_obj_set_pos(sub, 13, 27);

    s_time_label = lv_label_create(bar);
    lv_label_set_text(s_time_label, "---- -- -- --:--");
    lv_obj_set_style_text_color(s_time_label, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(s_time_label, &ui_font_lvgl, 0);
    /* 日期 + 时间放在同一行右对齐：YYYY-MM-DD HH:MM。
     * 只扩大右侧标签宽度，不侵占左侧品牌区域。 */
    lv_obj_set_width(s_time_label, 160);
    lv_obj_set_style_text_align(s_time_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_time_label, 148, 4);

    s_bat_label = lv_label_create(bar);
    lv_label_set_text(s_bat_label, "电量 --%");
    lv_obj_set_style_text_color(s_bat_label, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(s_bat_label, &ui_font_lvgl_10, 0);
    lv_obj_set_width(s_bat_label, 92);
    lv_obj_set_style_text_align(s_bat_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(s_bat_label, 216, 26);

    /* 状态胶囊 (音量/灯光等瞬时提示), 最后创建保证盖在品牌层之上 */
    s_toast = lv_obj_create(bar);
    lv_obj_set_size(s_toast, 128, 24);
    lv_obj_set_pos(s_toast, 10, 10);
    lv_obj_set_style_bg_color(s_toast, UI_THEME_ACCENT, 0);
    lv_obj_set_style_bg_opa(s_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_toast, 0, 0);
    lv_obj_set_style_radius(s_toast, 12, 0);
    lv_obj_set_style_pad_all(s_toast, 0, 0);
    lv_obj_clear_flag(s_toast, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(s_toast, LV_OBJ_FLAG_HIDDEN);
    s_toast_label = lv_label_create(s_toast);
    lv_obj_set_style_text_color(s_toast_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_toast_label, &ui_font_lvgl_10, 0);
    lv_obj_center(s_toast_label);

    /* 中部内容区 */
    s_container = lv_obj_create(scr);
    lv_obj_set_size(s_container, LV_HOR_RES, CONT_H);
    lv_obj_set_pos(s_container, 0, TOP_H);
    lv_obj_set_style_bg_color(s_container, UI_THEME_BG, 0);
    lv_obj_set_style_border_width(s_container, 0, 0);
    lv_obj_set_style_radius(s_container, 0, 0);
    lv_obj_set_style_pad_all(s_container, 0, 0);
    lv_obj_set_scroll_dir(s_container, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(s_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_snap_x(s_container, LV_SCROLL_SNAP_NONE);
    lv_obj_set_scroll_snap_y(s_container, LV_SCROLL_SNAP_NONE);

    for (int i = 0; i < (int)ITEM_COUNT; i++) create_item(s_container, i);

    /* 尾部锚点把滚动内容撑到完整页高，保证第二页能精确对齐。 */
    lv_obj_t *spacer = lv_obj_create(s_container);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_pos(spacer, 0, CONT_TOTAL - 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);
    lv_obj_set_user_data(spacer, (void *)(intptr_t)-1);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_SCROLLABLE);

    /* 底部固定提示 */
    lv_obj_t *footer = lv_obj_create(scr);
    lv_obj_set_size(footer, LV_HOR_RES, BOT_H);
    lv_obj_set_pos(footer, 0, 240 - BOT_H);
    lv_obj_set_style_bg_color(footer, UI_THEME_BAR, 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_style_radius(footer, 0, 0);
    lv_obj_set_style_pad_all(footer, 0, 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *hint = lv_label_create(footer);
    lv_label_set_text(hint, "五向浏览  KEY4打开");
    lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_align(hint, LV_ALIGN_LEFT_MID, 12, 0);

    for (int p = 0; p < PAGE_COUNT; p++) {
        lv_obj_t *dot = lv_obj_create(footer);
        lv_obj_set_size(dot, p == s_cur_page ? 14 : 6, 6);
        lv_obj_set_style_bg_color(dot,
            p == s_cur_page ? UI_THEME_ACCENT : UI_THEME_SEPARATOR, 0);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_set_pos(dot, 276 + p * 20, 8);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
        s_dots[p] = dot;
    }

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    /* 主菜单每次从应用返回都会重建；淡入加载并自动删除上一屏，
     * 否则每进出一次应用都会遗留一棵 LVGL 对象树。 */
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_FADE_ON, UI_TRANSITION_MS, 0, true);

    int target = s_cur_page * CONT_H;
    if (target > CONT_MAX) target = CONT_MAX;
    lv_obj_scroll_to_y(s_container, target, LV_ANIM_OFF);
    lv_obj_t *btn = find_btn(s_focus_idx);
    if (btn) lv_group_focus_obj(btn);
}

void lvgl_menu_run(void)
{
    ESP_LOGI(TAG, "LVGL menu start");
    menu_build();

    uint32_t last_b = 0, last_h = 0;
    time_t last_minute = (time_t)-1;
    bool unsynced_shown = false;
    uint32_t last_active = (uint32_t)(esp_timer_get_time() / 1000000);

    for (;;) {
        lv_timer_handler();
        uint32_t hnow = (uint32_t)(esp_timer_get_time() / 1000000);

        /* 低内存只记录告警，不在 UI 主循环里做重型恢复动作，避免二次卡顿。 */
        if (hnow - last_h >= 5) {
            last_h = hnow;
            size_t freeh = esp_get_free_heap_size();
            if (freeh < 256 * 1024) {
                ESP_LOGW(TAG, "LOW HEAP: %lu bytes free", (unsigned long)freeh);
            }
        }

        if (wifi_mgr_time_synced()) {
            unsynced_shown = false;
            time_t now_s = time(NULL);
            time_t minute = now_s / 60;
            /* 日期/时间只在分钟变化时刷新，避免每秒无意义地触发 LVGL 重绘。 */
            if (minute != last_minute) {
                last_minute = minute;
                struct tm ti;
                localtime_r(&now_s, &ti);
                char buf[24];
                /* strftime keeps the fixed YYYY-MM-DD HH:MM representation while avoiding
                 * GCC 14's -Wformat-truncation false positive on theoretically unbounded tm ints. */
                if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &ti) == 0) {
                    xm_strlcpy(buf, "---- -- -- --:--", sizeof(buf));
                }
                if (s_time_label) lv_label_set_text(s_time_label, buf);
            }
        } else if (!unsynced_shown) {
            unsynced_shown = true;
            last_minute = (time_t)-1;
            if (s_time_label) lv_label_set_text(s_time_label, "时间未同步");
        }

        if (hnow - last_b >= 5) {
            last_b = hnow;
            char buf[20];
            snprintf(buf, sizeof(buf), "电量 %d%%", battery_percent());
            if (s_bat_label) lv_label_set_text(s_bat_label, buf);
        }

        if (s_toast_until && hnow >= s_toast_until) {
            s_toast_until = 0;
            if (s_toast) lv_obj_add_flag(s_toast, LV_OBJ_FLAG_HIDDEN);
        }

        key_event_t evt;
        while (buttons_wait_event(&evt, 0)) {
            if (evt.evt != KEY_EVT_PRESS) continue;
            last_active = hnow;
            switch (evt.key) {
            case KEY_UP:
            case KEY_DOWN: {
                lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                int idx = f ? (int)(intptr_t)lv_obj_get_user_data(f) : -1;
                int start = s_cur_page * PAGE_SIZE;
                int count = ITEM_COUNT - start;
                if (count > PAGE_SIZE) count = PAGE_SIZE;
                int first_idx = start;
                int last_idx = start + count - 1;
                if (evt.key == KEY_UP && idx == first_idx && s_cur_page > 0) {
                    s_focus_idx = (s_cur_page - 1) * PAGE_SIZE + PAGE_SIZE - 1;
                    if (s_focus_idx >= ITEM_COUNT) s_focus_idx = ITEM_COUNT - 1;
                    page_scroll(s_cur_page - 1, true);
                } else if (evt.key == KEY_DOWN && idx == last_idx && s_cur_page < PAGE_COUNT - 1) {
                    s_focus_idx = (s_cur_page + 1) * PAGE_SIZE;
                    page_scroll(s_cur_page + 1, true);
                } else {
                    ui_nav_key(evt.key);
                    lv_obj_t *nf = lv_group_get_focused(lv_group_get_default());
                    if (nf) {
                        intptr_t u = (intptr_t)lv_obj_get_user_data(nf);
                        if (u >= 0 && u < ITEM_COUNT) s_focus_idx = (int)u;
                    }
                }
                break;
            }
            case KEY_LEFT:
            case KEY_RIGHT:
                ui_nav_key(evt.key);
                break;
            case KEY_A: {
                uint8_t v = audio_get_volume();
                audio_set_volume(v >= 100 ? 100 : v + 5);
                show_toast_inline(audio_get_volume());
                break;
            }
            case KEY_B: {
                uint8_t v = audio_get_volume();
                audio_set_volume(v <= 5 ? 0 : v - 5);
                show_toast_inline(audio_get_volume());
                break;
            }
            case KEY_BOOT: {
                led_mode_t m = (led_mode_t)((led_ctrl_get_mode() + 1) % LED_MODE_MAX);
                led_ctrl_set(led_ctrl_get_color(), led_ctrl_get_bright(), m);
                show_toast_led(m);
                break;
            }
            case KEY_CONFIRM: {
                lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                if (f) {
                    intptr_t u = (intptr_t)lv_obj_get_user_data(f);
                    if (u >= 0 && u < ITEM_COUNT) {
                        s_launch = (int)u;
                        s_focus_idx = (int)u;
                    }
                }
                break;
            }
            case KEY_HOME:
            case KEY_BACK:
            default:
                break;
            }
        }

        if (s_launch >= 0 && s_launch < (int)ITEM_COUNT) {
            int idx = s_launch;
            s_launch = -1;
            ESP_LOGI(TAG, "launch app: %s", s_items[idx].name);
            s_items[idx].run();
            menu_build();
        }

        /* 屏保: 无按键 30 秒后进入壁纸轮播, 任意键唤醒后重建主页 */
        if (hnow - last_active >= SCREENSAVER_IDLE_S) {
            screensaver_run();
            last_active = (uint32_t)(esp_timer_get_time() / 1000000);
            menu_build();
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}