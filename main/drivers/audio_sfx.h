/*
 * audio_sfx.h — 游戏音效 (短促合成音, 基于 audio_beep 滑音)
 */
#pragma once

typedef enum {
    SFX_CLICK,     /* 菜单选择 */
    SFX_MOVE,      /* 移动 (轻) */
    SFX_EAT,       /* 吃食物/吃分 */
    SFX_CLEAR,     /* 消行/清砖 */
    SFX_SHOOT,     /* 射击/发球 */
    SFX_HIT,       /* 命中/爆炸 */
    SFX_DIE,       /* 扣命/落底 */
    SFX_WIN,       /* 胜利 */
    SFX_GAMEOVER,  /* 游戏结束 */
    SFX_COUNT,
} sfx_id_t;

/* 播放音效 (阻塞短音 30-200ms) */
void sfx_play(sfx_id_t id);
