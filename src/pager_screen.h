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
} tam_pager_screen_t;

void tam_pager_screen_init(tam_pager_screen_t *s, tam_terminal_t *term);
int tam_pager_screen_update(tam_pager_screen_t *s,
                            const tam_buf_t *rows, int nrows, int width);
void tam_pager_screen_invalidate(tam_pager_screen_t *s);
void tam_pager_screen_free(tam_pager_screen_t *s);

#endif
