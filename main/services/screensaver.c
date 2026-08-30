/*
 * screensaver.c — 屏保: 壁纸轮播, 任意键退出
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "wallpaper.h"
#include "screensaver.h"

static const char *TAG = "saver";

void screensaver_run(void)
{
    wallpaper_init();

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    lv_obj_t *img = lv_image_create(scr);
    lv_obj_set_size(img, WP_W, WP_H);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    /* 等比适配显示, 不变形 */
    lv_obj_set_style_image_recolor_opa(img, LV_OPA_TRANSP, 0);

    lv_obj_t *old = lv_scr_act();
    if (old && old != scr) ui_group_cleanup(old);
    lv_screen_load(scr);

    int count = wallpaper_count();
    int cur = 0;
    uint32_t last = (uint32_t)(esp_timer_get_time() / 1000);

    ESP_LOGI(TAG, "screensaver start, %d wallpapers", count);

    for (;;) {
        lv_timer_handler();

        /* 任意按键退出 */
        key_event_t evt;
        if (buttons_wait_event(&evt, 0)) {
            if (evt.evt == KEY_EVT_PRESS) break;
            continue;
        }

        /* 5 秒轮播 */
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (now - last >= SCREENSAVER_INTERVAL_MS) {
            last = now;
            cur = (cur + 1) % count;
            char path[64];
            int r = wallpaper_load(cur, path, sizeof(path));
            if (r == 1) {
                lv_image_set_src(img, path);   /* SD jpg: LVGL 自动解码 */
            } else if (r == 0) {
                lv_image_dsc_t *d = wallpaper_dsc();
                if (d) lv_image_set_src(img, d);
            }
            lv_obj_invalidate(img);
            ESP_LOGI(TAG, "wallpaper %d (%s)", cur, (r == 1) ? path : "rgb565");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "screensaver exit");
}
