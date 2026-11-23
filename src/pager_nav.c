#include <tam.h>

#include "address.h"
#include "depth.h"
#include "display.h"
#include "external.h"
#include "index.h"
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
#include <strings.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "pager_int.h"

static int nearest_logical_line(const tam_document_t *doc, size_t block,
                         size_t offset, int fallback, int prefer_blank);
static int history_push(pager_t *st, const char *href, int top,
                 int cursor_line);
static int forward_push(pager_t *st, const char *href, int top,
                 int cursor_line);
static void forward_clear(pager_t *st);
static void follow_external(pager_t *st, const char *href);
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

static int sandbox_line(const pager_t *st, int id, int role)
{
    for (size_t i = 0; i < st->doc->nlines; i++) {
        if (st->doc->lines[i].sandbox_id == id &&
            st->doc->lines[i].sandbox_role == role)
            return (int)i;
    }
    return -1;
}

static int sync_sandbox(pager_t *st, int id, int expanded)
{
    int width = st->layout_width > 0 ? st->layout_width : content_width(st);
    return tam_document_sync_sandbox(st->doc, width, id,
                                     &st->sandboxes[id - 1], expanded);
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
    size_t old_link_block = 0, old_link_offset = 0;
    int have_old_link_pos = 0;
    if (old_link >= 0 && (size_t)old_link < st->nlinks) {
        size_t ll = st->links[old_link].line_start;
        if (ll < st->doc->nlines) {
            old_link_block = st->doc->lines[ll].logical_block;
            old_link_offset = st->doc->lines[ll].logical_offset;
            have_old_link_pos = 1;
        }
    }
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
    pager_ensure_sandboxes(st);
    int input_line = -1;
    if (st->active_sandbox && sync_sandbox(st, st->active_sandbox, 1) == 0)
        input_line = sandbox_line(st, st->active_sandbox, 3);
    st->cursor_line = input_line >= 0 ? input_line :
        nearest_logical_line(st->doc, cursor_block, cursor_offset,
                             st->cursor_line, cursor_blank);
    st->top = nearest_logical_line(st->doc, top_block, top_offset, st->top, -1);
    st->top = st->cursor_line - cursor_row;
    if (st->top < 0) st->top = 0;
    pager_build_links(st);
    if (old_link_id > 0 || old_href != NULL) {
        int found = -1;
        if (old_link_id > 0) {
            for (size_t i = 0; i < st->nlinks; i++) {
                if (st->links[i].link_id == old_link_id) {
                    found = (int)i;
                    break;
                }
            }
        }
        if (found < 0 && old_href != NULL) {
            size_t best_distance = (size_t)-1;
            for (size_t i = 0; i < st->nlinks; i++) {
                if (st->links[i].href == NULL ||
                    strcmp(old_href, st->links[i].href) != 0)
                    continue;
                if (!have_old_link_pos) {
                    found = (int)i;
                    break;
                }
                size_t ll = st->links[i].line_start;
                size_t block = ll < st->doc->nlines
                    ? st->doc->lines[ll].logical_block : 0;
                size_t offset = ll < st->doc->nlines
                    ? st->doc->lines[ll].logical_offset : 0;
                size_t distance;
                if (block != old_link_block)
                    distance = block > old_link_block
                        ? (block - old_link_block) * 4096
                        : (old_link_block - block) * 4096;
                else
                    distance = offset > old_link_offset
                        ? offset - old_link_offset
                        : old_link_offset - offset;
                if (distance < best_distance) {
                    best_distance = distance;
                    found = (int)i;
                }
            }
        }
        if (found >= 0) st->selected = found;
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
    if (input_line >= 0) set_cursor_line(st, input_line, 1);
}

static int same_opt_str(const char *a, const char *b)
{
    if (a == NULL && b == NULL) return 1;
    if (a == NULL || b == NULL) return 0;
    return strcmp(a, b) == 0;
}

static int nav_state_build(pager_t *st, const char *href, int top,
                           int cursor_line, nav_state_t *out)
{
    const linkref_t *sel = selected_link(st);
    memset(out, 0, sizeof(*out));
    out->href = strdup(href);
    out->selected_href = sel && sel->href ? strdup(sel->href) : NULL;
    out->active_path = st->active_path ? strdup(st->active_path) : NULL;
    if (out->href == NULL ||
        (sel && sel->href && out->selected_href == NULL) ||
        (st->active_path != NULL && out->active_path == NULL)) {
        free(out->href);
        free(out->selected_href);
        free(out->active_path);
        memset(out, 0, sizeof(*out));
        return -1;
    }
    out->selected_link_id = sel ? sel->link_id : 0;
    out->selected_header = sel ? sel->header : 0;
    out->top = top;
    out->cursor_line = cursor_line;
    if (st->doc != NULL && st->doc->nlines > 0) {
        size_t ti = top >= 0 && (size_t)top < st->doc->nlines ? (size_t)top : 0;
        size_t ci = cursor_line >= 0 && (size_t)cursor_line < st->doc->nlines
            ? (size_t)cursor_line : 0;
        out->top_block = st->doc->lines[ti].logical_block;
        out->top_offset = st->doc->lines[ti].logical_offset;
        out->cursor_block = st->doc->lines[ci].logical_block;
        out->cursor_offset = st->doc->lines[ci].logical_offset;
        out->width = content_width(st);
    }
    return 0;
}

void nav_state_free(nav_state_t *state)
{
    if (state == NULL) return;
    free(state->href);
    free(state->selected_href);
    free(state->active_path);
    memset(state, 0, sizeof(*state));
}

void nav_stack_clear(nav_state_t **stack, size_t *n)
{
    if (stack == NULL || n == NULL) return;
    while (*n > 0) {
        nav_state_free(&(*stack)[--(*n)]);
    }
    free(*stack);
    *stack = NULL;
    *n = 0;
}

static int nav_stack_push(nav_state_t **stack, size_t *n, pager_t *st,
                          const char *href, int top, int cursor_line)
{
    nav_state_t snap;
    if (nav_state_build(st, href, top, cursor_line, &snap) != 0)
        return -1;
    nav_state_t *next = realloc(*stack, (*n + 1) * sizeof(**stack));
    if (next == NULL) {
        nav_state_free(&snap);
        return -1;
    }
    *stack = next;
    (*stack)[(*n)++] = snap;
    return 0;
}

static int history_push(pager_t *st, const char *href, int top,
                        int cursor_line)
{
    return nav_stack_push(&st->history, &st->nhistory, st, href, top, cursor_line);
}

static int forward_push(pager_t *st, const char *href, int top,
                        int cursor_line)
{
    return nav_stack_push(&st->forward, &st->nforward, st, href, top, cursor_line);
}

static void forward_clear(pager_t *st)
{
    nav_stack_clear(&st->forward, &st->nforward);
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
                if (link->header || (link->line_start <= (size_t)st->cursor_line &&
                                     link->line_end >= (size_t)st->cursor_line)) {
                    st->selected = (int)i;
                }
                break;
            }
        }
    }
    if (st->selected < 0) {
        st->selected = find_first_link_on_line(st, (size_t)st->cursor_line);
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

/* Load `href` (stored without level= or path=) at the reader's lens and
 * active learning path. A view that cannot be shown at the lens is loaded
 * at every level instead. */
static tam_document_t *load_at_context(const pager_t *st, const char *href,
                                       const char *path_id)
{
    int with_path = path_id != NULL && path_id[0] != '\0' &&
                    tam_addr_kind(href) == TAM_ADDR_GUIDE &&
                    !tam_addr_param(href, "path", (char[8]){0}, 8);
    if (st->lens != 0 && tam_addr_takes_lens(href)) {
        size_t cap = strlen(href) + (with_path ? strlen(path_id) : 0) + 64;
        char *buf = malloc(cap);
        char *target = malloc(cap);
        if (!buf || !target) {
            free(buf);
            free(target);
            return NULL;
        }
        int ok = tam_href_put_level(href, st->lens, buf, cap);
        if (ok && with_path) {
            ok = tam_href_put_path(buf, path_id, target, cap);
        } else if (ok) {
            snprintf(target, cap, "%s", buf);
        }
        tam_document_t *nd = ok ? st->load(st->ctx, target, content_width(st)) : NULL;
        free(buf);
        free(target);
        if (nd != NULL) return nd;
    }
    if (with_path) {
        size_t cap = strlen(href) + strlen(path_id) + 32;
        char *target = malloc(cap);
        if (!target) return NULL;
        int ok = tam_href_put_path(href, path_id, target, cap);
        tam_document_t *nd = ok ? st->load(st->ctx, target, content_width(st)) : NULL;
        free(target);
        return nd;
    }
    return st->load(st->ctx, href, content_width(st));
}

tam_document_t *load_at_lens(pager_t *st, const char *href)
{
    return load_at_context(st, href, st->active_path);
}

/* tam never fetches a web page. An external link opens in the reader's own
 * browser on a second press of the same link; the first press names the
 * browser. A graphical browser starts detached; a text browser takes this
 * terminal until it exits. */
static void follow_external(pager_t *st, const char *href)
{
    if (!tam_external_url_ok(href)) {
        size_t scheme = strcspn(href, ":/?#");
        if (href[scheme] != ':' || (scheme == 4 && strncasecmp(href, "http", 4) == 0) ||
            (scheme == 5 && strncasecmp(href, "https", 5) == 0))
            snprintf(st->msg, sizeof(st->msg), "cannot open malformed link");
        else
            snprintf(st->msg, sizeof(st->msg), "cannot open %.*s: links",
                     scheme < 24 ? (int)scheme : 24, href);
        return;
    }
    int graphical = tam_external_graphical();
    if (st->pending_external == NULL || strcmp(st->pending_external, href) != 0) {
        char name[64];
        free(st->pending_external);
        st->pending_external = NULL;
        if (!tam_external_opener(graphical, name, sizeof(name))) {
            snprintf(st->msg, sizeof(st->msg), "%s", tam_external_missing(graphical));
            return;
        }
        st->pending_external = strdup(href);
        st->external_armed = st->pending_external != NULL;
        snprintf(st->msg, sizeof(st->msg), "Enter again opens it in %s", name);
        return;
    }
    free(st->pending_external);
    st->pending_external = NULL;
    if (graphical) {
        (void)tam_external_open_detached(href, st->msg, sizeof(st->msg));
        return;
    }
    pager_suspend(st);
    (void)tam_external_run_foreground(href, st->input_fd, st->msg, sizeof(st->msg));
    pager_resume(st);
}

static void wait_for_return_key(pager_t *st)
{
    static const char prompt[] = "\nPress any key to return to the guide...";
    ssize_t wr = write(STDOUT_FILENO, prompt, sizeof(prompt) - 1);
    (void)wr;
    if (st->tty_raw != NULL)
        (void)tcsetattr(st->input_fd, TCSANOW, st->tty_raw);
    unsigned char ch;
    for (;;) {
        ssize_t n = read(st->input_fd, &ch, 1);
        if (n > 0) break;
        if (n < 0 && errno == EINTR && !g_stop) continue;
        return;
    }
    /* Drain any trailing bytes of a multi-byte key (such as an arrow key). */
    struct pollfd pfd = { .fd = st->input_fd, .events = POLLIN };
    for (int i = 0; i < 16 && poll(&pfd, 1, 20) > 0 && (pfd.revents & POLLIN); i++) {
        if (read(st->input_fd, &ch, 1) <= 0) break;
    }
}

void follow(pager_t *st, const char *href)
{
    if (href == NULL || href[0] == '\0') {
        return;
    }
    unsigned action_id = 0;
    if (tam_addr_fold_id(href, &action_id)) {
        toggle_fold(st, action_id);
        return;
    }
    if (tam_addr_sandbox_id(href, &action_id)) {
        pager_activate_sandbox(st, (int)action_id);
        return;
    }
    if (strcmp(href, "tam:help") == 0) {
        follow(st, "help/index.html");
        return;
    }
    if (strcmp(href, "tam:start") == 0) {
        char *const argv[] = { "tam-start", NULL };
        pager_suspend(st);
        if (tam_run_foreground_command(argv, st->input_fd, st->msg, sizeof(st->msg)) == 0)
            wait_for_return_key(st);
        pager_resume(st);
        return;
    }
    if (strcmp(href, "tam:prompt:page-search") == 0) {
        pager_search_prompt(st);
        return;
    }
    if (tam_addr_is_prompt(href)) {
        char target[512];
        if (pager_prompt_action(st, href, target, sizeof(target)) && target[0] != '\0')
            follow(st, target);
        return;
    }
    if (href[0] == '#') {
        const char *target = href + 1;
        if (target[0] == '\0') {
            if (history_push(st, st->cur_href, st->top, st->cursor_line) != 0) {
                snprintf(st->msg, sizeof(st->msg), "out of memory");
                return;
            }
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
        if (history_push(st, st->cur_href, st->top, st->cursor_line) != 0) {
            snprintf(st->msg, sizeof(st->msg), "out of memory");
            return;
        }
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
        follow_external(st, href);
        return;
    }
    /* A link that names a level sets the lens, and a guide link carrying
     * ?path= sets/switches/clears the active learning path; history keeps
     * cur_href without them while tracking active_path per history frame. */
    char bare[600];
    snprintf(bare, sizeof(bare), "%s", href);
    unsigned named = 0;
    if (tam_addr_takes_lens(bare) && tam_href_take_level(bare, &named) &&
        named != st->lens) {
        st->lens = named;
        tam_lens_set(named);
    }
    char next_path_buf[256] = {0};
    int path_cleared = 0;
    int had_path_param = tam_addr_kind(bare) == TAM_ADDR_GUIDE &&
                         tam_href_take_path(bare, next_path_buf,
                                            sizeof(next_path_buf), &path_cleared);
    char *next_active = NULL;
    if (had_path_param) {
        if (!path_cleared && next_path_buf[0] != '\0')
            next_active = strdup(next_path_buf);
    } else if (st->active_path != NULL) {
        next_active = strdup(st->active_path);
    }

    href = bare;
    tam_document_t *nd = load_at_context(st, href, next_active);
    if (nd == NULL) {
        free(next_active);
        if (strcmp(href, "index.html") == 0)
            snprintf(st->msg, sizeof(st->msg), "Guide unavailable; Left Arrow returns to the previous page");
        else
            snprintf(st->msg, sizeof(st->msg), "cannot open %.140s", href);
        return;
    }
    if (had_path_param && st->cur_href != NULL && strcmp(href, st->cur_href) == 0) {
        tam_document_free(nd);
        if (history_push(st, st->cur_href, st->top, st->cursor_line) != 0) {
            free(next_active);
            snprintf(st->msg, sizeof(st->msg), "out of memory");
            return;
        }
        forward_clear(st);
        free(st->active_path);
        st->active_path = next_active;
        if (!reload_in_place(st)) {
            st->msg[0] = '\0';
        } else if (st->active_path != NULL) {
            snprintf(st->msg, sizeof(st->msg), "Following path: %.120s", st->active_path);
        } else {
            snprintf(st->msg, sizeof(st->msg), "Left learning path");
        }
        return;
    }
    free(st->fold_open); st->fold_open = NULL; st->nfold_open = 0;
    tam_document_ensure_location(nd, href);
    if (history_push(st, st->cur_href, st->top, st->cursor_line) != 0) {
        tam_document_free(nd);
        free(next_active);
        snprintf(st->msg, sizeof(st->msg), "out of memory");
        return;
    }
    forward_clear(st);
    pager_clear_search(st);
    free(st->cur_href);
    st->cur_href = strdup(href);
    free(st->active_path);
    st->active_path = next_active;
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

static int is_lens_line(const tam_line_t *line)
{
    if (line == NULL) {
        return 0;
    }
    for (size_t i = 0; i < line->nspans; i++) {
        if (line->spans[i].style & TAM_STYLE_LVL_MASK) {
            return 1;
        }
    }
    return 0;
}

int lens_line_here(const pager_t *st)
{
    if (st == NULL || st->doc == NULL || st->doc->lines == NULL ||
        breadcrumb_focused(st)) {
        return 0;
    }
    if (st->cursor_line < 0 || (size_t)st->cursor_line >= st->doc->nlines) {
        return 0;
    }
    return is_lens_line(&st->doc->lines[st->cursor_line]);
}

static size_t lens_line_ordinal(const pager_t *st)
{
    size_t ord = 0;
    for (int i = 0; i < st->cursor_line; i++) {
        if (is_lens_line(&st->doc->lines[i])) {
            ord++;
        }
    }
    return ord;
}

static void anchor_to_lens_line(pager_t *st, size_t ord, int cursor_row)
{
    if (st->doc == NULL || st->doc->lines == NULL) {
        return;
    }
    size_t seen = 0;
    int target = -1;
    for (size_t i = 0; i < st->doc->nlines; i++) {
        if (is_lens_line(&st->doc->lines[i])) {
            if (seen == ord) {
                target = (int)i;
                break;
            }
            seen++;
        }
    }
    if (target < 0) {
        return;
    }
    int usable = content_rows(st);
    if (cursor_row < 0) cursor_row = 0;
    if (cursor_row >= usable) cursor_row = usable - 1;
    int max_top = (int)st->doc->nlines - usable;
    if (max_top < 0) max_top = 0;
    int top = target - cursor_row;
    if (top < 0) top = 0;
    if (top > max_top) top = max_top;
    st->top = top;
    set_cursor_line(st, target, 1);
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
    int on_lens = lens_line_here(st);
    size_t ord = on_lens ? lens_line_ordinal(st) : 0;
    int cursor_row = st->cursor_line - st->top;

    st->lens = level;
    tam_lens_set(level);
    if (!tam_addr_takes_lens(st->cur_href) || !reload_in_place(st)) {
        snprintf(st->msg, sizeof(st->msg),
                 "Level %d %s set; this page has one level", number, label);
        return;
    }
    if (on_lens) {
        anchor_to_lens_line(st, ord, cursor_row);
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

void cycle_lens(pager_t *st, int dir)
{
    if (!lens_line_here(st)) {
        return;
    }
    int cur = tam_level_number(st->lens);
    int next = (cur + (dir < 0 ? 3 : 1)) % 4;
    set_lens(st, (unsigned char)('0' + next));
}

int is_help_page(const char *href)
{
    if (href == NULL) return 0;
    href = tam_addr_strip_page_prefix(href);
    if (strcmp(href, "help/index.html") == 0 || strcmp(href, "help") == 0 ||
        strcmp(href, "help/") == 0)
        return 1;
    if (strncmp(href, "help/index.html?", 16) == 0 ||
        strncmp(href, "help/index.html#", 16) == 0)
        return 1;
    return 0;
}

void pager_return_from_help(pager_t *st)
{
    while (st->nhistory > 0 && is_help_page(st->history[st->nhistory - 1].href)) {
        nav_state_t discarded = st->history[--st->nhistory];
        nav_state_free(&discarded);
    }
    go_back(st);
}

static void nav_traverse(pager_t *st, nav_state_t **src, size_t *nsrc,
                         int (*push)(pager_t *st, const char *href, int top,
                                     int cursor_line))
{
    nav_state_t prev = (*src)[--(*nsrc)];
    if (strcmp(prev.href, st->cur_href) == 0 &&
        same_opt_str(prev.active_path, st->active_path)) {
        if (push(st, st->cur_href, st->top, st->cursor_line) != 0) {
            (*nsrc)++;
            snprintf(st->msg, sizeof(st->msg), "out of memory");
            return;
        }
        pager_clear_search(st);
        free(prev.href);
        free(prev.active_path);
        restore_logical_nav_state(st, &prev);
        free(prev.selected_href);
        st->msg[0] = '\0';
        return;
    }
    tam_document_t *nd = load_at_context(st, prev.href, prev.active_path);
    if (nd == NULL) {
        /* The entry returns to the stack, which owns its strings again. */
        (*nsrc)++;
        snprintf(st->msg, sizeof(st->msg), "cannot open %s",
                 (*src)[(*nsrc) - 1].href);
        return;
    }
    if (push(st, st->cur_href, st->top, st->cursor_line) != 0) {
        tam_document_free(nd);
        (*nsrc)++;
        snprintf(st->msg, sizeof(st->msg), "out of memory");
        return;
    }
    free(st->fold_open); st->fold_open = NULL; st->nfold_open = 0;
    tam_document_ensure_location(nd, prev.href);
    pager_clear_search(st);
    free(st->cur_href);
    st->cur_href = prev.href;
    free(st->active_path);
    st->active_path = prev.active_path;
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    restore_logical_nav_state(st, &prev);
    free(prev.selected_href);
    st->msg[0] = '\0';
}

void go_back(pager_t *st)
{
    if (st->nhistory == 0) {
        snprintf(st->msg, sizeof(st->msg), "no earlier page");
        return;
    }
    nav_traverse(st, &st->history, &st->nhistory, forward_push);
}

void go_forward(pager_t *st)
{
    if (st->nforward == 0) {
        return;
    }
    nav_traverse(st, &st->forward, &st->nforward, history_push);
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

void pager_nav_next_name(const pager_t *st, const char *href, char *out, size_t cap)
{
    if (out == NULL || cap == 0) return;
    out[0] = '\0';
    if (href == NULL || href[0] == '\0') return;

    const char *p = href;
    if (strncmp(p, "page:", 5) == 0) p += 5;

    char clean_path[256];
    size_t i = 0;
    while (p[i] != '\0' && p[i] != '?' && p[i] != '#' && i < sizeof(clean_path) - 1) {
        clean_path[i] = p[i];
        i++;
    }
    clean_path[i] = '\0';

    const char *root = NULL;
    char *owned_root = NULL;
    if (st != NULL && st->ctx != NULL) {
        const tam_loader_ctx_t *lctx = st->ctx;
        root = lctx->root;
    }
    if (root == NULL) {
        owned_root = tam_find_knowledge_root();
        root = owned_root;
    }
    if (root != NULL) {
        tam_index_t *idx = tam_index_load(root);
        if (idx != NULL) {
            const tam_index_entry_t *entry = tam_index_lookup_path(idx, clean_path);
            if (entry != NULL && entry->title != NULL && entry->title[0] != '\0') {
                snprintf(out, cap, "%s", entry->title);
            }
            tam_index_free(idx);
        }
        if (owned_root != NULL) free(owned_root);
    }
    if (out[0] == '\0') {
        snprintf(out, cap, "%s", p);
    }
}

void pager_handle_right_arrow_unselected(pager_t *st)
{
    if (st == NULL) return;

    if (st->right_arrow_state == 0) {
        snprintf(st->msg, sizeof(st->msg), "No forward history. Move to next link? (-> again)");
        st->right_arrow_state = 1;
        st->right_arrow_armed = 1;
        return;
    }

    if (st->right_arrow_state == 1) {
        int next_link = -1;
        for (size_t i = 0; i < st->nlinks; i++) {
            const linkref_t *l = &st->links[i];
            if (l->header) continue;
            if (l->line_start > (size_t)st->cursor_line) {
                next_link = (int)i;
                break;
            }
        }
        if (next_link >= 0) {
            st->cursor_line = (int)st->links[next_link].line_start;
            st->selected = next_link;
            ensure_link_visible(st, 1);
            st->msg[0] = '\0';
            st->right_arrow_state = 0;
            return;
        }

        if (st->doc != NULL && st->doc->nav_next != NULL && st->doc->nav_next[0] != '\0') {
            char name[100];
            pager_nav_next_name(st, st->doc->nav_next, name, sizeof(name));
            snprintf(st->msg, sizeof(st->msg), "No more links. Open %.*s? (-> again)",
                     (int)sizeof(name), name);
            st->right_arrow_state = 2;
            st->right_arrow_armed = 1;
            return;
        }

        snprintf(st->msg, sizeof(st->msg), "No more links and no next page");
        st->right_arrow_state = 0;
        return;
    }

    if (st->right_arrow_state == 2) {
        if (st->doc != NULL && st->doc->nav_next != NULL && st->doc->nav_next[0] != '\0') {
            char *target = strdup(st->doc->nav_next);
            st->right_arrow_state = 0;
            st->msg[0] = '\0';
            if (target != NULL) {
                follow(st, target);
                free(target);
            }
        } else {
            st->right_arrow_state = 0;
            st->msg[0] = '\0';
        }
        return;
    }
}

void pager_free_sandboxes(pager_t *st)
{
    if (st == NULL) return;
    for (size_t i = 0; i < st->nsandboxes; i++)
        tam_sandbox_state_free(&st->sandboxes[i]);
    free(st->sandboxes);
    st->sandboxes = NULL;
    st->nsandboxes = 0;
    st->sandbox_doc = NULL;
    st->active_sandbox = 0;
}

void pager_ensure_sandboxes(pager_t *st)
{
    if (st == NULL || st->doc == NULL) return;
    if (st->sandbox_doc == st->doc) return;
    pager_free_sandboxes(st);
    st->sandbox_doc = st->doc;
    if (st->doc->nsandboxes == 0) return;
    st->sandboxes = calloc(st->doc->nsandboxes, sizeof(*st->sandboxes));
    if (st->sandboxes == NULL) return;
    st->nsandboxes = st->doc->nsandboxes;
    for (size_t i = 0; i < st->nsandboxes; i++)
        tam_sandbox_state_init(&st->sandboxes[i], &st->doc->sandboxes[i]);
}

static void focus_sandbox_line(pager_t *st, int id, int role)
{
    pager_build_links(st);
    if (st->search_query[0] != '\0') {
        char query[sizeof(st->search_query)];
        snprintf(query, sizeof(query), "%s", st->search_query);
        pager_perform_search(st, query);
    }
    int line = sandbox_line(st, id, role);
    if (line >= 0) set_cursor_line(st, line, 1);
}

int pager_update_sandbox_input(pager_t *st)
{
    int id = st->active_sandbox;
    if (sync_sandbox(st, id, 1) < 0) return -1;
    focus_sandbox_line(st, id, 3);
    return 0;
}

void pager_activate_sandbox(pager_t *st, int sb_id)
{
    pager_ensure_sandboxes(st);
    if (sb_id < 1 || (size_t)sb_id > st->nsandboxes || st->active_sandbox) return;
    st->active_sandbox = sb_id;
    if (pager_update_sandbox_input(st) < 0) st->active_sandbox = 0;
}

void pager_exit_sandbox(pager_t *st)
{
    int id = st->active_sandbox;
    if (sync_sandbox(st, id, 0) < 0) return;
    st->active_sandbox = 0;
    focus_sandbox_line(st, id, 1);
    st->msg[0] = '\0';
}
