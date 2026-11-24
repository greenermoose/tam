#ifndef TAM_UTF8_H
#define TAM_UTF8_H

#include <stddef.h>

/* Return the byte offset of the previous UTF-8 codepoint start before pos.
 * Moves back one byte, then over continuation bytes. Used for backspace and
 * truncation on raw UTF-8 buffers. */
static inline size_t tam_utf8_prev(const char *s, size_t pos)
{
    if (s == NULL || pos == 0) return 0;
    pos--;
    while (pos > 0 && (((unsigned char)s[pos] & 0xC0) == 0x80))
        pos--;
    return pos;
}

#endif /* TAM_UTF8_H */
