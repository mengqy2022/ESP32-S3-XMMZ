/*
 * lvgl_port.h — LVGL 显示/输入移植层 (esp_lvgl_port + 按键)
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t lvgl_sys_init(void);   /* 初始化 LVGL 显示 + 输入 (app_main 调用) */
void lvgl_input_set_enabled(bool enabled); /* 独占游戏输入时临时禁用 LVGL keypad */
