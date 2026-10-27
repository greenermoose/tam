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

volatile sig_atomic_t g_stop = 0;
volatile sig_atomic_t g_resized = 0;
tam_terminal_t *g_term;
tam_buf_t *g_capture;
tam_pager_screen_t g_screen;
int g_attention;
unsigned char g_pending[128];
size_t g_pending_len;
int g_last_width = 80;
int g_last_height = 24;

static void on_stop(int sig);
static void on_resize(int sig);

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

/* Hand the terminal to another program: leave the alternate screen and
 * restore the reader's terminal modes. */
void pager_suspend(pager_t *st)
{
    if (g_term == NULL) return;
    tam_buf_t b = {0};
    if (tam_terminal_leave(g_term, &b) == 0)
        (void)tam_terminal_write(g_term, &b);
    tam_buf_free(&b);
    if (st->tty_orig != NULL)
        (void)tcsetattr(st->input_fd, TCSANOW, st->tty_orig);
    g_term->raw_mode = 0;
}

/* Take the terminal back; the next draw repaints every row. */
void pager_resume(pager_t *st)
{
    if (g_term == NULL) return;
    if (st->tty_raw != NULL &&
        tcsetattr(st->input_fd, TCSANOW, st->tty_raw) == 0)
        g_term->raw_mode = 1;
    tam_buf_t b = {0};
    if (tam_terminal_enter(g_term, &b) == 0)
        (void)tam_terminal_write(g_term, &b);
    tam_buf_free(&b);
    tam_pager_screen_invalidate(&g_screen);
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
    {
        char *root = tam_find_knowledge_root();
        char *corpus = tam_corpus_version(root);
        if (corpus != NULL)
            snprintf(st.version, sizeof(st.version), "v%s (%s)", TAM_COMMAND_VERSION, corpus);
        else
            snprintf(st.version, sizeof(st.version), "v%s", TAM_COMMAND_VERSION);
        free(corpus);
        free(root);
    }
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
    st.cursor_line = (int)st.doc->nlead; /* Open on the title. */

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
    st.tty_orig = &orig;
    st.tty_raw = &raw;

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
    free(st.pending_external);
    pager_clear_search(&st);
    tam_document_free(st.doc);
    tam_pager_screen_free(&g_screen);
    g_term = NULL;

    if (have_int) (void)sigaction(SIGINT, &old_int, NULL);
    if (have_term) (void)sigaction(SIGTERM, &old_term, NULL);
    if (have_winch) (void)sigaction(SIGWINCH, &old_winch, NULL);
    return 0;
}
