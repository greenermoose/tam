#include <tam.h>

#include "address.h"
#include "depth.h"
#include "display.h"
#include "external.h"
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

static void outn(const char *s, size_t n);
static void emit_location_line(pager_t *st, int width, const linkref_t *sel);
static void emit_attention(int enabled);
static void emit_reset(int is_attention);
static void emit_search_style(int type);
static int column_band(const pager_t *st, int *start, int *end);
static void emit_band_padding(const pager_t *st, int col, int width);
static int emit_style(const tam_span_t *sp, int is_sel);
static int link_space_is_interior(const char *text, size_t len, size_t offset);
static void emit_line(const tam_line_t *line, int width,
               const linkref_t *sel, size_t line_idx,
               const pager_t *st);
static size_t status_byte_limit(const char *s, int max_cols);
static const char *hyperlink_target(const tam_span_t *sp);
static void emit_hyperlink(const tam_span_t *sp, int open);
static void pager_address(const pager_t *st, char *out, size_t cap);

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

int terminal_height(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) {
        g_last_height = ws.ws_row;
        return g_last_height;
    }
    return g_last_height;
}

void out(const char *s)
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

int content_rows(const pager_t *st)
{
    if (st->height <= 2) return st->height <= 1 ? 1 : st->height - 1;
    return st->height - 2; /* Fixed location row plus footer. */
}

int content_width(const pager_t *st)
{
    int width = st->width > 1 ? st->width - 1 : 1;
    if (g_term != NULL && g_term->gutter && width > 1) width--;
    return width;
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
    if (sp->style & TAM_STYLE_DIM) {
        /* Dim text on the attention line's background would be hard to read. */
        if (g_term != NULL && !g_attention && (g_term->ncv < 0 || !(g_term->ncv & 16)))
            (void)tam_terminal_cap(g_term, g_capture, g_term->dim, 0, 0);
        styled = 1;
    }
    if (sp->style & TAM_STYLE_ITALIC) {
        if (g_term != NULL && g_term->italic != NULL) {
            (void)tam_terminal_cap(g_term, g_capture, g_term->italic, 0, 0);
            styled = 1;
        }
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

/* An external link the terminal can open itself, as an OSC 8 hyperlink;
 * NULL for every other span. */
static const char *hyperlink_target(const tam_span_t *sp)
{
    if (g_term == NULL || !g_term->hyperlinks || sp->href == NULL ||
        tam_addr_kind(sp->href) != TAM_ADDR_EXTERNAL ||
        !tam_external_url_ok(sp->href)) return NULL;
    return sp->href;
}

/* Each span opens and closes its own hyperlink, so none runs past a row; the
 * link id joins the rows of a wrapped link into one. */
static void emit_hyperlink(const tam_span_t *sp, int open)
{
    char seq[64];
    if (!open) {
        outn("\033]8;;\033\\", 7);
        return;
    }
    if (sp->link_id > 0) snprintf(seq, sizeof(seq), "\033]8;id=tam%d;", sp->link_id);
    else snprintf(seq, sizeof(seq), "\033]8;;");
    outn(seq, strlen(seq));
    outn(sp->href, strlen(sp->href));
    outn("\033\\", 2);
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
        int hyperlink = hyperlink_target(sp) != NULL;
        int hyperlink_open = 0;
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
            if (hyperlink && !hyperlink_open && !link_space) {
                emit_hyperlink(sp, 1);
                hyperlink_open = 1;
            }
            if (wc == 0xfffd) outn("?", 1);
            else outn(t + char_start, bytes);
            col += cells;
        }
        if (hyperlink_open) emit_hyperlink(sp, 0);
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

static void pager_address(const pager_t *st, char *out, size_t cap)
{
    char with_lens[600];
    if (st->cur_href != NULL && lens_href(st->cur_href) &&
        tam_href_put_level(st->cur_href, st->lens, with_lens,
                           sizeof(with_lens))) {
        snprintf(out, cap, "%s%.240s",
                 tam_addr_kind(with_lens) == TAM_ADDR_GUIDE &&
                 strncmp(with_lens, "page:", 5) != 0 ? "page:" : "", with_lens);
        return;
    }
    tam_addr_kind_t kind = tam_addr_kind(st->cur_href);
    if (kind == TAM_ADDR_NONE || kind == TAM_ADDR_STDIN) {
        snprintf(out, cap, "(standard input has no address)");
        return;
    }
    snprintf(out, cap, "%s%s", kind == TAM_ADDR_GUIDE &&
             strncmp(st->cur_href, "page:", 5) != 0 ? "page:" : "", st->cur_href);
}

int draw(pager_t *st)
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
                "",				
                "Hopes this helps! (Press Esc to close)",
                "",
                "Key                     Command",
                "----------------------  ---------------------------",
                "Esq, q                  Quit",
                "`                       Go to guide index page",
                "Down Arrow, d, k        Move attention line down",
                "Up Arrow, s, l          Move attention line up",
                "Right Arrow, Return, g  Follow selected link",
                "Left Arrow              Go back to previous page",
                "Home, x, .              Go to first line",
                "End, c, Comma           Go to last line",
                "Tab, ]                  Select next link",
                "Shift-Tab, [            Select previous link",
                "f, ;                    Move attention column right",
                "a, j                    Move attention column left",
                "/                       Search on this page",
                "\\                       Search the whole guide",
                "n                       Next search hit",
                "N                       Previous search hit",
                "=                       Show page address",
                "",
                "Web links: press Enter twice to open them in your browser",
                "Answers: Right Arrow or + shows, Left Arrow or - hides, r next, * all",
                "Info Up: u   Next/Previous node: n/p",
                "Level, kept across pages: 0 All, 1 Beginner, 2 Intermediate, 3 Expert",
                "",
                "Close this help with h, ? or Esc."
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
    size_t lens_digit = (size_t)-1; /* byte of the lens digit to underline */
    int max_cols = paint_width;
    if (st->show_address) {
        char addr[256];
        pager_address(st, addr, sizeof(addr));
        snprintf(status, sizeof(status), "Path = %s", addr);
    } else {
        const char *title = st->doc->title ? st->doc->title : "tam";
        /* The title is line 1: the level lines above it are lettered from
         * b and the breadcrumb row is a, and the range and total count
         * only numbered lines. */
        int lead = (int)st->doc->nlead;
        int total = (int)st->doc->nlines - lead;
        if (st->doc->nlines == 1 && st->doc->lines[0].nspans == 0) {
            total = 0;
        }
        int first = st->top < lead ? 1 : st->top - lead + 1;
        int last = st->top + usable - lead;
        if (last > total) {
            last = total;
        }
        if (total <= 0 || last < first) {
            first = last = 0;
        }
        char line_label[16];
        if (breadcrumb_focused(st))
            snprintf(line_label, sizeof(line_label), "La");
        else if (st->cursor_line < lead)
            snprintf(line_label, sizeof(line_label), "L%c",
                     st->cursor_line < 25 ? 'b' + st->cursor_line : 'z');
        else
            snprintf(line_label, sizeof(line_label), "L%d",
                     st->cursor_line - lead + 1);
        char essential[96];
        snprintf(essential, sizeof(essential), "%s %d-%d/%d",
                 line_label, first, last, total < 0 ? 0 : total);
        strncat(essential, max_cols >= 60 ? " | " : " ",
                sizeof(essential) - strlen(essential) - 1);
        /* The lens as "All text 0123", its digit underlined, or in brackets
         * where the terminal cannot underline. */
        int lens_number = tam_level_number(st->lens);
        int underline = g_term != NULL && g_term->underline != NULL &&
                        (g_term->ncv < 0 || !(g_term->ncv & 2));
        if (max_cols >= 100) {
            strncat(essential, tam_level_label(st->lens),
                    sizeof(essential) - strlen(essential) - 1);
            strncat(essential, " text ", sizeof(essential) - strlen(essential) - 1);
        } else if (max_cols >= 60) {
            strncat(essential, tam_level_label(st->lens),
                    sizeof(essential) - strlen(essential) - 1);
            strncat(essential, " ", sizeof(essential) - strlen(essential) - 1);
        }
        size_t digit_at = 0;
        for (int i = 0; i <= 3; i++) {
            char digit[4];
            snprintf(digit, sizeof(digit), i == lens_number && !underline ? "[%d]" : "%d", i);
            if (i == lens_number)
                digit_at = strlen(essential) + (underline ? 0 : 1);
            strncat(essential, digit, sizeof(essential) - strlen(essential) - 1);
        }
        if (st->doc->levels != 0 && st->lens != 0 && !(st->doc->levels & st->lens))
            strncat(essential, " (not on page)", sizeof(essential) - strlen(essential) - 1);

        /* Title | line and range | lens | keys  -> link  ...  version, the
         * version at the right edge. Narrow rows drop the title first, then
         * the link, then the keys, though a message stays ahead of the keys;
         * the line, range and lens always show. */
        static const char *const keys[] = {
            "h Help  Esc Quit  [] Select links", "h Help  Esc Quit"
        };
        char detail[192] = {0};
        if (st->msg[0] != '\0')
            snprintf(detail, sizeof(detail), "[%s]", st->msg);
        else if (sel != NULL && sel->href != NULL)
            snprintf(detail, sizeof(detail), "-> %s", sel->href);
        int ess_w = (int)tam_display_width(essential, strlen(essential));
        int ver_w = (int)tam_display_width(st->version, strlen(st->version));
        int room = max_cols - ess_w - (ver_w > 0 ? ver_w + 2 : 0);
        int detail_w = (int)tam_display_width(detail, strlen(detail));
        /* A message answers the reader's last key, so it outranks the key
         * hints; a selected link's target gives way to them. */
        int message = st->msg[0] != '\0';
        /* A confirmation must remain visible even when the usual status
         * fields leave too little room for it. */
        if (st->pending_external != NULL && detail_w + 2 > room) {
            snprintf(status, sizeof(status), "%s", detail);
            lens_digit = (size_t)-1;
            goto status_ready;
        }
        int show_detail = message && detail_w + 2 <= room;
        if (show_detail)
            room -= detail_w + 2;
        const char *key_text = NULL;
        for (size_t k = 0; k < sizeof(keys) / sizeof(keys[0]) && key_text == NULL; k++)
            if ((int)tam_display_width(keys[k], strlen(keys[k])) + 3 <= room)
                key_text = keys[k];
        if (key_text != NULL)
            room -= (int)tam_display_width(key_text, strlen(key_text)) + 3;
        if (!message && detail[0] != '\0' && detail_w + 2 <= room) {
            show_detail = 1;
            room -= detail_w + 2;
        }
        int title_w = (int)tam_display_width(title, strlen(title));
        if (title_w + 3 <= room && max_cols >= 60) {
            strncat(status, title, sizeof(status) - strlen(status) - 1);
            strncat(status, " | ", sizeof(status) - strlen(status) - 1);
        }
        lens_digit = strlen(status) + digit_at;
        strncat(status, essential, sizeof(status) - strlen(status) - 1);
        if (key_text != NULL) {
            strncat(status, " | ", sizeof(status) - strlen(status) - 1);
            strncat(status, key_text, sizeof(status) - strlen(status) - 1);
        }
        if (show_detail) {
            strncat(status, "  ", sizeof(status) - strlen(status) - 1);
            strncat(status, detail, sizeof(status) - strlen(status) - 1);
        }
        if (ver_w > 0) {
            int used = (int)tam_display_width(status, strlen(status));
            int pad = max_cols - used - ver_w;
            if (pad < 2) pad = 2;
            for (int i = 0; i < pad && strlen(status) + 1 < sizeof(status); i++)
                strncat(status, " ", sizeof(status) - strlen(status) - 1);
            strncat(status, st->version, sizeof(status) - strlen(status) - 1);
        }
    }

status_ready:
    if (st->height >= 2) {
        g_capture = &frame[st->height - 1];
        size_t slen = status_byte_limit(status, paint_width);
        if (lens_digit < slen && g_term != NULL && g_term->underline != NULL &&
            (g_term->ncv < 0 || !(g_term->ncv & 2))) {
            outn(status, lens_digit);
            (void)tam_terminal_cap(g_term, g_capture, g_term->underline, 0, 0);
            outn(status + lens_digit, 1);
            emit_reset(0);
            outn(status + lens_digit + 1, slen - lens_digit - 1);
        } else {
            outn(status, slen);
        }
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
