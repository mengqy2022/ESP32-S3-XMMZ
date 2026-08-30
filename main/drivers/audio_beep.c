/*
 * audio_beep.c — 功放提示音
 * MAX98357A: SD_MODE# 经 100kΩ 接地 → 右声道, 常开; GAIN_SLOT 接地
 * I2S: 标准模式 (i2s_std), 16bit, 单声道数据放 RIGHT slot (slot_mask=I2S_STD_SLOT_RIGHT)
 */
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "audio_beep.h"
#include "board_config.h"

static const char *TAG = "audio";

/* 采样率 44100Hz: 音乐播放标准采样率, 提示音同样适用 (频率只与样本计数相关) */
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_CH 1

static i2s_chan_handle_t s_tx = NULL;
static uint32_t s_sample_rate = AUDIO_SAMPLE_RATE;
static volatile bool s_muted = false;
static volatile bool s_tx_enabled = false;
/* UI 仍使用 0-100，但 MAX98357A 是固定增益功放，数字满幅会非常响。
 * 默认降低到 35；audio_apply_volume() 再使用平方曲线并把 100% 限制为
 * 约 30% PCM 满幅（约 -10.5 dB），给小扬声器留出安全余量。 */
static volatile uint8_t s_volume = 35;

esp_err_t audio_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 6;
    chan_cfg.dma_frame_num = 240;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, &s_tx, NULL), TAG, "new i2s tx");

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_SAMPLE_RATE),
        /* Stereo 双声道输出: 左右声道放相同数据, 兼容 SD_MODE# 接左或右声道
         * (原理图 SD_MODE# 经 R12 100kΩ 上拉到 3V3M = 左声道模式;
         *  若只发单声道且声道选错 → 功放无声!) */
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = SPK_PIN_BCLK,
            .ws   = SPK_PIN_LRCK,
            .dout = SPK_PIN_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false, .bclk_inv = false, .ws_inv = false,
            },
        },
    };
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(s_tx, &std_cfg), TAG, "init std");

    /* 初始化后保持 disabled。提示音 / 音乐 / NES 在真正输出前再 enable，
     * 这样空闲时 BCLK/LRCK 不跑，也方便按音源动态切换采样率。 */
    s_sample_rate = AUDIO_SAMPLE_RATE;

    ESP_LOGI(TAG, "audio init OK (BCLK=%d LRCK=%d DOUT=%d, %dHz stereo)",
             SPK_PIN_BCLK, SPK_PIN_LRCK, SPK_PIN_DOUT, AUDIO_SAMPLE_RATE);
    return ESP_OK;
}

void audio_set_mute(bool mute)
{
    s_muted = mute;
}

bool audio_is_muted(void)
{
    return s_muted;
}

void audio_set_volume(uint8_t vol)
{
    if (vol > 100) vol = 100;
    s_volume = vol;
}

uint8_t audio_get_volume(void)
{
    return s_volume;
}

/* 供音乐播放器直接写入 PCM (int16 mono) */
i2s_chan_handle_t audio_get_tx_handle(void)
{
    return s_tx;
}

esp_err_t audio_tx_enable(void)
{
    if (!s_tx) return ESP_ERR_INVALID_STATE;
    if (s_tx_enabled) return ESP_OK;
    esp_err_t err = i2s_channel_enable(s_tx);
    if (err == ESP_OK) s_tx_enabled = true;
    return err;
}

esp_err_t audio_tx_disable(void)
{
    if (!s_tx || !s_tx_enabled) return ESP_OK;
    esp_err_t err = i2s_channel_disable(s_tx);
    if (err == ESP_OK) s_tx_enabled = false;
    return err;
}

uint32_t audio_get_sample_rate(void)
{
    return s_sample_rate;
}

esp_err_t audio_set_sample_rate(uint32_t sample_rate)
{
    if (!s_tx || sample_rate < 8000 || sample_rate > 96000) return ESP_ERR_INVALID_ARG;
    if (sample_rate == s_sample_rate) return ESP_OK;

    /* ESP-IDF 要求 reconfig 时 channel 处于 disabled；用幂等 wrapper 避免
     * 对已关闭通道调用 disable 所产生的误导性 ERROR 日志。 */
    ESP_RETURN_ON_ERROR(audio_tx_disable(), TAG, "disable before reconfig");
    i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate);
    esp_err_t err = i2s_channel_reconfig_std_clock(s_tx, &clk_cfg);
    if (err == ESP_OK) {
        s_sample_rate = sample_rate;
        ESP_LOGI(TAG, "I2S sample rate -> %lu Hz", (unsigned long)sample_rate);
    } else {
        ESP_LOGE(TAG, "I2S reconfig %lu Hz failed: %s",
                 (unsigned long)sample_rate, esp_err_to_name(err));
    }
    return err;
}

/* 音量衰减: 就地处理 int16 样本。
 *
 * MAX98357A 没有软件可控的模拟增益脚，若把 UI 的 80% 直接映射为 PCM 80%，
 * 小尺寸扬声器会非常响并容易出现“炸麦”听感。这里采用平方曲线：低音量区
 * 更细腻；即使 UI=100%，数字峰值也只到原始 PCM 的约 30%。 */
#define AUDIO_SAFE_MAX_GAIN_Q15  9830U   /* 0.30 * 32768 */
void audio_apply_volume(int16_t *buf, size_t n)
{
    if (!buf || n == 0) return;
    uint32_t v = s_volume;
    if (s_muted || v == 0) {
        memset(buf, 0, n * sizeof(int16_t));
        return;
    }

    uint32_t gain_q15 = (v * v * AUDIO_SAFE_MAX_GAIN_Q15 + 5000U) / 10000U;
    for (size_t i = 0; i < n; i++) {
        int32_t x = (int32_t)buf[i] * (int32_t)gain_q15;
        buf[i] = (int16_t)(x >> 15);
    }
}

esp_err_t audio_beep(uint16_t freq_hz, uint16_t dur_ms, uint8_t vol_pct)
{
    if (s_muted || vol_pct > 100) return ESP_OK;

    const int n = AUDIO_SAMPLE_RATE * dur_ms / 1000;   /* 每声道样本数 */
    int16_t *buf = heap_caps_malloc(n * 2 * sizeof(int16_t), MALLOC_CAP_DMA);   /* stereo */
    if (!buf) return ESP_ERR_NO_MEM;

    /* 音量 = 音效参数 × 系统音量 (受菜单 KEY5/BOOT 控制) */
    float amp = 32767.0f * vol_pct / 100.0f;
    /* 指数衰减包络, 避免爆音; 左右声道相同 */
    for (int i = 0; i < n; i++) {
        float t = (float)i / AUDIO_SAMPLE_RATE;
        float env = 1.0f - (float)i / n;
        int16_t s = (int16_t)(amp * env * sinf(2.0f * M_PI * freq_hz * t));
        buf[2 * i] = s;
        buf[2 * i + 1] = s;
    }
    audio_apply_volume(buf, (size_t)n * 2);

    /* NES/WAV 可能把 I2S 改成别的采样率，提示音统一切回 44.1kHz。 */
    ESP_RETURN_ON_ERROR(audio_set_sample_rate(AUDIO_SAMPLE_RATE), TAG, "beep sample rate");

    /* 播放前开启 I2S, 播完立即关闭: 空闲时零时钟零输出, 功放完全静音
     * (否则 I2S 使能时 BCLK/LRCK 持续产生, DOUT 残差被放大 → 持续嘟嘟声) */
    ESP_RETURN_ON_ERROR(audio_tx_enable(), TAG, "beep enable");
    size_t written = 0;
    esp_err_t werr = i2s_channel_write(s_tx, buf, n * 2 * sizeof(int16_t), &written, portMAX_DELAY);
    (void)audio_tx_disable();
    ESP_LOGI(TAG, "beep %dHz %dms vol%d sysvol%u -> write err=%s bytes=%d",
             freq_hz, dur_ms, vol_pct, (unsigned)s_volume, esp_err_to_name(werr), (int)written);
    heap_caps_free(buf);
    return ESP_OK;
}

/* 滑音: 频率从 f_start 线性滑到 f_end (游戏音效用, stereo) */
esp_err_t audio_tone_glide(uint16_t f_start, uint16_t f_end, uint16_t dur_ms, uint8_t vol_pct)
{
    if (s_muted || vol_pct > 100) return ESP_OK;

    const int n = AUDIO_SAMPLE_RATE * dur_ms / 1000;
    if (n <= 0) return ESP_OK;
    int16_t *buf = heap_caps_malloc(n * 2 * sizeof(int16_t), MALLOC_CAP_DMA);
    if (!buf) return ESP_ERR_NO_MEM;

    float amp = 32767.0f * vol_pct / 100.0f;
    float phase = 0.0f;
    for (int i = 0; i < n; i++) {
        float t = (float)i / n;
        float freq = f_start + (f_end - f_start) * t;
        float env = 1.0f - (float)i / n;
        phase += 2.0f * M_PI * freq / AUDIO_SAMPLE_RATE;
        int16_t s = (int16_t)(amp * env * sinf(phase));
        buf[2 * i] = s;
        buf[2 * i + 1] = s;
    }
    audio_apply_volume(buf, (size_t)n * 2);

    ESP_RETURN_ON_ERROR(audio_set_sample_rate(AUDIO_SAMPLE_RATE), TAG, "glide sample rate");
    ESP_RETURN_ON_ERROR(audio_tx_enable(), TAG, "glide enable");
    size_t written = 0;
    i2s_channel_write(s_tx, buf, n * 2 * sizeof(int16_t), &written, portMAX_DELAY);
    (void)audio_tx_disable();
    heap_caps_free(buf);
    return ESP_OK;
}
