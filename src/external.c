#define _POSIX_C_SOURCE 200809L
#include "external.h"

#include <tam.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_URL  2048
#define MAX_ARGS 16

typedef struct {
    char *argv[MAX_ARGS + 2];
    int   argc;
} command_t;

static const char *const text_browsers[] = { "w3m", "lynx", "elinks", "links" };

int tam_external_url_ok(const char *url)
{
    if (url == NULL) return 0;
    size_t skip;
    if (strncasecmp(url, "https://", 8) == 0) skip = 8;
    else if (strncasecmp(url, "http://", 7) == 0) skip = 7;
    else return 0;
    if (url[skip] == '\0' || strlen(url) > MAX_URL) return 0;
    /* An HTTP URL needs an authority, not just a path, query or fragment. */
    if (strchr("/?#", url[skip]) != NULL) return 0;
    /* URLs are ASCII; a space, control byte or raw UTF-8 byte is refused,
     * which also keeps the OSC 8 escape the pager wraps around it intact. */
    for (const unsigned char *p = (const unsigned char *)url; *p; p++)
        if (*p <= 0x20 || *p >= 0x7f || strchr("\\\"<>^`{|}", *p) != NULL) return 0;
    return 1;
}

int tam_external_refusal_reason(const char *url, char *msg, size_t cap)
{
    if (tam_external_url_ok(url)) return 0;
    if (msg == NULL || cap == 0) return -1;
    size_t scheme = 0;
    if (url != NULL) scheme = strcspn(url, ":/?#");
    if (url == NULL || url[scheme] != ':' ||
        (scheme == 4 && strncasecmp(url, "http", 4) == 0) ||
        (scheme == 5 && strncasecmp(url, "https", 5) == 0))
        snprintf(msg, cap, "cannot open malformed link");
    else
        snprintf(msg, cap, "cannot open %.*s: links",
                 scheme < 24 ? (int)scheme : 24, url);
    return -1;
}

int tam_external_graphical(void)
{
    const char *wayland = getenv("WAYLAND_DISPLAY");
    const char *x11 = getenv("DISPLAY");
    return (wayland != NULL && wayland[0] != '\0') || (x11 != NULL && x11[0] != '\0');
}

static void command_free(command_t *c)
{
    for (int i = 0; i < c->argc; i++) free(c->argv[i]);
    c->argc = 0;
    c->argv[0] = NULL;
}

/* One word of a browser entry: %s becomes the URL and %% a percent sign. */
static char *expand_word(const char *w, size_t len, const char *url, int *used)
{
    size_t ulen = url != NULL ? strlen(url) : 0;
    size_t need = 1;
    for (size_t i = 0; i < len; i++)
        need += (w[i] == '%' && i + 1 < len && w[i + 1] == 's') ? ulen : 1;
    char *out = malloc(need);
    if (out == NULL) return NULL;
    size_t o = 0;
    for (size_t i = 0; i < len; i++) {
        if (w[i] == '%' && i + 1 < len && w[i + 1] == 's') {
            if (ulen > 0) memcpy(out + o, url, ulen);
            o += ulen;
            *used = 1;
            i++;
        } else if (w[i] == '%' && i + 1 < len && w[i + 1] == '%') {
            out[o++] = '%';
            i++;
        } else {
            out[o++] = w[i];
        }
    }
    out[o] = '\0';
    return out;
}

/* Split one browser entry on blanks, without a shell. The URL is appended
 * when no word holds %s. Returns 0 for an empty or overlong entry. */
static int command_parse(command_t *c, const char *entry, size_t len, const char *url)
{
    c->argc = 0;
    int used = 0;
    size_t i = 0;
    while (i < len) {
        while (i < len && (entry[i] == ' ' || entry[i] == '\t')) i++;
        size_t start = i;
        while (i < len && entry[i] != ' ' && entry[i] != '\t') i++;
        if (i == start) break;
        if (c->argc == MAX_ARGS) { command_free(c); return 0; }
        char *word = expand_word(entry + start, i - start, url, &used);
        if (word == NULL) { command_free(c); return 0; }
        c->argv[c->argc++] = word;
    }
    if (c->argc == 0) return 0;
    if (!used && url != NULL) {
        c->argv[c->argc] = strdup(url);
        if (c->argv[c->argc] == NULL) { command_free(c); return 0; }
        c->argc++;
    }
    c->argv[c->argc] = NULL;
    return 1;
}

static int program_found(const char *name)
{
    char *path = tam_which(name);
    free(path);
    return path != NULL;
}

/* The first installed entry of a colon-separated list such as $BROWSER. */
static int choose_from(const char *list, const char *url, command_t *out)
{
    if (list == NULL) return 0;
    for (const char *p = list; ; ) {
        const char *end = strchr(p, ':');
        size_t len = end != NULL ? (size_t)(end - p) : strlen(p);
        if (command_parse(out, p, len, url)) {
            if (program_found(out->argv[0])) return 1;
            command_free(out);
        }
        if (end == NULL) return 0;
        p = end + 1;
    }
}

static int choose(int graphical, const char *url, command_t *out)
{
    if (graphical)
        return choose_from(getenv("BROWSER"), url, out) ||
               choose_from("xdg-open", url, out);
    if (choose_from(getenv("TAM_TEXT_BROWSER"), url, out)) return 1;
    for (size_t i = 0; i < sizeof(text_browsers) / sizeof(text_browsers[0]); i++)
        if (choose_from(text_browsers[i], url, out)) return 1;
    return 0;
}

static const char *base_name(const char *path)
{
    const char *slash = strrchr(path, '/');
    return slash != NULL ? slash + 1 : path;
}

int tam_external_opener(int graphical, char *buf, size_t cap)
{
    command_t c = {0};
    if (!choose(graphical, NULL, &c)) {
        if (cap > 0) buf[0] = '\0';
        return 0;
    }
    snprintf(buf, cap, "%s", base_name(c.argv[0]));
    command_free(&c);
    return 1;
}

const char *tam_external_missing(int graphical)
{
    return graphical ? "no browser found; set BROWSER"
                     : "no text browser found; install w3m or lynx, or set TAM_TEXT_BROWSER";
}

int tam_external_open_detached(const char *url, char *msg, size_t cap)
{
    command_t c = {0};
    if (tam_external_refusal_reason(url, msg, cap) != 0)
        return -1;
    if (!choose(1, url, &c)) {
        snprintf(msg, cap, "%s", tam_external_missing(1));
        return -1;
    }
    const char *name = base_name(c.argv[0]);
    /* Detached in its own session, with the defaults a shell would give it
     * and none of the pager's descriptors (the /dev/tty one included);
     * reaped by init. */
    int rc = tam_spawn_detached((const char *const *)c.argv);
    if (rc != 0) snprintf(msg, cap, "cannot start %s: %s", name, strerror(errno));
    else snprintf(msg, cap, "opened in %s", name);
    command_free(&c);
    return rc != 0 ? -1 : 0;
}

int tam_run_foreground_command(char *const argv[], int tty_fd,
                               char *msg, size_t cap)
{
    const char *name = base_name(argv[0]);
    /* When our stdin is a pipe (`... | tam show -`), the program reads the
     * terminal instead. Ctrl+C and Ctrl+\ reach only the program. */
    tam_proc_opts_t opts = {0};
    if (tty_fd > 0 && !isatty(STDIN_FILENO)) opts.stdin_fd = tty_fd;
    int code = 0;
    if (tam_run_foreground_opts((const char *const *)argv, &opts, &code) != 0) {
        snprintf(msg, cap, "cannot start %s: %s", name, strerror(errno));
        return -1;
    }
    if (code != 0) {
        snprintf(msg, cap, "%s exited with status %d", name, code);
        return -1;
    }
    snprintf(msg, cap, "back from %s", name);
    return 0;
}

int tam_external_run_foreground(const char *url, int tty_fd,
                                char *msg, size_t cap)
{
    command_t c = {0};
    if (tam_external_refusal_reason(url, msg, cap) != 0)
        return -1;
    if (!choose(0, url, &c)) {
        snprintf(msg, cap, "%s", tam_external_missing(0));
        return -1;
    }
    int rc = tam_run_foreground_command(c.argv, tty_fd, msg, cap);
    command_free(&c);
    return rc;
}
