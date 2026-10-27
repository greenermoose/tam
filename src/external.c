#define _POSIX_C_SOURCE 200809L
#include "external.h"

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

static int is_program(const char *path)
{
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISREG(sb.st_mode) && access(path, X_OK) == 0;
}

static int program_found(const char *name)
{
    if (name[0] == '\0') return 0;
    if (strchr(name, '/') != NULL) return is_program(name);
    const char *path = getenv("PATH");
    if (path == NULL || path[0] == '\0') path = "/usr/bin:/bin";
    char buf[4096];
    for (const char *p = path; ; ) {
        const char *end = strchr(p, ':');
        size_t dlen = end != NULL ? (size_t)(end - p) : strlen(p);
        int n = snprintf(buf, sizeof(buf), "%.*s/%s", (int)dlen,
                         dlen > 0 ? p : ".", name);
        if (n > 0 && (size_t)n < sizeof(buf) && is_program(buf)) return 1;
        if (end == NULL) return 0;
        p = end + 1;
    }
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

/* A browser starts with the signal handling a shell would give it, not the
 * pager's: ignored signals stay ignored across exec. */
static void default_signals(void)
{
    static const int sigs[] = { SIGINT, SIGQUIT, SIGTERM, SIGWINCH, SIGPIPE, SIGTSTP };
    for (size_t i = 0; i < sizeof(sigs) / sizeof(sigs[0]); i++)
        (void)signal(sigs[i], SIG_DFL);
    sigset_t none;
    sigemptyset(&none);
    (void)sigprocmask(SIG_SETMASK, &none, NULL);
}

/* A close-on-exec pipe carries errno back when exec fails; it reads empty
 * once the program has started. */
static int exec_pipe(int fds[2])
{
    if (pipe(fds) != 0) return -1;
    (void)fcntl(fds[0], F_SETFD, FD_CLOEXEC);
    (void)fcntl(fds[1], F_SETFD, FD_CLOEXEC);
    return 0;
}

static void report_errno(int fd, int err)
{
    ssize_t n;
    do n = write(fd, &err, sizeof(err)); while (n < 0 && errno == EINTR);
}

static int read_errno(int fd)
{
    int err = 0;
    ssize_t n;
    do n = read(fd, &err, sizeof(err)); while (n < 0 && errno == EINTR);
    return n == (ssize_t)sizeof(err) ? err : 0;
}

static int wait_child(pid_t pid, int *status)
{
    pid_t r;
    do r = waitpid(pid, status, 0); while (r < 0 && errno == EINTR);
    return r == pid ? 0 : -1;
}

int tam_external_open_detached(const char *url, char *msg, size_t cap)
{
    command_t c = {0};
    if (!tam_external_url_ok(url)) {
        snprintf(msg, cap, "cannot open malformed link");
        return -1;
    }
    if (!choose(1, url, &c)) {
        snprintf(msg, cap, "%s", tam_external_missing(1));
        return -1;
    }
    const char *name = base_name(c.argv[0]);
    int fds[2];
    if (exec_pipe(fds) != 0) {
        snprintf(msg, cap, "cannot start %s: %s", name, strerror(errno));
        command_free(&c);
        return -1;
    }
    pid_t pid = fork();
    int fork_err = errno;
    if (pid == 0) {
        /* A second fork leaves the browser to init, so it never becomes a
         * zombie of the pager and outlives it. */
        close(fds[0]);
        (void)setsid();
        pid_t grandchild = fork();
        if (grandchild < 0) report_errno(fds[1], errno);
        if (grandchild != 0) _exit(grandchild < 0 ? 1 : 0);
        int null = open("/dev/null", O_RDWR);
        if (null >= 0) {
            (void)dup2(null, STDIN_FILENO);
            (void)dup2(null, STDOUT_FILENO);
            (void)dup2(null, STDERR_FILENO);
            if (null > STDERR_FILENO) close(null);
        }
        default_signals();
        execvp(c.argv[0], c.argv);
        report_errno(fds[1], errno);
        _exit(127);
    }
    close(fds[1]);
    int err = pid < 0 ? fork_err : read_errno(fds[0]);
    close(fds[0]);
    if (pid > 0) (void)wait_child(pid, NULL);
    if (err != 0) snprintf(msg, cap, "cannot start %s: %s", name, strerror(err));
    else snprintf(msg, cap, "opened in %s", name);
    command_free(&c);
    return err != 0 ? -1 : 0;
}

int tam_external_run_foreground(const char *url, int tty_fd,
                                char *msg, size_t cap)
{
    command_t c = {0};
    if (!tam_external_url_ok(url)) {
        snprintf(msg, cap, "cannot open malformed link");
        return -1;
    }
    if (!choose(0, url, &c)) {
        snprintf(msg, cap, "%s", tam_external_missing(0));
        return -1;
    }
    const char *name = base_name(c.argv[0]);
    int fds[2];
    if (exec_pipe(fds) != 0) {
        snprintf(msg, cap, "cannot start %s: %s", name, strerror(errno));
        command_free(&c);
        return -1;
    }
    /* Interrupt and quit keys belong to the browser while it runs. */
    struct sigaction ign, old_int, old_quit;
    memset(&ign, 0, sizeof(ign));
    ign.sa_handler = SIG_IGN;
    sigemptyset(&ign.sa_mask);
    int have_int = sigaction(SIGINT, &ign, &old_int) == 0;
    int have_quit = sigaction(SIGQUIT, &ign, &old_quit) == 0;
    pid_t pid = fork();
    int fork_err = errno;
    if (pid == 0) {
        close(fds[0]);
        if (tty_fd >= 0 && !isatty(STDIN_FILENO)) (void)dup2(tty_fd, STDIN_FILENO);
        default_signals();
        execvp(c.argv[0], c.argv);
        report_errno(fds[1], errno);
        _exit(127);
    }
    close(fds[1]);
    int err = pid < 0 ? fork_err : read_errno(fds[0]);
    close(fds[0]);
    int status = 0;
    int waited = pid > 0 && wait_child(pid, &status) == 0;
    if (have_int) (void)sigaction(SIGINT, &old_int, NULL);
    if (have_quit) (void)sigaction(SIGQUIT, &old_quit, NULL);
    int rc = -1;
    if (err != 0)
        snprintf(msg, cap, "cannot start %s: %s", name, strerror(err));
    else if (!waited)
        snprintf(msg, cap, "lost track of %s", name);
    else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        snprintf(msg, cap, "%s exited with status %d", name, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        snprintf(msg, cap, "%s ended by signal %d", name, WTERMSIG(status));
    else {
        snprintf(msg, cap, "back from %s", name);
        rc = 0;
    }
    command_free(&c);
    return rc;
}
