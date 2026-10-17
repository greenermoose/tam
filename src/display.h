#ifndef TAM_DISPLAY_H
#define TAM_DISPLAY_H

#include <stddef.h>
#include <wchar.h>

/* Decode one UTF-8 character. Invalid bytes become a one-cell replacement. */
int tam_display_next(const char *s, size_t len, size_t *offset,
                     size_t *byte_len, wchar_t *wc, int *cells);
size_t tam_display_width(const char *s, size_t len);
size_t tam_display_prefix(const char *s, size_t len, size_t max_cells,
                          size_t *used_cells);

#endif
