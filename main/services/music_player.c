/*
 * music_player.c — 音乐播放器
 * 数据源: SD 卡 MP3 / WAV(PCM16) 文件，或网络 MP3 URL
 * 播放线程: 解码/读取 PCM → 混音单声道 → stereo → 音量 → I2S(MAX98357A)
 *
 * 说明:
 * - MP4/M4A 通常是 AAC + MP4 容器，本工程没有 AAC/MP4 demux/decoder，不直接播放。
 * - 推荐 SD 音频使用 MP3，或 WAV PCM signed 16-bit little-endian (mono/stereo)。
 */
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "driver/i2s_std.h"
#include "audio_beep.h"
#include "music_player.h"
#include "wifi_mgr.h"
#include "safe_string.h"

/* helix MP3 解码器 (components/mp3dec) */
#include "mp3dec.h"

static const char *TAG = "music";

#define INBUF_SIZE   2048        /* MP3 输入缓冲 */
#define OUT_SAMPS    1152        /* 每帧每声道最大 PCM frame 数 */

static volatile mp_state_t s_state = MP_IDLE;
static volatile bool s_stop_req = false;
static volatile bool s_pause_req = false;
static char s_src[160];          /* 当前源: 路径或 URL */
static char s_err[96];
static TaskHandle_t s_task = NULL;
static volatile uint32_t s_completed_count = 0;

/* Helix 最大输出 = 1152 samples/channel * 2ch。WAV 也复用该缓冲。 */
static int16_t s_pcm[OUT_SAMPS * 2];
static uint8_t s_inbuf[INBUF_SIZE + 4];

const char *mp_source(void) { return s_src; }
mp_state_t mp_state(void) { return s_state; }
const char *mp_error_str(void) { return s_err[0] ? s_err : "(无)"; }
uint32_t mp_completed_count(void) { return s_completed_count; }

static void mp_set_err(const char *msg)
{
    snprintf(s_err, sizeof(s_err), "%s", msg);
    s_state = MP_ERROR;
    ESP_LOGE(TAG, "%s", msg);
}

/* stereo 交织 → mono，结果写回 pcm[0..frames-1] */
static void downmix(int16_t *pcm, int chans, int frames)
{
    if (chans == 1) return;
    for (int i = 0; i < frames; i++) {
        int32_t l = pcm[2 * i], r = pcm[2 * i + 1];
        pcm[i] = (int16_t)((l + r) >> 1);
    }
}

/* mono pcm[0..frames-1] → stereo，就地从尾部展开，避免覆盖尚未读取的 mono 数据。 */
static void mono_to_stereo(int16_t *pcm, int frames)
{
    for (int i = frames - 1; i >= 0; i--) {
        int16_t m = pcm[i];
        pcm[2 * i] = m;
        pcm[2 * i + 1] = m;
    }
}

static esp_err_t audio_stream_enable_rate(uint32_t rate, bool *enabled)
{
    if (!enabled) return ESP_ERR_INVALID_ARG;

    if (audio_get_sample_rate() != rate) {
        if (*enabled) {
            (void)audio_tx_disable();
            *enabled = false;
        }
        esp_err_t err = audio_set_sample_rate(rate);
        if (err != ESP_OK) return err;
    }

    if (!*enabled) {
        esp_err_t err = audio_tx_enable();
        if (err != ESP_OK) return err;
        *enabled = true;
    }
    return ESP_OK;
}

/* 数据源: 0=文件, 1=网络 */
typedef struct {
    int type;
    FILE *fp;
    esp_http_client_handle_t client;
    bool eof;
    int err_cnt;   /* 网络连续错误计数 */
} mp_src_t;

static int src_read(mp_src_t *s, uint8_t *buf, int len)
{
    if (s->type == 0) {
        size_t rd = fread(buf, 1, (size_t)len, s->fp);
        if (rd == 0) s->eof = true;
        return (int)rd;
    }
    int rd = esp_http_client_read(s->client, (char *)buf, len);
    if (rd == 0) s->eof = true;
    else if (rd < 0) {
        if (++s->err_cnt > 5) { s->eof = true; return -1; }
        return 0;
    } else {
        s->err_cnt = 0;
    }
    return rd;
}

static uint16_t rd_le16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t rd_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}


/* Skip ID3v2 metadata before giving a local MP3 to Helix. Album art/lyrics can easily be
 * tens or hundreds of KB; the old byte-by-byte resync gave up after 8192 failed bytes and
 * therefore reported perfectly valid tagged MP3 files as "not a valid MP3 stream". */
static void mp3_skip_id3v2(FILE *fp)
{
    if (!fp) return;
    long start = ftell(fp);
    if (start < 0) start = 0;
    uint8_t h[10];
    if (fread(h, 1, sizeof(h), fp) != sizeof(h)) {
        (void)fseek(fp, start, SEEK_SET);
        return;
    }
    if (memcmp(h, "ID3", 3) != 0 ||
        ((((uint32_t)h[6] | h[7] | h[8] | h[9]) & 0x80U) != 0U)) {
        (void)fseek(fp, start, SEEK_SET);
        return;
    }

    uint32_t tag_size = ((uint32_t)h[6] << 21) | ((uint32_t)h[7] << 14) |
                        ((uint32_t)h[8] << 7) | (uint32_t)h[9];
    uint32_t total = 10U + tag_size + ((h[5] & 0x10U) ? 10U : 0U); /* optional v2.4 footer */
    if (total > (32U * 1024U * 1024U)) { /* corrupt/suspicious tag */
        (void)fseek(fp, start, SEEK_SET);
        return;
    }
    if (fseek(fp, start + (long)total, SEEK_SET) == 0) {
        ESP_LOGI(TAG, "skip ID3v2 tag: %lu bytes", (unsigned long)total);
    } else {
        (void)fseek(fp, start, SEEK_SET);
    }
}

typedef struct {
    uint16_t format;
    uint16_t channels;
    uint32_t sample_rate;
    uint16_t bits_per_sample;
    uint32_t data_size;
} wav_info_t;

/* 解析 RIFF/WAVE；返回时文件指针位于 data chunk 首字节。 */
static esp_err_t wav_parse(FILE *fp, wav_info_t *wi)
{
    uint8_t h[12];
    if (!fp || !wi || fread(h, 1, sizeof(h), fp) != sizeof(h)) return ESP_FAIL;
    if (memcmp(h, "RIFF", 4) != 0 || memcmp(h + 8, "WAVE", 4) != 0) return ESP_ERR_INVALID_RESPONSE;

    memset(wi, 0, sizeof(*wi));
    bool got_fmt = false;
    for (int chunks = 0; chunks < 64; chunks++) {
        uint8_t ch[8];
        if (fread(ch, 1, sizeof(ch), fp) != sizeof(ch)) break;
        uint32_t sz = rd_le32(ch + 4);

        if (memcmp(ch, "fmt ", 4) == 0) {
            if (sz < 16) return ESP_ERR_INVALID_RESPONSE;
            uint8_t fmt[16];
            if (fread(fmt, 1, sizeof(fmt), fp) != sizeof(fmt)) return ESP_FAIL;
            wi->format = rd_le16(fmt + 0);
            wi->channels = rd_le16(fmt + 2);
            wi->sample_rate = rd_le32(fmt + 4);
            wi->bits_per_sample = rd_le16(fmt + 14);
            got_fmt = true;
            uint32_t remain = sz - 16;
            if (remain && fseek(fp, (long)remain, SEEK_CUR) != 0) return ESP_FAIL;
        } else if (memcmp(ch, "data", 4) == 0) {
            if (!got_fmt) return ESP_ERR_INVALID_RESPONSE;
            wi->data_size = sz;
            return ESP_OK;
        } else {
            if (sz && fseek(fp, (long)sz, SEEK_CUR) != 0) return ESP_FAIL;
        }

        /* RIFF chunks are word-aligned. */
        if (sz & 1U) {
            if (fseek(fp, 1, SEEK_CUR) != 0) return ESP_FAIL;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

static esp_err_t play_wav_file(mp_src_t *src, bool *i2s_enabled)
{
    wav_info_t wi;
    esp_err_t err = wav_parse(src->fp, &wi);
    if (err != ESP_OK) {
        mp_set_err("WAV header invalid");
        return err;
    }
    if (wi.format != 1 || wi.bits_per_sample != 16 || (wi.channels != 1 && wi.channels != 2)) {
        mp_set_err("WAV must be PCM16 mono/stereo");
        return ESP_ERR_NOT_SUPPORTED;
    }
    if (wi.sample_rate < 8000 || wi.sample_rate > 96000) {
        mp_set_err("WAV sample rate unsupported");
        return ESP_ERR_NOT_SUPPORTED;
    }

    err = audio_stream_enable_rate(wi.sample_rate, i2s_enabled);
    if (err != ESP_OK) {
        mp_set_err("I2S sample rate failed");
        return err;
    }

    ESP_LOGI(TAG, "WAV PCM16 %luHz %uch data=%lu",
             (unsigned long)wi.sample_rate, (unsigned)wi.channels, (unsigned long)wi.data_size);

    uint32_t left = wi.data_size;
    const uint32_t bytes_per_frame = (uint32_t)wi.channels * sizeof(int16_t);
    bool played = false;

    while (!s_stop_req && left >= bytes_per_frame) {
        while (s_pause_req && !s_stop_req) vTaskDelay(pdMS_TO_TICKS(20));
        if (s_stop_req) break;

        uint32_t frames = left / bytes_per_frame;
        if (frames > OUT_SAMPS) frames = OUT_SAMPS;
        size_t want = (size_t)frames * bytes_per_frame;
        size_t rd = fread(s_pcm, 1, want, src->fp);
        if (rd < bytes_per_frame) break;
        frames = (uint32_t)(rd / bytes_per_frame);
        left -= (uint32_t)rd;

        downmix(s_pcm, wi.channels, (int)frames);
        mono_to_stereo(s_pcm, (int)frames);
        audio_apply_volume(s_pcm, (size_t)frames * 2);

        size_t written = 0;
        err = i2s_channel_write(audio_get_tx_handle(), s_pcm,
                                (size_t)frames * 2 * sizeof(int16_t),
                                &written, pdMS_TO_TICKS(500));
        if (err != ESP_OK) {
            mp_set_err("I2S write failed");
            return err;
        }
        if (!played) {
            played = true;
            s_state = MP_PLAYING;
        }
        if (rd < want) break;
    }

    if (!played && !s_stop_req) {
        mp_set_err("WAV contains no PCM data");
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t play_mp3_stream(mp_src_t *src, bool *i2s_enabled)
{
    HMP3Decoder dec = MP3InitDecoder();
    if (!dec) {
        mp_set_err("MP3 decoder init failed");
        return ESP_ERR_NO_MEM;
    }

    MP3FrameInfo fi;
    uint8_t *inbuf = s_inbuf;
    int bytes_left = 0;
    int good_frames = 0;
    int decode_errors = 0;
    uint32_t skipped_bytes = 0;
    esp_err_t ret = ESP_OK;

    while (!s_stop_req) {
        while (s_pause_req && !s_stop_req) vTaskDelay(pdMS_TO_TICKS(20));
        if (s_stop_req) break;

        /* Compact remaining bytes and refill. Keeping at least one trailing byte allows a
         * 0xFFEx sync word split across read boundaries to be found on the next pass. */
        if (!src->eof && bytes_left < 1200) {
            if (inbuf != s_inbuf && bytes_left > 0) memmove(s_inbuf, inbuf, (size_t)bytes_left);
            inbuf = s_inbuf;
            int want = INBUF_SIZE - bytes_left;
            int rd = src_read(src, s_inbuf + bytes_left, want);
            if (rd > 0) bytes_left += rd;
            else if (rd < 0) { mp_set_err("source read failed"); ret = ESP_FAIL; break; }
            else if (!src->eof) { vTaskDelay(pdMS_TO_TICKS(5)); continue; }
        }

        if (bytes_left <= 1 && src->eof) break;

        int sync = MP3FindSyncWord(inbuf, bytes_left);
        if (sync < 0) {
            if (src->eof) break;
            if (bytes_left > 0) {
                uint8_t tail = inbuf[bytes_left - 1];
                skipped_bytes += (uint32_t)(bytes_left - 1);
                s_inbuf[0] = tail;
                inbuf = s_inbuf;
                bytes_left = 1;
            }
            continue;
        }
        if (sync > 0) {
            inbuf += sync;
            bytes_left -= sync;
            skipped_bytes += (uint32_t)sync;
        }

        uint8_t *frame_start = inbuf;
        int bytes_before = bytes_left;
        uint8_t *p = inbuf;
        int errc = MP3Decode(dec, &p, &bytes_left, s_pcm, 0);

        if (errc == ERR_MP3_NONE) {
            inbuf = p;
            MP3GetLastFrameInfo(dec, &fi);
            int chans = fi.nChans;
            int total = fi.outputSamps;
            if ((chans != 1 && chans != 2) || total <= 0 || total > OUT_SAMPS * 2 || (total % chans) != 0) {
                mp_set_err("MP3 frame format invalid");
                ret = ESP_ERR_INVALID_RESPONSE;
                break;
            }
            int frames = total / chans;

            esp_err_t aerr = audio_stream_enable_rate((uint32_t)fi.samprate, i2s_enabled);
            if (aerr != ESP_OK) {
                mp_set_err("I2S sample rate failed");
                ret = aerr;
                break;
            }

            downmix(s_pcm, chans, frames);
            mono_to_stereo(s_pcm, frames);
            audio_apply_volume(s_pcm, (size_t)frames * 2);

            size_t written = 0;
            aerr = i2s_channel_write(audio_get_tx_handle(), s_pcm,
                                     (size_t)frames * 2 * sizeof(int16_t),
                                     &written, pdMS_TO_TICKS(500));
            if (aerr != ESP_OK) {
                mp_set_err("I2S write failed");
                ret = aerr;
                break;
            }
            if (good_frames++ == 0) {
                s_state = MP_PLAYING;
                ESP_LOGI(TAG, "MP3 playing %dHz %dch bitrate=%d (skipped=%lu)",
                         fi.samprate, fi.nChans, fi.bitrate, (unsigned long)skipped_bytes);
            }
            decode_errors = 0;
        } else if (errc == ERR_MP3_INDATA_UNDERFLOW) {
            /* This frame is incomplete: MP3Decode consumed only header/side-info before it
             * discovered that nSlots exceeds bytesLeft. Restore the frame start and refill. */
            inbuf = frame_start;
            bytes_left = bytes_before;
            if (src->eof) break;
            if (inbuf != s_inbuf && bytes_left > 0) memmove(s_inbuf, inbuf, (size_t)bytes_left);
            inbuf = s_inbuf;
            if (bytes_left >= INBUF_SIZE) { /* impossible/false sync: advance one byte */
                inbuf++;
                bytes_left--;
                skipped_bytes++;
            }
        } else if (errc == ERR_MP3_MAINDATA_UNDERFLOW) {
            /* Helix has consumed this complete frame into its bit reservoir. This is normal
             * when playback starts at a frame whose main_data_begin refers to earlier audio.
             * Keep the advanced pointer and continue; do NOT decode the same frame twice. */
            inbuf = p;
            decode_errors++;
        } else {
            /* False sync or damaged frame: advance one byte and let MP3FindSyncWord locate
             * the next candidate efficiently. Unlike the old implementation, there is no
             * arbitrary 8 KB metadata limit. */
            inbuf = frame_start + 1;
            bytes_left = bytes_before - 1;
            skipped_bytes++;
            decode_errors++;
            if (decode_errors > 256 && good_frames == 0 && skipped_bytes > (2U * 1024U * 1024U)) {
                mp_set_err("MP3 sync not found (check file format)");
                ret = ESP_ERR_INVALID_RESPONSE;
                break;
            }
        }
    }

    MP3FreeDecoder(dec);
    if (good_frames == 0 && !s_stop_req && s_state != MP_ERROR) {
        mp_set_err("no MP3 audio frame (may be AAC/renamed file)");
        ret = ESP_ERR_INVALID_RESPONSE;
    }
    return ret;
}

static bool path_ext_is(const char *path, const char *ext)
{
    const char *dot = path ? strrchr(path, '.') : NULL;
    return dot && strcasecmp(dot, ext) == 0;
}

static void mp_task(void *arg)
{
    mp_src_t src;
    memset(&src, 0, sizeof(src));
    src.type = (int)(intptr_t)arg;
    bool i2s_enabled = false;
    esp_err_t play_err = ESP_OK;

    /* 打开数据源 */
    if (src.type == 0) {
        src.fp = fopen(s_src, "rb");
        if (!src.fp) {
            mp_set_err("open file failed");
            goto done;
        }
        ESP_LOGI(TAG, "open file: %s", s_src);
        if (path_ext_is(s_src, ".mp3")) mp3_skip_id3v2(src.fp);
    } else {
        esp_http_client_config_t cfg = {
            .url = s_src,
            .timeout_ms = 1000,
            .buffer_size = 4096,
            .keep_alive_enable = false,
        };
        src.client = esp_http_client_init(&cfg);
        if (!src.client) {
            mp_set_err("http init failed");
            goto done;
        }
        if (esp_http_client_open(src.client, 0) != ESP_OK) {
            mp_set_err("connect failed");
            goto done;
        }
        esp_http_client_fetch_headers(src.client);
        int st = esp_http_client_get_status_code(src.client);
        ESP_LOGI(TAG, "http status %d, url=%s", st, s_src);
        if (st != 200) {
            mp_set_err("http status != 200");
            goto done;
        }
        wifi_mgr_set_power_save(false);   /* 网络播放: 关省电防断流 */
    }

    s_state = MP_LOADING;
    if (src.type == 0 && (path_ext_is(s_src, ".wav") || path_ext_is(s_src, ".wave"))) {
        play_err = play_wav_file(&src, &i2s_enabled);
    } else if (src.type == 1 || path_ext_is(s_src, ".mp3")) {
        play_err = play_mp3_stream(&src, &i2s_enabled);
    } else {
        mp_set_err("unsupported audio: use MP3/WAV");
        play_err = ESP_ERR_NOT_SUPPORTED;
    }

 done:
    /* Snapshot natural completion before cleanup changes shared state. A user stop / manual
     * previous-next never increments this counter, so the UI can auto-advance without races. */
    bool natural_eof = (!s_stop_req && play_err == ESP_OK && s_state != MP_ERROR);

    if (i2s_enabled) (void)audio_tx_disable();
    if (src.fp) fclose(src.fp);
    if (src.client) esp_http_client_cleanup(src.client);
    if (src.type == 1) wifi_mgr_set_power_save(true);

    if (natural_eof) s_completed_count++;
    if (s_state != MP_ERROR) s_state = MP_STOPPED;
    s_task = NULL;
    ESP_LOGI(TAG, "task exit: %s%s", esp_err_to_name(play_err), natural_eof ? " (eof)" : "");
    vTaskDeleteWithCaps(NULL);
}

esp_err_t mp_init(void)
{
    /* 不主动打断正在播放的任务；首次调用时清空状态。 */
    if (!s_task) {
        s_state = MP_IDLE;
        s_src[0] = 0;
        s_err[0] = 0;
    }
    return ESP_OK;
}

static esp_err_t mp_start(int type)
{
    if (s_task) mp_stop();
    s_err[0] = 0;
    s_stop_req = false;
    s_pause_req = false;
    s_state = MP_LOADING;
    /* 解码任务需要较大栈，优先放 PSRAM，避免挤占 NES/LVGL/WiFi 所需内部 SRAM。 */
    BaseType_t rc = xTaskCreatePinnedToCoreWithCaps(mp_task, "music", 16384,
                                                    (void *)(intptr_t)type, 5, &s_task, tskNO_AFFINITY,
                                                    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (rc != pdPASS) {
        ESP_LOGW(TAG, "PSRAM music stack failed, fallback internal");
        rc = xTaskCreatePinnedToCoreWithCaps(mp_task, "music", 16384,
                                             (void *)(intptr_t)type, 5, &s_task, tskNO_AFFINITY,
                                             MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    if (rc != pdPASS) {
        s_task = NULL;
        mp_set_err("music task create failed");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

esp_err_t mp_play_file(const char *path)
{
    if (!path || !path[0]) return ESP_ERR_INVALID_ARG;
    /* 切歌时先让旧任务完全退出，再改共享 s_src，避免旧任务尚未打开数据源时
     * 误读到新路径。对快速上一首/下一首尤其重要。 */
    if (s_task) mp_stop();
    xm_strlcpy(s_src, path, sizeof(s_src));
    ESP_LOGI(TAG, "play file %s", s_src);
    return mp_start(0);
}

esp_err_t mp_play_url(const char *url)
{
    if (!url || !url[0]) return ESP_ERR_INVALID_ARG;
    if (s_task) mp_stop();
    xm_strlcpy(s_src, url, sizeof(s_src));
    ESP_LOGI(TAG, "play url %s", s_src);
    return mp_start(1);
}

void mp_stop(void)
{
    if (!s_task) {
        if (s_state != MP_ERROR) s_state = MP_STOPPED;
        return;
    }
    s_stop_req = true;
    s_pause_req = false;
    for (int i = 0; i < 250 && s_task; i++) vTaskDelay(pdMS_TO_TICKS(10));
    if (s_state != MP_ERROR) s_state = MP_STOPPED;
}

void mp_pause(void)
{
    if (s_state == MP_PLAYING || s_state == MP_LOADING) {
        s_pause_req = true;
        s_state = MP_PAUSED;
    }
}

void mp_resume(void)
{
    if (s_state == MP_PAUSED) {
        s_pause_req = false;
        s_state = MP_PLAYING;
    }
}
