/*
 * buttons.c — 按键驱动
 * 原理图: 按键接 GND 有外部上拉(低电平有效); 五向开关同为低有效
 * 实现: GPIO 轮询任务 + 10ms 消抖 + 700ms 长按 + 开机按下抑制；实时游戏可走 fast 原始采样
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "buttons.h"
#include "board_config.h"

static const char *TAG = "buttons";

static const gpio_num_t s_key_pins[KEY_MAX] = {
    BTN_PIN_BOOT, BTN_PIN_MENU, BTN_PIN_OPTION, BTN_PIN_SELECT,
    BTN_PIN_START, BTN_PIN_A, BTN_PIN_B,
    BTN_PIN_LEFT, BTN_PIN_RIGHT, BTN_PIN_UP, BTN_PIN_DOWN,
};

static QueueHandle_t s_evt_queue = NULL;
static volatile uint32_t s_longpress_ms = 700;
static volatile uint32_t s_pressed_mask = 0;   /* 已消抖的稳定按下状态 */
/* 所有按键固定低电平有效 (按下=0), 依据原理图; 不做开机极性自适应
 * (自适应会在开机瞬间读到低电平时把该键极性判反 → 松开也显示"按下") */
static bool s_active_low[KEY_MAX] = {
    true, true, true, true, true, true, true, true, true, true, true
};

static void buttons_task(void *arg);   /* 前向声明 */

static const char *key_name(key_id_t k)
{
    static const char *n[] = {"BOOT","MENU","OPTION","SELECT","START","A","B","LEFT","RIGHT","UP","DOWN"};
    return n[k];
}

/* 原始电平 -> 是否按下 (按检测到的极性) */
static inline bool level_pressed(key_id_t k, int level)
{
    return s_active_low[k] ? (level == 0) : (level == 1);
}

esp_err_t buttons_init(void)
{
    s_evt_queue = xQueueCreate(32, sizeof(key_event_t));
    if (!s_evt_queue) return ESP_ERR_NO_MEM;

    gpio_config_t cfg = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,   /* 先开内部上拉测静止电平 */
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    for (int i = 0; i < KEY_MAX; i++) cfg.pin_bit_mask |= (1ULL << s_key_pins[i]);
    gpio_config(&cfg);

    /* 极性固定低有效 (原理图). 仅打印静止电平, 便于诊断开机即被拉低的引脚 */
    vTaskDelay(pdMS_TO_TICKS(50));
    for (int i = 0; i < KEY_MAX; i++) {
        ESP_LOGI(TAG, "%s: rest=%d active_low=1", key_name(i), gpio_get_level(s_key_pins[i]));
    }

    xTaskCreatePinnedToCore(buttons_task, "buttons", 2048, NULL, 5, NULL, 1);
    ESP_LOGI(TAG, "buttons init OK (%d keys)", KEY_MAX);
    return ESP_OK;
}

bool buttons_get_state(key_id_t key)
{
    if (key >= KEY_MAX) return false;
    /* 菜单/UI 使用稳定状态，避免机械按键抖动造成重复操作。 */
    return (s_pressed_mask & (1u << key)) != 0;
}

bool buttons_get_state_fast(key_id_t key)
{
    if (key >= KEY_MAX) return false;
    /* NES/实时游戏使用：直接读当前 GPIO，不依赖后台按键任务调度。
     * 机械抖动通常只有数毫秒，游戏每帧连续采样比额外 10~20ms 消抖延迟更重要。
     * 菜单、音量设置等非实时操作仍应使用 buttons_get_state()/事件队列。 */
    return level_pressed(key, gpio_get_level(s_key_pins[key]));
}

static void buttons_task(void *arg)
{
    uint8_t raw[KEY_MAX] = {0};
    uint8_t stable[KEY_MAX] = {0};
    uint32_t pressed_ms[KEY_MAX] = {0};
    uint8_t long_sent[KEY_MAX] = {0};

    /* 初始状态采样: 开机已按下的键(如损坏常闭的 KEY6)不产生假事件 */
    for (int i = 0; i < KEY_MAX; i++) {
        bool p = level_pressed(i, gpio_get_level(s_key_pins[i]));
        stable[i] = p ? 1 : 0;
        raw[i] = p ? 0x03 : 0x00;
        if (p) s_pressed_mask |= (1u << i);
        else   s_pressed_mask &= ~(1u << i);
        if (p) ESP_LOGW(TAG, "%s: 开机即按下 (检查硬件/焊接)", key_name(i));
    }

    for (;;) {
        /* 读原始电平并消抖: 连续 2 次一致才翻转 */
        for (int i = 0; i < KEY_MAX; i++) {
            uint8_t lvl = level_pressed(i, gpio_get_level(s_key_pins[i])) ? 1 : 0;
            raw[i] = (raw[i] << 1) | lvl;
            uint8_t v = (raw[i] & 0x03) == 0x03 ? 1 : ((raw[i] & 0x03) == 0x00 ? 0 : stable[i]);
            if (v != stable[i]) {
                stable[i] = v;
                if (v) s_pressed_mask |= (1u << i);
                else   s_pressed_mask &= ~(1u << i);
                key_event_t evt = { .key = i, .evt = v ? KEY_EVT_PRESS : KEY_EVT_RELEASE };
                xQueueSend(s_evt_queue, &evt, 0);
                ESP_LOGD(TAG, "%s %s", key_name(i), v ? "PRESS" : "RELEASE");
                if (v) { pressed_ms[i] = xTaskGetTickCount(); long_sent[i] = 0; }
            } else if (v) {
                uint32_t now = xTaskGetTickCount();
                if (!long_sent[i] && (now - pressed_ms[i]) >= pdMS_TO_TICKS(s_longpress_ms)) {
                    long_sent[i] = 1;
                    key_event_t evt = { .key = i, .evt = KEY_EVT_LONG_PRESS };
                    xQueueSend(s_evt_queue, &evt, 0);
                    ESP_LOGD(TAG, "%s LONG_PRESS", key_name(i));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));    /* 10ms 消抖窗口 = 2 次采样，UI 响应更快 */
    }
}

bool buttons_wait_event(key_event_t *evt, uint32_t timeout_ms)
{
    return xQueueReceive(s_evt_queue, evt, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void buttons_set_longpress_ms(uint32_t ms)
{
    s_longpress_ms = ms;
}

void buttons_flush_events(void)
{
    if (!s_evt_queue) return;
    key_event_t evt;
    while (xQueueReceive(s_evt_queue, &evt, 0) == pdTRUE) { }
}
