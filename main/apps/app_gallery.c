/*
 * app_gallery.c — SD 卡图库
 *
 * 读取策略:
 *   1) 优先扫描 /sdcard/gallery；同时兼容 DCIM、Pictures、wallpaper 和 SD 根目录。
 *   2) 仅直接解码 JPG/JPEG。工程已启用 LVGL TJPGD，它是 MCU block 流式解码，
 *      不需要把整张照片一次性读入 RAM。
 *   3) 使用 LV_IMAGE_ALIGN_CONTAIN 在 320x240 中等比缩放显示；不再因为原图大于
 *      480x360 就直接丢弃。
 *   4) JPEG 尺寸解析改为流式 marker 读取，不再在 app_main 的 8KB 栈上申请 16KB
 *      临时数组（旧实现会直接造成栈溢出，表现为“图库打不开/重启”）。
 *
 * 注意: 当前未启用 PNG 全图解码，主要是避免高分辨率 PNG 带来的大块内存占用。
 */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "freertos/event_groups.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "apps.h"
#include "safe_string.h"

static const char *TAG = "gallery";

LV_FONT_DECLARE(ui_font_lvgl_10);

#define GAL_MAX            40
#define GAL_PATH_MAX       160
#define SLIDE_INTERVAL_MS  5000

typedef struct {
    char path[GAL_PATH_MAX];  /* POSIX: /sdcard/... */
    uint16_t w;
    uint16_t h;
} gallery_item_t;

static gallery_item_t s_items[GAL_MAX];
static int s_count = 0;

static bool is_jpeg_name(const char *name)
{
    const char *dot = name ? strrchr(name, '.') : NULL;
    return dot && (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0);
}

static bool is_sof_marker(uint8_t m)
{
    return m >= 0xC0 && m <= 0xCF && m != 0xC4 && m != 0xC8 && m != 0xCC;
}

/* 只流式读取 JPEG marker，不申请大数组。返回 0 成功。 */
static int jpg_dimensions(const char *path, int *w, int *h)
{
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    uint8_t soi[2];
    if (fread(soi, 1, 2, f) != 2 || soi[0] != 0xFF || soi[1] != 0xD8) {
        fclose(f);
        return -1;
    }

    for (int segments = 0; segments < 256; segments++) {
        int c;
        do {
            c = fgetc(f);
            if (c == EOF) { fclose(f); return -1; }
        } while (c != 0xFF);

        do {
            c = fgetc(f);
            if (c == EOF) { fclose(f); return -1; }
        } while (c == 0xFF);

        uint8_t marker = (uint8_t)c;
        if (marker == 0xD9 || marker == 0xDA) break; /* EOI / SOS */
        if (marker == 0xD8 || marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7)) continue;

        uint8_t lb[2];
        if (fread(lb, 1, 2, f) != 2) break;
        uint16_t seglen = ((uint16_t)lb[0] << 8) | lb[1];
        if (seglen < 2) break;

        if (is_sof_marker(marker)) {
            uint8_t sof[5]; /* precision + height + width */
            if (seglen < 7 || fread(sof, 1, sizeof(sof), f) != sizeof(sof)) break;
            *h = ((int)sof[1] << 8) | sof[2];
            *w = ((int)sof[3] << 8) | sof[4];
            fclose(f);
            return (*w > 0 && *h > 0) ? 0 : -1;
        }

        if (fseek(f, (long)seglen - 2L, SEEK_CUR) != 0) break;
    }

    fclose(f);
    return -1;
}

static bool item_exists(const char *path)
{
    for (int i = 0; i < s_count; i++) {
        if (strcmp(s_items[i].path, path) == 0) return true;
    }
    return false;
}

static void add_jpeg(const char *path)
{
    if (s_count >= GAL_MAX || item_exists(path)) return;
    if (strlen(path) >= sizeof(s_items[0].path)) {
        ESP_LOGW(TAG, "skip too-long path: %s", path);
        return;
    }

    int w = 0, h = 0;
    if (jpg_dimensions(path, &w, &h) != 0) {
        ESP_LOGW(TAG, "skip invalid JPEG: %s", path);
        return;
    }

    xm_strlcpy(s_items[s_count].path, path, sizeof(s_items[s_count].path));
    s_items[s_count].w = (uint16_t)(w > 65535 ? 65535 : w);
    s_items[s_count].h = (uint16_t)(h > 65535 ? 65535 : h);
    ESP_LOGI(TAG, "image[%d] %dx%d %s", s_count, w, h, path);
    s_count++;
}

/* depth=0 只扫当前目录；depth>0 继续进入子目录。 */
static void scan_dir(const char *dir, int depth)
{
    if (s_count >= GAL_MAX) return;
    DIR *d = opendir(dir);
    if (!d) return;

    struct dirent *e;
    while ((e = readdir(d)) && s_count < GAL_MAX) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;

        char full[GAL_PATH_MAX];
        int wr = snprintf(full, sizeof(full), "%s/%s", dir, e->d_name);
        if (wr < 0 || wr >= (int)sizeof(full)) continue;

        struct stat st;
        bool is_dir = (stat(full, &st) == 0) && S_ISDIR(st.st_mode);

        if (is_dir) {
            if (depth > 0) scan_dir(full, depth - 1);
        } else if (is_jpeg_name(e->d_name)) {
            add_jpeg(full);
        }
    }
    closedir(d);
}

static int scan(void)
{
    s_count = 0;

    /* 专用目录优先；手机/相机常见目录兼容；最后扫 SD 根目录的直属 JPEG。 */
    scan_dir("/sdcard/gallery", 2);
    scan_dir("/sdcard/DCIM", 2);
    scan_dir("/sdcard/Pictures", 2);
    scan_dir("/sdcard/wallpaper", 1);
    scan_dir("/sdcard", 0);

    for (int i = 0; i < s_count - 1; i++) {
        for (int j = i + 1; j < s_count; j++) {
            if (strcmp(s_items[i].path, s_items[j].path) > 0) {
                gallery_item_t t = s_items[i];
                s_items[i] = s_items[j];
                s_items[j] = t;
            }
        }
    }
    ESP_LOGI(TAG, "gallery: %d JPEG(s)", s_count);
    return s_count;
}

static void show_empty(void)
{
    lv_obj_t *scr = ui_screen_new("图库");
    ui_label(scr, "没有找到可显示的 JPG/JPEG", 12, 70);
    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text(sub, "推荐放入 SD:/gallery\n也兼容 DCIM/Pictures/wallpaper");
    lv_obj_set_style_text_color(sub, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(sub, &ui_font_lvgl_10, 0);
    lv_obj_set_pos(sub, 12, 105);
    ui_screen_show(scr);
    while (!ui_app_loop_wait()) {}
}

/* LVGL POSIX driver: P: -> /sdcard。
 * TJPGD 的扩展名判断区分大小写，因此把路径副本的扩展名改成小写；FATFS 本身不区分大小写。 */
static bool make_lv_path(const char *posix, char *out, size_t out_sz)
{
    static const char prefix[] = "/sdcard";
    if (!posix || strncmp(posix, prefix, sizeof(prefix) - 1) != 0) return false;
    int wr = snprintf(out, out_sz, "P:%s", posix + sizeof(prefix) - 1);
    if (wr < 0 || wr >= (int)out_sz) return false;

    char *dot = strrchr(out, '.');
    if (dot) {
        for (char *p = dot; *p; p++) *p = (char)tolower((unsigned char)*p);
    }
    return true;
}

static void show_image(lv_obj_t *img, lv_obj_t *info, int cur)
{
    char lvpath[GAL_PATH_MAX];
    if (!make_lv_path(s_items[cur].path, lvpath, sizeof(lvpath))) {
        ESP_LOGE(TAG, "path convert failed: %s", s_items[cur].path);
        return;
    }

    const char *name = strrchr(s_items[cur].path, '/');
    name = name ? name + 1 : s_items[cur].path;
    ESP_LOGI(TAG, "load img: %s (%ux%u) -> %s", name,
             (unsigned)s_items[cur].w, (unsigned)s_items[cur].h, lvpath);

    lv_image_set_src(img, lvpath);
    lv_image_set_inner_align(img, LV_IMAGE_ALIGN_CONTAIN);
    lv_label_set_text_fmt(info, "%d/%d  %ux%u", cur + 1, s_count,
                          (unsigned)s_items[cur].w, (unsigned)s_items[cur].h);
}

static esp_err_t gallery_run_impl(void)
{
    ESP_LOGI(TAG, "gallery open...");
    if (scan() == 0) {
        ESP_LOGW(TAG, "no usable JPEG");
        show_empty();
        return ESP_OK;
    }

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    lv_obj_t *img = lv_image_create(scr);
    lv_obj_set_size(img, 320, 240);
    lv_obj_set_pos(img, 0, 0);
    lv_image_set_inner_align(img, LV_IMAGE_ALIGN_CONTAIN);

    lv_obj_t *info = lv_label_create(scr);
    lv_obj_set_style_text_color(info, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(info, &ui_font_lvgl_10, 0);
    lv_obj_set_style_bg_color(info, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(info, LV_OPA_50, 0);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 6, 4);

    lv_obj_t *hint = lv_label_create(scr);
    lv_obj_set_style_text_color(hint, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(hint, &ui_font_lvgl_10, 0);
    lv_obj_set_style_bg_color(hint, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(hint, LV_OPA_50, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_label_set_text(hint, "KEY1 播放  KEY3 返回");

    /* 全屏图库也走统一 screen 生命周期，退出后不会遗留上一屏对象树。 */
    ui_screen_show(scr);

    int cur = 0;
    bool slide = false;
    uint32_t last = 0;
    show_image(img, info, cur);

    for (;;) {
        lv_timer_handler();
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);

        if (slide && now - last >= SLIDE_INTERVAL_MS) {
            last = now;
            cur = (cur + 1) % s_count;
            show_image(img, info, cur);
        }

        key_event_t evt;
        if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
            switch (evt.key) {
            case KEY_LEFT:
            case KEY_A:
                cur = (cur > 0) ? cur - 1 : s_count - 1;
                show_image(img, info, cur);
                break;
            case KEY_RIGHT:
            case KEY_B:
                cur = (cur + 1) % s_count;
                show_image(img, info, cur);
                break;
            case KEY_MENU:
                slide = !slide;
                last = now;
                lv_label_set_text(hint, slide ? "自动播放中..." : "KEY1 播放  KEY3 返回");
                break;
            case KEY_BACK:
                /* KEY_HOME is an alias of KEY_MENU in buttons.h.
                 * KEY_MENU is intentionally used above to toggle slideshow,
                 * so only physical KEY3 / KEY_BACK exits the gallery. */
                return ESP_OK;
            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* JPEG decoder and LVGL draw stack depth are much larger than ordinary menu pages.
 * Keep them off app_main's 8 KB stack. This also makes future larger JPEGs safer
 * without permanently consuming another 16-24 KB of scarce internal SRAM. */
#define GALLERY_EVT_DONE  BIT0
static EventGroupHandle_t s_gallery_evt = NULL;
static esp_err_t s_gallery_result = ESP_OK;

static void gallery_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "gallery task start, stack HWM=%u", (unsigned)uxTaskGetStackHighWaterMark(NULL));
    s_gallery_result = gallery_run_impl();
    ESP_LOGI(TAG, "gallery task exit, stack HWM=%u", (unsigned)uxTaskGetStackHighWaterMark(NULL));
    if (s_gallery_evt) xEventGroupSetBits(s_gallery_evt, GALLERY_EVT_DONE);
    vTaskDeleteWithCaps(NULL);
}

esp_err_t app_gallery_run(void)
{
    s_gallery_result = ESP_FAIL;
    s_gallery_evt = xEventGroupCreate();
    if (!s_gallery_evt) return ESP_ERR_NO_MEM;

    BaseType_t rc = xTaskCreatePinnedToCoreWithCaps(gallery_task, "gallery", 24576, NULL, 5,
                                                    NULL, 1,
                                                    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (rc != pdPASS) {
        ESP_LOGW(TAG, "PSRAM gallery stack failed, fallback 12KB internal");
        rc = xTaskCreatePinnedToCoreWithCaps(gallery_task, "gallery", 12288, NULL, 5,
                                             NULL, 1,
                                             MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    if (rc != pdPASS) {
        vEventGroupDelete(s_gallery_evt);
        s_gallery_evt = NULL;
        ESP_LOGE(TAG, "gallery task create failed");
        return ESP_ERR_NO_MEM;
    }

    xEventGroupWaitBits(s_gallery_evt, GALLERY_EVT_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
    vEventGroupDelete(s_gallery_evt);
    s_gallery_evt = NULL;
    return s_gallery_result;
}

