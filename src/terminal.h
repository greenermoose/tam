#ifndef TAM_TERMINAL_H
#define TAM_TERMINAL_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} tam_tbuf_t;

typedef struct {
    int fd;
    int rows;
    int cols;
    int colors;
    int interactive;
    int reverse;
    int use_color;
    int gutter;
    int bce;
    int sync_output;
    int sync_queried;
    int hyperlinks;
    int alt_screen;
    int cursor_hidden;
    int cursor_underlined;
    int raw_mode;
    const char *cup;
    const char *clear;
    const char *el;
    const char *el1;
    const char *sgr0;
    const char *smso;
    const char *rmso;
    const char *rev;
    const char *bold;
    const char *dim;
    const char *italic;
    const char *underline;
    const char *setaf;
    const char *setab;
    const char *smcup;
    const char *rmcup;
    const char *civis;
    const char *cnorm;
    const char *cvvis;
    const char *set_cursor_style;
    const char *reset_cursor_style;
    const char *flash;
    int ncv;
} tam_terminal_t;

int tam_tbuf_append(tam_tbuf_t *b, const void *data, size_t len);
int tam_tbuf_puts(tam_tbuf_t *b, const char *s);
void tam_tbuf_clear(tam_tbuf_t *b);
void tam_tbuf_free(tam_tbuf_t *b);

/* Returns 1 for a cursor-addressable terminal, 0 for plain-output fallback. */
int tam_terminal_init(tam_terminal_t *t, int fd, int rows, int cols);
int tam_terminal_cap(tam_terminal_t *t, tam_tbuf_t *b, const char *cap,
                     long p1, long p2);
int tam_terminal_write(tam_terminal_t *t, const tam_tbuf_t *b);
int tam_terminal_enter(tam_terminal_t *t, tam_tbuf_t *b);
int tam_terminal_leave(tam_terminal_t *t, tam_tbuf_t *b);
int tam_terminal_move(tam_terminal_t *t, tam_tbuf_t *b, int row, int col);
int tam_terminal_input_cursor(tam_terminal_t *t, tam_tbuf_t *b, int visible);
int tam_terminal_erase_eol(tam_terminal_t *t, tam_tbuf_t *b);

#endif
