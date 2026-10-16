#include <tam.h>

#include "pager.h"
#include "render.h"

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
#include <unistd.h>

static volatile sig_atomic_t g_stop = 0;
static volatile sig_atomic_t g_resized = 0;

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
        return ws.ws_col;
    }
    const char *c = getenv("COLUMNS");
    if (c != NULL) {
        int v = atoi(c);
        if (v > 0) {
            return v;
        }
    }
    return 80;
}

static int terminal_height(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) {
        return ws.ws_row;
    }
    return 24;
}

static void out(const char *s)
{
    size_t n = strlen(s);
    if (n > 0) {
        ssize_t r = write(STDOUT_FILENO, s, n);
        (void)r;
    }
}

static void outn(const char *s, size_t n)
{
    if (n > 0) {
        ssize_t r = write(STDOUT_FILENO, s, n);
        (void)r;
    }
}

static int read_byte(int fd, int timeout_ms, unsigned char *c);

typedef struct {
    int               link_id;
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
    int   top;
    int   cursor_line;
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
    int             height;
    int             input_fd;
    char            msg[160];
    tam_page_loader_t load;
    void              *ctx;
    char            search_query[128];
    search_hit_t   *hits;
    size_t          nhits;
    int             cur_hit;
} pager_t;

static void pager_build_links(pager_t *st)
{
    free(st->links);
    st->links = NULL;
    st->nlinks = 0;
    st->selected = -1;

    for (size_t i = 0; i < st->doc->nlines; i++) {
        int col = 0;
        for (size_t j = 0; j < st->doc->lines[i].nspans; j++) {
            const tam_span_t *sp = &st->doc->lines[i].spans[j];
            size_t text_len = sp->text != NULL ? strlen(sp->text) : 0;
            int span_col_start = col;
            int span_col_end = col + (int)text_len;
            col += (int)text_len;

            if (sp->href == NULL) {
                continue;
            }

            size_t lead = 0;
            if (sp->text != NULL) {
                while (lead < text_len && isspace((unsigned char)sp->text[lead])) {
                    lead++;
                }
            }
            int link_col_start = span_col_start + (int)lead;
            int link_col_end = span_col_end;
            if (link_col_start >= link_col_end) {
                link_col_start = span_col_start;
            }

            if (st->nlinks > 0) {
                linkref_t *prev = &st->links[st->nlinks - 1];
                int is_same = 0;
                if (sp->link_id > 0 && prev->link_id > 0) {
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
            st->links[st->nlinks].href = sp->href;
            st->links[st->nlinks].line_start = i;
            st->links[st->nlinks].line_end = i;
            st->links[st->nlinks].col_start = link_col_start;
            st->links[st->nlinks].col_end = link_col_end;
            st->links[st->nlinks].span = sp;
            st->nlinks++;
        }
    }
}

static const linkref_t *selected_link(const pager_t *st)
{
    if (st->selected < 0 || (size_t)st->selected >= st->nlinks) {
        return NULL;
    }
    return &st->links[st->selected];
}

#define TAM_ATTENTION_BG "\033[48;5;236m"

static void emit_reset(int is_attention)
{
    if (is_attention) {
        out("\033[0m" TAM_ATTENTION_BG);
    } else {
        out("\033[0m");
    }
}

static int link_is_visible(const pager_t *st, int link_idx)
{
    if (link_idx < 0 || (size_t)link_idx >= st->nlinks) {
        return 0;
    }
    int usable = st->height - 1;
    if (usable < 1) {
        usable = 1;
    }
    size_t top_line = (size_t)st->top;
    size_t bot_line = (size_t)(st->top + usable - 1);
    const linkref_t *l = &st->links[link_idx];
    return (l->line_end >= top_line && l->line_start <= bot_line);
}

static int find_first_link_on_line(const pager_t *st, size_t line_idx)
{
    for (size_t i = 0; i < st->nlinks; i++) {
        if (st->links[i].line_start <= line_idx && st->links[i].line_end >= line_idx) {
            return (int)i;
        }
    }
    return -1;
}

static int find_last_link_on_line(const pager_t *st, size_t line_idx)
{
    int last = -1;
    for (size_t i = 0; i < st->nlinks; i++) {
        if (st->links[i].line_start <= line_idx && st->links[i].line_end >= line_idx) {
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

    int usable = st->height - 1;
    if (usable < 1) {
        usable = 1;
    }
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
    if (dir >= 0) {
        caught = find_first_link_on_line(st, (size_t)st->cursor_line);
    } else {
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
            out("\033[7;1m");
        } else {
            out("\033[4;34m");
        }
        return 1;
    }
    if (sp->style & TAM_STYLE_HEADING) {
        out("\033[1;36m");
        return 1;
    }
    int styled = 0;
    if (sp->style & TAM_STYLE_BOLD) {
        out("\033[1m");
        styled = 1;
    }
    if (sp->style & TAM_STYLE_ITALIC) {
        out("\033[3m");
        styled = 1;
    }
    if (sp->style & TAM_STYLE_CODE) {
        out("\033[33m");
        styled = 1;
    }
    return styled;
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

    int usable = st->height - 1;
    if (usable < 1) usable = 1;
    int hit_line = (int)st->hits[st->cur_hit].line_idx;
    if (hit_line < st->top || hit_line >= st->top + usable) {
        st->top = hit_line - usable / 3;
        int max_top = (int)st->doc->nlines - usable;
        if (max_top < 0) max_top = 0;
        if (st->top > max_top) st->top = max_top;
        if (st->top < 0) st->top = 0;
    }
    set_cursor_line(st, hit_line, 1);

    snprintf(st->msg, sizeof(st->msg), "[%d/%zu] %s", st->cur_hit + 1, st->nhits, st->search_query);
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

    int usable = st->height - 1;
    if (usable < 1) usable = 1;
    int hit_line = (int)st->hits[st->cur_hit].line_idx;
    if (hit_line < st->top || hit_line >= st->top + usable) {
        st->top = hit_line - usable / 3;
        int max_top = (int)st->doc->nlines - usable;
        if (max_top < 0) max_top = 0;
        if (st->top > max_top) st->top = max_top;
        if (st->top < 0) st->top = 0;
    }
    set_cursor_line(st, hit_line, dir);

    snprintf(st->msg, sizeof(st->msg), "[%d/%zu] %s", st->cur_hit + 1, st->nhits, st->search_query);
}

static void pager_search_prompt(pager_t *st)
{
    char prompt[32];
    snprintf(prompt, sizeof(prompt), "\033[%d;1H\033[K/", st->height);
    out(prompt);
    out("\033[?25h");

    char query[128];
    size_t qlen = 0;
    query[0] = '\0';
    int cancelled = 0;

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
                query[qlen] = '\0';
                out("\b \b");
            }
            continue;
        }
        if ((isprint(ch) || (unsigned char)ch >= 0x80) && qlen < sizeof(query) - 1) {
            query[qlen++] = (char)ch;
            query[qlen] = '\0';
            outn((char *)&ch, 1);
        }
    }

    out("\033[?25l");

    if (!cancelled) {
        if (qlen > 0) {
            pager_perform_search(st, query);
        } else if (st->search_query[0] != '\0') {
            pager_next_hit(st, 1);
        }
    }
}

static void emit_line(const tam_line_t *line, int width,
                      const linkref_t *sel, size_t line_idx,
                      const pager_t *st)
{
    int is_attention = (st != NULL && (int)line_idx == st->cursor_line);
    if (is_attention) {
        out(TAM_ATTENTION_BG);
    }

    if (line->nspans == 0) {
        if (is_attention) {
            for (int c = 0; c < width; c++) {
                outn(" ", 1);
            }
            out("\033[0m");
        }
        return;
    }

    size_t hit_start = 0, hit_count = 0;
    if (st != NULL && st->nhits > 0) {
        while (hit_start < st->nhits && st->hits[hit_start].line_idx < line_idx) {
            hit_start++;
        }
        while (hit_start + hit_count < st->nhits &&
               st->hits[hit_start + hit_count].line_idx == line_idx) {
            hit_count++;
        }
    }

    if (hit_count == 0) {
        int col = 0;
        for (size_t j = 0; j < line->nspans && col < width; j++) {
            const tam_span_t *sp = &line->spans[j];
            int is_link = (sp->href != NULL);
            int is_sel = 0;
            if (is_link && sel != NULL) {
                if (sp->link_id > 0 && sel->link_id > 0) {
                    is_sel = (sp->link_id == sel->link_id);
                } else if (sel->href != NULL && strcmp(sp->href, sel->href) == 0) {
                    is_sel = 1;
                }
            }

            const char *t = sp->text;
            size_t tl = strlen(t);

            if (is_link) {
                size_t k = 0;
                while (k < tl && isspace((unsigned char)t[k]) && col < width) {
                    outn(t + k, 1);
                    col++;
                    k++;
                }
                if (k < tl && col < width) {
                    int styled = emit_style(sp, is_sel);
                    while (k < tl && col < width) {
                        outn(t + k, 1);
                        col++;
                        k++;
                    }
                    if (styled) {
                        emit_reset(is_attention);
                    }
                }
            } else {
                int styled = emit_style(sp, 0);
                for (size_t k = 0; k < tl && col < width; k++) {
                    outn(t + k, 1);
                    col++;
                }
                if (styled) {
                    emit_reset(is_attention);
                }
            }
        }
        if (is_attention) {
            while (col < width) {
                outn(" ", 1);
                col++;
            }
            out("\033[0m");
        }
        return;
    }

    int col = 0;
    size_t line_byte_pos = 0;
    int active_hit_style = 0;

    for (size_t j = 0; j < line->nspans && col < width; j++) {
        const tam_span_t *sp = &line->spans[j];
        int is_link = (sp->href != NULL);
        int is_sel = 0;
        if (is_link && sel != NULL) {
            if (sp->link_id > 0 && sel->link_id > 0) {
                is_sel = (sp->link_id == sel->link_id);
            } else if (sel->href != NULL && strcmp(sp->href, sel->href) == 0) {
                is_sel = 1;
            }
        }

        const char *t = sp->text;
        size_t tl = strlen(t);
        size_t k = 0;

        if (is_link) {
            while (k < tl && isspace((unsigned char)t[k]) && col < width) {
                size_t cpos = line_byte_pos + k;
                int hit_type = 0;
                for (size_t m = 0; m < hit_count; m++) {
                    const search_hit_t *h = &st->hits[hit_start + m];
                    if (cpos >= h->byte_start && cpos < h->byte_end) {
                        hit_type = ((int)(hit_start + m) == st->cur_hit) ? 2 : 1;
                        break;
                    }
                }
                if (hit_type != active_hit_style) {
                    if (active_hit_style != 0) {
                        emit_reset(is_attention);
                    }
                    if (hit_type == 2) {
                        out("\033[7;1m");
                    } else if (hit_type == 1) {
                        out("\033[7m");
                    }
                    active_hit_style = hit_type;
                }
                outn(t + k, 1);
                col++;
                k++;
            }
        }

        int span_styled = 0;
        if (k < tl && col < width) {
            if (active_hit_style == 0) {
                span_styled = emit_style(sp, is_sel);
            }
            while (k < tl && col < width) {
                size_t cpos = line_byte_pos + k;
                int hit_type = 0;
                for (size_t m = 0; m < hit_count; m++) {
                    const search_hit_t *h = &st->hits[hit_start + m];
                    if (cpos >= h->byte_start && cpos < h->byte_end) {
                        hit_type = ((int)(hit_start + m) == st->cur_hit) ? 2 : 1;
                        break;
                    }
                }
                if (hit_type != active_hit_style) {
                    emit_reset(is_attention);
                    if (hit_type == 2) {
                        out("\033[7;1m");
                    } else if (hit_type == 1) {
                        out("\033[7m");
                    } else {
                        span_styled = emit_style(sp, is_sel);
                    }
                    active_hit_style = hit_type;
                }
                outn(t + k, 1);
                col++;
                k++;
            }
            if (active_hit_style != 0) {
                emit_reset(is_attention);
                active_hit_style = 0;
            } else if (span_styled) {
                emit_reset(is_attention);
            }
        }
        line_byte_pos += tl;
    }
    if (active_hit_style != 0) {
        emit_reset(is_attention);
    }
    if (is_attention) {
        while (col < width) {
            outn(" ", 1);
            col++;
        }
        out("\033[0m");
    }
}

static size_t status_byte_limit(const char *s, int max_cols)
{
    if (max_cols <= 0) {
        return 0;
    }
    int cols = 0;
    size_t i = 0;
    while (s[i] != '\0') {
        unsigned char c = (unsigned char)s[i];
        size_t clen = 1;
        if ((c & 0x80) == 0) {
            clen = 1;
        } else if ((c & 0xE0) == 0xC0) {
            clen = 2;
        } else if ((c & 0xF0) == 0xE0) {
            clen = 3;
        } else if ((c & 0xF8) == 0xF0) {
            clen = 4;
        }
        if (cols + 1 > max_cols) {
            break;
        }
        cols++;
        i += clen;
    }
    return i;
}

static void draw(pager_t *st)
{
    st->width = tam_terminal_width();
    int h = terminal_height();
    st->height = h > 0 ? h : 24;

    int usable = st->height - 1;
    if (usable < 1) {
        usable = 1;
    }
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

    out("\033[2J\033[H");
    const linkref_t *sel = selected_link(st);
    for (int row = 0; row < usable; row++) {
        int li = st->top + row;
        if (li < (int)st->doc->nlines) {
            emit_line(&st->doc->lines[li], st->width, sel, (size_t)li, st);
        }
        if (row < usable - 1) {
            out("\r\n");
        }
    }

    char status[512];
    const char *title = st->doc->title ? st->doc->title : "tam";
    char pos_metric[64];
    if (st->doc->nlines <= (size_t)usable) {
        snprintf(pos_metric, sizeof(pos_metric), "L%d/%zu (All)", st->cursor_line + 1, st->doc->nlines);
    } else if (st->top == 0) {
        snprintf(pos_metric, sizeof(pos_metric), "L%d/%zu (Top)", st->cursor_line + 1, st->doc->nlines);
    } else if (st->top >= max_top) {
        snprintf(pos_metric, sizeof(pos_metric), "L%d/%zu (Bot)", st->cursor_line + 1, st->doc->nlines);
    } else {
        int pct = (int)(((long)st->cursor_line * 100) / (st->doc->nlines > 1 ? (int)st->doc->nlines - 1 : 1));
        snprintf(pos_metric, sizeof(pos_metric), "L%d/%zu (%d%%)", st->cursor_line + 1, st->doc->nlines, pct);
    }

    int off;
    if (st->doc->nav_next != NULL || st->doc->nav_prev != NULL || st->doc->nav_up != NULL) {
        off = snprintf(status, sizeof(status),
                       " %s  |  %s  |  n/p next/prev  u up  t top  i dir  ` index  "
                       "a/s/d/f link  Enter open  q quit",
                       title, pos_metric);
    } else {
        off = snprintf(status, sizeof(status),
                       " %s  |  %s  |  / find  n/N next  a/s/d/f link  Enter/\xe2\x86\x92 open  "
                       "` index  u/\xe2\x86\x90 back  q/Esc quit",
                       title, pos_metric);
    }
    if (st->msg[0] != '\0') {
        off += snprintf(status + off, sizeof(status) - (size_t)off, "  [%s]",
                        st->msg);
    } else if (sel != NULL && sel->href != NULL) {
        off += snprintf(status + off, sizeof(status) - (size_t)off, "  -> %s",
                        sel->href);
    }
    (void)off;

    char pos[32];
    int pl = snprintf(pos, sizeof(pos), "\033[%d;1H\033[K", st->height);
    outn(pos, (size_t)pl);
    size_t slen = status_byte_limit(status, st->width);
    outn(status, slen);
}

static void history_push(pager_t *st, const char *href, int top,
                         int cursor_line)
{
    st->history = realloc(st->history,
                          (st->nhistory + 1) * sizeof(*st->history));
    st->history[st->nhistory].href = strdup(href);
    st->history[st->nhistory].top = top;
    st->history[st->nhistory].cursor_line = cursor_line;
    st->nhistory++;
}

static void forward_push(pager_t *st, const char *href, int top,
                         int cursor_line)
{
    st->forward = realloc(st->forward,
                          (st->nforward + 1) * sizeof(*st->forward));
    st->forward[st->nforward].href = strdup(href);
    st->forward[st->nforward].top = top;
    st->forward[st->nforward].cursor_line = cursor_line;
    st->nforward++;
}

static void forward_clear(pager_t *st)
{
    while (st->nforward > 0) {
        free(st->forward[--st->nforward].href);
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
    int usable = st->height - 1;
    if (usable < 1) {
        usable = 1;
    }
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

static void follow(pager_t *st, const char *href)
{
    if (href == NULL || href[0] == '\0') {
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
        for (size_t i = 0; i < st->doc->nlines; i++) {
            for (size_t j = 0; j < st->doc->lines[i].nspans; j++) {
                if (match_anchor(st->doc->lines[i].spans[j].text, target)) {
                    history_push(st, st->cur_href, st->top, st->cursor_line);
                    forward_clear(st);
                    set_cursor_line(st, (int)i, 1);
                    snprintf(st->msg, sizeof(st->msg), "jumped to %s", target);
                    return;
                }
            }
        }
        snprintf(st->msg, sizeof(st->msg), "anchor not found: %s", target);
        return;
    }
    if (is_external(href)) {
        snprintf(st->msg, sizeof(st->msg), "external link (not opened)");
        return;
    }
    tam_document_t *nd = st->load(st->ctx, href);
    if (nd == NULL) {
        snprintf(st->msg, sizeof(st->msg), "cannot open %s", href);
        return;
    }
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
        restore_nav_state(st, prev.top, prev.cursor_line);
        st->msg[0] = '\0';
        return;
    }
    tam_document_t *nd = st->load(st->ctx, prev.href);
    if (nd == NULL) {
        st->nhistory++;
        snprintf(st->msg, sizeof(st->msg), "cannot open %s", prev.href);
        free(prev.href);
        return;
    }
    forward_push(st, st->cur_href, st->top, st->cursor_line);
    pager_clear_search(st);
    free(st->cur_href);
    st->cur_href = prev.href;
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    restore_nav_state(st, prev.top, prev.cursor_line);
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
        restore_nav_state(st, next.top, next.cursor_line);
        st->msg[0] = '\0';
        return;
    }
    tam_document_t *nd = st->load(st->ctx, next.href);
    if (nd == NULL) {
        st->nforward++;
        snprintf(st->msg, sizeof(st->msg), "cannot open %s", next.href);
        free(next.href);
        return;
    }
    history_push(st, st->cur_href, st->top, st->cursor_line);
    pager_clear_search(st);
    free(st->cur_href);
    st->cur_href = next.href;
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    restore_nav_state(st, next.top, next.cursor_line);
    st->msg[0] = '\0';
}

static int find_first_visible_link(const pager_t *st)
{
    if (st->nlinks == 0) {
        return -1;
    }
    int usable = st->height - 1;
    if (usable < 1) {
        usable = 1;
    }
    size_t top_line = (size_t)st->top;
    size_t bot_line = (size_t)(st->top + usable - 1);

    for (size_t i = 0; i < st->nlinks; i++) {
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
    int usable = st->height - 1;
    if (usable < 1) {
        usable = 1;
    }
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

static void select_next_link(pager_t *st, int dir)
{
    if (st->nlinks == 0) {
        return;
    }
    if (st->selected < 0) {
        st->selected = find_first_visible_link(st);
        st->cursor_line = (int)st->links[st->selected].line_start;
        ensure_link_visible(st, dir);
        st->msg[0] = '\0';
        return;
    }
    st->selected += dir;
    if (st->selected < 0) {
        st->selected = (int)st->nlinks - 1;
    } else if ((size_t)st->selected >= st->nlinks) {
        st->selected = 0;
    }
    st->cursor_line = (int)st->links[st->selected].line_start;
    ensure_link_visible(st, dir);
    st->msg[0] = '\0';
}

static void select_vertical_link(pager_t *st, int dir)
{
    if (st->nlinks == 0) {
        return;
    }
    if (st->selected < 0) {
        st->selected = find_first_visible_link(st);
        st->cursor_line = (int)st->links[st->selected].line_start;
        ensure_link_visible(st, dir);
        st->msg[0] = '\0';
        return;
    }

    int cur = st->selected;
    size_t cur_line = (dir < 0) ? st->links[cur].line_start : st->links[cur].line_end;
    int cur_col = (st->links[cur].col_start + st->links[cur].col_end) / 2;

    int target_line = -1;

    if (dir < 0) {
        /* Move up: find nearest line strictly above cur_line with links */
        for (size_t i = 0; i < st->nlinks; i++) {
            if (st->links[i].line_start < cur_line) {
                if (target_line < 0 || (int)st->links[i].line_start > target_line) {
                    target_line = (int)st->links[i].line_start;
                }
            }
        }
        /* If no link above, wrap around to the bottom-most line with links */
        if (target_line < 0) {
            for (size_t i = 0; i < st->nlinks; i++) {
                if (target_line < 0 || (int)st->links[i].line_start > target_line) {
                    target_line = (int)st->links[i].line_start;
                }
            }
        }
    } else {
        /* Move down: find nearest line strictly below cur_line with links */
        for (size_t i = 0; i < st->nlinks; i++) {
            if (st->links[i].line_start > cur_line) {
                if (target_line < 0 || (int)st->links[i].line_start < target_line) {
                    target_line = (int)st->links[i].line_start;
                }
            }
        }
        /* If no link below, wrap around to the top-most line with links */
        if (target_line < 0) {
            for (size_t i = 0; i < st->nlinks; i++) {
                if (target_line < 0 || (int)st->links[i].line_start < target_line) {
                    target_line = (int)st->links[i].line_start;
                }
            }
        }
    }

    /* Among links on target_line, find the one closest horizontally */
    int best_k = -1;
    int best_dist = -1;
    for (size_t i = 0; i < st->nlinks; i++) {
        if ((int)st->links[i].line_start == target_line) {
            int k_col = (st->links[i].col_start + st->links[i].col_end) / 2;
            int dist = abs(k_col - cur_col);
            if (best_dist < 0 || dist < best_dist) {
                best_dist = dist;
                best_k = (int)i;
            }
        }
    }

    /* If all links are on the same line and best_k == cur, cycle */
    if (best_k == cur && st->nlinks > 1) {
        if (dir < 0) {
            best_k = (cur - 1 + (int)st->nlinks) % (int)st->nlinks;
        } else {
            best_k = (cur + 1) % (int)st->nlinks;
        }
    }

    if (best_k >= 0) {
        st->selected = best_k;
        st->cursor_line = (int)st->links[best_k].line_start;
    }
    ensure_link_visible(st, dir);
    st->msg[0] = '\0';
}

static void follow_selected(pager_t *st)
{
    int cur_link = find_first_link_on_line(st, (size_t)st->cursor_line);
    if (cur_link >= 0) {
        if (st->selected >= 0 && (size_t)st->selected < st->nlinks &&
            st->links[st->selected].line_start <= (size_t)st->cursor_line &&
            st->links[st->selected].line_end >= (size_t)st->cursor_line) {
            /* Keep currently selected link on this line */
        } else {
            st->selected = cur_link;
        }
    } else if (!link_is_visible(st, st->selected)) {
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

/* Returns 1 if the escape sequence was consumed, and sets *handled to the
 * action code. This keeps arrow/page keys working without a full key table. */
static void handle_key(pager_t *st, unsigned char c)
{
    int usable = st->height - 1;
    if (usable < 1) {
        usable = 1;
    }
    int max_top = (int)st->doc->nlines - usable;
    if (max_top < 0) {
        max_top = 0;
    }

    switch (c) {
    case 'q':
        g_stop = 1;
        return;
    case '/':
        pager_search_prompt(st);
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
    case 'a':
        select_next_link(st, -1);
        break;
    case 's':
        select_vertical_link(st, -1);
        break;
    case 'd':
        select_vertical_link(st, 1);
        break;
    case 'f':
        select_next_link(st, 1);
        break;
    case 'j':
        set_cursor_line(st, st->cursor_line + 1, 1);
        break;
    case 'k':
        set_cursor_line(st, st->cursor_line - 1, -1);
        break;
    case ' ':
        set_cursor_line(st, st->cursor_line + usable, 1);
        break;
    case 'b':
        set_cursor_line(st, st->cursor_line - usable, -1);
        break;
    case 'g':
        set_cursor_line(st, 0, -1);
        break;
    case 'G':
        set_cursor_line(st, (int)st->doc->nlines - 1, 1);
        break;
    case '\t':
        select_next_link(st, 1);
        break;
    case 'N':
        if (st->search_query[0] != '\0') {
            pager_next_hit(st, -1);
        } else {
            select_next_link(st, -1);
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
            if (st->search_query[0] != '\0') {
                pager_clear_search(st);
                st->msg[0] = '\0';
            } else {
                g_stop = 1;
            }
            return;
        }
        if (c1 == '\t') {
            select_next_link(st, -1);
            return;
        }
        if (c1 != '[') {
            return;
        }
        char seq[16];
        size_t n = 0;
        unsigned char c2;
        while (n < sizeof(seq) - 1 && read_byte(st->input_fd, 60, &c2) == 1) {
            seq[n++] = (char)c2;
            if ((c2 >= 'A' && c2 <= 'Z') || (c2 >= 'a' && c2 <= 'z') ||
                c2 == '~') {
                break;
            }
        }
        seq[n] = '\0';
        if (strcmp(seq, "A") == 0) {
            set_cursor_line(st, st->cursor_line - 1, -1);
        } else if (strcmp(seq, "B") == 0) {
            set_cursor_line(st, st->cursor_line + 1, 1);
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
            set_cursor_line(st, st->cursor_line - usable, -1);
        } else if (strcmp(seq, "6~") == 0) {
            set_cursor_line(st, st->cursor_line + usable, 1);
        } else if (strcmp(seq, "Z") == 0 || strcmp(seq, "1;2Z") == 0 ||
                   strcmp(seq, "9;2u") == 0 || strcmp(seq, "27;2;9~") == 0) {
            select_next_link(st, -1);
        }
        break;
    }
    default:
        if (c >= '1' && c <= '9') {
            int idx = c - '1';
            if ((size_t)idx < st->nlinks) {
                st->selected = idx;
                st->cursor_line = (int)st->links[idx].line_start;
                follow(st, st->links[idx].href);
            }
        }
        break;
    }

    if (st->top < 0) {
        st->top = 0;
    }
    if (st->top > max_top) {
        st->top = max_top;
    }
}

int tam_pager_run(tam_document_t *initial, const char *initial_href,
                  tam_page_loader_t load, void *ctx)
{
    pager_t st;
    memset(&st, 0, sizeof(st));
    st.doc = initial;
    st.cur_href = strdup(initial_href);
    st.selected = -1;
    st.cursor_line = 0;
    st.sweep_dir = 1;
    st.load = load;
    st.ctx = ctx;
    st.width = tam_terminal_width();
    st.height = terminal_height();
    pager_build_links(&st);
    st.selected = -1;

    int input_fd = STDIN_FILENO;
    int tty_fd = -1;
    if (!isatty(input_fd)) {
        tty_fd = open("/dev/tty", O_RDONLY);
        if (tty_fd >= 0) {
            input_fd = tty_fd;
        } else {
            tam_document_free(st.doc);
            free(st.cur_href);
            return 1;
        }
    }
    st.input_fd = input_fd;

    struct termios orig;
    if (tcgetattr(input_fd, &orig) != 0) {
        if (tty_fd >= 0) {
            close(tty_fd);
        }
        tam_document_free(st.doc);
        free(st.cur_href);
        return 1;
    }
    struct termios raw = orig;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_stop;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_resize;
    sigaction(SIGWINCH, &sa, NULL);

    if (tcsetattr(input_fd, TCSANOW, &raw) != 0) {
        if (tty_fd >= 0) {
            close(tty_fd);
        }
        tam_document_free(st.doc);
        free(st.cur_href);
        return 1;
    }

    out("\033[?1049h\033[?25l");
    draw(&st);

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
            int usable = st.height - 1;
            if (usable < 1) {
                usable = 1;
            }
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

    out("\033[?25h\033[?1049l");

    tcsetattr(input_fd, TCSANOW, &orig);
    if (tty_fd >= 0) {
        close(tty_fd);
    }

    while (st.nhistory > 0) {
        free(st.history[--st.nhistory].href);
    }
    free(st.history);
    while (st.nforward > 0) {
        free(st.forward[--st.nforward].href);
    }
    free(st.forward);
    free(st.links);
    free(st.cur_href);
    pager_clear_search(&st);
    tam_document_free(st.doc);

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGWINCH, SIG_DFL);
    return 0;
}
