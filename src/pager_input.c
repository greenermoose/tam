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

static void pending_prepend(const unsigned char *data, size_t len);
static int consume_sync_reply(pager_t *st, const unsigned char *bytes,
                       size_t len);
static void read_sync_reply(pager_t *st, int fd);
static void handle_key_inner(pager_t *st, unsigned char c);

static void pending_prepend(const unsigned char *data, size_t len)
{
    if (len == 0 || len > sizeof(g_pending) - g_pending_len) return;
    memmove(g_pending + len, g_pending, g_pending_len);
    memcpy(g_pending, data, len);
    g_pending_len += len;
}

/* Returns 1 on a byte, 0 on timeout (or an interrupted wait), and -1 on EOF
 * or a read error. Distinguishing EOF lets the pager exit when its input is
 * closed rather than spinning. */
int read_byte(int fd, int timeout_ms, unsigned char *c)
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
    case '+':
        set_fold_here(st, 1);
        break;
    case '-':
        set_fold_here(st, 0);
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
        go_page_top(st);
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
            go_page_top(st);
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
            if (toggle_fold_here(st)) {
                /* An answer on the attention line. */
            } else if (st->selected >= 0) {
                follow_selected(st);
            } else if (st->nforward > 0) {
                go_forward(st);
            }
        } else if (strcmp(seq, "D") == 0) {
            if (!hide_fold_here(st)) go_back(st);
        } else if (strcmp(seq, "H") == 0) {
            go_page_top(st);
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
void handle_key(pager_t *st, unsigned char c)
{
    int column_key = c == 'a' || c == 'j' || c == 'f' || c == ';';
    const tam_document_t *doc = st->doc;
    int line = st->cursor_line;
    int selected = st->selected;
    char msg[sizeof(st->msg)];
    memcpy(msg, st->msg, sizeof(msg));
    st->external_armed = 0;
    handle_key_inner(st, c);
    /* An external link waits for one press only: any other key forgets it,
     * and the prompt with it unless the key said something else. */
    if (st->pending_external != NULL && !st->external_armed) {
        free(st->pending_external);
        st->pending_external = NULL;
        if (memcmp(msg, st->msg, sizeof(msg)) == 0) st->msg[0] = '\0';
    }
    if (st->doc != doc)
        st->show_address = 0;
    if (c == 'g' || c == '\r' || c == '\n' ||
        (!column_key && (st->doc != doc || st->cursor_line != line ||
                         st->selected != selected)))
        st->column_active = 0;
}
