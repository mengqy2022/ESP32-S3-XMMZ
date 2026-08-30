/*
 * kbd.c — 虚拟键盘 (中英文输入)
 * 拼音: 打字母 -> 底部候选行自动重建 -> 五向移动 -> KEY4 选字
 * 操作: 五向=移动, KEY4=按键, KEY3=退格, KEY1=完成, KEY5=下一种模式, BOOT=上一种模式
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "pinyin_data.h"
#include "kbd.h"

LV_FONT_DECLARE(ui_font_lvgl_10);


typedef enum { KBD_PINYIN = 0, KBD_ABC, KBD_abc, KBD_123, KBD_SYM, KBD_MODE_MAX } kbd_mode_t;
static const char *s_mode_names[KBD_MODE_MAX] = { "拼音", "ABC", "abc", "123", "符号" };

typedef enum { KT_CHAR, KT_SPACE, KT_BACK, KT_DONE } key_type_t;
typedef struct {
    const char *label;
    key_type_t type;
} kbd_key_t;

static const char *s_rows_alpha[3] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };
static const char *s_rows_digit[3] = { "1234567890", "-_@.,;:/?!", "()[]{}<>=+" };
static const char *s_rows_sym[3]   = { "~`|\\^%$#*&", "'\"<>{}[]()", "+=_-,.;:!?" };

static const uint8_t ROW_Y[4] = { 86, 118, 150, 182 };
#define KEY_W 28
#define KEY_H 26
#define KEY_X0 8
#define KEY_STEP 31

/* 状态 */
static char *s_buf = NULL;
static uint32_t s_maxlen = 0;
static uint32_t s_len = 0;
static kbd_mode_t s_mode = KBD_PINYIN;
static char s_pinyin[16] = {0};
static int s_py_len = 0;
static kbd_done_cb_t s_cb = NULL;
static void *s_ctx = NULL;
static bool s_done = false;

static lv_obj_t *s_scr = NULL;
static lv_obj_t *s_input_label = NULL;
static lv_obj_t *s_mode_label = NULL;
static lv_obj_t *s_keys_area = NULL;
static lv_obj_t *s_status = NULL;

static kbd_key_t s_keys[48];
static int s_key_count = 0;
static lv_obj_t *s_btns[48];      /* 按键按钮句柄 (2D 导航用) */
static int s_rows[48];            /* 按键所在行 */
static int s_cols[48];            /* 按键所在列 */

static void rebuild_keys(void);
static void create_key_button(int x, int y, int idx, int row, int col);

/* 2D 导航: 按行列移动焦点 */
static void nav_2d(int dr, int dc)
{
    lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
    int cur = f ? (int)(intptr_t)lv_obj_get_user_data(f) : -1;
    if (cur < 0 || cur >= s_key_count) return;
    for (int i = 0; i < s_key_count; i++) {
        if (s_rows[i] == s_rows[cur] + dr && s_cols[i] == s_cols[cur] + dc) {
            lv_group_focus_obj(s_btns[i]);
            return;
        }
    }
}

/* ---------- 拼音查表 (前缀匹配, 最多9个候选) ---------- */
static int pinyin_find(const char *py, char *out, int max)
{
    int n = 0;
    int lo = 0, hi = pinyin_table_count - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (strcmp(pinyin_table[mid].pinyin, py) < 0) lo = mid + 1;
        else hi = mid - 1;
    }
    for (int i = lo; i < pinyin_table_count && n < max; i++) {
        if (strncmp(pinyin_table[i].pinyin, py, s_py_len) != 0) break;
        const char *cs = pinyin_table[i].chars;
        for (int j = 0; cs[j] && n < max; j += 3) {
            out[n++] = cs[j];
            out[n++] = cs[j + 1];
            out[n++] = cs[j + 2];
        }
    }
    return n * 3;
}

/* ---------- 文本操作 ---------- */
static void append_bytes(const char *s, int n)
{
    if (s_len + n + 1 >= s_maxlen) return;
    memcpy(s_buf + s_len, s, n);
    s_len += n;
    s_buf[s_len] = 0;
}

static void backspace(void)
{
    if (s_py_len > 0) {
        s_pinyin[--s_py_len] = 0;
        rebuild_keys();
        return;
    }
    if (s_len == 0) return;
    uint32_t i = s_len - 1;
    while (i > 0 && (s_buf[i] & 0xC0) == 0x80) i--;
    s_len = i;
    s_buf[s_len] = 0;
}

static void refresh_input(void)
{
    char tmp[96];
    if (s_py_len > 0)
        snprintf(tmp, sizeof(tmp), "%.*s|%.*s", (int)s_len, s_buf, s_py_len, s_pinyin);
    else
        snprintf(tmp, sizeof(tmp), "%s", s_buf);
    lv_label_set_text(s_input_label, tmp);
}

/* ---------- 按键事件 ---------- */
static void key_click_cb(lv_event_t *e)
{
    lv_obj_t *b = lv_event_get_target(e);
    int idx = (int)(intptr_t)lv_obj_get_user_data(b);
    if (idx < 0 || idx >= s_key_count) return;
    kbd_key_t *k = &s_keys[idx];
    switch (k->type) {
    case KT_CHAR:
        append_bytes(k->label, (int)strlen(k->label));
        break;
    case KT_SPACE:
        append_bytes(" ", 1);
        break;
    case KT_BACK:
        backspace();
        break;
    case KT_DONE:
        s_done = true;
        break;
    }
    refresh_input();
    if (s_done) return;
    /* 拼音模式按字母后重建候选行 */
    if (k->type == KT_CHAR && s_mode == KBD_PINYIN &&
        k->label[0] >= 'a' && k->label[0] <= 'z') {
        rebuild_keys();
    }
}

/* ---------- 键盘重建 ---------- */
static void clear_keys(void)
{
    lv_group_t *g = lv_group_get_default();
    uint32_t n = lv_obj_get_child_count(s_keys_area);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *ch = lv_obj_get_child(s_keys_area, 0);
        if (g) lv_group_remove_obj(ch);
        lv_obj_delete(ch);
    }
}

static void rebuild_keys(void)
{
    clear_keys();
    s_key_count = 0;

    const char **rows;
    if (s_mode == KBD_123) rows = s_rows_digit;
    else if (s_mode == KBD_SYM) rows = s_rows_sym;
    else rows = s_rows_alpha;

    /* 前三行 */
    for (int r = 0; r < 3; r++) {
        const char *line = rows[r];
        int x = KEY_X0 + (r == 2 ? (10 - (int)strlen(line)) / 2 * KEY_STEP : 0);
        for (int c = 0; line[c]; c++) {
            static char charbufs[48][2];
            charbufs[s_key_count][0] = line[c];
            charbufs[s_key_count][1] = 0;
            s_keys[s_key_count] = (kbd_key_t){ charbufs[s_key_count], KT_CHAR };
            create_key_button(x, ROW_Y[r], s_key_count, r, c);
            x += KEY_STEP;
            s_key_count++;
        }
    }

    /* 动态行 */
    int x = KEY_X0;
    int dcol = 0;
    if (s_mode == KBD_PINYIN && s_py_len > 0) {
        char cands[32];
        int clen = pinyin_find(s_pinyin, cands, 8);
        if (clen > 0) {
            static char candbufs[8][4];
            for (int i = 0; i < clen; i += 3) {
                candbufs[s_key_count][0] = cands[i];
                candbufs[s_key_count][1] = cands[i + 1];
                candbufs[s_key_count][2] = cands[i + 2];
                candbufs[s_key_count][3] = 0;
                s_keys[s_key_count] = (kbd_key_t){ candbufs[s_key_count], KT_CHAR };
                create_key_button(x, ROW_Y[3], s_key_count, 3, dcol);
                x += KEY_STEP;
                s_key_count++;
                dcol++;
            }
        } else {
            s_keys[s_key_count] = (kbd_key_t){ "无匹配", KT_DONE };
            create_key_button(x, ROW_Y[3], s_key_count, 3, dcol);
            s_key_count++;
            dcol++;
        }
    } else {
        static const char *fn[3] = { "空格", "退格", "完成" };
        static const key_type_t ft[3] = { KT_SPACE, KT_BACK, KT_DONE };
        for (int i = 0; i < 3; i++) {
            s_keys[s_key_count] = (kbd_key_t){ fn[i], ft[i] };
            create_key_button(x, ROW_Y[3], s_key_count, 3, dcol);
            x += 66;
            s_key_count++;
            dcol++;
        }
    }

    lv_obj_t *first = lv_obj_get_child(s_keys_area, 0);
    if (first) lv_group_focus_obj(first);
    refresh_input();
}

static void create_key_button(int x, int y, int idx, int row, int col)
{
    lv_obj_t *b = lv_button_create(s_keys_area);
    int w = (s_keys[idx].label[0] >= 0x80) ? 30 : KEY_W;
    lv_obj_set_size(b, w, KEY_H);
    lv_obj_set_pos(b, x, y);
    ui_style_compact_button(b);
    lv_obj_set_user_data(b, (void *)(intptr_t)idx);
    lv_obj_add_event_cb(b, key_click_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(lv_group_get_default(), b);
    lv_obj_t *lb = lv_label_create(b);
    lv_label_set_text(lb, s_keys[idx].label);
    lv_obj_set_style_text_color(lb, UI_THEME_TEXT, 0);
    lv_obj_center(lb);
    s_btns[idx] = b;
    s_rows[idx] = row;
    s_cols[idx] = col;
}

/* ---------- 入口 ---------- */
void kbd_show(const char *title, const char *initial,
              char *buf, uint32_t maxlen, kbd_done_cb_t cb, void *ctx)
{
    s_buf = buf;
    s_maxlen = maxlen;
    s_cb = cb;
    s_ctx = ctx;
    s_done = false;
    s_mode = KBD_PINYIN;
    s_py_len = 0;
    s_pinyin[0] = 0;

    s_len = initial ? (uint32_t)strlen(initial) : 0;
    if (initial && s_len >= s_maxlen) s_len = s_maxlen - 1;
    memcpy(s_buf, initial ? initial : "", s_len);
    s_buf[s_len] = 0;

    s_scr = ui_screen_new_ex(title, false);

    /* 输入行 */
    s_input_label = lv_label_create(s_scr);
    lv_obj_set_style_text_color(s_input_label, UI_THEME_TEXT, 0);
    lv_obj_set_pos(s_input_label, 10, 40);
    lv_obj_set_width(s_input_label, 300);
    lv_label_set_long_mode(s_input_label, LV_LABEL_LONG_WRAP);

    /* 模式 + 状态 */
    s_mode_label = lv_label_create(s_scr);
    lv_obj_set_style_text_color(s_mode_label, UI_THEME_ACCENT, 0);
    lv_obj_set_pos(s_mode_label, 10, 64);

    s_status = lv_label_create(s_scr);
    lv_obj_set_style_text_color(s_status, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(s_status, &ui_font_lvgl_10, 0);
    lv_obj_align(s_status, LV_ALIGN_BOTTOM_MID, 0, -4);

    /* 键盘区 */
    s_keys_area = lv_obj_create(s_scr);
    lv_obj_set_pos(s_keys_area, 0, 80);
    lv_obj_set_size(s_keys_area, 320, 140);
    lv_obj_set_style_bg_opa(s_keys_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_keys_area, 0, 0);
    lv_obj_set_style_pad_all(s_keys_area, 0, 0);

    rebuild_keys();
    lv_label_set_text(s_mode_label, s_mode_names[s_mode]);
    lv_label_set_text(s_status, "KEY5切换模式  KEY3退格  KEY1完成");

    /* 加载键盘屏但【不删除】调用方屏幕 (退出时恢复, 避免调用方操作已删除对象) */
    lv_obj_t *prev_scr = lv_scr_act();
    lv_screen_load_anim(s_scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, false);

    for (;;) {
        lv_timer_handler();
        key_event_t evt;
        if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
            switch (evt.key) {
            case KEY_UP:
                nav_2d(-1, 0);
                break;
            case KEY_DOWN:
                nav_2d(1, 0);
                break;
            case KEY_LEFT:
                nav_2d(0, -1);
                break;
            case KEY_RIGHT:
                nav_2d(0, 1);
                break;
            case KEY_A:        /* KEY5: 下一模式 */
                s_mode = (kbd_mode_t)((s_mode + 1) % KBD_MODE_MAX);
                if (s_mode != KBD_PINYIN) s_py_len = 0, s_pinyin[0] = 0;
                lv_label_set_text(s_mode_label, s_mode_names[s_mode]);
                rebuild_keys();
                break;
            case KEY_B:     /* KEY6: 上一模式 */
                s_mode = (kbd_mode_t)((s_mode + KBD_MODE_MAX - 1) % KBD_MODE_MAX);
                if (s_mode != KBD_PINYIN) s_py_len = 0, s_pinyin[0] = 0;
                lv_label_set_text(s_mode_label, s_mode_names[s_mode]);
                rebuild_keys();
                break;
            case KEY_BACK:      /* KEY3: 退格 */
                backspace();
                refresh_input();
                break;
            case KEY_HOME:      /* KEY1: 完成 */
                s_done = true;
                break;
            default:
                break;
            }
        }
        if (s_done) break;
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    /* 退出: 恢复调用方屏幕, 删除键盘屏 (先移出分组) */
    ui_group_cleanup(s_scr);
    lv_obj_delete(s_scr);
    if (prev_scr) lv_screen_load(prev_scr);

    if (s_cb) s_cb(s_buf, s_ctx);
}
