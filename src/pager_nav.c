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

static int nearest_logical_line(const tam_document_t *doc, size_t block,
                         size_t offset, int fallback, int prefer_blank);
static void history_push(pager_t *st, const char *href, int top,
                  int cursor_line);
static void forward_push(pager_t *st, const char *href, int top,
                  int cursor_line);
static void forward_clear(pager_t *st);
static void restore_nav_state(pager_t *st, int top, int cursor_line);
static void restore_logical_nav_state(pager_t *st, const nav_state_t *nav);
static int match_anchor(const char *haystack, const char *needle);
static int find_anchor_text(const tam_document_t *doc, const char *target);
static int heading_at_or_above(const tam_document_t *doc, int line);
static int reload_in_place(pager_t *st);
static int fold_set(pager_t *st, unsigned id, int open);
static void fold_message(pager_t *st);
static void select_fold(pager_t *st, unsigned id);
static void toggle_fold(pager_t *st, unsigned id);

static int nearest_logical_line(const tam_document_t *doc, size_t block,
                                size_t offset, int fallback, int prefer_blank)
{
    if (doc == NULL || doc->nlines == 0) return 0;
    size_t best_distance = (size_t)-1;
    int best = -1;
    for (size_t i = 0; i < doc->nlines; i++) {
        if (doc->lines[i].logical_block != block) continue;
        size_t at = doc->lines[i].logical_offset;
        size_t distance = at > offset ? at - offset : offset - at;
        /* -1 leaves equal-distance blank/text ties in their existing order. */
        int is_preferred = prefer_blank >= 0 &&
            ((doc->lines[i].nspans == 0) == (prefer_blank != 0));
        int best_is_preferred = best >= 0 && prefer_blank >= 0 &&
            ((doc->lines[best].nspans == 0) == (prefer_blank != 0));
        if (distance < best_distance ||
            (distance == best_distance && is_preferred && !best_is_preferred)) {
            best_distance = distance;
            best = (int)i;
        }
    }
    if (best >= 0) return best;
    if (fallback < 0) return 0;
    if ((size_t)fallback >= doc->nlines) return (int)doc->nlines - 1;
    return fallback;
}

void pager_reflow(pager_t *st, int width)
{
    if (width < 1 || width == st->layout_width || st->doc == NULL) return;
    size_t cursor_block = 0, cursor_offset = 0, top_block = 0, top_offset = 0;
    int cursor_blank = 0;
    if (st->doc->nlines > 0) {
        size_t ci = (size_t)st->cursor_line < st->doc->nlines ? (size_t)st->cursor_line : 0;
        size_t ti = (size_t)st->top < st->doc->nlines ? (size_t)st->top : 0;
        cursor_block = st->doc->lines[ci].logical_block;
        cursor_offset = st->doc->lines[ci].logical_offset;
        cursor_blank = st->doc->lines[ci].nspans == 0;
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
                                           st->cursor_line, cursor_blank);
    st->top = nearest_logical_line(st->doc, top_block, top_offset, st->top, -1);
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
                                               nav->cursor_offset, st->cursor_line, -1);
        st->top = nearest_logical_line(st->doc, nav->top_block,
                                       nav->top_offset, st->top, -1);
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
int lens_href(const char *href)
{
    return tam_addr_takes_lens(href);
}

/* Load `href` (stored without level=) at the reader's lens. A view that
 * cannot be shown at the lens is loaded at every level instead. */
tam_document_t *load_at_lens(pager_t *st, const char *href)
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

void follow(pager_t *st, const char *href)
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
            go_page_top(st);
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
    if (tam_addr_kind(href) == TAM_ADDR_EXTERNAL) {
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
    st->cursor_line = (int)nd->nlead; /* The title, below any level lines. */
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

/* The answer whose [+]/[-] link is selected or on the attention line: its
 * fold id and whether it is shown. Returns 0 when there is none. */
static int fold_here(const pager_t *st, unsigned *id, int *shown)
{
    const linkref_t *sel = selected_link(st);
    const char *href = sel != NULL && strncmp(sel->href, "tam:fold:", 9) == 0
        ? sel->href : NULL;
    for (size_t i = 0; href == NULL && i < st->nlinks; i++) {
        if (strncmp(st->links[i].href, "tam:fold:", 9) == 0 &&
            st->links[i].line_start <= (size_t)st->cursor_line &&
            st->links[i].line_end >= (size_t)st->cursor_line)
            href = st->links[i].href;
    }
    if (href == NULL) return 0;
    *id = (unsigned)strtoul(href + 9, NULL, 10);
    *shown = 0;
    for (size_t i = 0; i < st->doc->nfolds; i++)
        if (st->doc->folds[i].id == *id) *shown = st->doc->folds[i].open != 0;
    return 1;
}

/* + shows and - hides the answer here; Enter and Right Arrow toggle it as a
 * link. */
void set_fold_here(pager_t *st, int open)
{
    unsigned id;
    int shown;
    if (!fold_here(st, &id, &shown)) {
        snprintf(st->msg, sizeof(st->msg), open
                 ? "Move to a [+] line to show an answer"
                 : "Move to a [-] line to hide an answer");
        return;
    }
    if (shown != (open != 0)) toggle_fold(st, id);
}

/* Right Arrow on an answer toggles it, unless another link is selected.
 * Returns 1 when it did. */
int toggle_fold_here(pager_t *st)
{
    const linkref_t *sel = selected_link(st);
    unsigned id;
    int shown;
    if ((sel != NULL && strncmp(sel->href, "tam:fold:", 9) != 0) ||
        !fold_here(st, &id, &shown))
        return 0;
    toggle_fold(st, id);
    return 1;
}

/* Left Arrow on a shown answer hides it instead of going back. Returns 1
 * when it did. */
int hide_fold_here(pager_t *st)
{
    unsigned id;
    int shown;
    if (!fold_here(st, &id, &shown) || !shown) return 0;
    toggle_fold(st, id);
    return 1;
}

void reveal_next(pager_t *st)
{
    for (size_t i = 0; i < st->nlinks; i++) {
        if (st->links[i].line_start < (size_t)st->cursor_line || strncmp(st->links[i].href, "tam:fold:", 9)) continue;
        unsigned id = (unsigned)strtoul(st->links[i].href + 9, NULL, 10);
        if (id < st->nfold_open && st->fold_open[id]) continue;
        toggle_fold(st, id); return;
    }
    snprintf(st->msg, sizeof(st->msg), "all answers shown");
}

void toggle_all_folds(pager_t *st)
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

void set_lens(pager_t *st, unsigned char key)
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
                 "Level %d %s set; this page has no %s text", number,
                 label, label);
    else if (st->doc->hidden > 0)
        snprintf(st->msg, sizeof(st->msg), "Level %d %s: %d section%s hidden",
                 number, label, st->doc->hidden,
                 st->doc->hidden == 1 ? "" : "s");
    else
        snprintf(st->msg, sizeof(st->msg), "Level %d %s set", number, label);
}

void go_back(pager_t *st)
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

void go_forward(pager_t *st)
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

void follow_selected(pager_t *st)
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
