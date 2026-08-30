/*
 * app_music.c — 音乐播放
 * 列表页: 曲目选择 (SD 卡 /sdcard/music 目录的 MP3/WAV 优先, 无 SD 时用网络列表备用)
 * 播放页: 曲名+状态+音量 (KEY4 播放/暂停, 左/右=上一首/下一首, 播完自动下一首, KEY5=音量+, KEY6=音量-, KEY3=停止返回)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "audio_beep.h"
#include "services/music_player.h"
#include "apps.h"
#include "music_list.h"
#include "safe_string.h"

LV_FONT_DECLARE(ui_font_lvgl_10);
LV_FONT_DECLARE(book_font_lvgl);

static const char *TAG = "music";

/* 320x240 屏幕一次只绘制 5 行，但曲库本身不再限制为 5/24 首。
 * 采用虚拟化窗口避免歌曲很多时创建大量 LVGL 对象导致卡顿或内存压力。 */
#define MUSIC_VISIBLE_ROWS 5

typedef struct {
    char name[80];        /* 显示名 */
    char path[160];       /* 文件路径或 URL */
    int type;             /* 0=SD文件 1=网络 */
} track_t;

static track_t *s_tracks = NULL;
static int s_track_count = 0;
static int s_unsupported_mp4 = 0;
static bool s_catalog_oom = false;

static lv_obj_t *s_status_label = NULL;
static lv_obj_t *s_vol_label = NULL;

static void free_tracks(void)
{
    if (s_tracks) {
        heap_caps_free(s_tracks);
        s_tracks = NULL;
    }
    s_track_count = 0;
}

static track_t *alloc_tracks(int count)
{
    if (count <= 0 || (size_t)count > SIZE_MAX / sizeof(track_t)) return NULL;
    size_t bytes = (size_t)count * sizeof(track_t);

    /* 曲库元数据优先放 PSRAM，避免挤占 LVGL / Wi-Fi / 解码任务的内部 SRAM。 */
    track_t *p = (track_t *)heap_caps_calloc((size_t)count, sizeof(track_t),
                                              MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) {
        ESP_LOGW(TAG, "track catalog PSRAM alloc failed (%u bytes), fallback internal", (unsigned)bytes);
        p = (track_t *)heap_caps_calloc((size_t)count, sizeof(track_t),
                                        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    return p;
}

static bool is_playable_name(const char *name)
{
    const char *dot = name ? strrchr(name, '.') : NULL;
    if (!dot) return false;
    return strcasecmp(dot, ".mp3") == 0 ||
           strcasecmp(dot, ".wav") == 0 ||
           strcasecmp(dot, ".wave") == 0;
}

static bool is_unsupported_aac_container(const char *name)
{
    const char *dot = name ? strrchr(name, '.') : NULL;
    if (!dot) return false;
    return strcasecmp(dot, ".mp4") == 0 || strcasecmp(dot, ".m4a") == 0;
}

static int track_name_cmp(const void *a, const void *b)
{
    const track_t *ta = (const track_t *)a;
    const track_t *tb = (const track_t *)b;
    return strcasecmp(ta->name, tb->name);
}

/* 扫描 SD 卡 /sdcard/music 下所有可直接播放的 MP3/WAV。
 * 先计数再一次性分配曲库数组，避免固定上限，也避免循环 realloc 造成碎片。
 * MP4/M4A 一般是 AAC 容器，本工程没有 AAC 解码器，只统计后在 UI 给出转换提示。 */
static int scan_sd_music(void)
{
    s_unsupported_mp4 = 0;
    DIR *d = opendir("/sdcard/music");
    if (!d) return 0;

    struct dirent *e;
    int count = 0;
    while ((e = readdir(d))) {
        if (is_playable_name(e->d_name)) {
            if (count < INT_MAX) count++;
        } else if (is_unsupported_aac_container(e->d_name)) {
            s_unsupported_mp4++;
        }
    }
    closedir(d);

    if (count <= 0) {
        ESP_LOGI(TAG, "SD music: 0 playable, %d MP4/M4A unsupported", s_unsupported_mp4);
        return 0;
    }

    s_tracks = alloc_tracks(count);
    if (!s_tracks) {
        s_catalog_oom = true;
        ESP_LOGE(TAG, "track catalog alloc failed: %d entries", count);
        return 0;
    }

    d = opendir("/sdcard/music");
    if (!d) {
        free_tracks();
        return 0;
    }

    int n = 0;
    while ((e = readdir(d)) && n < count) {
        if (!is_playable_name(e->d_name)) continue;
        if (strlen(e->d_name) >= sizeof(s_tracks[n].name)) {
            ESP_LOGW(TAG, "skip too-long filename: %s", e->d_name);
            continue;
        }
        track_t *t = &s_tracks[n];
        xm_strlcpy(t->name, e->d_name, sizeof(t->name));
        int wr = snprintf(t->path, sizeof(t->path), "/sdcard/music/%s", e->d_name);
        if (wr < 0 || wr >= (int)sizeof(t->path)) continue;
        t->type = 0;
        n++;
    }
    closedir(d);

    if (n == 0) {
        free_tracks();
        return 0;
    }

    /* qsort 避免旧版 O(n^2) 冒泡排序在大曲库下拖慢进入音乐页面。 */
    qsort(s_tracks, (size_t)n, sizeof(track_t), track_name_cmp);
    ESP_LOGI(TAG, "SD music: %d playable, %d MP4/M4A unsupported", n, s_unsupported_mp4);
    return n;
}

/* 构建播放列表: SD 优先, 网络备用 */
static int build_tracks(void)
{
    free_tracks();
    s_catalog_oom = false;

    int n = scan_sd_music();
    /* 如果 SD 里确实有 MP4/M4A，就不要悄悄切到网络列表；直接告诉用户格式不受支持。 */
    if (n == 0 && s_unsupported_mp4 == 0 && !s_catalog_oom) {
        int net_count = (int)MUSIC_TRACK_COUNT;
        if (net_count > 0) {
            s_tracks = alloc_tracks(net_count);
            if (!s_tracks) {
                s_catalog_oom = true;
                return 0;
            }
            for (int i = 0; i < net_count; i++) {
                xm_strlcpy(s_tracks[i].name, s_music_tracks[i].name, sizeof(s_tracks[i].name));
                xm_strlcpy(s_tracks[i].path, s_music_tracks[i].url, sizeof(s_tracks[i].path));
                s_tracks[i].type = 1;
            }
            n = net_count;
        }
    }
    s_track_count = n;
    return n;
}

static const char *state_str(mp_state_t st)
{
    switch (st) {
    case MP_LOADING: return "打开中...";
    case MP_PLAYING: return "播放中";
    case MP_PAUSED:  return "已暂停";
    case MP_STOPPED: return "已停止";
    case MP_ERROR:   return "出错了";
    default:         return "空闲";
    }
}

static esp_err_t start_track(int idx)
{
    if (!s_tracks || idx < 0 || idx >= s_track_count) return ESP_ERR_INVALID_ARG;
    if (s_tracks[idx].type == 0) return mp_play_file(s_tracks[idx].path);
    return mp_play_url(s_tracks[idx].path);
}

static void update_player_meta(lv_obj_t *title, lv_obj_t *source, lv_obj_t *position, int idx)
{
    if (!s_tracks || idx < 0 || idx >= s_track_count) return;

    lv_label_set_text(title, s_tracks[idx].name);

    const char *ext = strrchr(s_tracks[idx].path, '.');
    lv_label_set_text_fmt(source, "%s  %s",
                          s_tracks[idx].type == 0 ? "SD" : "NET",
                          ext ? ext + 1 : "audio");
    lv_label_set_text_fmt(position, "%d / %d", idx + 1, s_track_count);
}

/* ---------- 播放页 ---------- */
static void player_page(int idx)
{
    int current_idx = idx;
    lv_obj_t *scr = ui_screen_new_ex("音乐播放", false);

    /* 曲名 */
    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_color(title, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(title, &book_font_lvgl, 0);
    lv_obj_set_width(title, 250);
    lv_obj_set_pos(title, 10, 55);
    lv_label_set_long_mode(title, LV_LABEL_LONG_SCROLL_CIRCULAR);

    /* 当前曲目序号 */
    lv_obj_t *position = lv_label_create(scr);
    lv_obj_set_style_text_color(position, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(position, &ui_font_lvgl_10, 0);
    lv_obj_align(position, LV_ALIGN_TOP_RIGHT, -10, 58);

    /* 来源 (小字) */
    lv_obj_t *source = lv_label_create(scr);
    lv_obj_set_style_text_color(source, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(source, &ui_font_lvgl_10, 0);
    lv_obj_set_width(source, 300);
    lv_obj_set_pos(source, 10, 82);
    lv_label_set_long_mode(source, LV_LABEL_LONG_SCROLL_CIRCULAR);

    /* 状态 */
    s_status_label = lv_label_create(scr);
    lv_obj_set_style_text_color(s_status_label, UI_THEME_ACCENT, 0);
    lv_obj_set_pos(s_status_label, 10, 112);

    /* 音量 */
    s_vol_label = lv_label_create(scr);
    lv_obj_set_style_text_color(s_vol_label, UI_THEME_TEXT, 0);
    lv_obj_set_pos(s_vol_label, 10, 142);

    /* 操作提示分两行，避免 320px 宽屏幕文字重叠。 */
    lv_obj_t *hint1 = lv_label_create(scr);
    lv_label_set_text(hint1, "← 上一首    下一首 →");
    lv_obj_set_style_text_color(hint1, UI_THEME_ACCENT, 0);
    lv_obj_set_style_text_font(hint1, &book_font_lvgl, 0);
    lv_obj_align(hint1, LV_ALIGN_BOTTOM_MID, 0, -25);

    lv_obj_t *hint2 = lv_label_create(scr);
    lv_label_set_text(hint2, "KEY4 播放/暂停  KEY5+/KEY6-  KEY3 返回");
    lv_obj_set_style_text_color(hint2, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(hint2, &ui_font_lvgl_10, 0);
    lv_obj_align(hint2, LV_ALIGN_BOTTOM_MID, 0, -5);

    update_player_meta(title, source, position, current_idx);
    ui_screen_show(scr);
    (void)start_track(current_idx);

    uint32_t last_poll = 0;
    uint32_t seen_completed = mp_completed_count();
    int shown_state = -1;
    int shown_volume = -1;
    for (;;) {
        lv_timer_handler();
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (now - last_poll > 100) {
            last_poll = now;

            /* Natural EOF -> next track (wrap at the end). The completion counter is only
             * incremented by the decoder task on successful EOF, never by manual stop/switch. */
            uint32_t completed = mp_completed_count();
            if (completed != seen_completed && s_track_count > 0) {
                seen_completed = completed;
                current_idx = (current_idx + 1 >= s_track_count) ? 0 : (current_idx + 1);
                update_player_meta(title, source, position, current_idx);
                ESP_LOGI(TAG, "auto next: %d/%d %s", current_idx + 1, s_track_count,
                         s_tracks[current_idx].name);
                (void)start_track(current_idx);
                /* start_track can synchronously stop an old task in edge cases; absorb any
                 * completion count change so a single EOF can never skip two tracks. */
                seen_completed = mp_completed_count();
                shown_state = -1;
            }

            /* Only touch labels when values actually change. This avoids invalidating LVGL
             * text objects several times per second during long playback. */
            mp_state_t st = mp_state();
            if (s_status_label && shown_state != (int)st) {
                shown_state = (int)st;
                if (st == MP_ERROR) {
                    lv_label_set_text_fmt(s_status_label, "出错: %s", mp_error_str());
                    lv_obj_set_style_text_color(s_status_label, UI_THEME_DIM, 0);
                } else {
                    lv_label_set_text(s_status_label, state_str(st));
                    lv_obj_set_style_text_color(s_status_label, UI_THEME_ACCENT, 0);
                }
            }
            int volume = audio_get_volume();
            if (s_vol_label && shown_volume != volume) {
                shown_volume = volume;
                lv_label_set_text_fmt(s_vol_label, "音量 %d%%", volume);
            }
        }

        key_event_t evt;
        if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
            switch (evt.key) {
            case KEY_CONFIRM:
                if (mp_state() == MP_PAUSED) mp_resume();
                else mp_pause();
                break;

            case KEY_LEFT:
                if (s_track_count > 0) {
                    current_idx = (current_idx <= 0) ? (s_track_count - 1) : (current_idx - 1);
                    update_player_meta(title, source, position, current_idx);
                    (void)start_track(current_idx);
                    seen_completed = mp_completed_count();
                    shown_state = -1;
                    ESP_LOGI(TAG, "previous track: %d/%d %s", current_idx + 1, s_track_count,
                             s_tracks[current_idx].name);
                }
                break;

            case KEY_RIGHT:
                if (s_track_count > 0) {
                    current_idx = (current_idx + 1 >= s_track_count) ? 0 : (current_idx + 1);
                    update_player_meta(title, source, position, current_idx);
                    (void)start_track(current_idx);
                    seen_completed = mp_completed_count();
                    shown_state = -1;
                    ESP_LOGI(TAG, "next track: %d/%d %s", current_idx + 1, s_track_count,
                             s_tracks[current_idx].name);
                }
                break;

            case KEY_A: {   /* 音量+ */
                uint8_t v = audio_get_volume();
                audio_set_volume(v >= 100 ? 100 : v + 5);
                ESP_LOGI(TAG, "music volume %u%% (+)", (unsigned)audio_get_volume());
                break;
            }

            case KEY_B: {  /* 物理 KEY6: 音量- */
                uint8_t v = audio_get_volume();
                audio_set_volume(v <= 5 ? 0 : v - 5);
                ESP_LOGI(TAG, "music volume %u%% (-)", (unsigned)audio_get_volume());
                break;
            }

            case KEY_BACK:
            case KEY_HOME:
                mp_stop();
                return;

            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static void render_track_window(lv_obj_t **rows,
                                lv_obj_t **name_labels,
                                lv_obj_t **src_labels,
                                int row_count,
                                int top_idx,
                                lv_obj_t *footer)
{
    for (int row = 0; row < row_count; row++) {
        int idx = top_idx + row;
        if (idx < 0 || idx >= s_track_count) continue;

        lv_obj_set_user_data(rows[row], (void *)(intptr_t)idx);
        lv_label_set_text(name_labels[row], s_tracks[idx].name);
        lv_label_set_text(src_labels[row], s_tracks[idx].type == 0 ? "SD" : "NET");
        lv_obj_set_style_text_color(src_labels[row],
                                    s_tracks[idx].type == 0 ? UI_THEME_SUCCESS : UI_THEME_DIM, 0);
    }

    if (footer && s_track_count > 0) {
        int last = top_idx + row_count;
        if (last > s_track_count) last = s_track_count;
        lv_label_set_text_fmt(footer, "%d-%d / %d   KEY4 播放   KEY3 返回",
                              top_idx + 1, last, s_track_count);
    }
}

/* ---------- 列表页 ---------- */
static void list_page(void)
{
    lv_obj_t *scr = ui_screen_new_ex("音乐", false);
    int n = s_track_count;

    if (n == 0) {
        lv_obj_t *tip = lv_label_create(scr);
        if (s_catalog_oom) {
            lv_label_set_text(tip, "曲库太大 / 内存不足\n请减少音乐数量后重试");
        } else if (s_unsupported_mp4 > 0) {
            lv_label_set_text_fmt(tip,
                                  "发现 %d 个 MP4/M4A\n"
                                  "请转为 WAV(PCM16) 或 MP3",
                                  s_unsupported_mp4);
        } else {
            lv_label_set_text(tip, "没有找到音乐\n请把 MP3/WAV 放到 SD 卡 music 文件夹");
        }
        lv_obj_set_style_text_color(tip, UI_THEME_DIM, 0);
        lv_obj_set_pos(tip, 20, 68);
    }

    /* 虚拟曲库：无论有多少首，只创建最多 5 个按钮；方向键移动时替换行内容。
     * 这样所有歌曲都可浏览，又不会因为曲目多而堆积 LVGL 对象。 */
    int row_count = n < MUSIC_VISIBLE_ROWS ? n : MUSIC_VISIBLE_ROWS;
    lv_obj_t *rows[MUSIC_VISIBLE_ROWS] = {0};
    lv_obj_t *name_labels[MUSIC_VISIBLE_ROWS] = {0};
    lv_obj_t *src_labels[MUSIC_VISIBLE_ROWS] = {0};

    lv_obj_t *list = lv_obj_create(scr);
    lv_obj_set_pos(list, 0, UI_TOPBAR_H);
    lv_obj_set_size(list, 320, 178);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_radius(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);

    for (int row = 0; row < row_count; row++) {
        lv_obj_t *b = lv_button_create(list);
        lv_obj_set_size(b, 300, 28);
        lv_obj_set_pos(b, 10, 4 + row * 32);
        ui_style_list_button(b);
        lv_group_add_obj(lv_group_get_default(), b);
        rows[row] = b;

        lv_obj_t *nm = lv_label_create(b);
        lv_obj_set_width(nm, 235);
        lv_label_set_long_mode(nm, LV_LABEL_LONG_MODE_DOTS);
        lv_obj_set_style_text_color(nm, UI_THEME_TEXT, 0);
        lv_obj_set_style_text_font(nm, &book_font_lvgl, 0);
        lv_obj_align(nm, LV_ALIGN_LEFT_MID, 12, 0);
        name_labels[row] = nm;

        lv_obj_t *src = lv_label_create(b);
        lv_obj_set_style_text_font(src, &ui_font_lvgl_10, 0);
        lv_obj_align(src, LV_ALIGN_RIGHT_MID, -12, 0);
        src_labels[row] = src;
    }

    lv_obj_t *warn = NULL;
    if (n > 0 && s_unsupported_mp4 > 0) {
        warn = lv_label_create(scr);
        lv_label_set_text_fmt(warn, "另有 %d 个 MP4/M4A 未支持", s_unsupported_mp4);
        lv_obj_set_style_text_color(warn, UI_THEME_DIM, 0);
        lv_obj_set_style_text_font(warn, &ui_font_lvgl_10, 0);
        lv_obj_align(warn, LV_ALIGN_BOTTOM_MID, 0, -22);
    }

    lv_obj_t *footer = lv_label_create(scr);
    lv_obj_set_style_text_color(footer, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(footer, &ui_font_lvgl_10, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -4);

    int top_idx = 0;
    int current_idx = 0;
    if (row_count > 0) {
        render_track_window(rows, name_labels, src_labels, row_count, top_idx, footer);
        lv_group_focus_obj(rows[0]);
    } else {
        lv_label_set_text(footer, "KEY3 返回");
    }

    ui_screen_show(scr);

    for (;;) {
        lv_timer_handler();
        key_event_t evt;
        if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
            switch (evt.key) {
            case KEY_UP:
            case KEY_LEFT:
            case KEY_DOWN:
            case KEY_RIGHT:
                if (n > 0) {
                    int delta = (evt.key == KEY_UP || evt.key == KEY_LEFT) ? -1 : 1;
                    int target = current_idx + delta;
                    if (target < 0) target = 0;
                    if (target >= n) target = n - 1;

                    if (target != current_idx) {
                        current_idx = target;
                        if (current_idx < top_idx) {
                            top_idx = current_idx;
                        } else if (current_idx >= top_idx + row_count) {
                            top_idx = current_idx - row_count + 1;
                        }
                        render_track_window(rows, name_labels, src_labels, row_count, top_idx, footer);
                        int focus_row = current_idx - top_idx;
                        if (focus_row >= 0 && focus_row < row_count) lv_group_focus_obj(rows[focus_row]);
                    }
                }
                break;

            case KEY_CONFIRM: {
                lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                int selected = f ? (int)(intptr_t)lv_obj_get_user_data(f) : -1;
                if (selected >= 0 && selected < n) {
                    player_page(selected);
                    return;
                }
                break;
            }

            case KEY_BACK:
            case KEY_HOME:
                return;

            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    (void)warn;
}

esp_err_t app_music_run(void)
{
    mp_init();
    s_track_count = build_tracks();
    list_page();
    free_tracks();
    return ESP_OK;
}
