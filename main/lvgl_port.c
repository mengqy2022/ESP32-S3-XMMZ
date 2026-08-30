/*
 * lvgl_port.c — LVGL 9 + esp_lvgl_port 移植
 * 显示: ST7789 240x320 (esp_lcd), 方向 90°CW (swap_xy + mirror_x)
 * 输入: 按键矩阵 -> LVGL keypad (SW2方向 + KEY4确定 + KEY3返回 + KEY1主界面)
 *       KEY2(设置) 由菜单循环轮询处理
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "lcd_st7789.h"
#include "buttons.h"

static const char *TAG = "lvgl_port";

static lv_indev_t *s_indev = NULL;
static volatile bool s_input_enabled = true;
static const key_id_t s_act_keys_id[3] = { KEY_CONFIRM, KEY_BACK, KEY_HOME };
static const uint32_t s_act_lv_keys[3] = { LV_KEY_ENTER, LV_KEY_ESC, LV_KEY_HOME };
static bool s_act_prev[3] = {false};

/* 按键 -> LVGL 键
 * 注意: LVGL9 的 keypad 只在 LV_KEY_NEXT/PREV 时自动移动焦点;
 * UP/DOWN/LEFT/RIGHT 只是事件, 普通按钮不处理 → 方向导航由应用循环
 * (队列) 自己驱动! 这里只映射动作键 (确定/返回/主界面). */
static void input_read_cb(lv_indev_t *drv, lv_indev_data_t *data)
{
    if (!s_input_enabled) {
        data->key = 0;
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    for (int i = 0; i < 3; i++) {
        bool now = buttons_get_state(s_act_keys_id[i]);
        if (now && !s_act_prev[i]) {
            s_act_prev[i] = true;
            data->key = s_act_lv_keys[i];
            data->state = LV_INDEV_STATE_PRESSED;
            ESP_LOGI(TAG, "key->lvgl: %lu", (unsigned long)s_act_lv_keys[i]);
            return;
        }
        if (!now) s_act_prev[i] = false;
    }
    data->key = 0;
    data->state = LV_INDEV_STATE_RELEASED;
}

esp_err_t lvgl_sys_init(void)
{
    /* esp_lvgl_port 内部互斥/上下文必须首先初始化 */
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    esp_err_t perr = lvgl_port_init(&port_cfg);
    if (perr != ESP_OK) {
        ESP_LOGE(TAG, "lvgl_port_init failed: %s", esp_err_to_name(perr));
        return perr;
    }

    /* 关键: 删除 esp_lvgl_port 自带的 "taskLVGL" 渲染任务!
     * 它会在自己的线程里跑 lv_timer_handler, 与我们的应用循环并发渲染
     * LVGL → 渲染中失效区死循环 → 看门狗卡死. 必须单线程独占. */
    TaskHandle_t lvgl_task = xTaskGetHandle("taskLVGL");
    if (lvgl_task) {
        vTaskDelete(lvgl_task);
        ESP_LOGI(TAG, "removed esp_lvgl_port internal task (single-threaded LVGL)");
    } else {
        ESP_LOGW(TAG, "taskLVGL not found");
    }

    /* --- 显示 --- */
    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = (esp_lcd_panel_io_handle_t)lcd_get_io_handle(),
        .panel_handle = (esp_lcd_panel_handle_t)lcd_get_panel_handle(),
        .buffer_size = LCD_W * LCD_H / 8,   /* 1/8 屏缓冲 (19.2KB, 内部 DMA 内存, 必须足够小) */
        .double_buffer = false,   /* 单缓冲! 双缓冲在 SPI 屏上会因 flush_ready 竞态卡死 */
        .trans_size = 0,
        .hres = LCD_W,       /* 逻辑横屏 320x240 */
        .vres = LCD_H,
        .monochrome = false,
        .rotation = {
            .swap_xy = true,    /* 320x240 横屏玻璃 */
            .mirror_x = false,
            .mirror_y = true,   /* 修正上下翻转 */
        },
        .flags = {
            /* 关键: 绘制缓冲必须在内部 DMA 内存 (buff_dma=true, 不能放 PSRAM)!
             * esp_lcd 的 SPI panel io 不设置 SPI_TRANS_DMA_USE_PSRAM,
             * PSRAM 颜色缓冲每次刷新都要临时分配"内部 DMA 拷贝缓冲";
             * BLE/WiFi 初始化后内部 DMA RAM 所剩无几 → 分配失败 →
             * DMA 从未启动 → on_color_trans_done 永不触发 →
             * LVGL wait_for_flushing 永久自旋 → 看门狗卡死.
             * 用内部 DMA 缓冲则刷新零分配零拷贝, 免疫其他子系统吃内存. */
            .buff_dma = true,
            .buff_spiram = false,
            .swap_bytes = true,    /* RGB565 字节序 */
        },
    };
    lv_display_t *disp = lvgl_port_add_disp(&disp_cfg);
    if (!disp) {
        ESP_LOGE(TAG, "add display failed");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "display added (LVGL v%d.%d.%d, %dx%d)",
             LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH, LCD_W, LCD_H);

    /* --- 输入: keypad --- */
    s_indev = lv_indev_create();
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(s_indev, input_read_cb);
    if (!s_indev) {
        ESP_LOGE(TAG, "create indev failed");
        return ESP_FAIL;
    }
    lv_group_t *grp = lv_group_create();
    lv_group_set_default(grp);
    lv_indev_set_group(s_indev, grp);
    ESP_LOGI(TAG, "keypad indev created");

    /* 说明: 不单独创建 LVGL 任务 — UI 循环由 lvgl_menu_run() 单一驱动 */
    return ESP_OK;
}

void lvgl_input_set_enabled(bool enabled)
{
    /* 恢复时先把当前仍按住的系统动作键记为“已按”，必须释放后才能形成下一次边沿。
     * 这样长按 MENU 退出 NES 后，不会因为 MENU 还没来得及松开又触发 LV_KEY_HOME。 */
    for (int i = 0; i < 3; i++) s_act_prev[i] = buttons_get_state(s_act_keys_id[i]);
    s_input_enabled = enabled;
    if (!enabled && s_indev) lv_indev_reset(s_indev, NULL);
}
