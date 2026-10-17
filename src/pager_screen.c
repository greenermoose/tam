#include "pager_screen.h"

#include <stdlib.h>
#include <string.h>

void tam_pager_screen_init(tam_pager_screen_t *s, tam_terminal_t *term)
{
    memset(s, 0, sizeof(*s));
    s->term = term;
}

void tam_pager_screen_invalidate(tam_pager_screen_t *s)
{
    s->valid = 0;
}

static int reserve_rows(tam_pager_screen_t *s, int nrows)
{
    if (nrows == s->nrows) return 0;
    char **rows = calloc((size_t)nrows, sizeof(*rows));
    size_t *lengths = calloc((size_t)nrows, sizeof(*lengths));
    if (rows == NULL || lengths == NULL) {
        free(rows);
        free(lengths);
        return -1;
    }
    for (int i = 0; i < s->nrows; i++) free(s->rows[i]);
    free(s->rows);
    free(s->lengths);
    s->rows = rows;
    s->lengths = lengths;
    s->nrows = nrows;
    s->valid = 0;
    return 0;
}

int tam_pager_screen_update(tam_pager_screen_t *s,
                            const tam_buf_t *rows, int nrows, int width)
{
    if (nrows <= 0 || width <= 0 || reserve_rows(s, nrows) < 0) return -1;
    tam_buf_t out = {0};
    int full = !s->valid || s->width != width;
    int changed = 0;
    if (s->term->sync_output) (void)tam_buf_puts(&out, "\033[?2026h");
    for (int i = 0; i < nrows; i++) {
        if (!full && s->lengths[i] == rows[i].len &&
            memcmp(s->rows[i], rows[i].data, rows[i].len) == 0) continue;
        changed++;
        if (tam_terminal_move(s->term, &out, i, 0) < 0 ||
            tam_buf_append(&out, rows[i].data, rows[i].len) < 0 ||
            tam_terminal_erase_eol(s->term, &out) < 0 ||
            tam_terminal_cap(s->term, &out, s->term->sgr0, 0, 0) < 0) {
            tam_buf_free(&out);
            return -1;
        }
    }
    if (changed && s->term->sync_output) (void)tam_buf_puts(&out, "\033[?2026l");
    if (changed && tam_terminal_write(s->term, &out) < 0) {
        tam_buf_free(&out);
        s->valid = 0;
        return -1;
    }
    tam_buf_free(&out);
    for (int i = 0; i < nrows; i++) {
        char *copy = malloc(rows[i].len + 1);
        if (copy == NULL) { s->valid = 0; return -1; }
        memcpy(copy, rows[i].data, rows[i].len);
        copy[rows[i].len] = '\0';
        free(s->rows[i]);
        s->rows[i] = copy;
        s->lengths[i] = rows[i].len;
    }
    s->width = width;
    s->valid = 1;
    return changed;
}

void tam_pager_screen_free(tam_pager_screen_t *s)
{
    for (int i = 0; i < s->nrows; i++) free(s->rows[i]);
    free(s->rows);
    free(s->lengths);
    memset(s, 0, sizeof(*s));
}
