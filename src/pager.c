#include <tam.h>

#include "depth.h"
#include "display.h"
#include "pager.h"
#include "pager_screen.h"
#include "render.h"
#include "terminal.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t g_stop = 0;
static volatile sig_atomic_t g_resized = 0;
static tam_terminal_t *g_term;
static tam_buf_t *g_capture;
static tam_pager_screen_t g_screen;
static int g_attention;
static unsigned char g_pending[128];
static size_t g_pending_len;
static int g_last_width = 80;
static int g_last_height = 24;

static void on_stop(int sig)
{
    (void)sig;
    g_stop = 1;
}

static void on_resize(int sig)
{
    (void)sig;
    g_resized = 1;
}

int tam_terminal_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        g_last_width = ws.ws_col;
        return g_last_width;
    }
    const char *c = getenv("COLUMNS");
    if (c != NULL) {
        int v = atoi(c);
        if (v > 0) {
            g_last_width = v;
            return g_last_width;
        }
    }
    return g_last_width;
}

static int terminal_height(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) {
        g_last_height = ws.ws_row;
        return g_last_height;
    }
    return g_last_height;
}

static void out(const char *s)
{
    if (s == NULL) return;
    size_t n = strlen(s);
    if (g_capture != NULL) (void)tam_buf_append(g_capture, s, n);
    else if (g_term != NULL) {
        tam_buf_t b = {0};
        (void)tam_buf_append(&b, s, n);
        (void)tam_terminal_write(g_term, &b);
        tam_buf_free(&b);
    }
}

static void outn(const char *s, size_t n)
{
    if (n == 0) return;
    if (g_capture != NULL) (void)tam_buf_append(g_capture, s, n);
    else if (g_term != NULL) {
        tam_buf_t b = {0};
        (void)tam_buf_append(&b, s, n);
        (void)tam_terminal_write(g_term, &b);
        tam_buf_free(&b);
    }
}

static void pending_prepend(const unsigned char *data, size_t len)
{
    if (len == 0 || len > sizeof(g_pending) - g_pending_len) return;
    memmove(g_pending + len, g_pending, g_pending_len);
    memcpy(g_pending, data, len);
    g_pending_len += len;
}

static int read_byte(int fd, int timeout_ms, unsigned char *c);

typedef struct {
    int               link_id;
    int               header;
    const char       *href;
    size_t            line_start;
    size_t            line_end;
    int               col_start;
    int               col_end;
    const tam_span_t *span;
} linkref_t;

typedef struct {
    size_t line_idx;
    size_t byte_start;
    size_t byte_end;
} search_hit_t;

typedef struct {
    char *href;
    char *selected_href;
    int selected_link_id;
    int selected_header;
    int   top;
    int   cursor_line;
    size_t top_block;
    size_t top_offset;
    size_t cursor_block;
    size_t cursor_offset;
    int width;
} nav_state_t;

typedef struct {
    tam_document_t *doc;
    char           *cur_href;
    linkref_t      *links;
    size_t          nlinks;
    nav_state_t    *history;
    size_t          nhistory;
    nav_state_t    *forward;
    size_t          nforward;
    int             top;
    int             selected;
    int             cursor_line;
    int             sweep_dir;
    int             width;
    int             layout_width;
    int             height;
    int             input_fd;
    char            msg[160];
    tam_page_loader_t load;
    void              *ctx;
    char            search_query[128];
    search_hit_t   *hits;
    size_t          nhits;
    int             cur_hit;
    int             sync_probe_sent;
    int             sync_queried;
    int             sync_output;
    int             help_open;
    int             column_active;
    int             show_address;
    unsigned        lens; /* The reader's level lens (depth.h). */
    unsigned char  *fold_open;
    size_t          nfold_open;
} pager_t;

static void follow(pager_t *st, const char *href);
static void toggle_fold(pager_t *st, unsigned id);

static void emit_reset(int is_attention);
static int emit_style(const tam_span_t *sp, int is_sel);
static size_t status_byte_limit(const char *s, int max_cols);

static int content_rows(const pager_t *st)
{
    if (st->height <= 2) return st->height <= 1 ? 1 : st->height - 1;
    return st->height - 2; /* Fixed location row plus footer. */
}

static int content_width(const pager_t *st)
{
    int width = st->width > 1 ? st->width - 1 : 1;
    if (g_term != NULL && g_term->gutter && width > 1) width--;
    return width;
}

static void pager_build_links(pager_t *st)
{
    free(st->links);
    st->links = NULL;
    st->nlinks = 0;
    st->selected = -1;

    for (size_t i = 0; i < st->doc->nlines; i++) {
        int col = g_term != NULL && g_term->gutter ? 1 : 0;
        for (size_t j = 0; j < st->doc->lines[i].nspans; j++) {
            const tam_span_t *sp = &st->doc->lines[i].spans[j];
            size_t text_len = sp->text != NULL ? strlen(sp->text) : 0;
            int span_col_start = col;
            int span_col_end = col + (int)tam_display_width(sp->text, text_len);
            col = span_col_end;

            if (sp->href == NULL) {
                continue;
            }

            size_t lead = 0;
            if (sp->text != NULL) {
                while (lead < text_len && isspace((unsigned char)sp->text[lead])) {
                    lead++;
                }
            }
            int link_col_start = span_col_start +
                                 (int)tam_display_width(sp->text, lead);
            int link_col_end = span_col_end;
            if (link_col_start >= link_col_end) {
                link_col_start = span_col_start;
            }

            if (st->nlinks > 0) {
                linkref_t *prev = &st->links[st->nlinks - 1];
                int is_same = 0;
                if (!prev->header && sp->link_id > 0 && prev->link_id > 0) {
                    is_same = (sp->link_id == prev->link_id);
                } else if (prev->href != NULL && strcmp(prev->href, sp->href) == 0 &&
                           i <= prev->line_end + 1) {
                    is_same = 1;
                }
                if (is_same) {
                    prev->line_end = i;
                    if (i == prev->line_start) {
                        prev->col_end = link_col_end;
                    }
                    continue;
                }
            }
            st->links = realloc(st->links, (st->nlinks + 1) * sizeof(*st->links));
            st->links[st->nlinks].link_id = sp->link_id;
            st->links[st->nlinks].header = 0;
            st->links[st->nlinks].href = sp->href;
            st->links[st->nlinks].line_start = i;
            st->links[st->nlinks].line_end = i;
            st->links[st->nlinks].col_start = link_col_start;
            st->links[st->nlinks].col_end = link_col_end;
            st->links[st->nlinks].span = sp;
            st->nlinks++;
        }
    }

    int header_col = 0;
    for (size_t j = 0; j < st->doc->location.nspans; j++) {
        const tam_span_t *sp = &st->doc->location.spans[j];
        int start = header_col;
        header_col += (int)tam_display_width(sp->text, strlen(sp->text));
        if (sp->href == NULL) continue;
        st->links = realloc(st->links, (st->nlinks + 1) * sizeof(*st->links));
        st->links[st->nlinks++] = (linkref_t){
            .link_id = sp->link_id, .header = 1, .href = sp->href,
            .line_start = 0, .line_end = 0, .col_start = start,
            .col_end = header_col, .span = sp
        };
    }
}

static void emit_location_line(pager_t *st, int width, const linkref_t *sel)
{
    int col = 0;
    for (size_t i = 0; i < st->doc->location.nspans && col < width; i++) {
        const tam_span_t *sp = &st->doc->location.spans[i];
        const char *text = sp->text ? sp->text : "";
        size_t bytes = status_byte_limit(text, width - col);
        if (bytes == 0) break;
        int selected = sel != NULL && sel->header && sel->span == sp;
        (void)emit_style(sp, selected);
        outn(text, bytes);
        if (sp->href != NULL) emit_reset(0);
        col += (int)tam_display_width(text, bytes);
    }
}

static const linkref_t *selected_link(const pager_t *st)
{
    if (st->selected < 0 || (size_t)st->selected >= st->nlinks) {
        return NULL;
    }
    return &st->links[st->selected];
}

static void emit_attention(int enabled)
{
    g_attention = enabled;
    if (g_term == NULL) return;
    if (enabled && g_term->colors >= 256 && g_term->use_color) {
        (void)tam_terminal_cap(g_term, g_capture, g_term->setaf, 15, 0);
        (void)tam_terminal_cap(g_term, g_capture, g_term->setab, 236, 0);
    } else if (enabled && g_term->reverse) {
        (void)tam_terminal_cap(g_term, g_capture,
                              g_term->rev != NULL ? g_term->rev : g_term->smso,
                              0, 0);
    }
}

static void emit_reset(int is_attention)
{
    if (g_term != NULL) (void)tam_terminal_cap(g_term, g_capture, g_term->sgr0, 0, 0);
    emit_attention(is_attention);
}

static void emit_search_style(int type)
{
    if (g_term == NULL || type == 0) return;
    if (type == 2 && (g_term->ncv < 0 || !(g_term->ncv & 32)))
        (void)tam_terminal_cap(g_term, g_capture, g_term->bold, 0, 0);
    if (g_term->ncv < 0 || !(g_term->ncv & 2))
        (void)tam_terminal_cap(g_term, g_capture, g_term->underline, 0, 0);
}

/* The attention column is as wide as the caught link and spans every content
 * row. Columns are content-relative, so the gutter is subtracted. */
static int column_band(const pager_t *st, int *start, int *end)
{
    const linkref_t *sel = st != NULL && st->column_active ? selected_link(st) : NULL;
    if (sel == NULL) return 0;
    int gutter = g_term != NULL && g_term->gutter ? 1 : 0;
    *start = sel->col_start - gutter;
    *end = sel->col_end - gutter;
    if (*start < 0) *start = 0;
    if (*end <= *start) *end = *start + 1;
    return 1;
}

static void emit_band_padding(const pager_t *st, int col, int width)
{
    int start, end;
    if (!column_band(st, &start, &end) || col >= end) return;
    while (col < start && col < width) {
        outn(" ", 1);
        col++;
    }
    emit_attention(1);
    while (col < end && col < width) {
        outn(" ", 1);
        col++;
    }
    emit_reset(0);
}

static int link_is_visible(const pager_t *st, int link_idx)
{
    if (link_idx < 0 || (size_t)link_idx >= st->nlinks) {
        return 0;
    }
    int usable = content_rows(st);
    size_t top_line = (size_t)st->top;
    size_t bot_line = (size_t)(st->top + usable - 1);
    const linkref_t *l = &st->links[link_idx];
    if (l->header) return 1;
    return (l->line_end >= top_line && l->line_start <= bot_line);
}

static int find_first_link_on_line(const pager_t *st, size_t line_idx)
{
    for (size_t i = 0; i < st->nlinks; i++) {
        if (!st->links[i].header && st->links[i].line_start <= line_idx &&
            st->links[i].line_end >= line_idx) {
            return (int)i;
        }
    }
    return -1;
}

static int find_last_link_on_line(const pager_t *st, size_t line_idx)
{
    int last = -1;
    for (size_t i = 0; i < st->nlinks; i++) {
        if (!st->links[i].header && st->links[i].line_start <= line_idx &&
            st->links[i].line_end >= line_idx) {
            last = (int)i;
        }
    }
    return last;
}

static void set_cursor_line(pager_t *st, int new_line, int dir)
{
    if (st->doc == NULL || st->doc->nlines == 0) {
        st->cursor_line = 0;
        st->selected = -1;
        return;
    }
    if (new_line < 0) {
        new_line = 0;
    }
    if (new_line >= (int)st->doc->nlines) {
        new_line = (int)st->doc->nlines - 1;
    }
    st->cursor_line = new_line;
    st->sweep_dir = dir;

    int usable = content_rows(st);
    int max_top = (int)st->doc->nlines - usable;
    if (max_top < 0) {
        max_top = 0;
    }

    if (st->cursor_line < st->top) {
        st->top = st->cursor_line;
    } else if (st->cursor_line >= st->top + usable) {
        st->top = st->cursor_line - usable + 1;
    }
    if (st->top < 0) {
        st->top = 0;
    }
    if (st->top > max_top) {
        st->top = max_top;
    }

    int caught = -1;
    if (dir > 0) {
        caught = find_first_link_on_line(st, (size_t)st->cursor_line);
    } else if (dir < 0) {
        caught = find_last_link_on_line(st, (size_t)st->cursor_line);
    }

    if (caught >= 0) {
        st->selected = caught;
    } else {
        if (!link_is_visible(st, st->selected)) {
            st->selected = -1;
        }
    }
}

static int emit_style(const tam_span_t *sp, int is_sel)
{
    if (sp->href != NULL) {
        if (is_sel) {
            if (g_term != NULL) {
                if (g_term->ncv < 0 || !(g_term->ncv & 32))
                    (void)tam_terminal_cap(g_term, g_capture, g_term->bold, 0, 0);
                const char *reverse = g_term->rev != NULL ? g_term->rev : g_term->smso;
                if (reverse != NULL)
                    (void)tam_terminal_cap(g_term, g_capture, reverse, 0, 0);
                else if (g_term->ncv < 0 || !(g_term->ncv & 2))
                    (void)tam_terminal_cap(g_term, g_capture, g_term->underline, 0, 0);
            }
        } else {
            if (g_term != NULL) {
                if (g_term->ncv < 0 || !(g_term->ncv & 2))
                    (void)tam_terminal_cap(g_term, g_capture, g_term->underline, 0, 0);
                if (!g_attention && !g_term->reverse && g_term->colors >= 8 && g_term->use_color)
                    (void)tam_terminal_cap(g_term, g_capture, g_term->setaf, 4, 0);
            }
        }
        return g_term != NULL;
    }
    if (sp->style & TAM_STYLE_HEADING) {
        if (g_term != NULL) {
            if (g_term->ncv < 0 || !(g_term->ncv & 32))
                (void)tam_terminal_cap(g_term, g_capture, g_term->bold, 0, 0);
            if (!g_attention && g_term->colors >= 8 && g_term->use_color)
                (void)tam_terminal_cap(g_term, g_capture, g_term->setaf, 6, 0);
        }
        return g_term != NULL;
    }
    int styled = 0;
    if (sp->style & TAM_STYLE_BOLD) {
        if (g_term != NULL && (g_term->ncv < 0 || !(g_term->ncv & 32)))
            (void)tam_terminal_cap(g_term, g_capture, g_term->bold, 0, 0);
        styled = 1;
    }
    if (sp->style & TAM_STYLE_ITALIC) {
        /* Italic is absent from the portable terminfo profile. */
    }
    if (sp->style & TAM_STYLE_CODE) {
        if (g_term != NULL && !g_attention && g_term->colors >= 8 && g_term->use_color)
            (void)tam_terminal_cap(g_term, g_capture, g_term->setaf, 3, 0);
        styled = 1;
    }
    return styled;
}

static int link_space_is_interior(const char *text, size_t len, size_t offset)
{
    int has_text_before = 0;
    int has_text_after = 0;
    for (size_t i = offset; i > 0; i--) {
        if (!isspace((unsigned char)text[i - 1])) {
            has_text_before = 1;
            break;
        }
    }
    for (size_t i = offset + 1; i < len; i++) {
        if (!isspace((unsigned char)text[i])) {
            has_text_after = 1;
            break;
        }
    }
    return has_text_before && has_text_after;
}

static const char *tam_casestr(const char *haystack, const char *needle)
{
    if (haystack == NULL || needle == NULL) {
        return NULL;
    }
    if (*needle == '\0') {
        return haystack;
    }
    size_t nlen = strlen(needle);
    for (size_t i = 0; haystack[i] != '\0'; i++) {
        if (strncasecmp(haystack + i, needle, nlen) == 0) {
            return haystack + i;
        }
    }
    return NULL;
}

static char *pager_line_text(const tam_line_t *line)
{
    size_t total = 0;
    for (size_t j = 0; j < line->nspans; j++) {
        if (line->spans[j].text != NULL) {
            total += strlen(line->spans[j].text);
        }
    }
    char *buf = malloc(total + 1);
    if (buf == NULL) {
        return NULL;
    }
    char *p = buf;
    for (size_t j = 0; j < line->nspans; j++) {
        if (line->spans[j].text != NULL) {
            size_t n = strlen(line->spans[j].text);
            memcpy(p, line->spans[j].text, n);
            p += n;
        }
    }
    *p = '\0';
    return buf;
}

static void pager_clear_search(pager_t *st)
{
    st->search_query[0] = '\0';
    free(st->hits);
    st->hits = NULL;
    st->nhits = 0;
    st->cur_hit = -1;
}

static void pager_perform_search(pager_t *st, const char *query)
{
    pager_clear_search(st);
    if (query == NULL || query[0] == '\0') {
        return;
    }
    snprintf(st->search_query, sizeof(st->search_query), "%s", query);
    size_t qlen = strlen(query);

    for (size_t i = 0; i < st->doc->nlines; i++) {
        char *ltext = pager_line_text(&st->doc->lines[i]);
        if (ltext == NULL) {
            continue;
        }
        const char *p = ltext;
        while ((p = tam_casestr(p, query)) != NULL) {
            size_t start = (size_t)(p - ltext);
            search_hit_t *nh = realloc(st->hits, (st->nhits + 1) * sizeof(*st->hits));
            if (nh == NULL) {
                break;
            }
            st->hits = nh;
            st->hits[st->nhits].line_idx = i;
            st->hits[st->nhits].byte_start = start;
            st->hits[st->nhits].byte_end = start + qlen;
            st->nhits++;
            p += qlen > 0 ? qlen : 1;
        }
        free(ltext);
    }

    if (st->nhits == 0) {
        snprintf(st->msg, sizeof(st->msg), "Pattern not found: %s", query);
        st->cur_hit = -1;
        return;
    }

    st->cur_hit = 0;
    for (size_t h = 0; h < st->nhits; h++) {
        if (st->hits[h].line_idx >= (size_t)st->top) {
            st->cur_hit = (int)h;
            break;
        }
    }

    int usable = content_rows(st);
    int hit_line = (int)st->hits[st->cur_hit].line_idx;
    if (hit_line < st->top || hit_line >= st->top + usable) {
        st->top = hit_line - usable / 3;
        int max_top = (int)st->doc->nlines - usable;
        if (max_top < 0) max_top = 0;
        if (st->top > max_top) st->top = max_top;
        if (st->top < 0) st->top = 0;
    }
    set_cursor_line(st, hit_line, 1);

    snprintf(st->msg, sizeof(st->msg), "[%d/%zu] %.120s", st->cur_hit + 1,
             st->nhits, st->search_query);
}

static void pager_next_hit(pager_t *st, int dir)
{
    if (st->search_query[0] == '\0') {
        return;
    }
    if (st->nhits == 0) {
        snprintf(st->msg, sizeof(st->msg), "Pattern not found: %s", st->search_query);
        return;
    }
    if (st->cur_hit < 0) {
        st->cur_hit = 0;
    } else {
        st->cur_hit += dir;
        if (st->cur_hit < 0) {
            st->cur_hit = (int)st->nhits - 1;
        } else if ((size_t)st->cur_hit >= st->nhits) {
            st->cur_hit = 0;
        }
    }

    int usable = content_rows(st);
    int hit_line = (int)st->hits[st->cur_hit].line_idx;
    if (hit_line < st->top || hit_line >= st->top + usable) {
        st->top = hit_line - usable / 3;
        int max_top = (int)st->doc->nlines - usable;
        if (max_top < 0) max_top = 0;
        if (st->top > max_top) st->top = max_top;
        if (st->top < 0) st->top = 0;
    }
    set_cursor_line(st, hit_line, dir);

    snprintf(st->msg, sizeof(st->msg), "[%d/%zu] %.120s", st->cur_hit + 1,
             st->nhits, st->search_query);
}

static void pager_draw_prompt(pager_t *st, const char *label,
                              const char *query, size_t qlen)
{
    int row = st->height >= 2 ? st->height - 1 : 0;
    int max = st->width > 1 ? st->width - 1 : 0;
    tam_buf_t b = {0};
    (void)tam_terminal_move(g_term, &b, row, 0);
    (void)tam_terminal_cap(g_term, &b, g_term->sgr0, 0, 0);
    (void)tam_terminal_erase_eol(g_term, &b);
    (void)tam_buf_puts(&b, label);
    size_t off = 0;
    size_t n = tam_display_prefix(query, qlen, (size_t)max, NULL);
    while (off < n) {
        size_t start = off, bytes = 0;
        wchar_t wc;
        int cells = 0;
        (void)tam_display_next(query, n, &off, &bytes, &wc, &cells);
        if (wc == 0xfffd) (void)tam_buf_puts(&b, "?");
        else (void)tam_buf_append(&b, query + start, bytes);
    }
    (void)tam_terminal_cap(g_term, &b, g_term->cnorm, 0, 0);
    (void)tam_terminal_write(g_term, &b);
    tam_buf_free(&b);
}

static int pager_read_prompt(pager_t *st, const char *label,
                             char *query, size_t query_cap)
{
    size_t qlen = 0;
    query[0] = '\0';
    int cancelled = 0;
    pager_draw_prompt(st, label, query, qlen);

    while (!g_stop) {
        unsigned char ch;
        int r = read_byte(st->input_fd, -1, &ch);
        if (r <= 0) {
            break;
        }
        if (ch == '\r' || ch == '\n') {
            query[qlen] = '\0';
            break;
        }
        if (ch == 0x1b || ch == 0x03) {
            cancelled = 1;
            break;
        }
        if (ch == '\b' || ch == 127) {
            if (qlen > 0) {
                qlen--;
                while (qlen > 0 && ((unsigned char)query[qlen] & 0xc0) == 0x80)
                    qlen--;
                query[qlen] = '\0';
                pager_draw_prompt(st, label, query, qlen);
            }
            continue;
        }
        if ((isprint(ch) || (unsigned char)ch >= 0x80) && qlen < query_cap - 1) {
            query[qlen++] = (char)ch;
            query[qlen] = '\0';
            pager_draw_prompt(st, label, query, qlen);
        }
    }

    tam_buf_t b = {0};
    (void)tam_terminal_cap(g_term, &b, g_term->civis, 0, 0);
    (void)tam_terminal_write(g_term, &b);
    tam_buf_free(&b);
    tam_pager_screen_invalidate(&g_screen);

    return !cancelled;
}

static void pager_search_prompt(pager_t *st)
{
    char query[128];
    if (pager_read_prompt(st, "Search in page /", query, sizeof(query))) {
        size_t qlen = strlen(query);
        if (qlen > 0) {
            pager_perform_search(st, query);
        } else if (st->search_query[0] != '\0') {
            pager_next_hit(st, 1);
        }
    }
}

static char *pager_uri_encode(const char *text)
{
    static const char hex[] = "0123456789ABCDEF";
    size_t n = strlen(text);
    char *out = malloc(n * 3 + 1);
    if (out == NULL) return NULL;
    size_t used = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)text[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            out[used++] = (char)c;
        else {
            out[used++] = '%';
            out[used++] = hex[c >> 4];
            out[used++] = hex[c & 15];
        }
    }
    out[used] = '\0';
    return out;
}

static int pager_prompt_action(pager_t *st, const char *href, char *out,
                               size_t out_cap)
{
    const char *label = NULL;
    const char *route = NULL;
    if (strcmp(href, "tam:prompt:guide-search") == 0) {
        label = "Search in guide \\"; route = "tam:search?query=";
    } else if (strcmp(href, "tam:prompt:man-open") == 0) {
        label = "Manual page [section]: "; route = "tam:man:open?query=";
    } else if (strcmp(href, "tam:prompt:man-keyword") == 0) {
        label = "Manual keyword: "; route = "tam:man:keyword?query=";
    } else if (strcmp(href, "tam:prompt:man-section") == 0) {
        label = "Manual section: "; route = "tam:man:section?section=";
    } else if (strcmp(href, "tam:prompt:tldr-open") == 0) {
        label = "tldr command: "; route = "tam:tldr:open?query=";
    } else if (strcmp(href, "tam:prompt:tldr-keyword") == 0) {
        label = "tldr keyword: "; route = "tam:tldr:keyword?query=";
    } else return 0;

    char input[128];
    out[0] = '\0';
    if (!pager_read_prompt(st, label, input, sizeof(input)) || input[0] == '\0')
        return 1;
    char *encoded = pager_uri_encode(input);
    if (encoded == NULL) {
        snprintf(st->msg, sizeof(st->msg), "could not encode lookup input");
        return 1;
    }
    snprintf(out, out_cap, "%s%s", route, encoded);
    free(encoded);
    return 1;
}

static void emit_line(const tam_line_t *line, int width,
                      const linkref_t *sel, size_t line_idx,
                      const pager_t *st)
{
    int attention = st != NULL && (int)line_idx == st->cursor_line;
    int col = 0;
    size_t byte_pos = 0;
    size_t hit_first = 0;
    size_t hit_end = 0;
    if (st != NULL && st->nhits > 0) {
        while (hit_first < st->nhits && st->hits[hit_first].line_idx < line_idx)
            hit_first++;
        hit_end = hit_first;
        while (hit_end < st->nhits && st->hits[hit_end].line_idx == line_idx)
            hit_end++;
    }

    int band_start = 0, band_end = 0;
    int band = !attention && column_band(st, &band_start, &band_end);

    if (attention) emit_attention(1);
    int active_style = 0;
    int active_tint = attention;
    const tam_span_t *active_span = NULL;
    for (size_t j = 0; j < line->nspans && col < width; j++) {
        const tam_span_t *sp = &line->spans[j];
        const char *t = sp->text != NULL ? sp->text : "";
        size_t len = strlen(t);
        size_t off = 0;
        int is_link = sp->href != NULL;
        int is_sel = 0;
        if (is_link && sel != NULL && !sel->header) {
            if (sp->link_id > 0 && sel->link_id > 0)
                is_sel = sp->link_id == sel->link_id;
            else if (sel->href != NULL)
                is_sel = strcmp(sp->href, sel->href) == 0;
        }
        while (off < len) {
            size_t char_start = off;
            size_t bytes = 0;
            wchar_t wc;
            int cells = 0;
            (void)tam_display_next(t, len, &off, &bytes, &wc, &cells);
            if (cells > 0 && col + cells > width) break;
            size_t absolute = byte_pos + char_start;
            int hit = 0;
            for (size_t m = hit_first; m < hit_end; m++) {
                const search_hit_t *h = &st->hits[m];
                if (absolute >= h->byte_start && absolute < h->byte_end) {
                    hit = (int)m == st->cur_hit ? 2 : 1;
                    break;
                }
            }
            int link_space = is_link && isspace((unsigned char)t[char_start]);
            int selected_interior_space = link_space && is_sel &&
                link_space_is_interior(t, len, char_start);
            int span_style = !hit &&
                (sp->href != NULL ? (!link_space || selected_interior_space)
                                  : sp->style != 0);
            int style_key = hit ? 2 + hit : span_style ? 1 : 0;
            int tint = attention || (band && col >= band_start && col < band_end);
            if (style_key != active_style || (style_key == 1 && active_span != sp) ||
                tint != active_tint) {
                if (active_style != 0 || tint != active_tint) emit_reset(tint);
                if (hit) emit_search_style(hit);
                else if (span_style) (void)emit_style(sp, is_sel);
                active_style = style_key;
                active_span = sp;
                active_tint = tint;
            }
            if (wc == 0xfffd) outn("?", 1);
            else outn(t + char_start, bytes);
            col += cells;
        }
        byte_pos += len;
    }
    if (active_style != 0 || active_tint != attention) emit_reset(attention);
    if (attention) {
        emit_attention(1);
        while (col < width) {
            outn(" ", 1);
            col++;
        }
        emit_reset(0);
    } else if (band) {
        emit_band_padding(st, col, width);
    }
}

static size_t status_byte_limit(const char *s, int max_cols)
{
    if (max_cols <= 0) return 0;
    return tam_display_prefix(s, strlen(s), (size_t)max_cols, NULL);
}

static int nearest_logical_line(const tam_document_t *doc, size_t block,
                                size_t offset, int fallback)
{
    if (doc == NULL || doc->nlines == 0) return 0;
    size_t best_distance = (size_t)-1;
    int best = -1;
    for (size_t i = 0; i < doc->nlines; i++) {
        if (doc->lines[i].logical_block != block) continue;
        size_t at = doc->lines[i].logical_offset;
        size_t distance = at > offset ? at - offset : offset - at;
        if (distance < best_distance) { best_distance = distance; best = (int)i; }
    }
    if (best >= 0) return best;
    if (fallback < 0) return 0;
    if ((size_t)fallback >= doc->nlines) return (int)doc->nlines - 1;
    return fallback;
}

static void pager_reflow(pager_t *st, int width)
{
    if (width < 1 || width == st->layout_width || st->doc == NULL) return;
    size_t cursor_block = 0, cursor_offset = 0, top_block = 0, top_offset = 0;
    if (st->doc->nlines > 0) {
        size_t ci = (size_t)st->cursor_line < st->doc->nlines ? (size_t)st->cursor_line : 0;
        size_t ti = (size_t)st->top < st->doc->nlines ? (size_t)st->top : 0;
        cursor_block = st->doc->lines[ci].logical_block;
        cursor_offset = st->doc->lines[ci].logical_offset;
        top_block = st->doc->lines[ti].logical_block;
        top_offset = st->doc->lines[ti].logical_offset;
    }
    int cursor_row = st->cursor_line - st->top;
    if (cursor_row < 0) cursor_row = 0;
    int old_link = st->selected;
    int old_link_id = old_link >= 0 && (size_t)old_link < st->nlinks
        ? st->links[old_link].link_id : 0;
    char *old_href = old_link >= 0 && (size_t)old_link < st->nlinks &&
                     st->links[old_link].href != NULL
        ? strdup(st->links[old_link].href) : NULL;
    int had_search = st->search_query[0] != '\0';
    char query[sizeof(st->search_query)];
    snprintf(query, sizeof(query), "%s", st->search_query);
    size_t hit_block = 0, hit_offset = 0;
    int had_hit = had_search && st->cur_hit >= 0 &&
                  (size_t)st->cur_hit < st->nhits;
    if (had_hit) {
        size_t hi = st->hits[st->cur_hit].line_idx;
        if (hi < st->doc->nlines) {
            hit_block = st->doc->lines[hi].logical_block;
            hit_offset = st->doc->lines[hi].logical_offset;
        }
    }
    if (tam_document_reflow(st->doc, width) < 0) {
        free(old_href);
        return;
    }
    st->layout_width = width;
    st->cursor_line = nearest_logical_line(st->doc, cursor_block, cursor_offset,
                                           st->cursor_line);
    st->top = nearest_logical_line(st->doc, top_block, top_offset, st->top);
    st->top = st->cursor_line - cursor_row;
    if (st->top < 0) st->top = 0;
    pager_build_links(st);
    if (old_link_id > 0 || old_href != NULL) {
        for (size_t i = 0; i < st->nlinks; i++) {
            int same_id = old_link_id > 0 && st->links[i].link_id == old_link_id;
            int same_href = old_href != NULL && st->links[i].href != NULL &&
                            strcmp(old_href, st->links[i].href) == 0;
            if (same_id || same_href) { st->selected = (int)i; break; }
        }
    }
    free(old_href);
    if (had_search) {
        pager_perform_search(st, query);
        if (had_hit && st->nhits > 0) {
            size_t best_distance = (size_t)-1;
            int best = st->cur_hit;
            for (size_t i = 0; i < st->nhits; i++) {
                size_t li = st->hits[i].line_idx;
                if (li >= st->doc->nlines ||
                    st->doc->lines[li].logical_block != hit_block) continue;
                size_t at = st->doc->lines[li].logical_offset;
                size_t distance = at > hit_offset ? at - hit_offset : hit_offset - at;
                if (distance < best_distance) { best_distance = distance; best = (int)i; }
            }
            st->cur_hit = best;
            st->cursor_line = (int)st->hits[best].line_idx;
        }
    }
}

static int lens_href(const char *href);

static void pager_address(const pager_t *st, char *out, size_t cap)
{
    char with_lens[600];
    if (st->cur_href != NULL && lens_href(st->cur_href) &&
        tam_href_put_level(st->cur_href, st->lens, with_lens,
                           sizeof(with_lens))) {
        snprintf(out, cap, "%s%.240s",
                 strncmp(with_lens, "tam:", 4) == 0 ? "" : "page:", with_lens);
        return;
    }
    if (st->cur_href == NULL ||
        strcmp(st->cur_href, "-") == 0 ||
        st->cur_href[0] == '<') {
        snprintf(out, cap, "(standard input has no address)");
        return;
    }
    if (strncmp(st->cur_href, "man:", 4) == 0 ||
        strncmp(st->cur_href, "info:", 5) == 0 ||
        strncmp(st->cur_href, "tldr:", 5) == 0 ||
        strncmp(st->cur_href, "tam:", 4) == 0 ||
        strncmp(st->cur_href, "page:", 5) == 0) {
        snprintf(out, cap, "%s", st->cur_href);
        return;
    }
    snprintf(out, cap, "page:%s", st->cur_href);
}

static int draw(pager_t *st)
{
    st->width = tam_terminal_width();
    int h = terminal_height();
    st->height = h > 0 ? h : 24;
    pager_reflow(st, content_width(st));
    if (g_term != NULL) {
        g_term->rows = st->height;
        g_term->cols = st->width;
    }

    int usable = content_rows(st);
    int max_top = (int)st->doc->nlines - usable;
    if (max_top < 0) {
        max_top = 0;
    }
    if (st->top > max_top) {
        st->top = max_top;
    }
    if (st->top < 0) {
        st->top = 0;
    }

    const linkref_t *sel = selected_link(st);
    int paint_width = st->width > 1 ? st->width - 1 : 1;
    if (g_term != NULL && g_term->gutter && paint_width > 1) paint_width--;
    int nframe = st->height;
    tam_buf_t *frame = calloc((size_t)nframe, sizeof(*frame));
    if (frame == NULL) return -1;
    g_capture = NULL;
    int content_row = 0;
    int has_header = st->height > 2 && st->doc->location.nspans > 0;
    if (has_header) {
        g_capture = &frame[0];
        emit_location_line(st, paint_width, sel);
        content_row = 1;
    }
    for (int row = 0; row < usable; row++) {
        g_capture = &frame[content_row + row];
        if (st->help_open) {
            static const char *help[] = {
                "Navigation help",
                "Back: Left Arrow   Guide: `   Help: h or ?   Quit: q",
                "Line: d/k down, s/l up   Top: x or .   Bottom: c or ,",
                "Links on the line: a/j left, f/; right   Any link: ] next, [ previous",
                "Follow: g, Enter or Right Arrow   Tab / Shift-Tab: same as ] / [",
                "Search: / this page, \\ the guide   Next/previous hit: n/N   Address: =",
                "Answers: Enter toggles selected, r reveals next, * toggles all",
                "Info Up: u   Next/Previous node: n/p",
                "Level, kept across pages: 0 All, 1 Beginner, 2 Intermediate, 3 Expert",
                "Close help with h, ? or Esc. Focus, position and history stay put."
            };
            if ((size_t)row < sizeof(help) / sizeof(help[0]))
                outn(help[row], status_byte_limit(help[row], paint_width));
            continue;
        }
        int li = st->top + row;
        if (g_term != NULL && g_term->gutter) {
            char marker = li == st->cursor_line ? '>' : ' ';
            outn(&marker, 1);
        }
        if (li < (int)st->doc->nlines) {
            emit_line(&st->doc->lines[li], paint_width, sel, (size_t)li, st);
        }
        if (li >= (int)st->doc->nlines && (int)li == st->cursor_line) {
            emit_attention(1);
            for (int c = 0; c < paint_width; c++) out(" ");
            emit_reset(0);
        } else if (li >= (int)st->doc->nlines) {
            emit_band_padding(st, 0, paint_width);
        }
    }

    char status[512] = {0};
    int max_cols = paint_width;
    if (st->show_address) {
        char addr[256];
        pager_address(st, addr, sizeof(addr));
        snprintf(status, sizeof(status), "Path = %s", addr);
    } else {
        const char *actions = max_cols >= 60 ? "< Back  ` Guide  h Help  q Quit" :
                              max_cols >= 40 ? "<Back `Guide hHelp qQuit" : "< ` h q";
        const char *title = st->doc->title ? st->doc->title : "tam";
        size_t total = st->doc->nlines;
        if (total == 1 && st->doc->lines[0].nspans == 0) {
            total = 0;
        }
        int first = total == 0 ? 0 : st->top + 1;
        int last = st->top + usable;
        if (last > (int)total) {
            last = (int)total;
        }
        if (total == 0) {
            last = 0;
        }
        char essential[64];
        snprintf(essential, sizeof(essential), "L%d %d-%d/%zu",
                 st->cursor_line + 1, first, last, total);
        char lens[48];
        int lens_number = tam_level_number(st->lens);
        if (max_cols >= 100)
            snprintf(lens, sizeof(lens), "Level %d %s", lens_number,
                     tam_level_label(st->lens));
        else if (max_cols >= 60)
            snprintf(lens, sizeof(lens), "Lv%d", lens_number);
        else
            snprintf(lens, sizeof(lens), "[%d]", lens_number);
        if (st->doc->levels != 0 && st->lens != 0 && !(st->doc->levels & st->lens))
            strncat(lens, " (not on page)", sizeof(lens) - strlen(lens) - 1);
        strncat(essential, max_cols >= 60 ? " | " : " ",
                sizeof(essential) - strlen(essential) - 1);
        strncat(essential, lens, sizeof(essential) - strlen(essential) - 1);
        int essential_width = (int)tam_display_width(essential, strlen(essential));
        if (max_cols >= 60) {
            int action_width = (int)tam_display_width(actions, strlen(actions));
            int title_budget = max_cols - action_width - essential_width - 6;
            if (title_budget > 0) {
                size_t title_bytes = status_byte_limit(title, title_budget);
                strncat(status, title, title_bytes);
                strncat(status, " | ", sizeof(status) - strlen(status) - 1);
            }
            strncat(status, essential, sizeof(status) - strlen(status) - 1);
            strncat(status, " | ", sizeof(status) - strlen(status) - 1);
            strncat(status, actions, sizeof(status) - strlen(status) - 1);
        } else {
            strncat(status, essential, sizeof(status) - 1);
            int used = (int)tam_display_width(status, strlen(status));
            int action_width = (int)tam_display_width(actions, strlen(actions));
            if (max_cols - used >= action_width + 1) {
                strncat(status, " ", sizeof(status) - strlen(status) - 1);
                strncat(status, actions, sizeof(status) - strlen(status) - 1);
            }
        }
        if (st->doc->nav_up != NULL && max_cols >= 39)
            strncat(status, "  u Up", sizeof(status) - strlen(status) - 1);

        const char *hints = (st->doc->nav_next != NULL || st->doc->nav_prev != NULL ||
                             st->doc->nav_up != NULL)
            ? " n/p t i asdf jkl; g"
            : " / n/N asdf jkl; g u `";
        int used = (int)tam_display_width(status, strlen(status));
        int hint_width = (int)tam_display_width(hints, strlen(hints));
        if (used + hint_width <= max_cols)
            strncat(status, hints, sizeof(status) - strlen(status) - 1);
        const char *detail = st->msg[0] != '\0' ? st->msg :
                             sel != NULL && sel->href != NULL ? sel->href : NULL;
        if (detail != NULL) {
            char part[192];
            snprintf(part, sizeof(part), st->msg[0] != '\0' ? " [%s]" : " -> %s",
                     detail);
            used = (int)tam_display_width(status, strlen(status));
            int part_width = (int)tam_display_width(part, strlen(part));
            if (used + part_width <= max_cols)
                strncat(status, part, sizeof(status) - strlen(status) - 1);
        }
    }

    if (st->height >= 2) {
        g_capture = &frame[st->height - 1];
        size_t slen = status_byte_limit(status, paint_width);
        outn(status, slen);
    }
    g_capture = NULL;
    int rc = tam_pager_screen_update(&g_screen, frame, nframe, paint_width);
    for (int i = 0; i < nframe; i++) tam_buf_free(&frame[i]);
    free(frame);
    if (rc < 0) {
        g_stop = 1;
        return -1;
    }
    return 0;
}

static void history_push(pager_t *st, const char *href, int top,
                         int cursor_line)
{
    st->history = realloc(st->history,
                          (st->nhistory + 1) * sizeof(*st->history));
    st->history[st->nhistory].href = strdup(href);
    const linkref_t *sel = selected_link(st);
    st->history[st->nhistory].selected_href = sel ? strdup(sel->href) : NULL;
    st->history[st->nhistory].selected_link_id = sel ? sel->link_id : 0;
    st->history[st->nhistory].selected_header = sel ? sel->header : 0;
    st->history[st->nhistory].top = top;
    st->history[st->nhistory].cursor_line = cursor_line;
    if (st->doc != NULL && st->doc->nlines > 0) {
        size_t ti = top >= 0 && (size_t)top < st->doc->nlines ? (size_t)top : 0;
        size_t ci = cursor_line >= 0 && (size_t)cursor_line < st->doc->nlines
            ? (size_t)cursor_line : 0;
        st->history[st->nhistory].top_block = st->doc->lines[ti].logical_block;
        st->history[st->nhistory].top_offset = st->doc->lines[ti].logical_offset;
        st->history[st->nhistory].cursor_block = st->doc->lines[ci].logical_block;
        st->history[st->nhistory].cursor_offset = st->doc->lines[ci].logical_offset;
        st->history[st->nhistory].width = content_width(st);
    }
    st->nhistory++;
}

static void forward_push(pager_t *st, const char *href, int top,
                         int cursor_line)
{
    st->forward = realloc(st->forward,
                          (st->nforward + 1) * sizeof(*st->forward));
    st->forward[st->nforward].href = strdup(href);
    const linkref_t *sel = selected_link(st);
    st->forward[st->nforward].selected_href = sel ? strdup(sel->href) : NULL;
    st->forward[st->nforward].selected_link_id = sel ? sel->link_id : 0;
    st->forward[st->nforward].selected_header = sel ? sel->header : 0;
    st->forward[st->nforward].top = top;
    st->forward[st->nforward].cursor_line = cursor_line;
    if (st->doc != NULL && st->doc->nlines > 0) {
        size_t ti = top >= 0 && (size_t)top < st->doc->nlines ? (size_t)top : 0;
        size_t ci = cursor_line >= 0 && (size_t)cursor_line < st->doc->nlines
            ? (size_t)cursor_line : 0;
        st->forward[st->nforward].top_block = st->doc->lines[ti].logical_block;
        st->forward[st->nforward].top_offset = st->doc->lines[ti].logical_offset;
        st->forward[st->nforward].cursor_block = st->doc->lines[ci].logical_block;
        st->forward[st->nforward].cursor_offset = st->doc->lines[ci].logical_offset;
        st->forward[st->nforward].width = content_width(st);
    }
    st->nforward++;
}

static void forward_clear(pager_t *st)
{
    while (st->nforward > 0) {
        nav_state_t *state = &st->forward[--st->nforward];
        free(state->href);
        free(state->selected_href);
    }
    free(st->forward);
    st->forward = NULL;
    st->nforward = 0;
}

static void restore_nav_state(pager_t *st, int top, int cursor_line)
{
    int nlines = (st->doc != NULL) ? (int)st->doc->nlines : 0;
    if (nlines <= 0) {
        st->top = 0;
        st->cursor_line = 0;
        st->selected = -1;
        return;
    }
    int usable = content_rows(st);
    int max_top = nlines - usable;
    if (max_top < 0) {
        max_top = 0;
    }
    if (top < 0) {
        top = 0;
    }
    if (top > max_top) {
        top = max_top;
    }
    if (cursor_line < 0) {
        cursor_line = 0;
    }
    if (cursor_line >= nlines) {
        cursor_line = nlines - 1;
    }
    if (cursor_line < top) {
        top = cursor_line;
    } else if (cursor_line >= top + usable) {
        top = cursor_line - usable + 1;
        if (top > max_top) {
            top = max_top;
        }
        if (top < 0) {
            top = 0;
        }
    }
    st->top = top;
    st->cursor_line = cursor_line;
    st->selected = -1;
}

static void restore_logical_nav_state(pager_t *st, const nav_state_t *nav)
{
    restore_nav_state(st, nav->top, nav->cursor_line);
    if (nav->width != content_width(st) && st->doc != NULL && st->doc->nlines > 0) {
        st->cursor_line = nearest_logical_line(st->doc, nav->cursor_block,
                                               nav->cursor_offset, st->cursor_line);
        st->top = nearest_logical_line(st->doc, nav->top_block,
                                       nav->top_offset, st->top);
        int usable = content_rows(st);
        if (st->cursor_line < st->top) st->top = st->cursor_line;
        if (st->cursor_line >= st->top + usable)
            st->top = st->cursor_line - usable + 1;
        if (st->top < 0) st->top = 0;
    }
    if (nav->selected_href != NULL) {
        for (size_t i = 0; i < st->nlinks; i++) {
            const linkref_t *link = &st->links[i];
            if (link->href != NULL && strcmp(link->href, nav->selected_href) == 0 &&
                link->header == nav->selected_header &&
                (nav->selected_link_id == 0 || link->link_id == nav->selected_link_id)) {
                st->selected = (int)i;
                break;
            }
        }
    }
}

static int is_external(const char *href)
{
    return strncmp(href, "http:", 5) == 0 || strncmp(href, "https:", 6) == 0 ||
           strncmp(href, "mailto:", 7) == 0 || strncmp(href, "ftp:", 4) == 0 ||
           strstr(href, "://") != NULL;
}

static int match_anchor(const char *haystack, const char *needle)
{
    if (haystack == NULL || needle == NULL) {
        return 0;
    }
    size_t nlen = strlen(needle);
    if (nlen == 0) {
        return 0;
    }
    for (size_t i = 0; haystack[i] != '\0'; i++) {
        if (strncasecmp(haystack + i, needle, nlen) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Fallback for pages without id= anchors: a heading containing the target,
 * else any unlinked text. A link never counts as its own target. */
static int find_anchor_text(const tam_document_t *doc, const char *target)
{
    int plain = -1;
    for (size_t i = 0; i < doc->nlines; i++) {
        for (size_t j = 0; j < doc->lines[i].nspans; j++) {
            const tam_span_t *sp = &doc->lines[i].spans[j];
            if (sp->href != NULL || !match_anchor(sp->text, target)) continue;
            if (sp->style & TAM_STYLE_HEADING) return (int)i;
            if (plain < 0) plain = (int)i;
        }
    }
    return plain;
}

/* Guide pages and the generated topic views take the reader's lens; man,
 * Info, tldr, other tam: pages and standard input have one level. */
static int lens_href(const char *href)
{
    if (href == NULL || href[0] == '\0' || href[0] == '<' || href[0] == '#' ||
        strcmp(href, "-") == 0) {
        return 0;
    }
    size_t scheme = strcspn(href, ":/?#");
    if (href[scheme] != ':') {
        return 1;
    }
    return strncmp(href, "tam:explain", 11) == 0 ||
           strncmp(href, "tam:topics", 10) == 0 ||
           strncmp(href, "tam:keywords", 12) == 0;
}

/* Load `href` (stored without level=) at the reader's lens. A view that
 * cannot be shown at the lens is loaded at every level instead. */
static tam_document_t *load_at_lens(pager_t *st, const char *href)
{
    if (st->lens != 0 && lens_href(href)) {
        size_t cap = strlen(href) + 32;
        char *target = malloc(cap);
        if (!target) return NULL;
        int ok = tam_href_put_level(href, st->lens, target, cap);
        tam_document_t *nd = ok ? st->load(st->ctx, target, content_width(st)) : NULL;
        free(target);
        if (nd != NULL) return nd;
    }
    return st->load(st->ctx, href, content_width(st));
}

static void follow(pager_t *st, const char *href)
{
    if (href == NULL || href[0] == '\0') {
        return;
    }
    if (strncmp(href, "tam:fold:", 9) == 0) {
        char *end;
        unsigned long id = strtoul(href + 9, &end, 10);
        if (id > 0 && id <= (unsigned)-1 && *end == 0) toggle_fold(st, (unsigned)id);
        return;
    }
    if (strcmp(href, "tam:help") == 0) {
        st->help_open = 1;
        return;
    }
    if (strcmp(href, "tam:prompt:page-search") == 0) {
        pager_search_prompt(st);
        return;
    }
    if (strncmp(href, "tam:prompt:", 11) == 0) {
        char target[512];
        if (pager_prompt_action(st, href, target, sizeof(target)) && target[0] != '\0')
            follow(st, target);
        return;
    }
    if (href[0] == '#') {
        const char *target = href + 1;
        if (target[0] == '\0') {
            history_push(st, st->cur_href, st->top, st->cursor_line);
            forward_clear(st);
            set_cursor_line(st, 0, -1);
            st->msg[0] = '\0';
            return;
        }
        int line = tam_document_anchor_line(st->doc, target);
        if (line < 0) line = find_anchor_text(st->doc, target);
        if (line < 0) {
            snprintf(st->msg, sizeof(st->msg), "anchor not found: %s", target);
            return;
        }
        history_push(st, st->cur_href, st->top, st->cursor_line);
        forward_clear(st);
        set_cursor_line(st, line, 1);
        /* Show the target at the top of the view, as less and browsers do. */
        int max_top = (int)st->doc->nlines - content_rows(st);
        st->top = line < max_top ? line : (max_top > 0 ? max_top : 0);
        if (!link_is_visible(st, st->selected)) st->selected = -1;
        snprintf(st->msg, sizeof(st->msg), "jumped to %s", target);
        return;
    }
    if (is_external(href)) {
        snprintf(st->msg, sizeof(st->msg), "external link (not opened)");
        return;
    }
    /* A link that names a level sets the lens; history keeps the address
     * without it, so Back and Forward show pages at the current lens. */
    char bare[600];
    snprintf(bare, sizeof(bare), "%s", href);
    unsigned named = 0;
    if (lens_href(bare) && tam_href_take_level(bare, &named) &&
        named != st->lens) {
        st->lens = named;
        tam_lens_set(named);
    }
    href = bare;
    tam_document_t *nd = load_at_lens(st, href);
    if (nd == NULL) {
        if (strcmp(href, "index.html") == 0)
            snprintf(st->msg, sizeof(st->msg), "Guide unavailable; Left Arrow returns to the previous page");
        else
            snprintf(st->msg, sizeof(st->msg), "cannot open %.140s", href);
        return;
    }
    free(st->fold_open); st->fold_open = NULL; st->nfold_open = 0;
    tam_document_ensure_location(nd, href);
    history_push(st, st->cur_href, st->top, st->cursor_line);
    forward_clear(st);
    pager_clear_search(st);
    free(st->cur_href);
    st->cur_href = strdup(href);
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    st->top = 0;
    st->cursor_line = 0;
    st->selected = -1;
    st->msg[0] = '\0';
}

/* First heading line at or above `line`, or -1. */
static int heading_at_or_above(const tam_document_t *doc, int line)
{
    for (int i = line; i >= 0; i--) {
        if ((size_t)i < doc->nlines && doc->lines[i].nspans > 0 &&
            (doc->lines[i].spans[0].style & TAM_STYLE_HEADING)) {
            return i;
        }
    }
    return -1;
}

/* Re-render the current page at the lens without touching history. The view
 * stays on the heading it was under when that heading is still shown. */
static int reload_in_place(pager_t *st)
{
    size_t cap = strlen(st->cur_href) + st->nfold_open * 12 + 16;
    char *target = malloc(cap);
    if (!target) return 0;
    size_t used = (size_t)snprintf(target, cap, "%s%sfold=", st->cur_href,
                                  strchr(st->cur_href, '?') ? "&" : "?");
    int first = 1;
    for (size_t i = 1; i < st->nfold_open; i++) {
        if (!st->fold_open[i]) continue;
        used += (size_t)snprintf(target + used, cap - used, "%s%zu", first ? "" : ".", i);
        first = 0;
    }
    tam_document_t *nd = load_at_lens(st, target);
    free(target);
    if (nd == NULL) {
        return 0;
    }
    tam_document_ensure_location(nd, st->cur_href);
    int cursor_row = st->cursor_line - st->top;
    int top = st->top;
    char *heading = NULL;
    int under = heading_at_or_above(st->doc, st->top);
    if (under >= 0) {
        heading = strdup(st->doc->lines[under].spans[0].text);
    }
    int offset = st->top - under;
    pager_clear_search(st);
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    if (heading != NULL) {
        for (size_t i = 0; i < nd->nlines; i++) {
            if (nd->lines[i].nspans > 0 &&
                (nd->lines[i].spans[0].style & TAM_STYLE_HEADING) &&
                strcmp(nd->lines[i].spans[0].text, heading) == 0) {
                top = (int)i + offset;
                break;
            }
        }
        free(heading);
    }
    restore_nav_state(st, top, top + cursor_row);
    return 1;
}

/* Fold state belongs to this page, never to its address or history entry. */
static int fold_set(pager_t *st, unsigned id, int open)
{
    if (id >= st->nfold_open) {
        size_t n = (size_t)id + 1;
        unsigned char *next = realloc(st->fold_open, n);
        if (!next) return 0;
        memset(next + st->nfold_open, 0, n - st->nfold_open);
        st->fold_open = next; st->nfold_open = n;
    }
    st->fold_open[id] = (unsigned char)open;
    return 1;
}
static void fold_message(pager_t *st)
{
    size_t total = 0, shown = 0;
    for (size_t i = 0; i < st->doc->nfolds; i++) {
        tam_fold_t f = st->doc->folds[i];
        if (f.quiz) { total++; shown += f.open != 0; }
    }
    if (total) snprintf(st->msg, sizeof(st->msg), "Quiz %zu/%zu revealed", shown, total);
    else st->msg[0] = 0;
}
static void select_fold(pager_t *st, unsigned id)
{
    char href[40]; snprintf(href, sizeof(href), "tam:fold:%u", id);
    for (size_t i = 0; i < st->nlinks; i++) {
        if (strcmp(st->links[i].href, href)) continue;
        set_cursor_line(st, (int)st->links[i].line_start, 1);
        st->selected = (int)i;
        return;
    }
}
static void toggle_fold(pager_t *st, unsigned id)
{
    for (size_t i = 0; i < st->doc->nfolds; i++) {
        tam_fold_t f = st->doc->folds[i];
        if (f.id != id) continue;
        if (!fold_set(st, id, !f.open) || !reload_in_place(st)) {
            snprintf(st->msg, sizeof(st->msg), "cannot toggle answer"); return;
        }
        select_fold(st, id); fold_message(st); return;
    }
}
static void reveal_next(pager_t *st)
{
    for (size_t i = 0; i < st->nlinks; i++) {
        if (st->links[i].line_start < (size_t)st->cursor_line || strncmp(st->links[i].href, "tam:fold:", 9)) continue;
        unsigned id = (unsigned)strtoul(st->links[i].href + 9, NULL, 10);
        if (id < st->nfold_open && st->fold_open[id]) continue;
        toggle_fold(st, id); return;
    }
    snprintf(st->msg, sizeof(st->msg), "all answers shown");
}
static void toggle_all_folds(pager_t *st)
{
    if (!st->doc->nfolds) {
        snprintf(st->msg, sizeof(st->msg), "all answers shown"); return;
    }
    int all = 1;
    for (size_t i = 0; i < st->doc->nfolds; i++)
        if (!st->doc->folds[i].open) all = 0;
    for (size_t i = 0; i < st->doc->nfolds; i++)
        if (!fold_set(st, st->doc->folds[i].id, !all)) return;
    if (reload_in_place(st)) fold_message(st);
}

static void set_lens(pager_t *st, unsigned char key)
{
    unsigned level = 0;
    if (!tam_level_parse_arg((char[]){ (char)key, '\0' }, &level)) {
        return;
    }
    int number = tam_level_number(level);
    const char *label = tam_level_label(level);
    if (level == st->lens) {
        snprintf(st->msg, sizeof(st->msg), "already at Level %d %s", number,
                 label);
        return;
    }
    st->lens = level;
    tam_lens_set(level);
    if (!lens_href(st->cur_href) || !reload_in_place(st)) {
        snprintf(st->msg, sizeof(st->msg),
                 "Level %d %s set; this page has one level", number, label);
        return;
    }
    if (st->doc->levels != 0 && level != 0 && !(st->doc->levels & level))
        snprintf(st->msg, sizeof(st->msg),
                 "Level %d %s set; this page has no %s version", number,
                 label, label);
    else if (st->doc->hidden > 0)
        snprintf(st->msg, sizeof(st->msg), "Level %d %s: %d section%s hidden",
                 number, label, st->doc->hidden,
                 st->doc->hidden == 1 ? "" : "s");
    else
        snprintf(st->msg, sizeof(st->msg), "Level %d %s set", number, label);
}

static void go_back(pager_t *st)
{
    if (st->nhistory == 0) {
        snprintf(st->msg, sizeof(st->msg), "no earlier page");
        return;
    }
    nav_state_t prev = st->history[--st->nhistory];
    if (strcmp(prev.href, st->cur_href) == 0) {
        forward_push(st, st->cur_href, st->top, st->cursor_line);
        pager_clear_search(st);
        free(prev.href);
        restore_logical_nav_state(st, &prev);
        free(prev.selected_href);
        st->msg[0] = '\0';
        return;
    }
    tam_document_t *nd = load_at_lens(st, prev.href);
    if (nd == NULL) {
        st->nhistory++;
        snprintf(st->msg, sizeof(st->msg), "cannot open %s", prev.href);
        free(prev.href);
        free(prev.selected_href);
        return;
    }
    free(st->fold_open); st->fold_open = NULL; st->nfold_open = 0;
    tam_document_ensure_location(nd, prev.href);
    forward_push(st, st->cur_href, st->top, st->cursor_line);
    pager_clear_search(st);
    free(st->cur_href);
    st->cur_href = prev.href;
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    restore_logical_nav_state(st, &prev);
    free(prev.selected_href);
    st->msg[0] = '\0';
}

static void go_forward(pager_t *st)
{
    if (st->nforward == 0) {
        return;
    }
    nav_state_t next = st->forward[--st->nforward];
    if (strcmp(next.href, st->cur_href) == 0) {
        history_push(st, st->cur_href, st->top, st->cursor_line);
        pager_clear_search(st);
        free(next.href);
        restore_logical_nav_state(st, &next);
        free(next.selected_href);
        st->msg[0] = '\0';
        return;
    }
    tam_document_t *nd = load_at_lens(st, next.href);
    if (nd == NULL) {
        st->nforward++;
        snprintf(st->msg, sizeof(st->msg), "cannot open %s", next.href);
        free(next.href);
        free(next.selected_href);
        return;
    }
    free(st->fold_open); st->fold_open = NULL; st->nfold_open = 0;
    tam_document_ensure_location(nd, next.href);
    history_push(st, st->cur_href, st->top, st->cursor_line);
    pager_clear_search(st);
    free(st->cur_href);
    st->cur_href = next.href;
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    restore_logical_nav_state(st, &next);
    free(next.selected_href);
    st->msg[0] = '\0';
}

static int find_first_visible_link(const pager_t *st)
{
    if (st->nlinks == 0) {
        return -1;
    }
    int usable = content_rows(st);
    size_t top_line = (size_t)st->top;
    size_t bot_line = (size_t)(st->top + usable - 1);

    for (size_t i = 0; i < st->nlinks; i++) {
        if (st->links[i].header) continue;
        if (st->links[i].line_start >= (size_t)st->cursor_line &&
            st->links[i].line_start <= bot_line) {
            return (int)i;
        }
    }
    for (size_t i = 0; i < st->nlinks; i++) {
        if (st->links[i].line_end >= top_line && st->links[i].line_start <= bot_line) {
            return (int)i;
        }
    }
    return 0;
}

static void ensure_link_visible(pager_t *st, int dir)
{
    if (st->selected < 0 || (size_t)st->selected >= st->nlinks) {
        return;
    }
    if (st->links[st->selected].header) return;
    int usable = content_rows(st);
    int max_top = (int)st->doc->nlines - usable;
    if (max_top < 0) {
        max_top = 0;
    }
    size_t l_start = st->links[st->selected].line_start;
    size_t l_end = st->links[st->selected].line_end;
    if (dir >= 0) {
        if ((int)l_start < st->top) {
            st->top = (int)l_start;
        } else if ((int)l_end >= st->top + usable) {
            st->top = (int)l_end - usable + 1;
        }
    } else {
        if ((int)l_start < st->top) {
            st->top = (int)l_start;
        } else if ((int)l_start >= st->top + usable) {
            st->top = (int)l_start - usable + 1;
        }
    }
    if (st->top < 0) {
        st->top = 0;
    }
    if (st->top > max_top) {
        st->top = max_top;
    }
}

static void select_next_link(pager_t *st, int dir, int include_header)
{
    if (st->nlinks == 0) {
        return;
    }
    if (st->selected < 0) {
        if (dir < 0 && include_header) {
            st->selected = (int)st->nlinks - 1;
        } else {
            st->selected = find_first_visible_link(st);
        }
        if (!st->links[st->selected].header)
            st->cursor_line = (int)st->links[st->selected].line_start;
        ensure_link_visible(st, dir);
        st->msg[0] = '\0';
        return;
    }
    int candidate = st->selected;
    for (size_t tries = 0; tries < st->nlinks; tries++) {
        candidate += dir;
        if (candidate < 0) candidate = (int)st->nlinks - 1;
        else if ((size_t)candidate >= st->nlinks) candidate = 0;
        if (include_header || !st->links[candidate].header) break;
    }
    if (!include_header && st->links[candidate].header) return;
    st->selected = candidate;
    if (!st->links[st->selected].header)
        st->cursor_line = (int)st->links[st->selected].line_start;
    ensure_link_visible(st, dir);
    st->msg[0] = '\0';
}

static int header_link_range(const pager_t *st, int *first, int *last)
{
    *first = -1;
    *last = -1;
    for (size_t i = 0; i < st->nlinks; i++) {
        if (!st->links[i].header) continue;
        if (*first < 0) *first = (int)i;
        *last = (int)i;
    }
    return *first >= 0;
}

static int select_breadcrumb(pager_t *st)
{
    int first, last;
    if (!header_link_range(st, &first, &last)) return 0;
    st->selected = last;
    st->msg[0] = '\0';
    return 1;
}

/* Down from the breadcrumb row returns to the attention line, which stays
 * where it was while the row is focused, even when the page has no links. */
static void leave_breadcrumb(pager_t *st)
{
    st->selected = -1;
    set_cursor_line(st, st->cursor_line, 1);
    st->msg[0] = '\0';
}

static void select_horizontal_header_link(pager_t *st, int dir)
{
    int first, last;
    if (!header_link_range(st, &first, &last) || st->selected < first ||
        st->selected > last) return;
    int next = st->selected + dir;
    if (next < first) next = last;
    else if (next > last) next = first;
    st->selected = next;
    st->msg[0] = '\0';
}

/* Up and down move the attention line one row. Up from the first line enters
 * the breadcrumb row; there, up moves to the previous breadcrumb link and
 * down returns to the attention line. */
static void move_line_up(pager_t *st)
{
    if (st->selected >= 0 && (size_t)st->selected < st->nlinks &&
        st->links[st->selected].header) {
        int previous = st->selected - 1;
        if (previous >= 0 && st->links[previous].header) st->selected = previous;
        st->msg[0] = '\0';
    } else if (st->cursor_line <= 0 && st->height > 2 &&
               st->doc->location.nspans > 0 && select_breadcrumb(st)) {
        /* The fixed location line is above the document's first line. */
    } else {
        set_cursor_line(st, st->cursor_line - 1, -1);
    }
}

static void move_line_down(pager_t *st)
{
    if (st->selected >= 0 && (size_t)st->selected < st->nlinks &&
        st->links[st->selected].header)
        leave_breadcrumb(st);
    else
        set_cursor_line(st, st->cursor_line + 1, 1);
}

/* Left and right move the attention column from link to link along the
 * attention line, or along the breadcrumb row while it is focused, wrapping
 * at either end. A line without links has nothing to catch. */
static void move_column(pager_t *st, int dir)
{
    if (st->selected >= 0 && (size_t)st->selected < st->nlinks &&
        st->links[st->selected].header) {
        select_horizontal_header_link(st, dir);
        st->column_active = 1;
        return;
    }
    int first = -1, last = -1, prev = -1, next = -1, on_line = 0;
    for (size_t i = 0; i < st->nlinks; i++) {
        const linkref_t *l = &st->links[i];
        if (l->header || l->line_start > (size_t)st->cursor_line ||
            l->line_end < (size_t)st->cursor_line) continue;
        if (first < 0) first = (int)i;
        last = (int)i;
        if ((int)i < st->selected) prev = (int)i;
        else if ((int)i == st->selected) on_line = 1;
        else if (next < 0) next = (int)i;
    }
    if (first < 0) return;
    if (!on_line) st->selected = dir > 0 ? first : last;
    else if (dir > 0) st->selected = next >= 0 ? next : first;
    else st->selected = prev >= 0 ? prev : last;
    st->column_active = 1;
    st->msg[0] = '\0';
}

static void follow_selected(pager_t *st)
{
    const linkref_t *current = selected_link(st);
    if (current != NULL && current->href != NULL) {
        follow(st, current->href);
        return;
    }
    int cur_link = find_first_link_on_line(st, (size_t)st->cursor_line);
    if (cur_link >= 0) {
        if (st->selected >= 0 && (size_t)st->selected < st->nlinks &&
            st->links[st->selected].line_start <= (size_t)st->cursor_line &&
            st->links[st->selected].line_end >= (size_t)st->cursor_line) {
            /* Keep currently selected link on this line */
        } else {
            st->selected = cur_link;
        }
    } else {
        snprintf(st->msg, sizeof(st->msg), "no link selected");
        return;
    }

    const linkref_t *sel = selected_link(st);
    if (sel != NULL && sel->href != NULL) {
        follow(st, sel->href);
    }
}

/* Returns 1 on a byte, 0 on timeout (or an interrupted wait), and -1 on EOF
 * or a read error. Distinguishing EOF lets the pager exit when its input is
 * closed rather than spinning. */
static int read_byte(int fd, int timeout_ms, unsigned char *c)
{
    if (g_pending_len > 0) {
        *c = g_pending[0];
        memmove(g_pending, g_pending + 1, --g_pending_len);
        return 1;
    }
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    int r = poll(&pfd, 1, timeout_ms);
    if (r < 0) {
        return errno == EINTR ? 0 : -1;
    }
    if (r == 0) {
        return 0;
    }
    ssize_t n = read(fd, c, 1);
    if (n == 1) {
        return 1;
    }
    if (n == 0) {
        return -1;
    }
    return errno == EINTR ? 0 : -1;
}

static int consume_sync_reply(pager_t *st, const unsigned char *bytes,
                              size_t len)
{
    static const unsigned char prefix[] = {'?', '2', '0', '2', '6', ';'};
    for (size_t start = 0; start < len; start++) {
        if (bytes[start] != '?') continue;
        size_t matched[9];
        size_t count = 1;
        matched[0] = start;
        size_t at = start + 1;
        int ok = 1;
        for (size_t p = 1; p < sizeof(prefix); p++) {
            while (at < len && bytes[at] != prefix[p]) at++;
            if (at == len) { ok = 0; break; }
            matched[count++] = at++;
        }
        if (!ok) continue;
        while (at < len && (bytes[at] < '0' || bytes[at] > '4')) at++;
        if (at == len) continue;
        unsigned char status = bytes[at];
        matched[count++] = at++;
        while (at < len && bytes[at] != '$') at++;
        if (at == len) continue;
        matched[count++] = at++;
        while (at < len && bytes[at] != 'y') at++;
        if (at == len) continue;
        matched[count++] = at;

        unsigned char interleaved[128];
        size_t ninterleaved = 0;
        size_t mi = 0;
        for (size_t i = 0; i < len; i++) {
            if (mi < count && i == matched[mi]) { mi++; continue; }
            if (ninterleaved < sizeof(interleaved)) interleaved[ninterleaved++] = bytes[i];
        }
        pending_prepend(interleaved, ninterleaved);
        st->sync_queried = 1;
        st->sync_output = status != '0';
        if (g_term != NULL) g_term->sync_output = st->sync_output;
        return 1;
    }
    pending_prepend(bytes, len);
    return 0;
}

static void read_sync_reply(pager_t *st, int fd)
{
    unsigned char bytes[64];
    size_t len = 1;
    bytes[0] = '?';
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (len < sizeof(bytes)) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed = (now.tv_sec - start.tv_sec) * 1000L +
                      (now.tv_nsec - start.tv_nsec) / 1000000L;
        int remaining = 120 - (int)elapsed;
        if (remaining <= 0) break;
        unsigned char c;
        int rc = read_byte(fd, remaining, &c);
        if (rc != 1) break;
        bytes[len++] = c;
        if (len >= 2 && bytes[len - 2] == '$' && bytes[len - 1] == 'y') {
            if (consume_sync_reply(st, bytes, len)) return;
        }
    }
    if (!st->sync_queried) {
        pending_prepend(bytes, len);
        st->sync_queried = 1;
        st->sync_output = 0;
        if (g_term != NULL) g_term->sync_output = 0;
    }
}

/* Returns 1 if the escape sequence was consumed, and sets *handled to the
 * action code. This keeps arrow/page keys working without a full key table. */
static void handle_key_inner(pager_t *st, unsigned char c)
{
    if (st->help_open && c != '?' && c != 'h' && c != 'q' && c != 0x1b) return;
    int usable = content_rows(st);
    int max_top = (int)st->doc->nlines - usable;
    if (max_top < 0) {
        max_top = 0;
    }

    switch (c) {
    case 'q':
        g_stop = 1;
        return;
    case '?':
    case 'h':
        st->help_open = !st->help_open;
        break;
    case 'r':
        reveal_next(st);
        break;
    case '*':
        toggle_all_folds(st);
        break;
    case '=':
        st->show_address = !st->show_address;
        break;
    case '/':
        pager_search_prompt(st);
        break;
    case '\\':
        follow(st, "tam:prompt:guide-search");
        break;
    case 'n':
        if (st->search_query[0] != '\0') {
            pager_next_hit(st, 1);
        } else if (st->doc->nav_next != NULL) {
            follow(st, st->doc->nav_next);
        }
        break;
    case 'p':
        if (st->doc->nav_prev != NULL) {
            follow(st, st->doc->nav_prev);
        }
        break;
    case 't':
        if (st->doc->nav_top != NULL) {
            follow(st, st->doc->nav_top);
        }
        break;
    case 'i':
        follow(st, "info:dir");
        break;
    /* Home-row keys: each hand has left, up, down, right under one finger
     * each, mirrored (D175). */
    case 'a':
    case 'j':
        move_column(st, -1);
        break;
    case 's':
    case 'l':
        move_line_up(st);
        break;
    case 'd':
    case 'k':
        move_line_down(st);
        break;
    case 'f':
    case ';':
        move_column(st, 1);
        break;
    /* The same fingers one row down go all the way. */
    case 'x':
    case '.':
        set_cursor_line(st, 0, -1);
        break;
    case 'c':
    case ',':
        set_cursor_line(st, (int)st->doc->nlines - 1, 1);
        break;
    case ' ':
        set_cursor_line(st, st->cursor_line + usable, 0);
        break;
    case 'b':
        set_cursor_line(st, st->cursor_line - usable, 0);
        break;
    case 'g':
        follow_selected(st);
        break;
    case ']':
    case '\t':
        select_next_link(st, 1, 1);
        break;
    case '[':
        select_next_link(st, -1, 1);
        break;
    case 'N':
        if (st->search_query[0] != '\0') {
            pager_next_hit(st, -1);
        }
        break;
    case '\r':
    case '\n': {
        follow_selected(st);
        break;
    }
    case 'u':
        if (st->doc->nav_up != NULL) {
            follow(st, st->doc->nav_up);
        } else {
            go_back(st);
        }
        break;
    case '`':
        if (strcmp(st->cur_href, "index.html") != 0) {
            follow(st, "index.html");
        } else {
            set_cursor_line(st, 0, -1);
            st->msg[0] = '\0';
        }
        break;
    case 0x1b: {
        unsigned char c1;
        if (read_byte(st->input_fd, 60, &c1) != 1) {
            if (st->help_open) {
                st->help_open = 0;
            } else if (st->show_address) {
                st->show_address = 0;
            } else if (st->search_query[0] != '\0') {
                pager_clear_search(st);
                st->msg[0] = '\0';
            } else {
                g_stop = 1;
            }
            return;
        }
        if (c1 == '\t') {
            select_next_link(st, -1, 1);
            return;
        }
        if (c1 != '[') {
            return;
        }
        char seq[16];
        size_t n = 0;
        unsigned char c2;
        if (read_byte(st->input_fd, 60, &c2) != 1) return;
        if (c2 == '?') {
            read_sync_reply(st, st->input_fd);
            return;
        }
        seq[n++] = (char)c2;
        if (!((c2 >= 'A' && c2 <= 'Z') || (c2 >= 'a' && c2 <= 'z') || c2 == '~')) {
            while (n < sizeof(seq) - 1 && read_byte(st->input_fd, 60, &c2) == 1) {
                seq[n++] = (char)c2;
                if ((c2 >= 'A' && c2 <= 'Z') || (c2 >= 'a' && c2 <= 'z') ||
                    c2 == '~') {
                    break;
                }
            }
        }
        seq[n] = '\0';
        if (strcmp(seq, "A") == 0) {
            move_line_up(st);
        } else if (strcmp(seq, "B") == 0) {
            move_line_down(st);
        } else if (strcmp(seq, "C") == 0) {
            if (st->selected >= 0) {
                follow_selected(st);
            } else if (st->nforward > 0) {
                go_forward(st);
            }
        } else if (strcmp(seq, "D") == 0) {
            go_back(st);
        } else if (strcmp(seq, "H") == 0) {
            set_cursor_line(st, 0, -1);
        } else if (strcmp(seq, "F") == 0) {
            set_cursor_line(st, (int)st->doc->nlines - 1, 1);
        } else if (strcmp(seq, "5~") == 0) {
            set_cursor_line(st, st->cursor_line - usable, 0);
        } else if (strcmp(seq, "6~") == 0) {
            set_cursor_line(st, st->cursor_line + usable, 0);
        } else if (strcmp(seq, "Z") == 0 || strcmp(seq, "1;2Z") == 0 ||
                   strcmp(seq, "9;2u") == 0 || strcmp(seq, "27;2;9~") == 0) {
            select_next_link(st, -1, 1);
        }
        break;
    }
    case '0':
    case '1':
    case '2':
    case '3':
        set_lens(st, c);
        break;
    default:
        break;
    }

    if (st->top < 0) {
        st->top = 0;
    }
    if (st->top > max_top) {
        st->top = max_top;
    }
}


/* The attention column lives only on the line where it was raised: any key
 * that moves the attention line, changes the selection or the page clears
 * it. Only the left and right keys raise it. */
static void handle_key(pager_t *st, unsigned char c)
{
    int column_key = c == 'a' || c == 'j' || c == 'f' || c == ';';
    const tam_document_t *doc = st->doc;
    int line = st->cursor_line;
    int selected = st->selected;
    handle_key_inner(st, c);
    if (st->doc != doc)
        st->show_address = 0;
    if (c == 'g' || c == '\r' || c == '\n' ||
        (!column_key && (st->doc != doc || st->cursor_line != line ||
                         st->selected != selected)))
        st->column_active = 0;
}

int tam_pager_run(tam_document_t *initial, const char *initial_href,
                  tam_page_loader_t load, void *ctx)
{
    g_stop = 0;
    g_resized = 0;
    g_pending_len = 0;
    pager_t st;
    memset(&st, 0, sizeof(st));
    st.doc = initial;
    st.cur_href = strdup(initial_href);
    /* An address that names a level sets the lens for this run only. */
    unsigned named = 0;
    int named_level = lens_href(st.cur_href) &&
                      tam_href_take_level(st.cur_href, &named);
    if (named_level)
        tam_lens_override(named);
    st.lens = tam_lens_get();
    tam_document_ensure_location(st.doc, initial_href);
    st.selected = -1;
    st.cursor_line = 0;
    st.sweep_dir = 1;
    st.load = load;
    st.ctx = ctx;
    st.width = tam_terminal_width();
    st.height = terminal_height();
    st.selected = -1;

    if (!isatty(STDOUT_FILENO)) {
        char *plain = tam_document_text(st.doc);
        int rc = plain == NULL ? 1 : 0;
        if (plain != NULL) { fputs(plain, stdout); free(plain); }
        tam_document_free(st.doc);
        free(st.fold_open);
        free(st.cur_href);
        free(st.links);
        return rc;
    }

    tam_terminal_t term;
    if (!tam_terminal_init(&term, STDOUT_FILENO, st.height, st.width)) {
        char *plain = tam_document_text(st.doc);
        int rc = plain == NULL ? 1 : 0;
        if (plain != NULL) { fputs(plain, stdout); free(plain); }
        tam_document_free(st.doc);
        free(st.fold_open);
        free(st.cur_href);
        free(st.links);
        return rc;
    }
    g_term = &term;
    tam_pager_screen_init(&g_screen, &term);
    if (!named_level && st.lens != 0 && st.load != NULL && lens_href(st.cur_href)) {
        tam_document_t *nd = load_at_lens(&st, st.cur_href);
        if (nd != NULL) {
            tam_document_ensure_location(nd, st.cur_href);
            tam_document_free(st.doc);
            st.doc = nd;
        }
    }
    pager_build_links(&st);

    int input_fd = STDIN_FILENO;
    int tty_fd = -1;
    if (!isatty(input_fd)) {
        tty_fd = open("/dev/tty", O_RDONLY);
        if (tty_fd >= 0) {
            input_fd = tty_fd;
        } else {
            tam_pager_screen_free(&g_screen);
            g_term = NULL;
            tam_document_free(st.doc);
            free(st.fold_open);
            free(st.cur_href);
            free(st.links);
            return 1;
        }
    }
    st.input_fd = input_fd;

    struct termios orig;
    if (tcgetattr(input_fd, &orig) != 0) {
        if (tty_fd >= 0) {
            close(tty_fd);
        }
        tam_pager_screen_free(&g_screen);
        g_term = NULL;
        tam_document_free(st.doc);
        free(st.fold_open);
        free(st.cur_href);
        free(st.links);
        return 1;
    }
    struct termios raw = orig;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(input_fd, TCSANOW, &raw) != 0) {
        if (tty_fd >= 0) {
            close(tty_fd);
        }
        tam_pager_screen_free(&g_screen);
        g_term = NULL;
        tam_document_free(st.doc);
        free(st.fold_open);
        free(st.cur_href);
        free(st.links);
        return 1;
    }
    term.raw_mode = 1;

    struct sigaction sa, old_int, old_term, old_winch;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_stop;
    sigemptyset(&sa.sa_mask);
    int have_int = sigaction(SIGINT, &sa, &old_int) == 0;
    int have_term = sigaction(SIGTERM, &sa, &old_term) == 0;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_resize;
    sigemptyset(&sa.sa_mask);
    int have_winch = sigaction(SIGWINCH, &sa, &old_winch) == 0;

    tam_buf_t entry = {0};
    if (tam_terminal_enter(&term, &entry) == 0)
        (void)tam_terminal_write(&term, &entry);
    tam_buf_free(&entry);
    draw(&st);
    const char *term_name = getenv("TERM");
    if (term_name != NULL && strcmp(term_name, "linux") != 0) {
        tam_buf_t probe = {0};
        (void)tam_buf_puts(&probe, "\033[?2026$p");
        if (tam_terminal_write(&term, &probe) == 0) st.sync_probe_sent = 1;
        tam_buf_free(&probe);
    }

    while (!g_stop) {
        unsigned char c;
        int r = read_byte(st.input_fd, -1, &c);
        if (r < 0) {
            break;
        }
        if (r == 0) {
            if (g_resized) {
                g_resized = 0;
                draw(&st);
            }
            continue;
        }
        if (g_resized) {
            g_resized = 0;
            int usable = content_rows(&st);
            int max_top = (int)st.doc->nlines - usable;
            if (max_top < 0) {
                max_top = 0;
            }
            if (st.top > max_top) {
                st.top = max_top;
            }
            if (st.cursor_line >= (int)st.doc->nlines) {
                st.cursor_line = (int)st.doc->nlines - 1;
            }
            if (!link_is_visible(&st, st.selected)) {
                st.selected = -1;
            }
        }
        handle_key(&st, c);
        if (g_stop) {
            break;
        }
        draw(&st);
    }

    tam_buf_t leave = {0};
    if (tam_terminal_leave(&term, &leave) == 0)
        (void)tam_terminal_write(&term, &leave);
    tam_buf_free(&leave);

    tcsetattr(input_fd, TCSANOW, &orig);
    term.raw_mode = 0;
    if (tty_fd >= 0) {
        close(tty_fd);
    }

    while (st.nhistory > 0) {
        nav_state_t *state = &st.history[--st.nhistory];
        free(state->href);
        free(state->selected_href);
    }
    free(st.history);
    while (st.nforward > 0) {
        nav_state_t *state = &st.forward[--st.nforward];
        free(state->href);
        free(state->selected_href);
    }
    free(st.forward);
    free(st.links);
    free(st.fold_open);
    free(st.cur_href);
    pager_clear_search(&st);
    tam_document_free(st.doc);
    tam_pager_screen_free(&g_screen);
    g_term = NULL;

    if (have_int) (void)sigaction(SIGINT, &old_int, NULL);
    if (have_term) (void)sigaction(SIGTERM, &old_term, NULL);
    if (have_winch) (void)sigaction(SIGWINCH, &old_winch, NULL);
    return 0;
}
