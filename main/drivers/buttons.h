/*
 * buttons.h — 按键驱动 (11 键, 低电平有效, 消抖 + 长按)
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    KEY_BOOT = 0,   /* GPIO0  */
    KEY_MENU,       /* GPIO18 */
    KEY_OPTION,     /* GPIO8  */
    KEY_SELECT,     /* GPIO16 */
    KEY_START,      /* GPIO17 */
    KEY_A,          /* GPIO15 */
    KEY_B,          /* GPIO5  */
    KEY_LEFT,       /* GPIO19 */
    KEY_RIGHT,      /* GPIO6  */
    KEY_UP,         /* GPIO7  */
    KEY_DOWN,       /* GPIO20 */
    KEY_MAX
} key_id_t;

typedef enum {
    KEY_EVT_PRESS,      /* 按下 (短按) */
    KEY_EVT_RELEASE,    /* 释放 */
    KEY_EVT_LONG_PRESS, /* 长按 (~700ms) */
} key_evt_t;

typedef struct {
    key_id_t key;
    key_evt_t evt;
} key_event_t;

/* 逻辑按键角色 (按用户定义的物理布局):
 *   物理 KEY1(菜单GPIO18)=主界面  KEY2(选项GPIO8)=设置/系统信息
 *   物理 KEY3(选择GPIO16)=返回    KEY4(开始GPIO17)=确定
 *   导航: 五向-上(GPIO7)=上翻, 五向-下(GPIO20)=下翻
 *   KEY5(A GPIO15)/KEY6(B GPIO5) = 预留 (游戏键等) */
#define KEY_CONFIRM  KEY_START
#define KEY_BACK     KEY_SELECT
#define KEY_HOME     KEY_MENU
#define KEY_SETTINGS KEY_OPTION

esp_err_t buttons_init(void);
bool buttons_get_state(key_id_t key);          /* 已消抖的稳定状态：菜单/UI 使用 */
bool buttons_get_state_fast(key_id_t key);     /* 低延迟原始 GPIO 状态：实时游戏使用 */
bool buttons_wait_event(key_event_t *evt, uint32_t timeout_ms);
void buttons_set_longpress_ms(uint32_t ms);
void buttons_flush_events(void);                  /* 清空历史按键事件队列 */
