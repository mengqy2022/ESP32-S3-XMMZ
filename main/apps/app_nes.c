/*
 * app_nes.c — NES 模拟器
 * 从 SD 卡 /sdcard/games 里的 NES ROM, 在独立大栈任务中运行 infoNES 内核.
 * 内存布局:
 *   - ROM 文件   -> PSRAM 缓冲
 *   - 帧缓冲     -> nofrendo PSRAM RGB565 256x240，原生居中显示
 *   - 内核状态   -> nofrendo/PSRAM + 少量内部 SRAM
 *   - 任务栈     -> 优先 PSRAM (16KB)，失败时回退内部 SRAM
 * 说明: NES 内核 (infoNES) 为主体, 本文件是外壳 (列表/加载/任务/显示).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "apps.h"
#include "nes/nes_core.h"
#include "services/wifi_mgr.h"
#include "audio_beep.h"
#include "lvgl_port.h"
#include "safe_string.h"

static const char *TAG = "nes";

LV_FONT_DECLARE(ui_font_lvgl_10);
LV_FONT_DECLARE(book_font_lvgl);

#define NES_MAX_ROMS   24
#define NES_NAME_MAX   128
#define NES_ROM_MAX    (2 * 1024 * 1024)   /* PSRAM 足够，兼容较大的 iNES/NES2 ROM */
#define NES_SRC_W      256
#define NES_SRC_H      240
#define NES_FB_W       256
#define NES_FB_H       240

static char s_roms[NES_MAX_ROMS][NES_NAME_MAX];
static int s_rom_count = 0;

/* 已加载的 ROM + 帧缓冲 (由模拟器任务使用) */
static uint8_t *s_rom = NULL;
static uint32_t s_rom_len = 0;
static lv_obj_t *s_nes_img = NULL; /* 原生 256x240，水平居中，减少 20% SPI 像素量 */
static lv_image_dsc_t s_nes_dsc;   /* 必须长期有效，不能把栈上 dsc 指针交给 LVGL */
static char s_nes_error[96];

/* 扫描 /sdcard/games 下的 .nes */
static int scan_nes(void)
{
    s_rom_count = 0;
    DIR *d = opendir("/sdcard/games");
    if (!d) return 0;
    struct dirent *e;
    while ((e = readdir(d)) && s_rom_count < NES_MAX_ROMS) {
        const char *dot = strrchr(e->d_name, '.');
        if (!dot) continue;
        if (strcasecmp(dot, ".nes") == 0) {
            if (strlen(e->d_name) >= sizeof(s_roms[s_rom_count])) {
                ESP_LOGW(TAG, "skip too-long ROM name: %s", e->d_name);
                continue;
            }
            xm_strlcpy(s_roms[s_rom_count], e->d_name, sizeof(s_roms[s_rom_count]));
            s_rom_count++;
        }
    }
    closedir(d);
    for (int i = 0; i < s_rom_count - 1; i++)
        for (int j = i + 1; j < s_rom_count; j++)
            if (strcmp(s_roms[i], s_roms[j]) > 0) {
                char t[NES_NAME_MAX];
                strcpy(t, s_roms[i]);
                strcpy(s_roms[i], s_roms[j]);
                strcpy(s_roms[j], t);
            }
    ESP_LOGI(TAG, "found %d nes ROMs", s_rom_count);
    return s_rom_count;
}

/* 把 ROM 读到 PSRAM */
static int load_rom(const char *path)
{
    s_nes_error[0] = 0;
    FILE *f = fopen(path, "rb");
    if (!f) {
        snprintf(s_nes_error, sizeof(s_nes_error), "ROM 文件打开失败");
        ESP_LOGE(TAG, "open %s fail", path);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 16 || sz > NES_ROM_MAX) {
        snprintf(s_nes_error, sizeof(s_nes_error), "ROM 大小异常 (%ld bytes)", sz);
        fclose(f);
        return -1;
    }
    if (s_rom) { heap_caps_free(s_rom); s_rom = NULL; }
    s_rom = heap_caps_malloc((size_t)sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!s_rom) {
        fclose(f);
        snprintf(s_nes_error, sizeof(s_nes_error), "PSRAM 不足，无法载入 ROM");
        ESP_LOGE(TAG, "no PSRAM for ROM");
        return -1;
    }
    size_t rd = fread(s_rom, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) {
        heap_caps_free(s_rom); s_rom = NULL;
        snprintf(s_nes_error, sizeof(s_nes_error), "ROM 读取不完整");
        return -1;
    }
    if (memcmp(s_rom, "NES\x1A", 4) != 0) {
        heap_caps_free(s_rom); s_rom = NULL;
        snprintf(s_nes_error, sizeof(s_nes_error), "不是有效的 iNES/NES2 ROM");
        return -1;
    }
    s_rom_len = (uint32_t)sz;
    ESP_LOGI(TAG, "ROM %s loaded %u B", path, (unsigned)s_rom_len);
    return 0;
}

/* ------- NES 内核接口 (infoNES 移植点) -------
 * 集成内核后, 这里:
 *   nes_init(s_rom, s_rom_len);
 *   NesSetInput(pad1, pad2);       // 按键 -> 手柄
 *   NesEmulate();                  // 每帧渲染到 nofrendo 256x240 RGB565 帧缓冲
 *   sound: 读样本缓冲区 -> I2S
 */

/* 模拟器独立任务: 大栈 (16KB, 主任务栈仅 8KB 会溢出) */
#define EVT_DONE 1
static EventGroupHandle_t s_evt = NULL;

static void nes_emu_task(void *arg)
{
    (void)arg;
    if (!s_rom || !s_nes_img) {
        snprintf(s_nes_error, sizeof(s_nes_error), "NES 运行资源未就绪");
        xEventGroupSetBits(s_evt, EVT_DONE); vTaskDeleteWithCaps(NULL); return;
    }
    if (nes_core_load_rom(s_rom, s_rom_len) != 0) {
        snprintf(s_nes_error, sizeof(s_nes_error), "ROM/Mapper 不支持或 ROM 已损坏");
        ESP_LOGE(TAG, "ROM 载入失败");
        nes_core_release();
        xEventGroupSetBits(s_evt, EVT_DONE);
        vTaskDeleteWithCaps(NULL); return;
    }
    ESP_LOGI(TAG, "NES emu start, rom=%u B", (unsigned)s_rom_len);
    nes_core_run();   /* 阻塞直到 KEY3 退出; 每帧经回调(渲染/按键/声音)处理 */
    ESP_LOGI(TAG, "NES emu exit");
    nes_core_release();
    xEventGroupSetBits(s_evt, EVT_DONE);
    vTaskDeleteWithCaps(NULL);
}

static void show_nes_error(const char *msg)
{
    if (!msg || !msg[0]) return;
    lv_obj_t *scr = ui_screen_new("NES 错误");
    lv_obj_t *l = lv_label_create(scr);
    lv_label_set_text(l, msg);
    lv_obj_set_width(l, 292);
    lv_obj_set_style_text_color(l, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(l, &book_font_lvgl, 0);
    lv_obj_set_pos(l, 14, 72);
    lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
    ui_screen_show(scr);
    while (!ui_app_loop_wait()) {}
}

/* ---- 模拟器每帧回调 (C, 供 nes_core.cpp 调用) ---- */
void nes_host_render(void)
{
    if (!s_nes_img) return;
    const uint16_t *frame = nes_core_get_frame();
    if (!frame) return;

    if (s_nes_dsc.data != (const uint8_t *)frame) {
        s_nes_dsc.data = (const uint8_t *)frame;
        lv_image_set_src(s_nes_img, &s_nes_dsc);
    }
    lv_obj_invalidate(s_nes_img);
    /* NES 运行期间主 UI 线程阻塞；nes_core 的 Core0 视频 worker 是唯一 LVGL 驱动点。 */
    lv_timer_handler();
}

static bool s_vol_up_prev = false;
static bool s_vol_down_prev = false;
static int64_t s_vol_last_change_us = 0;
static int64_t s_menu_hold_us = 0;

static void nes_input_reset(void)
{
    s_vol_up_prev = false;
    s_vol_down_prev = false;
    s_vol_last_change_us = 0;
    s_menu_hold_us = 0;
}

uint32_t nes_host_pad(int *pquit)
{
    uint32_t pad = 0;

    /* 实时游戏键走 fast GPIO 路径：每个逻辑帧直接读取电平。
     * 菜单的后台消抖任务优先级低于 NES 模拟任务；如果仍读取已消抖 mask，
     * 在模拟器落后时会产生明显的“按下了，但画面晚几帧才响应”的手感。 */
    if (buttons_get_state_fast(KEY_A))      pad |= (1u << 0);  /* NES A      GPIO15 */
    if (buttons_get_state_fast(KEY_B))      pad |= (1u << 1);  /* NES B      GPIO5  */
    if (buttons_get_state_fast(KEY_SELECT)) pad |= (1u << 2);  /* NES SELECT GPIO16 */
    if (buttons_get_state_fast(KEY_START))  pad |= (1u << 3);  /* NES START  GPIO17 */
    if (buttons_get_state_fast(KEY_UP))     pad |= (1u << 4);
    if (buttons_get_state_fast(KEY_DOWN))   pad |= (1u << 5);
    if (buttons_get_state_fast(KEY_LEFT))   pad |= (1u << 6);
    if (buttons_get_state_fast(KEY_RIGHT))  pad |= (1u << 7);

    /* NES 内音量：物理 KEY2/OPTION = +5%，BOOT = -5%。
     * 直接读 GPIO 保证模拟任务忙时也能调音量；80ms 冷却抑制机械抖动重复触发。 */
    int64_t now = esp_timer_get_time();
    bool vol_up = buttons_get_state_fast(KEY_OPTION);
    bool vol_down = buttons_get_state_fast(KEY_BOOT);
    if (vol_up && !vol_down && !s_vol_up_prev && now - s_vol_last_change_us >= 80000) {
        uint8_t v = audio_get_volume();
        audio_set_volume(v >= 95 ? 100 : v + 5);
        s_vol_last_change_us = now;
        ESP_LOGI(TAG, "NES volume %u%% (KEY2 +)", (unsigned)audio_get_volume());
    }
    if (vol_down && !vol_up && !s_vol_down_prev && now - s_vol_last_change_us >= 80000) {
        uint8_t v = audio_get_volume();
        audio_set_volume(v <= 5 ? 0 : v - 5);
        s_vol_last_change_us = now;
        ESP_LOGI(TAG, "NES volume %u%% (BOOT -)", (unsigned)audio_get_volume());
    }
    s_vol_up_prev = vol_up;
    s_vol_down_prev = vol_down;

    /* 退出不占用 NES SELECT。长按 MENU 1 秒退出；同样直接 GPIO 采样。 */
    int quit = 0;
    if (buttons_get_state_fast(KEY_MENU)) {
        if (s_menu_hold_us == 0) s_menu_hold_us = now;
        else if (now - s_menu_hold_us >= 1000000) quit = 1;
    } else {
        s_menu_hold_us = 0;
    }
    if (pquit) *pquit = quit;

    return pad;
}

/* 运行选中的 ROM */
static void run_rom(int idx)
{
    if (idx < 0 || idx >= s_rom_count) return;
    char full[NES_NAME_MAX + 24];
    int nl = (int)strlen(s_roms[idx]);
    if (nl > (int)sizeof(full) - 20) return;
    memcpy(full, "/sdcard/games/", 14);
    memcpy(full + 14, s_roms[idx], (size_t)nl + 1);
    if (load_rom(full) != 0) {
        show_nes_error(s_nes_error);
        return;
    }


    /* 性能模式: NES 原生 256x240 居中显示。相比软件拉伸到 320x240，
     * 每次 LCD 刷新少 20% 像素，同时去掉一整帧的缩放/复制。 */
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    memset(&s_nes_dsc, 0, sizeof(s_nes_dsc));
    s_nes_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
    s_nes_dsc.header.w = NES_FB_W;
    s_nes_dsc.header.h = NES_FB_H;
    s_nes_dsc.header.stride = NES_FB_W * sizeof(uint16_t);
    s_nes_dsc.data_size = (uint32_t)((size_t)NES_FB_W * NES_FB_H * sizeof(uint16_t));
    s_nes_dsc.data = NULL; /* nes_host_render() first frame binds core PSRAM framebuffer */
    s_nes_img = lv_image_create(scr);
    lv_obj_set_size(s_nes_img, NES_FB_W, NES_FB_H);
    lv_obj_set_pos(s_nes_img, (320 - NES_FB_W) / 2, 0);
    /* NES 全屏也自动释放上一屏，避免反复进出 ROM 后 LVGL 对象累积。 */
    ui_screen_show(scr);

    /* 先让黑色 NES screen 完成一次 LVGL flush，然后 NES 期间独占实体键。 */
    lv_timer_handler();
    buttons_flush_events();
    nes_input_reset();
    lvgl_input_set_enabled(false);

    ESP_LOGI(TAG, "进 NES: 停 WiFi, 内部堆=%lu",
             (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    wifi_mgr_stop();   /* 释放内部 RAM, 让模拟任务能分配 */

    s_evt = xEventGroupCreate();
    ESP_LOGI(TAG, "建模拟任务前内部堆=%lu",
             (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    BaseType_t task_rc = pdFAIL;
    if (s_evt) {
        /* 旧实现用普通 xTaskCreatePinnedToCore，16KB 栈必须占内部 SRAM；
         * WiFi/BT/LVGL 常驻后最大连续内部块不足时，选择 ROM 会直接启动失败。
         * sdkconfig 已允许 external stack，因此优先把 NES 大栈放 PSRAM。 */
        task_rc = xTaskCreatePinnedToCoreWithCaps(nes_emu_task, "nes_emu", 16384, NULL, 6, NULL, 1,
                                                  MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (task_rc != pdPASS) {
            ESP_LOGW(TAG, "PSRAM task stack failed, fallback internal");
            task_rc = xTaskCreatePinnedToCoreWithCaps(nes_emu_task, "nes_emu", 16384, NULL, 6, NULL, 1,
                                                      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        }
    }
    if (s_evt && task_rc == pdPASS) {
        xEventGroupWaitBits(s_evt, EVT_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
    } else {
        snprintf(s_nes_error, sizeof(s_nes_error),
                 "NES 任务创建失败，内存不足");
        ESP_LOGE(TAG, "NES task create failed: rc=%ld free_internal=%lu largest_internal=%lu",
                 (long)task_rc,
                 (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
                 (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        if (s_evt) xEventGroupSetBits(s_evt, EVT_DONE);
    }
    if (s_evt) { vEventGroupDelete(s_evt); s_evt = NULL; }

    /* 丢弃游戏过程中积累在系统按键队列里的 PRESS/RELEASE，避免退出后菜单误操作。 */
    buttons_flush_events();
    lvgl_input_set_enabled(true);

    s_nes_img = NULL;
    s_nes_dsc.data = NULL;
    if (s_rom) { heap_caps_free(s_rom); s_rom = NULL; }
    wifi_mgr_start();   /* 恢复 WiFi */

    if (s_nes_error[0]) show_nes_error(s_nes_error);
}

esp_err_t app_nes_run(void)
{
    if (scan_nes() == 0) {
        lv_obj_t *scr = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(scr, UI_THEME_BG, 0);
        lv_obj_set_style_text_font(scr, &book_font_lvgl, 0);  /* 全 GB2312 */
        lv_obj_set_style_pad_all(scr, 0, 0);
        lv_obj_t *t = lv_label_create(scr);
        lv_label_set_text(t, "NES");
        lv_obj_set_style_text_color(t, UI_THEME_TEXT, 0);
        lv_obj_align(t, LV_ALIGN_TOP_LEFT, 12, 8);
        lv_obj_t *e1 = lv_label_create(scr);
        lv_label_set_text(e1, "没有找到 .nes 游戏");
        lv_obj_set_style_text_color(e1, UI_THEME_TEXT, 0);
        lv_obj_align(e1, LV_ALIGN_CENTER, 0, -20);
        lv_obj_t *e2 = lv_label_create(scr);
        lv_label_set_text(e2, "请把 .nes 放入 SD 卡 games");
        lv_obj_set_style_text_color(e2, UI_THEME_DIM, 0);
        lv_obj_align(e2, LV_ALIGN_CENTER, 0, 14);
        ui_screen_show(scr);
        while (!ui_app_loop_wait()) {}
        return ESP_OK;
    }

    for (;;) {
        lv_obj_t *scr = ui_screen_new_ex("NES", false);
        lv_obj_t *bar0 = lv_obj_get_child(scr, 0);          /* 标题栏 */
        lv_obj_t *ti = bar0 ? lv_obj_get_child(bar0, 0) : NULL;
        if (ti) lv_obj_set_style_text_font(ti, &book_font_lvgl, 0);  /* 全 GB2312 */

        /* ROM 列表独立滚动，标题和底部提示固定，最多 24 个 ROM 也不会互相覆盖。 */
        lv_obj_t *list = lv_obj_create(scr);
        lv_obj_set_pos(list, 0, UI_TOPBAR_H);
        lv_obj_set_size(list, 320, 180);
        lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(list, 0, 0);
        lv_obj_set_style_radius(list, 0, 0);
        lv_obj_set_style_pad_all(list, 0, 0);
        lv_obj_set_scroll_dir(list, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t *first_btn = NULL;
        for (int i = 0; i < s_rom_count; i++) {
            lv_obj_t *btn = lv_button_create(list);
            lv_obj_set_size(btn, 300, 28);
            lv_obj_set_pos(btn, 10, 4 + i * 32);
            ui_style_list_button(btn);
            lv_obj_set_user_data(btn, (void *)(intptr_t)i);
            lv_group_add_obj(lv_group_get_default(), btn);
            if (!first_btn) first_btn = btn;
            lv_obj_t *l = lv_label_create(btn);
            lv_label_set_text(l, s_roms[i]);
            lv_obj_set_style_text_color(l, UI_THEME_TEXT, 0);
            lv_obj_set_style_text_font(l, &book_font_lvgl, 0);   /* 全 GB2312, 中文名正常 */
            lv_obj_align(l, LV_ALIGN_LEFT_MID, 12, 0);
        }
        lv_obj_t *hint = lv_label_create(scr);
        lv_label_set_text(hint, "START 开始  SELECT 返回");
        lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
        lv_obj_set_style_text_font(hint, &book_font_lvgl, 0);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -4);
        if (first_btn) lv_group_focus_obj(first_btn);
        ui_screen_show(scr);

        int launch = -1;
        for (;;) {
            lv_timer_handler();
            key_event_t evt;
            if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
                switch (evt.key) {
                case KEY_UP: case KEY_DOWN: {
                    ui_nav_key(evt.key);
                    lv_obj_t *focused = lv_group_get_focused(lv_group_get_default());
                    if (focused) lv_obj_scroll_to_view(focused, LV_ANIM_OFF);
                    break;
                }
                case KEY_CONFIRM: {
                    lv_group_t *g = lv_group_get_default();
                    lv_obj_t *f = g ? lv_group_get_focused(g) : NULL;
                    if (f) launch = (int)(intptr_t)lv_obj_get_user_data(f);
                    break;
                }
                case KEY_BACK:
                    return ESP_OK;
                default: break;
                }
            }
            if (launch >= 0 && launch < s_rom_count) {
                int idx = launch;
                launch = -1;
                run_rom(idx);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}
