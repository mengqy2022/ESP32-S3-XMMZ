/*
 * audio_beep.h — 功放音频输出 (MAX98357A, I2S 标准模式)
 * 支持提示音 + 音乐 PCM 播放；I2S 采样率可按音源动态切换。
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2s_std.h"

esp_err_t audio_init(void);
esp_err_t audio_beep(uint16_t freq_hz, uint16_t dur_ms, uint8_t vol_pct);
esp_err_t audio_tone_glide(uint16_t f_start, uint16_t f_end, uint16_t dur_ms, uint8_t vol_pct);
void audio_set_mute(bool mute);
bool audio_is_muted(void);
void audio_set_volume(uint8_t vol);   /* 0-100 软件音量 */
uint8_t audio_get_volume(void);
i2s_chan_handle_t audio_get_tx_handle(void);   /* 供播放器写 PCM */
esp_err_t audio_tx_enable(void);                 /* 幂等开启 I2S TX */
esp_err_t audio_tx_disable(void);                /* 幂等关闭 I2S TX */
void audio_apply_volume(int16_t *buf, size_t n);   /* 就地音量衰减 */
esp_err_t audio_set_sample_rate(uint32_t sample_rate); /* 调整 I2S 时钟；返回时通道保持 disabled */
uint32_t audio_get_sample_rate(void);
