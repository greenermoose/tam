#include <tam.h>

#include "pager.h"
#include "render.h"

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

typedef struct {
    const tam_span_t *span;
    size_t            line;
} linkref_t;

typedef struct {
    tam_document_t *doc;
    char           *cur_href;
    linkref_t      *links;
    size_t          nlinks;
    char          **history;
    size_t          nhistory;
    int             top;
    int             selected;
    int             width;
    int             height;
    int             input_fd;
    char            msg[160];
    tam_page_loader_t load;
    void              *ctx;
} pager_t;

static void pager_build_links(pager_t *st)
{
    free(st->links);
    st->links = NULL;
    st->nlinks = 0;
    st->selected = -1;

    for (size_t i = 0; i < st->doc->nlines; i++) {
        for (size_t j = 0; j < st->doc->lines[i].nspans; j++) {
            const tam_span_t *sp = &st->doc->lines[i].spans[j];
            if (sp->href == NULL) {
                continue;
            }
            st->links = realloc(st->links, (st->nlinks + 1) * sizeof(*st->links));
            st->links[st->nlinks].span = sp;
            st->links[st->nlinks].line = i;
            st->nlinks++;
        }
    }
}

static const tam_span_t *selected_span(const pager_t *st)
{
    if (st->selected < 0 || (size_t)st->selected >= st->nlinks) {
        return NULL;
    }
    return st->links[st->selected].span;
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

static void emit_line(const tam_line_t *line, int width,
                      const tam_span_t *sel)
{
    int col = 0;
    for (size_t j = 0; j < line->nspans && col < width; j++) {
        const tam_span_t *sp = &line->spans[j];
        int styled = emit_style(sp, sp == sel);
        const char *t = sp->text;
        size_t tl = strlen(t);
        for (size_t k = 0; k < tl && col < width; k++) {
            outn(t + k, 1);
            col++;
        }
        if (styled) {
            out("\033[0m");
        }
    }
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
    const tam_span_t *sel = selected_span(st);
    for (int row = 0; row < usable; row++) {
        int li = st->top + row;
        if (li < (int)st->doc->nlines) {
            emit_line(&st->doc->lines[li], st->width, sel);
        }
        if (row < usable - 1) {
            out("\r\n");
        }
    }

    char status[512];
    const char *title = st->doc->title ? st->doc->title : "tam";
    int off = snprintf(status, sizeof(status),
                       " %s  |  %d/%zu  |  Tab link  Enter/\xe2\x86\x92 open  "
                       "u/\xe2\x86\x90 back  q/Esc quit",
                       title, st->top + 1, st->doc->nlines);
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
    size_t slen = strlen(status);
    if ((int)slen > st->width) {
        slen = (size_t)st->width;
    }
    outn(status, slen);
}

static void history_push(pager_t *st, const char *href)
{
    st->history = realloc(st->history, (st->nhistory + 1) * sizeof(char *));
    st->history[st->nhistory++] = strdup(href);
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
            st->top = 0;
            st->msg[0] = '\0';
            return;
        }
        for (size_t i = 0; i < st->doc->nlines; i++) {
            for (size_t j = 0; j < st->doc->lines[i].nspans; j++) {
                if (match_anchor(st->doc->lines[i].spans[j].text, target)) {
                    st->top = (int)i;
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
    history_push(st, st->cur_href);
    free(st->cur_href);
    st->cur_href = strdup(href);
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    st->top = 0;
    st->msg[0] = '\0';
}

static void go_back(pager_t *st)
{
    if (st->nhistory == 0) {
        snprintf(st->msg, sizeof(st->msg), "no earlier page");
        return;
    }
    char *prev = st->history[--st->nhistory];
    tam_document_t *nd = st->load(st->ctx, prev);
    if (nd == NULL) {
        st->nhistory++;
        snprintf(st->msg, sizeof(st->msg), "cannot open %s", prev);
        free(prev);
        return;
    }
    free(st->cur_href);
    st->cur_href = prev;
    tam_document_free(st->doc);
    st->doc = nd;
    pager_build_links(st);
    st->top = 0;
    st->msg[0] = '\0';
}

static void select_next_link(pager_t *st, int dir)
{
    if (st->nlinks == 0) {
        return;
    }
    if (st->selected < 0) {
        st->selected = dir > 0 ? 0 : (int)st->nlinks - 1;
    } else {
        st->selected += dir;
        if (st->selected < 0) {
            st->selected = (int)st->nlinks - 1;
        } else if ((size_t)st->selected >= st->nlinks) {
            st->selected = 0;
        }
    }
    int usable = st->height - 1;
    if (usable < 1) {
        usable = 1;
    }
    size_t l = st->links[st->selected].line;
    if ((int)l < st->top) {
        st->top = (int)l;
    } else if ((int)l >= st->top + usable) {
        st->top = (int)l - usable + 1;
    }
    st->msg[0] = '\0';
}

static void follow_selected(pager_t *st)
{
    const tam_span_t *sel = selected_span(st);
    if (sel != NULL) {
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
    case 'j':
        st->top++;
        break;
    case 'k':
        st->top--;
        break;
    case ' ':
        st->top += usable;
        break;
    case 'b':
        st->top -= usable;
        break;
    case 'g':
        st->top = 0;
        break;
    case 'G':
        st->top = max_top;
        break;
    case '\t':
        select_next_link(st, 1);
        break;
    case 'N':
        select_next_link(st, -1);
        break;
    case '\r':
    case '\n': {
        follow_selected(st);
        break;
    }
    case 'u':
        go_back(st);
        break;
    case 0x1b: {
        unsigned char c1;
        if (read_byte(st->input_fd, 60, &c1) != 1) {
            g_stop = 1;
            return;
        }
        if (c1 != '[') {
            return;
        }
        char seq[8];
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
            st->top--;
        } else if (strcmp(seq, "B") == 0) {
            st->top++;
        } else if (strcmp(seq, "C") == 0) {
            follow_selected(st);
        } else if (strcmp(seq, "D") == 0) {
            go_back(st);
        } else if (strcmp(seq, "H") == 0) {
            st->top = 0;
        } else if (strcmp(seq, "F") == 0) {
            st->top = max_top;
        } else if (strcmp(seq, "5~") == 0) {
            st->top -= usable;
        } else if (strcmp(seq, "6~") == 0) {
            st->top += usable;
        }
        break;
    }
    default:
        if (c >= '1' && c <= '9') {
            int idx = c - '1';
            if ((size_t)idx < st->nlinks) {
                st->selected = idx;
                follow(st, st->links[idx].span->href);
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
    st.load = load;
    st.ctx = ctx;
    st.width = tam_terminal_width();
    st.height = terminal_height();
    pager_build_links(&st);

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
        free(st.history[--st.nhistory]);
    }
    free(st.history);
    free(st.links);
    free(st.cur_href);
    tam_document_free(st.doc);

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGWINCH, SIG_DFL);
    return 0;
}
