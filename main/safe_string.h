#pragma once

#include <stddef.h>
#include <string.h>

/*
 * GCC 14 + ESP-IDF 5.5.x 对 strncpy(dst, src, size - 1) 会更积极地报告
 * -Wstringop-truncation。项目启用了 -Werror=all，因此统一使用明确语义的
 * NUL-terminated copy，避免靠关闭警告绕过真正的边界问题。
 */
static inline size_t xm_strlcpy(char *dst, const char *src, size_t dst_size)
{
    if (!src) src = "";

    size_t src_len = strlen(src);
    if (!dst || dst_size == 0) {
        return src_len;
    }

    size_t n = src_len;
    if (n >= dst_size) n = dst_size - 1;

    if (n > 0) memcpy(dst, src, n);
    dst[n] = '\0';
    return src_len;
}
