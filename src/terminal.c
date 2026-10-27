#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#include "terminal.h"

#include <curses.h>
#include <term.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static tam_buf_t *cap_buffer;

int tam_buf_append(tam_buf_t *b, const void *data, size_t len)
{
    if (len > (size_t)-1 - b->len - 1) return -1;
    size_t need = b->len + len + 1;
    if (need > b->cap) {
        size_t cap = b->cap ? b->cap : 256;
        while (cap < need) {
            if (cap > (size_t)-1 / 2) { cap = need; break; }
            cap *= 2;
        }
        char *p = realloc(b->data, cap);
        if (p == NULL) return -1;
        b->data = p;
        b->cap = cap;
    }
    if (len) memcpy(b->data + b->len, data, len);
    b->len += len;
    b->data[b->len] = '\0';
    return 0;
}

int tam_buf_puts(tam_buf_t *b, const char *s)
{
    return s == NULL ? 0 : tam_buf_append(b, s, strlen(s));
}

void tam_buf_clear(tam_buf_t *b)
{
    b->len = 0;
    if (b->data != NULL) b->data[0] = '\0';
}

void tam_buf_free(tam_buf_t *b)
{
    free(b->data);
    memset(b, 0, sizeof(*b));
}

static const char *str_cap(const char *name)
{
    char *s = tigetstr((char *)name);
    return (s == NULL || s == (char *)-1) ? NULL : s;
}

static int flag_cap(const char *name)
{
    int n = tigetflag((char *)name);
    return n > 0;
}

int tam_terminal_init(tam_terminal_t *t, int fd, int rows, int cols)
{
    memset(t, 0, sizeof(*t));
    t->fd = fd;
    t->rows = rows > 0 ? rows : 24;
    t->cols = cols > 0 ? cols : 80;
    const char *term = getenv("TERM");
    if (term == NULL || term[0] == '\0' || strcmp(term, "dumb") == 0) return 0;

    int err = 0;
    if (setupterm((char *)term, fd, &err) != OK || err <= 0) return 0;
    t->cup = str_cap("cup");
    t->clear = str_cap("clear");
    t->el = str_cap("el");
    t->el1 = str_cap("el1");
    t->sgr0 = str_cap("sgr0");
    t->smso = str_cap("smso");
    t->rmso = str_cap("rmso");
    t->rev = str_cap("rev");
    t->bold = str_cap("bold");
    t->dim = str_cap("dim");
    t->underline = str_cap("smul");
    t->setaf = str_cap("setaf");
    t->setab = str_cap("setab");
    t->smcup = str_cap("smcup");
    t->rmcup = str_cap("rmcup");
    t->civis = str_cap("civis");
    t->cnorm = str_cap("cnorm");
    t->flash = str_cap("flash");
    t->colors = tigetnum("colors");
    t->ncv = tigetnum("ncv");
    t->bce = flag_cap("bce");
    t->reverse = (t->rev != NULL || (t->smso != NULL && t->rmso != NULL));
    t->interactive = t->cup != NULL && t->sgr0 != NULL && t->el != NULL;
    if (!t->interactive) return 0;
    /* OSC 8 hyperlinks. The Linux console and hardware VT terminals print an
     * unknown OSC string's text, so they get none. */
    t->hyperlinks = strcmp(term, "linux") != 0 && strncmp(term, "vt", 2) != 0 &&
                    strncmp(term, "cons", 4) != 0;

    const char *no_color = getenv("NO_COLOR");
    t->use_color = no_color == NULL || no_color[0] == '\0';
    if (t->colors >= 256 && t->setaf != NULL && t->setab != NULL && t->use_color) {
        /* If the user has disabled colors, focus remains a reverse attribute. */
    } else if (t->reverse) {
        t->use_color = 0;
    } else {
        t->gutter = 1;
    }
    return 1;
}

static int cap_putc(int c)
{
    char ch = (char)c;
    return cap_buffer == NULL ? ERR : tam_buf_append(cap_buffer, &ch, 1);
}

int tam_terminal_cap(tam_terminal_t *t, tam_buf_t *b, const char *cap,
                     long p1, long p2)
{
    (void)t;
    if (cap == NULL) return 0;
    char *expanded = tparm((char *)cap, p1, p2);
    if (expanded == NULL || expanded == (char *)-1) return -1;
    cap_buffer = b;
    int rc = tputs(expanded, 1, cap_putc);
    cap_buffer = NULL;
    return rc == ERR ? -1 : 0;
}

int tam_terminal_write(tam_terminal_t *t, const tam_buf_t *b)
{
    size_t off = 0;
    while (off < b->len) {
        ssize_t n = write(t->fd, b->data + off, b->len - off);
        if (n > 0) { off += (size_t)n; continue; }
        if (n < 0 && errno == EINTR) continue;
        return -1;
    }
    return 0;
}

int tam_terminal_enter(tam_terminal_t *t, tam_buf_t *b)
{
    const char *enter_screen = t->smcup != NULL && t->rmcup != NULL ? t->smcup : NULL;
    const char *hide_cursor = t->civis != NULL && t->cnorm != NULL ? t->civis : NULL;
    if (tam_terminal_cap(t, b, enter_screen, 0, 0) < 0 ||
        tam_terminal_cap(t, b, hide_cursor, 0, 0) < 0) return -1;
    t->alt_screen = enter_screen != NULL;
    t->cursor_hidden = hide_cursor != NULL;
    return 0;
}

int tam_terminal_leave(tam_terminal_t *t, tam_buf_t *b)
{
    if (t->sync_output) (void)tam_buf_puts(b, "\033[?2026l");
    if (tam_terminal_cap(t, b, t->sgr0, 0, 0) < 0) return -1;
    if (t->cursor_hidden && tam_terminal_cap(t, b, t->cnorm, 0, 0) < 0) return -1;
    if (t->alt_screen && tam_terminal_cap(t, b, t->rmcup, 0, 0) < 0) return -1;
    return 0;
}

int tam_terminal_move(tam_terminal_t *t, tam_buf_t *b, int row, int col)
{
    if (row < 0) row = 0;
    if (col < 0) col = 0;
    return tam_terminal_cap(t, b, t->cup, row, col);
}

int tam_terminal_erase_eol(tam_terminal_t *t, tam_buf_t *b)
{
    return tam_terminal_cap(t, b, t->el, 0, 0);
}
