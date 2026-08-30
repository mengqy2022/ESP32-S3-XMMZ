/*
 * music_player.h — 音乐播放器 (SD 卡 MP3/WAV + 局域网 MP3 备用)
 * MP3: Helix 解码；WAV: PCM16 直读；统一混音为 stereo → I2S(MAX98357A)
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"

typedef enum {
    MP_IDLE = 0,   /* 空闲 */
    MP_LOADING,    /* 打开/缓冲中 */
    MP_PLAYING,    /* 播放中 */
    MP_PAUSED,     /* 已暂停 */
    MP_STOPPED,    /* 已停止 */
    MP_ERROR,      /* 出错 */
} mp_state_t;

esp_err_t mp_init(void);
/* 播放 SD 卡文件 (MP3 或 WAV PCM16), 播放中自动停止旧的 */
esp_err_t mp_play_file(const char *path);
/* 播放网络 URL (备用) */
esp_err_t mp_play_url(const char *url);
void mp_stop(void);
void mp_pause(void);
void mp_resume(void);
mp_state_t mp_state(void);
const char *mp_source(void);       /* 当前播放源 (文件名或 URL) */
const char *mp_error_str(void);
/* Monotonic counter incremented only when a track reaches natural EOF successfully. */
uint32_t mp_completed_count(void);
