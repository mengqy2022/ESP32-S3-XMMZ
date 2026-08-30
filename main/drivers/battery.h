/*
 * battery.h — 电池电压采样 (ADC1_CH3, GPIO4)
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"

esp_err_t battery_init(void);
float battery_voltage(void);        /* V */
uint8_t battery_percent(void);      /* 0-100 */
