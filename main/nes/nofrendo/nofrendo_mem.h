/*
 * nofrendo_mem.h — nofrendo 核心内存分配策略
 * 通过 CMake 的 -include 强制包含。
 *
 * v3 把所有 malloc/calloc 都强制放 PSRAM，虽然省内部 SRAM，但 PPU nametable、
 * APU buffer、dummy page 等每帧高频访问的小块内存也因此承受 PSRAM cache 延迟。
 * v4 改成：<=4KB 的热点小块优先内部 SRAM，失败自动回退 PSRAM；大 ROM/缓冲仍放 PSRAM。
 */
#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_heap_caps.h"

#define NOFRENDO_INTERNAL_MAX 4096U

static inline void *nofrendo_malloc_impl(size_t sz)
{
    size_t ask = sz ? sz : 1;
    if (ask <= NOFRENDO_INTERNAL_MAX) {
        void *p = heap_caps_malloc(ask, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (p) return p;
    }
    return heap_caps_malloc(ask, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

static inline void *nofrendo_calloc_impl(size_t n, size_t sz)
{
    if (n == 0 || sz == 0) return heap_caps_calloc(1, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (sz > SIZE_MAX / n) return NULL;
    size_t total = n * sz;
    if (total <= NOFRENDO_INTERNAL_MAX) {
        void *p = heap_caps_calloc(n, sz, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (p) return p;
    }
    return heap_caps_calloc(n, sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

static inline void *nofrendo_realloc_impl(void *ptr, size_t sz)
{
    size_t ask = sz ? sz : 1;
    if (ask <= NOFRENDO_INTERNAL_MAX) {
        void *p = heap_caps_realloc(ptr, ask, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (p) return p;
    }
    return heap_caps_realloc(ptr, ask, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

#define malloc(sz)         nofrendo_malloc_impl((sz))
#define calloc(n, sz)      nofrendo_calloc_impl((n), (sz))
#define realloc(p, sz)     nofrendo_realloc_impl((p), (sz))
#define free(p)            heap_caps_free((p))
