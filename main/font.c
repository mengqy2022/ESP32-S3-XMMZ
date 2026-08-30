/*
 * font.c — 内嵌字库访问
 */
#include "font.h"
#include "font_data.h"

esp_err_t font_load(const uint8_t **ascii_out, const uint8_t **gb_out)
{
    if (ascii_out) *ascii_out = ascii8x16_font;
    if (gb_out) *gb_out = gb2312_16_font;
    return ESP_OK;
}

int font_cp_index(uint32_t cp)
{
    /* 二分查找: gb2312_cp_table 升序 */
    int lo = 0, hi = GB2312_CHAR_COUNT - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (gb2312_cp_table[mid] == cp) return gb2312_idx_table[mid];
        if (gb2312_cp_table[mid] < cp) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

int font_gb2312_index(unsigned char hi, unsigned char lo)
{
    if (hi < 0xA1 || hi > 0xF7) return -1;
    if (lo < 0xA1 || lo > 0xFE) return -1;
    int q = hi - 0xA0;   /* 区 1..87 */
    int w = lo - 0xA0;   /* 位 1..94 */
    if (q < 1 || q > GB2312_GRID_ROWS) return -1;
    if (w < 1 || w > GB2312_GRID_COLS) return -1;
    return (q - 1) * GB2312_GRID_COLS + (w - 1);
}
