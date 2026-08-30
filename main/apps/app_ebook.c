/*
 * app_ebook.c — 电子书
 *
 * 书源: 内置 (books_data.c) + SD 卡 /sdcard/books 下的 .txt
 * 编码: UTF-8 / UTF-8 BOM / UTF-16 LE/BE / GBK(GB2312 常用区)
 * 阅读: 流式分页、按页读取，不再把整本大书一次性载入 PSRAM
 * 记忆: NVS 按“每本书”独立保存页码；兼容旧版单书签数据
 * 列表: 5 行虚拟列表，书很多时仍保持低 LVGL 对象数
 *
 * 按键: 上/下=翻页或选书, KEY4=打开, KEY3/KEY1=返回并保存进度
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "nvs.h"
#include "lvgl.h"
#include "buttons.h"
#include "lvgl_ui.h"
#include "apps.h"
#include "sd_card.h"
#include "books_data.h"
#include "font_data.h"
#include "safe_string.h"

static const char *TAG = "ebook";

LV_FONT_DECLARE(book_font_lvgl);
LV_FONT_DECLARE(ui_font_lvgl_10);

#define BOOK_VISIBLE_ROWS  5
#define BOOK_NAME_MAX      256     /* UTF-8 中文文件名按字节计，覆盖 FATFS 255-byte LFN，明显高于旧版 64 */
#define BOOK_PATH_MAX      320     /* /sdcard/books/ + FATFS 255-byte LFN */
#define BOOK_IO_BUF        4096
#define BOOK_PAGEBUF       1536
#define PAGE_LINE_UNITS    38      /* 汉字/全角=2, ASCII=1 */
#define PAGE_LINES         9

/* ---------- 书目 ---------- */
typedef struct {
    char name[BOOK_NAME_MAX];
    char path[BOOK_PATH_MAX];       /* SD 书路径；内置书为空 */
    const uint8_t *data;            /* 内置书数据；SD 书为 NULL */
    uint32_t len;                   /* 内置书长度 */
    uint32_t file_size;             /* SD 文件大小（仅提示/日志） */
} book_entry_t;

static book_entry_t *s_books = NULL;
static int s_book_count = 0;

static void free_books(void)
{
    if (s_books) {
        heap_caps_free(s_books);
        s_books = NULL;
    }
    s_book_count = 0;
}

static book_entry_t *alloc_books(int count)
{
    if (count <= 0 || (size_t)count > SIZE_MAX / sizeof(book_entry_t)) return NULL;
    size_t bytes = (size_t)count * sizeof(book_entry_t);
    book_entry_t *p = (book_entry_t *)heap_caps_calloc((size_t)count, sizeof(book_entry_t),
                                                        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) {
        ESP_LOGW(TAG, "book catalog PSRAM alloc failed (%u bytes), fallback internal", (unsigned)bytes);
        p = (book_entry_t *)heap_caps_calloc((size_t)count, sizeof(book_entry_t),
                                              MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    return p;
}

static bool is_txt_name(const char *name)
{
    const char *dot = name ? strrchr(name, '.') : NULL;
    return dot && strcasecmp(dot, ".txt") == 0;
}

static int count_sd_books(const char *dir)
{
    DIR *d = opendir(dir);
    if (!d) return 0;
    int count = 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.' || !is_txt_name(e->d_name)) continue;
        if (count < INT_MAX) count++;
    }
    closedir(d);
    return count;
}

static int book_name_cmp(const void *a, const void *b)
{
    const book_entry_t *ba = (const book_entry_t *)a;
    const book_entry_t *bb = (const book_entry_t *)b;
    return strcasecmp(ba->name, bb->name);
}

static int fill_sd_books(const char *dir, int start, int capacity)
{
    DIR *d = opendir(dir);
    if (!d) return 0;

    int n = 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL && n < capacity) {
        if (e->d_name[0] == '.' || !is_txt_name(e->d_name)) continue;
        if (strlen(e->d_name) >= BOOK_NAME_MAX) {
            ESP_LOGW(TAG, "skip too-long book filename (%u B): %s",
                     (unsigned)strlen(e->d_name), e->d_name);
            continue;
        }

        book_entry_t *b = &s_books[start + n];
        xm_strlcpy(b->name, e->d_name, sizeof(b->name));
        int wr = snprintf(b->path, sizeof(b->path), "%s/%s", dir, e->d_name);
        if (wr < 0 || wr >= (int)sizeof(b->path)) {
            ESP_LOGW(TAG, "skip too-long book path: %s", e->d_name);
            memset(b, 0, sizeof(*b));
            continue;
        }

        struct stat st;
        if (stat(b->path, &st) != 0 || !S_ISREG(st.st_mode)) {
            memset(b, 0, sizeof(*b));
            continue;
        }
        if (st.st_size > 0 && (uint64_t)st.st_size <= UINT32_MAX) {
            b->file_size = (uint32_t)st.st_size;
        }
        b->data = NULL;
        b->len = 0;
        ESP_LOGI(TAG, "found SD book: %s (%lu B)", b->path, (unsigned long)b->file_size);
        n++;
    }
    closedir(d);
    return n;
}

static int build_book_catalog(void)
{
    free_books();

    int embedded = embedded_books_count > 0 ? embedded_books_count : 0;
    int sd_count = sd_card_present() ? count_sd_books("/sdcard/books") : 0;
    int total = embedded + sd_count;
    if (total <= 0) return 0;

    s_books = alloc_books(total);
    if (!s_books) {
        ESP_LOGE(TAG, "book catalog alloc failed: %d entries", total);
        return 0;
    }

    int n = 0;
    for (int i = 0; i < embedded_books_count; i++) {
        book_entry_t *b = &s_books[n++];
        xm_strlcpy(b->name, embedded_books[i].name, sizeof(b->name));
        b->path[0] = 0;
        b->data = embedded_books[i].data;
        b->len = embedded_books[i].len;
    }

    int sd_start = n;
    if (sd_count > 0) n += fill_sd_books("/sdcard/books", n, sd_count);

    /* 只排序 SD 书；内置说明/三字经保持固定在前面。 */
    if (n - sd_start > 1) qsort(&s_books[sd_start], (size_t)(n - sd_start), sizeof(book_entry_t), book_name_cmp);

    s_book_count = n;
    ESP_LOGI(TAG, "book catalog: %d embedded + %d SD = %d", embedded, n - sd_start, n);
    return n;
}

/* ---------- 编码识别 / 解码 ---------- */
typedef enum {
    BOOK_ENC_UTF8 = 0,
    BOOK_ENC_GBK,
    BOOK_ENC_UTF16LE,
    BOOK_ENC_UTF16BE,
} book_encoding_t;

static const char *encoding_name(book_encoding_t enc)
{
    switch (enc) {
    case BOOK_ENC_GBK:     return "GBK";
    case BOOK_ENC_UTF16LE: return "UTF-16LE";
    case BOOK_ENC_UTF16BE: return "UTF-16BE";
    default:                return "UTF-8";
    }
}

/* GB2312 网格号 → Unicode (二分反查生成表) */
static uint32_t gb2312_grid_to_unicode(int g)
{
    int lo = 0, hi = GB2312_CHAR_COUNT - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int v = gb2312_idx_table[mid];
        if (v == g) return gb2312_cp_table[mid];
        if (v < g) lo = mid + 1; else hi = mid - 1;
    }
    return 0;
}

static size_t utf8_bad_bytes(const uint8_t *p, size_t len)
{
    size_t i = 0, bad = 0;
    while (i < len) {
        uint8_t b = p[i];
        if (b < 0x80) { i++; continue; }
        int n;
        if ((b & 0xE0) == 0xC0) n = 1;
        else if ((b & 0xF0) == 0xE0) n = 2;
        else if ((b & 0xF8) == 0xF0) n = 3;
        else { bad++; i++; continue; }
        if (i + 1 + (size_t)n > len) { bad++; break; }
        bool ok = true;
        for (int k = 1; k <= n; k++) {
            if ((p[i + k] & 0xC0) != 0x80) { ok = false; break; }
        }
        if (ok) i += (size_t)n + 1;
        else { bad++; i++; }
    }
    return bad;
}

static book_encoding_t detect_encoding_bytes(const uint8_t *p, size_t len, uint32_t *data_start)
{
    *data_start = 0;
    if (!p || len == 0) return BOOK_ENC_UTF8;

    if (len >= 3 && p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
        *data_start = 3;
        return BOOK_ENC_UTF8;
    }
    if (len >= 2 && p[0] == 0xFF && p[1] == 0xFE) {
        *data_start = 2;
        return BOOK_ENC_UTF16LE;
    }
    if (len >= 2 && p[0] == 0xFE && p[1] == 0xFF) {
        *data_start = 2;
        return BOOK_ENC_UTF16BE;
    }

    /* 无 BOM 的 UTF-16：中文文件通常也夹有 ASCII 标点/换行，零字节位置特征很明显。 */
    size_t pairs = len / 2;
    size_t zero_even = 0, zero_odd = 0;
    for (size_t i = 0; i + 1 < len && i < 4096; i += 2) {
        if (p[i] == 0) zero_even++;
        if (p[i + 1] == 0) zero_odd++;
    }
    size_t sampled_pairs = pairs > 2048 ? 2048 : pairs;
    if (sampled_pairs >= 16) {
        /* UTF-16 中文正文里的汉字本身通常没有 0 字节，但空格、数字、CR/LF 等 ASCII
         * 会稳定地只在一个字节位产生 0。用“少量但明显单边”的 0 分布识别无 BOM UTF-16。 */
        if (zero_odd > sampled_pairs / 32 && zero_odd > zero_even * 4 + 2) return BOOK_ENC_UTF16LE;
        if (zero_even > sampled_pairs / 32 && zero_even > zero_odd * 4 + 2) return BOOK_ENC_UTF16BE;
    }

    size_t bad = utf8_bad_bytes(p, len);
    /* 采样尾部可能刚好切断 1 个 UTF-8 字符，所以少量错误仍按 UTF-8 容错。
     * GBK 中文文本的“坏 UTF-8 字节”比例通常远高于此阈值。 */
    if (bad >= 3 && (bad >= 8 || bad * 512 > len)) return BOOK_ENC_GBK;
    return BOOK_ENC_UTF8;
}

/* ---------- 流式字节源 ---------- */
typedef struct {
    bool is_file;
    FILE *fp;
    const uint8_t *mem;
    uint32_t size;
    uint32_t pos;
    uint32_t data_start;
    book_encoding_t enc;

    uint8_t *io;
    uint32_t io_base;
    size_t io_len;
} book_source_t;

static void source_close(book_source_t *s)
{
    if (!s) return;
    if (s->fp) fclose(s->fp);
    if (s->io) heap_caps_free(s->io);
    memset(s, 0, sizeof(*s));
}

static bool source_seek(book_source_t *s, uint32_t pos)
{
    if (!s || pos > s->size) return false;
    s->pos = pos;
    return true;
}

static bool source_read_byte(book_source_t *s, uint8_t *out)
{
    if (!s || !out || s->pos >= s->size) return false;

    if (!s->is_file) {
        *out = s->mem[s->pos++];
        return true;
    }

    if (!s->io) return false;
    if (s->pos < s->io_base || s->pos >= s->io_base + s->io_len) {
        if (fseek(s->fp, (long)s->pos, SEEK_SET) != 0) return false;
        s->io_base = s->pos;
        s->io_len = fread(s->io, 1, BOOK_IO_BUF, s->fp);
        if (s->io_len == 0) return false;
    }

    *out = s->io[s->pos - s->io_base];
    s->pos++;
    return true;
}

static bool source_open(const book_entry_t *book, book_source_t *s, char *err, size_t err_cap)
{
    memset(s, 0, sizeof(*s));
    if (!book) return false;

    if (book->data) {
        s->is_file = false;
        s->mem = book->data;
        s->size = book->len;
        size_t sample = s->size < BOOK_IO_BUF ? s->size : BOOK_IO_BUF;
        s->enc = detect_encoding_bytes(s->mem, sample, &s->data_start);
        s->pos = s->data_start;
        return s->size > s->data_start;
    }

    s->fp = fopen(book->path, "rb");
    if (!s->fp) {
        /* Full 255-byte UTF-8 names can exceed the compact UI error buffer.
         * Keep the on-screen message bounded and log the exact path for diagnostics. */
        xm_strlcpy(err, "无法打开文件", err_cap);
        ESP_LOGE(TAG, "open fail: %s (name=%s)", book->path, book->name);
        return false;
    }

    if (fseek(s->fp, 0, SEEK_END) != 0) {
        snprintf(err, err_cap, "读取文件大小失败");
        source_close(s);
        return false;
    }
    long sz = ftell(s->fp);
    if (sz <= 0 || (uint64_t)sz > UINT32_MAX) {
        snprintf(err, err_cap, "文件为空或超过 4GB");
        source_close(s);
        return false;
    }
    s->size = (uint32_t)sz;
    s->is_file = true;

    /* 仅 4KB I/O 缓冲，整本小说不会再常驻 PSRAM。 */
    s->io = (uint8_t *)heap_caps_malloc(BOOK_IO_BUF, MALLOC_CAP_8BIT);
    if (!s->io) {
        snprintf(err, err_cap, "内存不足，无法建立阅读缓冲");
        source_close(s);
        return false;
    }

    if (fseek(s->fp, 0, SEEK_SET) != 0) {
        snprintf(err, err_cap, "文件定位失败");
        source_close(s);
        return false;
    }
    s->io_base = 0;
    s->io_len = fread(s->io, 1, BOOK_IO_BUF, s->fp);
    if (s->io_len == 0) {
        snprintf(err, err_cap, "文件读取失败");
        source_close(s);
        return false;
    }
    s->enc = detect_encoding_bytes(s->io, s->io_len, &s->data_start);
    s->pos = s->data_start;
    return s->size > s->data_start;
}

typedef struct {
    uint32_t cp;
    uint32_t start;
    uint32_t end;
} decoded_char_t;

static bool source_next_char(book_source_t *s, decoded_char_t *ch)
{
    if (!s || !ch || s->pos >= s->size) return false;
    ch->start = s->pos;
    ch->cp = '?';

    uint8_t b0 = 0, b1 = 0;
    if (!source_read_byte(s, &b0)) return false;

    if (s->enc == BOOK_ENC_UTF16LE || s->enc == BOOK_ENC_UTF16BE) {
        if (!source_read_byte(s, &b1)) {
            ch->cp = '?';
            ch->end = s->pos;
            return true;
        }
        uint16_t u = (s->enc == BOOK_ENC_UTF16LE)
                   ? (uint16_t)b0 | ((uint16_t)b1 << 8)
                   : ((uint16_t)b0 << 8) | (uint16_t)b1;
        if (u >= 0xD800 && u <= 0xDBFF && s->pos + 1 < s->size) {
            uint8_t c0 = 0, c1 = 0;
            if (source_read_byte(s, &c0) && source_read_byte(s, &c1)) {
                uint16_t lo = (s->enc == BOOK_ENC_UTF16LE)
                            ? (uint16_t)c0 | ((uint16_t)c1 << 8)
                            : ((uint16_t)c0 << 8) | (uint16_t)c1;
                if (lo >= 0xDC00 && lo <= 0xDFFF) {
                    ch->cp = 0x10000u + (((uint32_t)u - 0xD800u) << 10) + ((uint32_t)lo - 0xDC00u);
                } else {
                    ch->cp = '?';
                }
            }
        } else if (u >= 0xDC00 && u <= 0xDFFF) {
            ch->cp = '?';
        } else {
            ch->cp = u;
        }
        ch->end = s->pos;
        return true;
    }

    if (s->enc == BOOK_ENC_GBK) {
        if (b0 < 0x80) {
            ch->cp = b0;
        } else if (source_read_byte(s, &b1)) {
            /* CP936/GBK 的扩展字符本工程字库没有完整覆盖；GB2312 常用区可准确映射。 */
            if (b0 >= 0xA1 && b0 <= 0xF7 && b1 >= 0xA1 && b1 <= 0xFE) {
                int g = (b0 - 0xA1) * 94 + (b1 - 0xA1);
                uint32_t cp = gb2312_grid_to_unicode(g);
                ch->cp = cp ? cp : '?';
            } else {
                ch->cp = '?';
            }
        }
        ch->end = s->pos;
        return true;
    }

    /* UTF-8 */
    if (b0 < 0x80) {
        ch->cp = b0;
        ch->end = s->pos;
        return true;
    }

    int need = 0;
    uint32_t cp = 0;
    if ((b0 & 0xE0) == 0xC0) { need = 1; cp = b0 & 0x1F; }
    else if ((b0 & 0xF0) == 0xE0) { need = 2; cp = b0 & 0x0F; }
    else if ((b0 & 0xF8) == 0xF0) { need = 3; cp = b0 & 0x07; }
    else {
        ch->cp = '?';
        ch->end = s->pos;
        return true;
    }

    bool ok = true;
    for (int i = 0; i < need; i++) {
        uint8_t bx = 0;
        if (!source_read_byte(s, &bx) || (bx & 0xC0) != 0x80) { ok = false; break; }
        cp = (cp << 6) | (bx & 0x3F);
    }
    ch->cp = ok ? cp : '?';
    ch->end = s->pos;
    return true;
}

/* ---------- 分页索引 ---------- */
typedef struct {
    uint32_t start;
    uint32_t end;
} page_t;

static page_t *s_pages = NULL;
static int s_page_count = 0;
static int s_page_cap = 0;

static void clear_pages(void)
{
    if (s_pages) heap_caps_free(s_pages);
    s_pages = NULL;
    s_page_count = 0;
    s_page_cap = 0;
}

static bool page_push(uint32_t start, uint32_t end)
{
    if (end <= start) return true;
    if (s_page_count >= s_page_cap) {
        int next = s_page_cap > 0 ? s_page_cap * 2 : 256;
        if (next < s_page_cap || (size_t)next > SIZE_MAX / sizeof(page_t)) return false;
        size_t bytes = (size_t)next * sizeof(page_t);

        page_t *np = (page_t *)heap_caps_realloc(s_pages, bytes,
                                                  MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!np) {
            np = (page_t *)heap_caps_realloc(s_pages, bytes, MALLOC_CAP_8BIT);
        }
        if (!np) return false;
        s_pages = np;
        s_page_cap = next;
    }
    s_pages[s_page_count++] = (page_t){ start, end };
    return true;
}

static int char_width_units(uint32_t cp)
{
    if (cp == '\t') return 4;
    if (cp < 0x80) return 1;
    return 2;
}

static bool build_pages(book_source_t *src)
{
    clear_pages();
    if (!source_seek(src, src->data_start)) return false;

    uint32_t page_start = src->data_start;
    uint32_t line_units = 0;
    int lines = 0;
    decoded_char_t ch;

    while (source_next_char(src, &ch)) {
        uint32_t cp = ch.cp;
        if (cp == '\r' || cp == 0) continue;

        if (cp == '\n') {
            line_units = 0;
            lines++;
            if (lines >= PAGE_LINES) {
                if (!page_push(page_start, ch.end)) return false;
                page_start = ch.end;
                lines = 0;
            }
            continue;
        }

        int w = char_width_units(cp);
        if (line_units + (uint32_t)w > PAGE_LINE_UNITS) {
            lines++;
            line_units = 0;
            if (lines >= PAGE_LINES) {
                /* 当前字符应属于下一页，因此边界放在字符起点。 */
                if (!page_push(page_start, ch.start)) return false;
                page_start = ch.start;
                lines = 0;
            }
        }
        line_units += (uint32_t)w;
    }

    if (src->pos > page_start) {
        if (!page_push(page_start, src->pos)) return false;
    }
    return s_page_count > 0;
}

static size_t append_utf8(char *dst, size_t cap, size_t pos, uint32_t cp)
{
    if (!dst || cap == 0) return pos;
    if (cp == '\t') {
        for (int i = 0; i < 4 && pos + 1 < cap; i++) dst[pos++] = ' ';
        return pos;
    }
    if (cp < 0x20 && cp != '\n') return pos;

    if (cp < 0x80) {
        if (pos + 1 < cap) dst[pos++] = (char)cp;
    } else if (cp < 0x800) {
        if (pos + 2 < cap) {
            dst[pos++] = (char)(0xC0 | (cp >> 6));
            dst[pos++] = (char)(0x80 | (cp & 0x3F));
        }
    } else if (cp < 0x10000) {
        if (pos + 3 < cap) {
            dst[pos++] = (char)(0xE0 | (cp >> 12));
            dst[pos++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            dst[pos++] = (char)(0x80 | (cp & 0x3F));
        }
    } else if (cp <= 0x10FFFF) {
        if (pos + 4 < cap) {
            dst[pos++] = (char)(0xF0 | (cp >> 18));
            dst[pos++] = (char)(0x80 | ((cp >> 12) & 0x3F));
            dst[pos++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            dst[pos++] = (char)(0x80 | (cp & 0x3F));
        }
    }
    return pos;
}

static bool render_book_page(book_source_t *src, char *pagebuf, size_t cap,
                             lv_obj_t *body, lv_obj_t *foot, int cur)
{
    if (!src || !pagebuf || !body || !foot || cur < 0 || cur >= s_page_count) return false;
    if (!source_seek(src, s_pages[cur].start)) return false;

    size_t out = 0;
    decoded_char_t ch;
    uint32_t end = s_pages[cur].end;
    while (src->pos < end && source_next_char(src, &ch)) {
        if (ch.start >= end) break;
        if (ch.cp == '\r' || ch.cp == 0) continue;
        out = append_utf8(pagebuf, cap, out, ch.cp);
        if (out + 5 >= cap) break;  /* 防御性保护；正常分页远小于 1.5KB */
    }
    pagebuf[out] = 0;
    lv_label_set_text(body, pagebuf);
    lv_label_set_text_fmt(foot, "第 %d/%d 页", cur + 1, s_page_count);
    return true;
}

/* ---------- 每本书独立阅读记忆 ---------- */
static uint32_t fnv1a32(const char *s)
{
    uint32_t h = 2166136261u;
    while (s && *s) {
        h ^= (uint8_t)*s++;
        h *= 16777619u;
    }
    return h;
}

static void bookmark_key(const book_entry_t *book, char key[12])
{
    const char *id = (book && book->path[0]) ? book->path : (book ? book->name : "");
    snprintf(key, 12, "p%08lx", (unsigned long)fnv1a32(id));
}

static int32_t load_bookmark(const book_entry_t *book)
{
    nvs_handle_t h;
    if (nvs_open("ebook", NVS_READONLY, &h) != ESP_OK) return 0;

    int32_t page = 0;
    char key[12];
    bookmark_key(book, key);
    esp_err_t r = nvs_get_i32(h, key, &page);

    /* 兼容旧版：旧版只记最后一本书，且 book 缓冲曾只有 32 字节。 */
    if (r != ESP_OK) {
        char legacy[BOOK_NAME_MAX] = {0};
        size_t sz = sizeof(legacy);
        int32_t old_page = 0;
        if (nvs_get_str(h, "book", legacy, &sz) == ESP_OK &&
            nvs_get_i32(h, "page", &old_page) == ESP_OK &&
            strcmp(legacy, book->name) == 0) {
            page = old_page;
        } else {
            page = 0;
        }
    }
    nvs_close(h);
    return page;
}

static void save_bookmark(const book_entry_t *book, int32_t page)
{
    nvs_handle_t h;
    if (nvs_open("ebook", NVS_READWRITE, &h) != ESP_OK) return;
    char key[12];
    bookmark_key(book, key);
    nvs_set_i32(h, key, page);
    nvs_commit(h);
    nvs_close(h);
}

static void show_reader_error(const char *msg)
{
    lv_obj_t *scr = ui_screen_new("电子书");
    lv_obj_t *l = lv_label_create(scr);
    lv_label_set_text(l, msg ? msg : "电子书打开失败");
    lv_obj_set_width(l, 292);
    lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(l, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(l, &book_font_lvgl, 0);
    lv_obj_set_pos(l, 14, 72);
    ui_screen_show(scr);
    while (!ui_app_loop_wait()) {}
}

/* ---------- 阅读器 ---------- */
static void reader_screen(const book_entry_t *book)
{
    book_source_t src;
    char err[256] = {0};
    if (!source_open(book, &src, err, sizeof(err))) {
        show_reader_error(err[0] ? err : "电子书打开失败");
        return;
    }

    ESP_LOGI(TAG, "reader open: %s size=%lu encoding=%s heap=%lu psram_free=%lu",
             book->name, (unsigned long)src.size, encoding_name(src.enc),
             (unsigned long)esp_get_free_heap_size(),
             (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    /* 大型小说首次打开需要顺序扫描一次建立页索引。先显示轻量加载页，
     * 避免用户误以为按键没有响应；索引本身只保存页边界，不保存整本正文。 */
    lv_obj_t *loading = ui_screen_new_ex("电子书", false);
    lv_obj_t *loading_name = lv_label_create(loading);
    lv_label_set_text(loading_name, book->name);
    lv_obj_set_style_text_font(loading_name, &book_font_lvgl, 0);
    lv_obj_set_style_text_color(loading_name, UI_THEME_TEXT, 0);
    lv_obj_set_width(loading_name, 292);
    lv_label_set_long_mode(loading_name, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_pos(loading_name, 14, 74);
    lv_obj_t *loading_msg = lv_label_create(loading);
    lv_label_set_text(loading_msg, "正在建立分页索引...");
    lv_obj_set_style_text_font(loading_msg, &book_font_lvgl, 0);
    lv_obj_set_style_text_color(loading_msg, UI_THEME_DIM, 0);
    lv_obj_set_pos(loading_msg, 14, 108);
    ui_screen_show(loading);
    lv_timer_handler();

    if (!build_pages(&src)) {
        source_close(&src);
        clear_pages();
        show_reader_error("电子书分页失败\n可能是内存不足或文件内容异常");
        return;
    }

    char *pagebuf = (char *)heap_caps_malloc(BOOK_PAGEBUF, MALLOC_CAP_8BIT);
    if (!pagebuf) {
        source_close(&src);
        clear_pages();
        show_reader_error("内存不足，无法建立页面缓冲");
        return;
    }

    int32_t page = load_bookmark(book);
    if (page < 0 || page >= s_page_count) page = 0;
    int cur = (int)page;

    lv_obj_t *scr = ui_screen_new_ex("电子书", false);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, book->name);
    lv_obj_set_style_text_color(title, UI_THEME_TEXT, 0);
    lv_obj_set_style_text_font(title, &book_font_lvgl, 0);
    lv_obj_set_width(title, 304);
    lv_label_set_long_mode(title, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_pos(title, 8, 38);

    lv_obj_t *body = lv_label_create(scr);
    lv_obj_set_style_text_font(body, &book_font_lvgl, 0);
    lv_obj_set_style_text_color(body, UI_THEME_TEXT, 0);
    lv_obj_set_width(body, 304);
    lv_obj_set_height(body, 150);
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(body, 8, 58);

    lv_obj_t *enc = lv_label_create(scr);
    lv_label_set_text(enc, encoding_name(src.enc));
    lv_obj_set_style_text_color(enc, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(enc, &ui_font_lvgl_10, 0);
    lv_obj_align(enc, LV_ALIGN_BOTTOM_LEFT, 8, -4);

    lv_obj_t *foot = lv_label_create(scr);
    lv_obj_set_style_text_color(foot, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(foot, &ui_font_lvgl_10, 0);
    lv_obj_align(foot, LV_ALIGN_BOTTOM_RIGHT, -8, -4);

    ui_screen_show(scr);
    (void)render_book_page(&src, pagebuf, BOOK_PAGEBUF, body, foot, cur);

    for (;;) {
        lv_timer_handler();
        key_event_t evt;
        if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
            int ncur = cur;
            switch (evt.key) {
            case KEY_UP:
            case KEY_A:
                if (cur > 0) ncur = cur - 1;
                break;
            case KEY_DOWN:
            case KEY_B:
                if (cur < s_page_count - 1) ncur = cur + 1;
                break;
            case KEY_BACK:
            case KEY_HOME:
                save_bookmark(book, cur);
                heap_caps_free(pagebuf);
                source_close(&src);
                clear_pages();
                ESP_LOGI(TAG, "reader close: %s page=%d heap=%lu psram_free=%lu",
                         book->name, cur + 1,
                         (unsigned long)esp_get_free_heap_size(),
                         (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
                return;
            default:
                break;
            }
            if (ncur != cur) {
                cur = ncur;
                (void)render_book_page(&src, pagebuf, BOOK_PAGEBUF, body, foot, cur);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/* ---------- 5 行虚拟书单 ---------- */
static void render_book_window(lv_obj_t **rows, lv_obj_t **labels, int row_count,
                               int top_idx, lv_obj_t *footer)
{
    for (int row = 0; row < row_count; row++) {
        int idx = top_idx + row;
        if (idx < 0 || idx >= s_book_count) continue;
        lv_obj_set_user_data(rows[row], (void *)(intptr_t)idx);
        lv_label_set_text(labels[row], s_books[idx].name);
    }

    if (footer && s_book_count > 0) {
        int last = top_idx + row_count;
        if (last > s_book_count) last = s_book_count;
        lv_label_set_text_fmt(footer, "%d-%d / %d   KEY4 打开   KEY3 返回",
                              top_idx + 1, last, s_book_count);
    }
}

/* 返回选中的书索引；-1 表示退出电子书应用。 */
static int list_screen(int selected_idx)
{
    lv_obj_t *scr = ui_screen_new_ex("电子书", false);
    int n = s_book_count;

    if (n <= 0) {
        lv_obj_t *tip = lv_label_create(scr);
        lv_label_set_text(tip, "未找到电子书\n请把 TXT 放入 SD 卡 books 文件夹");
        lv_obj_set_style_text_color(tip, UI_THEME_DIM, 0);
        lv_obj_set_style_text_font(tip, &book_font_lvgl, 0);
        lv_obj_set_pos(tip, 20, 72);

        lv_obj_t *footer = lv_label_create(scr);
        lv_label_set_text(footer, "KEY3 返回");
        lv_obj_set_style_text_color(footer, UI_THEME_DIM, 0);
        lv_obj_set_style_text_font(footer, &ui_font_lvgl_10, 0);
        lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -4);
        ui_screen_show(scr);
        for (;;) {
            lv_timer_handler();
            key_event_t evt;
            if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS &&
                (evt.key == KEY_BACK || evt.key == KEY_HOME)) return -1;
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }

    if (selected_idx < 0) selected_idx = 0;
    if (selected_idx >= n) selected_idx = n - 1;

    int row_count = n < BOOK_VISIBLE_ROWS ? n : BOOK_VISIBLE_ROWS;
    lv_obj_t *rows[BOOK_VISIBLE_ROWS] = {0};
    lv_obj_t *labels[BOOK_VISIBLE_ROWS] = {0};

    lv_obj_t *list = lv_obj_create(scr);
    lv_obj_set_pos(list, 0, UI_TOPBAR_H);
    lv_obj_set_size(list, 320, 178);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_radius(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);

    for (int row = 0; row < row_count; row++) {
        lv_obj_t *btn = lv_button_create(list);
        lv_obj_set_size(btn, 300, 28);
        lv_obj_set_pos(btn, 10, 4 + row * 32);
        ui_style_list_button(btn);
        lv_group_add_obj(lv_group_get_default(), btn);
        rows[row] = btn;

        lv_obj_t *lb = lv_label_create(btn);
        lv_obj_set_width(lb, 272);
        lv_label_set_long_mode(lb, LV_LABEL_LONG_MODE_DOTS);
        lv_obj_set_style_text_font(lb, &book_font_lvgl, 0);
        lv_obj_set_style_text_color(lb, UI_THEME_TEXT, 0);
        lv_obj_align(lb, LV_ALIGN_LEFT_MID, 12, 0);
        labels[row] = lb;
    }

    lv_obj_t *footer = lv_label_create(scr);
    lv_obj_set_style_text_color(footer, UI_THEME_DIM, 0);
    lv_obj_set_style_text_font(footer, &ui_font_lvgl_10, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -4);

    int current_idx = selected_idx;
    int top_idx = current_idx - row_count / 2;
    if (top_idx < 0) top_idx = 0;
    if (top_idx > n - row_count) top_idx = n - row_count;

    render_book_window(rows, labels, row_count, top_idx, footer);
    lv_group_focus_obj(rows[current_idx - top_idx]);
    ui_screen_show(scr);

    for (;;) {
        lv_timer_handler();
        key_event_t evt;
        if (buttons_wait_event(&evt, 0) && evt.evt == KEY_EVT_PRESS) {
            switch (evt.key) {
            case KEY_UP:
            case KEY_LEFT:
            case KEY_DOWN:
            case KEY_RIGHT: {
                int delta = (evt.key == KEY_UP || evt.key == KEY_LEFT) ? -1 : 1;
                int target = current_idx + delta;
                if (target < 0) target = 0;
                if (target >= n) target = n - 1;
                if (target != current_idx) {
                    current_idx = target;
                    if (current_idx < top_idx) top_idx = current_idx;
                    else if (current_idx >= top_idx + row_count) top_idx = current_idx - row_count + 1;
                    render_book_window(rows, labels, row_count, top_idx, footer);
                    int focus_row = current_idx - top_idx;
                    if (focus_row >= 0 && focus_row < row_count) lv_group_focus_obj(rows[focus_row]);
                }
                break;
            }
            case KEY_CONFIRM: {
                lv_obj_t *f = lv_group_get_focused(lv_group_get_default());
                int selected = f ? (int)(intptr_t)lv_obj_get_user_data(f) : -1;
                if (selected >= 0 && selected < n) return selected;
                break;
            }
            case KEY_BACK:
            case KEY_HOME:
                return -1;
            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

esp_err_t app_ebook_run(void)
{
    (void)build_book_catalog();
    int selected = 0;

    for (;;) {
        int pick = list_screen(selected);
        if (pick < 0) break;
        selected = pick;
        reader_screen(&s_books[pick]);
    }

    clear_pages();
    free_books();
    return ESP_OK;
}
