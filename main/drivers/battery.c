/*
 * battery.c — 电池电压采样
 * 原理图: BAT -> 分压 (150k/830k ≈ 0.1807) -> GPIO4 (ADC1_CH3)
 * 校准: 用万用表实测电池电压, 调整 BAT_DIVIDER_RATIO
 * ⚠ 大坑: adc_oneshot_config_channel/read 的参数是"ADC 通道号"(adc_channel_t),
 *  不是 GPIO 编号! ESP32-S3: ADC1_CH3=GPIO4, ADC1_CH4=GPIO5(KEY6).
 *  早期代码误传 GPIO_NUM_4(=4) → 实际配置成 ADC_CHANNEL_4=GPIO5,
 *  把 KEY6 引脚改成了 ADC 模拟输入 → 按键测试 KEY6 一直"按下"且按了没反应
 *  (且电池读数一直在读按键脚, 电压全错). 必须用 ADC_CHANNEL_3 对应 GPIO4!
 */
#include "esp_adc/adc_oneshot.h"
#include "esp_check.h"
#include "esp_log.h"
#include "battery.h"
#include "board_config.h"

static const char *TAG = "battery";

/* GPIO4 在 ESP32-S3 上对应 ADC1 通道 3 (不是 4! 4 是 GPIO5) */
#define BAT_ADC_CHANNEL  ADC_CHANNEL_3

static adc_oneshot_unit_handle_t s_adc = NULL;
static int s_raw = 0;

esp_err_t battery_init(void)
{
    adc_oneshot_unit_init_cfg_t ucfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&ucfg, &s_adc), TAG, "new adc unit");

    adc_oneshot_chan_cfg_t ccfg = {
        .atten = ADC_ATTEN_DB_12,       /* 0-3.1V */
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(s_adc, BAT_ADC_CHANNEL, &ccfg), TAG, "config chan");

    /* 首次采样预热 */
    adc_oneshot_read(s_adc, BAT_ADC_CHANNEL, &s_raw);
    ESP_LOGI(TAG, "battery init OK (GPIO%d)", BAT_PIN_ADC);
    return ESP_OK;
}

float battery_voltage(void)
{
    int raw = 0;
    adc_oneshot_read(s_adc, BAT_ADC_CHANNEL, &raw);
    if (raw < 0) raw = s_raw;
    s_raw = raw;
    /* ADC 12bit, 衰减 12dB 量程约 3.1V */
    float v_adc = raw * 3.1f / 4095.0f;
    return v_adc / BAT_DIVIDER_RATIO;
}

uint8_t battery_percent(void)
{
    float v = battery_voltage();
    if (v >= BAT_FULL_V) return 100;
    if (v <= BAT_EMPTY_V) return 0;
    return (uint8_t)((v - BAT_EMPTY_V) / (BAT_FULL_V - BAT_EMPTY_V) * 100.0f);
}
