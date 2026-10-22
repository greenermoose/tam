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

static int find_last_link_on_line(const pager_t *st, size_t line_idx);
static int find_first_visible_link(const pager_t *st);
static void ensure_link_visible(pager_t *st, int dir);
static int header_link_range(const pager_t *st, int *first, int *last);
static int select_breadcrumb(pager_t *st);
static void leave_breadcrumb(pager_t *st);
static void select_horizontal_header_link(pager_t *st, int dir);

void pager_build_links(pager_t *st)
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

const linkref_t *selected_link(const pager_t *st)
{
    if (st->selected < 0 || (size_t)st->selected >= st->nlinks) {
        return NULL;
    }
    return &st->links[st->selected];
}

int link_is_visible(const pager_t *st, int link_idx)
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

int find_first_link_on_line(const pager_t *st, size_t line_idx)
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

/* The top of the page is its title, L1: the level lines above it control the
 * display and are not page content. They stay in view. */
void go_page_top(pager_t *st)
{
    st->top = 0;
    set_cursor_line(st, st->doc != NULL ? (int)st->doc->nlead : 0, -1);
}

void set_cursor_line(pager_t *st, int new_line, int dir)
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

/* The breadcrumb row has focus while one of its links is selected. */
int breadcrumb_focused(const pager_t *st)
{
    return st->selected >= 0 && (size_t)st->selected < st->nlinks &&
           st->links[st->selected].header;
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

void select_next_link(pager_t *st, int dir, int include_header)
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
void move_line_up(pager_t *st)
{
    if (breadcrumb_focused(st)) {
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

void move_line_down(pager_t *st)
{
    if (breadcrumb_focused(st))
        leave_breadcrumb(st);
    else
        set_cursor_line(st, st->cursor_line + 1, 1);
}

/* Left and right move the attention column from link to link along the
 * attention line, or along the breadcrumb row while it is focused, wrapping
 * at either end. A line without links has nothing to catch. */
void move_column(pager_t *st, int dir)
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
