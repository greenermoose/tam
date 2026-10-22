#include <tam.h>

#include "address.h"
#include "depth.h"
#include "display.h"
#include "pager.h"
#include "pager_screen.h"
#include "render.h"
#include "terminal.h"
#include "version.h"

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

#include "pager_int.h"

static const char *tam_casestr(const char *haystack, const char *needle);
static char *pager_line_text(const tam_line_t *line);
static void pager_draw_prompt(pager_t *st, const char *label,
                       const char *query, size_t qlen);
static int pager_read_prompt(pager_t *st, const char *label,
                      char *query, size_t query_cap);
static char *pager_uri_encode(const char *text);

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

void pager_clear_search(pager_t *st)
{
    st->search_query[0] = '\0';
    free(st->hits);
    st->hits = NULL;
    st->nhits = 0;
    st->cur_hit = -1;
}

void pager_perform_search(pager_t *st, const char *query)
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

void pager_next_hit(pager_t *st, int dir)
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

void pager_search_prompt(pager_t *st)
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

int pager_prompt_action(pager_t *st, const char *href, char *out,
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
