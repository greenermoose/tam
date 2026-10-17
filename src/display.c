#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#include "display.h"

#include <errno.h>
#include <string.h>

int tam_display_next(const char *s, size_t len, size_t *offset,
                     size_t *byte_len, wchar_t *wc, int *cells)
{
    if (s == NULL || offset == NULL || *offset >= len) return 0;
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    wchar_t value;
    size_t n = mbrtowc(&value, s + *offset, len - *offset, &state);
    if (n == (size_t)-1 || n == (size_t)-2 || n == 0) {
        n = 1;
        value = 0xfffd;
    }
    int width = wcwidth(value);
    if (value == 0xfffd || width < 0) {
        value = 0xfffd;
        width = 1;
    }
    if (byte_len != NULL) *byte_len = n;
    if (wc != NULL) *wc = value;
    if (cells != NULL) *cells = width;
    *offset += n;
    return 1;
}

size_t tam_display_width(const char *s, size_t len)
{
    size_t offset = 0;
    size_t cells = 0;
    while (offset < len) {
        int width = 0;
        (void)tam_display_next(s, len, &offset, NULL, NULL, &width);
        if (width > 0) cells += (size_t)width;
    }
    return cells;
}

size_t tam_display_prefix(const char *s, size_t len, size_t max_cells,
                          size_t *used_cells)
{
    size_t offset = 0;
    size_t cells = 0;
    while (offset < len) {
        size_t old = offset;
        size_t bytes = 0;
        int width = 0;
        (void)tam_display_next(s, len, &offset, &bytes, NULL, &width);
        if (width > 0 && cells + (size_t)width > max_cells) {
            offset = old;
            break;
        }
        if (width > 0) cells += (size_t)width;
    }
    if (used_cells != NULL) *used_cells = cells;
    return offset;
}
