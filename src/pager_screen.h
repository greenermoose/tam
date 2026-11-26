#ifndef TAM_PAGER_SCREEN_H
#define TAM_PAGER_SCREEN_H

#include "terminal.h"

typedef struct {
    tam_terminal_t *term;
    char **rows;
    size_t *lengths;
    int nrows;
    int width;
    int valid;
    int cursor_row;
    int cursor_col;
} tam_reader_screen_t;

void tam_reader_screen_init(tam_reader_screen_t *s, tam_terminal_t *term);
int tam_reader_screen_update(tam_reader_screen_t *s,
                            const tam_tbuf_t *rows, int nrows, int width,
                            int cursor_row, int cursor_col);
void tam_reader_screen_invalidate(tam_reader_screen_t *s);
void tam_reader_screen_free(tam_reader_screen_t *s);

#endif
