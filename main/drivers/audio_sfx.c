/*
 * audio_sfx.c — 游戏音效 (合成短音, 不占存储)
 */
#include "audio_beep.h"
#include "audio_sfx.h"

void sfx_play(sfx_id_t id)
{
    switch (id) {
    case SFX_CLICK:
        audio_beep(880, 40, 35);            /* 清脆选择音 */
        break;
    case SFX_MOVE:
        audio_beep(660, 20, 18);            /* 很轻的移动声 */
        break;
    case SFX_EAT:
        audio_tone_glide(900, 1500, 70, 40); /* 上行"叮" 吃食物 */
        break;
    case SFX_CLEAR:
        audio_beep(700, 50, 40);
        audio_beep(1050, 60, 40);           /* 双音上行 消行/清砖 */
        break;
    case SFX_SHOOT:
        audio_tone_glide(1200, 800, 60, 30); /* 下行"咻" 射击/发球 */
        break;
    case SFX_HIT:
        audio_beep(300, 50, 40);            /* 低沉"咚" 命中爆炸 */
        break;
    case SFX_DIE:
        audio_tone_glide(500, 180, 180, 45); /* 下行长音 扣命 */
        break;
    case SFX_WIN:
        audio_beep(660, 70, 45);
        audio_beep(880, 70, 45);
        audio_beep(1320, 120, 45);          /* 胜利三连音 */
        break;
    case SFX_GAMEOVER:
        audio_tone_glide(700, 200, 300, 45); /* 叹息下行 */
        break;
    default:
        break;
    }
}
