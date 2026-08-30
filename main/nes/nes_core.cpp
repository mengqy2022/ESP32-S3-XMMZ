/*
 * nes_core.cpp — NES 模拟器平台层 (nofrendo 内核)
 * 参考 retro-go main_nes.c 的做法, 适配本设备:
 *   - ROM: app 已载入 PSRAM, 用 rom_loadmem 交给 nofrendo
 *   - 显示: 双 8-bit vidbuf；Core1 只模拟，Core0 视频 worker 做 RGB565 转换 + LVGL flush
 *   - 声音: APU -> PSRAM ring -> Core0 I2S worker，与模拟帧解耦
 *   - 按键: 每个逻辑帧读手柄 (nes_host_pad)
 *   - 视频: 逻辑/APU 目标 60/50Hz，LCD 目标约 30/25Hz；显示忙时只丢显示帧，不阻塞逻辑
 * 内存: 大块 ROM/缓冲走 PSRAM；nofrendo 热点小块内存由 nofrendo_mem.h 优先放内部 SRAM.
 */
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "driver/i2s_std.h"

/* C 头文件: 必须 extern "C", 否则 C++ 名称修饰导致链接不到 */
#ifdef __cplusplus
extern "C" {
#endif
#include "audio_beep.h"
#include "nofrendo.h"
#include "nes/nes.h"
#include "nes/rom.h"
#include "nes/input.h"
#ifdef __cplusplus
}
#endif
#include "nes_core.h"

#define TAG "nes_sys"

#define NES_SAMPLE_RATE       22050
#define NES_FB_W              256
#define NES_FB_H              240
#define NES_VIDEO_FRAME_DIV   2   /* 逻辑 60/50Hz，显示目标 30/25Hz；显示不能反向拖慢逻辑 */
#define NES_VIDEO_SLOTS       2

/* nofrendo 的画面缓冲做成双缓冲：正在 LCD 转换/刷新的那一份绝不再交给 PPU 写。
 * 这样 Core1 的 CPU/PPU 不需要等待 Core0 的 RGB565 转换和 SPI/LVGL flush。 */
static uint8_t *s_vidbuf[NES_VIDEO_SLOTS] = {NULL, NULL};
static uint16_t s_palette[256];
static uint16_t *s_fb256 = NULL;          /* 视频 worker 的 RGB565 输出 (PSRAM) */
static nes_t *s_nes = NULL;
static bool s_core_inited = false;

enum {
    VIDEO_SLOT_FREE = 0,
    VIDEO_SLOT_DRAW = 1,
    VIDEO_SLOT_QUEUED = 2,
};
static volatile uint8_t s_video_state[NES_VIDEO_SLOTS] = {VIDEO_SLOT_FREE, VIDEO_SLOT_FREE};
static QueueHandle_t s_video_q = NULL;
static TaskHandle_t s_video_task = NULL;
static volatile bool s_video_run = false;
static volatile uint32_t s_video_presented = 0;
static volatile uint32_t s_video_drop = 0;
static int s_video_active_slot = -1;      /* 只由模拟 Core1/nes_blit 修改 */
static bool s_video_async = false;

static void video_convert_and_present(const uint8_t *vidbuf)
{
    if (!vidbuf || !s_fb256) return;
    for (int y = 0; y < NES_FB_H; y++) {
        const uint8_t *row = vidbuf + (size_t)y * NES_SCREEN_PITCH + NES_SCREEN_OVERDRAW;
        uint16_t *dst = s_fb256 + (size_t)y * NES_FB_W;
        for (int x = 0; x < NES_FB_W; x++) dst[x] = s_palette[row[x]];
    }
    /* app_nes.c 中只有这里会在 NES 运行期驱动 LVGL；主 UI 线程正阻塞等待 NES 结束。 */
    nes_host_render();
}

static void nes_video_worker(void *arg)
{
    (void)arg;
    for (;;) {
        uint8_t slot = 0;
        if (xQueueReceive(s_video_q, &slot, pdMS_TO_TICKS(10)) == pdTRUE) {
            if (slot < NES_VIDEO_SLOTS && s_vidbuf[slot]) {
                video_convert_and_present(s_vidbuf[slot]);
                __atomic_add_fetch(&s_video_presented, 1U, __ATOMIC_RELAXED);
                __atomic_store_n(&s_video_state[slot], VIDEO_SLOT_FREE, __ATOMIC_RELEASE);
            }
            continue;
        }
        if (!s_video_run && uxQueueMessagesWaiting(s_video_q) == 0) break;
    }
    s_video_task = NULL;
    vTaskDeleteWithCaps(NULL);
}

static bool nes_video_start(void)
{
    if (s_video_task || s_video_q) return true;
    for (int i = 0; i < NES_VIDEO_SLOTS; i++) {
        __atomic_store_n(&s_video_state[i], VIDEO_SLOT_FREE, __ATOMIC_RELAXED);
    }
    s_video_active_slot = -1;
    s_video_presented = 0;
    s_video_drop = 0;
    s_video_q = xQueueCreate(NES_VIDEO_SLOTS, sizeof(uint8_t));
    if (!s_video_q) return false;

    s_video_run = true;
    BaseType_t rc = xTaskCreatePinnedToCoreWithCaps(nes_video_worker, "nes_video", 6144,
                                                    NULL, 5, &s_video_task, 0,
                                                    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (rc != pdPASS) {
        s_video_run = false;
        s_video_task = NULL;
        vQueueDelete(s_video_q);
        s_video_q = NULL;
        return false;
    }
    return true;
}

static void nes_video_stop(void)
{
    if (!s_video_q) return;
    s_video_run = false;
    while (s_video_task) vTaskDelay(pdMS_TO_TICKS(1));
    vQueueDelete(s_video_q);
    s_video_q = NULL;
    s_video_active_slot = -1;
    for (int i = 0; i < NES_VIDEO_SLOTS; i++) {
        __atomic_store_n(&s_video_state[i], VIDEO_SLOT_FREE, __ATOMIC_RELAXED);
    }
}

/* 为下一次 PPU 绘制取得一个不被视频 worker 使用的 vidbuf；取不到就让本帧不绘制。
 * 重要：只丢“显示帧”，CPU/APU/输入逻辑仍继续跑，避免按键手感被 SPI 刷屏拖住。 */
static bool nes_video_acquire_draw_buffer(void)
{
    if (!s_video_async) return true;
    if (s_video_active_slot >= 0 &&
        __atomic_load_n(&s_video_state[s_video_active_slot], __ATOMIC_ACQUIRE) == VIDEO_SLOT_DRAW) {
        return true;
    }

    for (int i = 0; i < NES_VIDEO_SLOTS; i++) {
        uint8_t expected = VIDEO_SLOT_FREE;
        if (__atomic_compare_exchange_n(&s_video_state[i], &expected, VIDEO_SLOT_DRAW, false,
                                        __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
            s_video_active_slot = i;
            nes_setvidbuf(s_vidbuf[i]);
            return true;
        }
    }
    s_video_active_slot = -1;
    __atomic_add_fetch(&s_video_drop, 1U, __ATOMIC_RELAXED);
    return false;
}

/* ---------- blit 回调 ----------
 * 异步模式：只把完成的 8-bit vidbuf 所有权交给 Core0 worker，立即返回。
 * 回退模式：若视频任务创建失败，仍沿用同步转换/刷新，保证有画面。 */
static void nes_blit(uint8 *vidbuf)
{
    if (!vidbuf) return;
    if (!s_video_async || !s_video_q) {
        video_convert_and_present(vidbuf);
        return;
    }

    int slot = -1;
    for (int i = 0; i < NES_VIDEO_SLOTS; i++) {
        if (vidbuf == s_vidbuf[i]) { slot = i; break; }
    }
    if (slot < 0) return;

    uint8_t expected = VIDEO_SLOT_DRAW;
    if (!__atomic_compare_exchange_n(&s_video_state[slot], &expected, VIDEO_SLOT_QUEUED, false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        return;
    }
    s_video_active_slot = -1;
    uint8_t qslot = (uint8_t)slot;
    if (xQueueSend(s_video_q, &qslot, 0) != pdTRUE) {
        __atomic_store_n(&s_video_state[slot], VIDEO_SLOT_FREE, __ATOMIC_RELEASE);
        __atomic_add_fetch(&s_video_drop, 1U, __ATOMIC_RELAXED);
    }
}

/* ---------- 声音: APU -> lock-free mono ring -> 独立 I2S 任务 ----------
 * 模拟任务只把 APU 样本快速写入 PSRAM ring；Core0 音频任务独立按 I2S 实时时钟消费。 */
#define NES_AUDIO_RING_SAMPLES 4096U  /* power of two; ~186ms @22.05kHz，缩短异常堆积时的音频延迟 */
#define NES_AUDIO_RING_MASK    (NES_AUDIO_RING_SAMPLES - 1U)
#define NES_AUDIO_CHUNK        512
static int16_t *s_audio_ring = NULL;
static volatile uint32_t s_audio_wr = 0;
static volatile uint32_t s_audio_rd = 0;
static volatile bool s_audio_run = false;
static TaskHandle_t s_audio_task = NULL;
static int16_t s_audio_st[NES_AUDIO_CHUNK * 2]; /* internal/DMA-readable */
static uint32_t s_audio_drop_frames = 0;

static void nes_audio_worker(void *arg)
{
    (void)arg;
    i2s_chan_handle_t ch = audio_get_tx_handle();
    while (s_audio_run || s_audio_rd != s_audio_wr) {
        uint32_t rd = s_audio_rd;
        uint32_t wr = s_audio_wr;
        __sync_synchronize();
        uint32_t avail = wr - rd;
        if (avail == 0) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        uint32_t n = avail > NES_AUDIO_CHUNK ? NES_AUDIO_CHUNK : avail;
        for (uint32_t i = 0; i < n; i++) {
            int16_t m = s_audio_ring[(rd + i) & NES_AUDIO_RING_MASK];
            s_audio_st[2 * i] = m;
            s_audio_st[2 * i + 1] = m;
        }
        s_audio_rd = rd + n;
        audio_apply_volume(s_audio_st, (size_t)n * 2);

        if (ch) {
            size_t written = 0;
            esp_err_t err = i2s_channel_write(ch, s_audio_st, (size_t)n * 4,
                                              &written, portMAX_DELAY);
            if (err != ESP_OK) ESP_LOGW(TAG, "NES I2S write: %s", esp_err_to_name(err));
        }
    }
    s_audio_task = NULL;
    vTaskDeleteWithCaps(NULL);
}

static bool nes_audio_start(void)
{
    s_audio_ring = (int16_t *)heap_caps_malloc(NES_AUDIO_RING_SAMPLES * sizeof(int16_t),
                                                MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!s_audio_ring) return false;
    s_audio_wr = s_audio_rd = 0;
    s_audio_drop_frames = 0;
    s_audio_run = true;

    BaseType_t rc = xTaskCreatePinnedToCoreWithCaps(nes_audio_worker, "nes_audio", 6144,
                                                    NULL, 8, &s_audio_task, 0,
                                                    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (rc != pdPASS) {
        s_audio_run = false;
        s_audio_task = NULL;
        heap_caps_free(s_audio_ring);
        s_audio_ring = NULL;
        return false;
    }
    return true;
}

static void nes_audio_stop(void)
{
    s_audio_run = false;
    while (s_audio_task) vTaskDelay(pdMS_TO_TICKS(1));
    if (s_audio_ring) {
        heap_caps_free(s_audio_ring);
        s_audio_ring = NULL;
    }
}

static void nes_audio_submit(const int16_t *buf, int samples)
{
    if (!buf || samples <= 0 || !s_audio_ring) return;
    uint32_t wr = s_audio_wr;
    uint32_t rd = s_audio_rd;
    uint32_t used = wr - rd;
    if ((uint32_t)samples > NES_AUDIO_RING_SAMPLES - used) {
        s_audio_drop_frames++;
        return; /* never block emulation; audio worker will catch up */
    }
    for (int i = 0; i < samples; i++) {
        s_audio_ring[(wr + (uint32_t)i) & NES_AUDIO_RING_MASK] = buf[i];
    }
    __sync_synchronize();
    s_audio_wr = wr + (uint32_t)samples;
}

/* ---------- 平台接口 (供 app_nes.c) ---------- */
extern "C" int nes_core_load_rom(const uint8_t *rom, uint32_t len)
{
    if (!s_core_inited) {
        /* 双 8-bit PPU vidbuf + 单 RGB565 展示帧都放 PSRAM；热小块内核状态由
         * nofrendo_mem.h 选择内部 SRAM，避免每条 PPU/CPU 热路径都吃 PSRAM 延迟。 */
        for (int i = 0; i < NES_VIDEO_SLOTS; i++) {
            s_vidbuf[i] = (uint8_t *)heap_caps_malloc(NES_SCREEN_PITCH * NES_SCREEN_HEIGHT,
                                                       MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        }
        s_fb256 = (uint16_t *)heap_caps_malloc(NES_FB_W * NES_FB_H * 2,
                                               MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!s_vidbuf[0] || !s_vidbuf[1] || !s_fb256) {
            ESP_LOGE(TAG, "no PSRAM for video buffers");
            for (int i = 0; i < NES_VIDEO_SLOTS; i++) {
                if (s_vidbuf[i]) { heap_caps_free(s_vidbuf[i]); s_vidbuf[i] = NULL; }
            }
            if (s_fb256) { heap_caps_free(s_fb256); s_fb256 = NULL; }
            return -1;
        }
        uint16_t *pal = (uint16_t *)nofrendo_buildpalette(NES_PALETTE_PVM, 16);
        if (!pal) { ESP_LOGE(TAG, "palette fail"); return -1; }
        memcpy(s_palette, pal, 256 * 2);
        heap_caps_free(pal);

        s_nes = nes_init(SYS_DETECT, NES_SAMPLE_RATE, false, NULL);
        if (!s_nes) { ESP_LOGE(TAG, "nes_init fail"); return -1; }
        s_nes->blit_func = nes_blit;
        s_core_inited = true;
    }

    rom_t *cart = rom_loadmem((uint8_t *)rom, len);
    if (!cart) { ESP_LOGE(TAG, "rom_loadmem fail"); return -1; }
    if (nes_insertcart(cart) < 0) { ESP_LOGE(TAG, "insertcart fail"); return -1; }

    /* nes_insertcart 内部 reset 会清空 vidbuf，必须重新绑定。 */
    nes_setvidbuf(s_vidbuf[0]);

    nes_emulate(false);
    nes_emulate(false);
    ESP_LOGI(TAG, "NES loaded, mapper ok, refresh=%dHz", s_nes->refresh_rate);
    return 0;
}

extern "C" void nes_core_run(void)
{
    if (!s_nes) return;
    i2s_chan_handle_t ch = audio_get_tx_handle();
    if (audio_set_sample_rate(NES_SAMPLE_RATE) != ESP_OK) {
        ESP_LOGE(TAG, "set NES sample rate failed");
    }
    if (ch) (void)audio_tx_enable();

    bool async_audio = nes_audio_start();
    if (!async_audio) ESP_LOGW(TAG, "NES async audio unavailable; continuing muted");
    s_video_async = nes_video_start();
    if (!s_video_async) {
        ESP_LOGW(TAG, "NES async video unavailable; fallback synchronous");
        nes_setvidbuf(s_vidbuf[0]);
    }

    uint32_t frame_no = 0;
    const int refresh_rate = (s_nes->refresh_rate > 0) ? s_nes->refresh_rate : 60;
    const int64_t frame_period_us = 1000000LL / refresh_rate;
    int64_t next_deadline = esp_timer_get_time() + frame_period_us;
    ESP_LOGI(TAG, "run target: logic=%dHz audio=%dHz video<=%dHz, APU=%dHz asyncA=%d asyncV=%d",
             refresh_rate, refresh_rate, refresh_rate / NES_VIDEO_FRAME_DIV,
             NES_SAMPLE_RATE, async_audio ? 1 : 0, s_video_async ? 1 : 0);

    int64_t stat_t0 = esp_timer_get_time();
    uint32_t stat_logic = 0, stat_sync_video = 0;
    uint32_t last_presented = __atomic_load_n(&s_video_presented, __ATOMIC_RELAXED);

    for (;;) {
        /* 输入在每个逻辑帧最开头采样；app_nes.c 里 NES 游戏键走直接 GPIO 快速路径，
         * 不再依赖低优先级 10ms/20ms 菜单消抖任务。 */
        int quit = 0;
        uint32_t pad = nes_host_pad(&quit);
        if (quit) break;

        uint32_t buttons = 0;
        if (pad & (1u<<0)) buttons |= NES_PAD_A;
        if (pad & (1u<<1)) buttons |= NES_PAD_B;
        if (pad & (1u<<2)) buttons |= NES_PAD_SELECT;
        if (pad & (1u<<3)) buttons |= NES_PAD_START;
        if (pad & (1u<<4)) buttons |= NES_PAD_UP;
        if (pad & (1u<<5)) buttons |= NES_PAD_DOWN;
        if (pad & (1u<<6)) buttons |= NES_PAD_LEFT;
        if (pad & (1u<<7)) buttons |= NES_PAD_RIGHT;
        input_update(0, buttons);

        bool want_draw = ((frame_no % NES_VIDEO_FRAME_DIV) == 0);
        bool draw = want_draw && (s_video_async ? nes_video_acquire_draw_buffer() : true);
        nes_emulate(draw);
        stat_logic++;
        if (draw && !s_video_async) stat_sync_video++;

        if (async_audio) {
            nes_audio_submit((int16_t *)s_nes->apu->buffer, s_nes->apu->samples_per_frame);
        }
        frame_no++;

        /* 只在有余量时休眠；一旦落后就立刻跑下一逻辑帧。显示 worker 与这里并行，
         * 所以 SPI/LVGL 不再占用这条 16.67ms 的游戏逻辑时间预算。 */
        int64_t now = esp_timer_get_time();
        int64_t remain = next_deadline - now;
        if (remain > 1500) {
            TickType_t ticks = pdMS_TO_TICKS((uint32_t)((remain - 500) / 1000));
            if (ticks > 0) vTaskDelay(ticks);
        }
        now = esp_timer_get_time();
        next_deadline += frame_period_us;
        if (now - next_deadline > frame_period_us * 4) next_deadline = now + frame_period_us;

        if (now - stat_t0 >= 2000000) {
            uint64_t elapsed = (uint64_t)(now - stat_t0);
            uint32_t logic10 = (uint32_t)(((uint64_t)stat_logic * 10000000ULL) / elapsed);
            uint32_t video_frames;
            if (s_video_async) {
                uint32_t presented = __atomic_load_n(&s_video_presented, __ATOMIC_RELAXED);
                video_frames = presented - last_presented;
                last_presented = presented;
            } else {
                video_frames = stat_sync_video;
            }
            uint32_t video10 = (uint32_t)(((uint64_t)video_frames * 10000000ULL) / elapsed);
            uint32_t queued = s_audio_wr - s_audio_rd;
            ESP_LOGI(TAG,
                     "perf: logic=%u.%u fps, video=%u.%u fps, volume=%u%% audioQ=%lu adrop=%lu vdrop=%lu",
                     logic10 / 10, logic10 % 10, video10 / 10, video10 % 10,
                     (unsigned)audio_get_volume(), (unsigned long)queued,
                     (unsigned long)s_audio_drop_frames,
                     (unsigned long)__atomic_load_n(&s_video_drop, __ATOMIC_RELAXED));
            stat_t0 = now;
            stat_logic = stat_sync_video = 0;
        }
    }

    nes_video_stop();
    s_video_async = false;
    nes_audio_stop();
    if (ch) (void)audio_tx_disable();
    ESP_LOGI(TAG, "NES exit");
}

extern "C" void nes_core_release(void)
{
    /* 释放平台侧 PSRAM 缓冲；核心本身仍沿用原项目策略，不在这里调用 nes_shutdown。 */
    for (int i = 0; i < NES_VIDEO_SLOTS; i++) {
        if (s_vidbuf[i]) { heap_caps_free(s_vidbuf[i]); s_vidbuf[i] = NULL; }
    }
    if (s_fb256) { heap_caps_free(s_fb256); s_fb256 = NULL; }
    s_nes = NULL;
    s_core_inited = false;
}

extern "C" uint16_t *nes_core_get_frame(void) { return s_fb256; }
