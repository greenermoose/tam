#include <tam.h>

#include "ask.h"
#include "depth.h"
#include "index.h"
#include "install.h"
#include "man.h"
#include "pager.h"
#include "render.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "version.h"

#ifndef TAM_DATADIR
#define TAM_DATADIR "/usr/local/share/tamlinux/knowledge"
#endif

/* Set by the Makefile from `git describe` when building from a checkout;
 * empty for packaged builds, which have no Git metadata. */
#ifndef TAM_BUILD
#define TAM_BUILD ""
#endif

#define TAM_EXIT_USAGE 2
#define TAM_EXIT_FAILURE 1

static char *join_path(const char *dir, const char *rest)
{
    size_t dlen = strlen(dir);
    size_t rlen = strlen(rest);
    char *out = malloc(dlen + rlen + 1);
    if (out == NULL) {
        return NULL;
    }
    memcpy(out, dir, dlen);
    memcpy(out + dlen, rest, rlen + 1);
    return out;
}

static char *trim_in_place(char *s)
{
    size_t len = strlen(s);
    while (len > 0) {
        char c = s[len - 1];
        if (c != '\n' && c != '\r' && c != ' ' && c != '\t') {
            break;
        }
        s[--len] = '\0';
    }
    return s;
}

static char *cultivar_version_from_file(void)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");
    char *path = NULL;

    if (xdg != NULL && xdg[0] != '\0') {
        path = join_path(xdg, "/tamlinux/version");
    } else if (home != NULL && home[0] != '\0') {
        path = join_path(home, "/.config/tamlinux/version");
    }

    if (path == NULL) {
        return NULL;
    }

    char *buf = tam_fs_read_file(path, NULL);
    free(path);
    if (buf == NULL) {
        return NULL;
    }

    trim_in_place(buf);
    if (buf[0] == '\0') {
        free(buf);
        return NULL;
    }
    return buf;
}

char *cultivar_version(void)
{
    char *from_file = cultivar_version_from_file();
    if (from_file != NULL) {
        return from_file;
    }

    const char *env = getenv("TAMLINUX_VERSION");
    if (env != NULL && env[0] != '\0') {
        return strdup(env);
    }
    return NULL;
}

static void print_intro(void)
{
    tam_str_t s = tam_str_new("");
    if (s == NULL) {
        return;
    }
    s = tam_str_cat(s, TAM_COMMAND_NAME " - the Tamlinux command\n\n");
    s = tam_str_cat(s,
        "Learn about this system, keep it healthy, and find your way\n"
        "around Tamlinux, offline, from the terminal.\n\n");
    s = tam_str_cat_printf(s,
        "This is an early development build (tam %s).\n\n",
        TAM_COMMAND_VERSION);
    s = tam_str_cat(s,
        "Get started:\n"
        "  tam welcome          open the welcome orientation\n"
        "  tam show             open the offline guide\n"
        "  tam search TERM...   search the offline guide\n"
        "  tam explain [TOPIC]  explain a topic or list available topics\n"
        "  tam install inspect  inspect host and installation capabilities\n"
        "  tam --help           list options and where to begin\n"
        "  tam --version        show version information\n");
    fputs(s, stdout);
    tam_str_free(s);
}

static void print_help(void)
{
    fputs(
        "tam - the Tamlinux command\n\n"
        "Usage:\n"
        "  tam                         short introduction and exit\n"
        "  tam show [PAGE|-] [--pipe]  offline guide (or stdin/plain text)\n"
        "  tam search [--json] TERM... search guide topics and keywords\n"
        "  tam welcome [--pipe]        first-run orientation\n"
        "  tam explain [TOPIC]         usage overview or topic explanation\n"
        "  tam explain topics [--terse|--verbose] [--level L]  list available topics\n"
        "  tam man [SECTION] PAGE      hyperlinked manual page viewer\n"
        "  tam install inspect [--json|--check]  inspect host and check system health\n"
        "  tam ask [--json] QUESTION   grounded question answering via local model\n"
        "  tam --help | --version      show help or version information\n\n"
        "Output modifiers: --pipe (plain text), --terse (compact), --verbose (detail).\n",
        stdout);
}

static void print_version(void)
{
    char *cultivar = cultivar_version();

    if (TAM_BUILD[0] != '\0') {
        printf(TAM_COMMAND_NAME " command %s (dev %s)\n", TAM_COMMAND_VERSION,
               TAM_BUILD);
    } else {
        printf(TAM_COMMAND_NAME " command %s\n", TAM_COMMAND_VERSION);
    }
    printf("libtam %s\n", tam_version());
    if (cultivar != NULL) {
        printf("Tamlinux cultivar %s\n", cultivar);
        free(cultivar);
    } else {
        printf("Tamlinux cultivar: unavailable (no installed cultivar metadata)\n");
    }
}

static int usage_error(const char *bad)
{
    fprintf(stderr, TAM_COMMAND_NAME ": unrecognized argument '%s'\n", bad);
    fprintf(stderr, "Run '%s --help' for available options.\n",
            TAM_COMMAND_NAME);
    return TAM_EXIT_USAGE;
}

static int too_many_arguments(void)
{
    fprintf(stderr, TAM_COMMAND_NAME ": too many arguments\n");
    fprintf(stderr, "Run '%s --help' for available options.\n",
            TAM_COMMAND_NAME);
    return TAM_EXIT_USAGE;
}

/* ------------------------------------------------------------------------- */
/* Offline guide                                                              */
/* ------------------------------------------------------------------------- */

static char *safe_join(const char *root, const char *href)
{
    if (href == NULL || href[0] == '\0') {
        return NULL;
    }
    if (href[0] == '/') {
        return NULL;
    }
    if (strstr(href, "://") != NULL || strstr(href, "..") != NULL) {
        return NULL;
    }
    char *rest = malloc(strlen(href) + 2);
    if (rest == NULL) {
        return NULL;
    }
    rest[0] = '/';
    memcpy(rest + 1, href, strlen(href) + 1);
    char *out = join_path(root, rest);
    free(rest);
    return out;
}

static int knowledge_ok(const char *dir)
{
    if (dir == NULL || dir[0] == '\0') {
        return 0;
    }
    char *p = join_path(dir, "/index.html");
    int ok = p != NULL && tam_fs_file_exists(p);
    free(p);
    return ok;
}

char *tam_find_knowledge_root(void)
{
    const char *env = getenv("TAM_KNOWLEDGE_DIR");
    if (env != NULL && knowledge_ok(env)) {
        return strdup(env);
    }

    /* The corpus installed with this binary comes before any shared data
     * directory, so a development install under one prefix never renders
     * the pages of a packaged install under another. */
    const char *compiled = getenv("TAM_DATADIR");
    if (compiled == NULL || compiled[0] == '\0') {
        compiled = TAM_DATADIR;
    }
    if (knowledge_ok(compiled)) {
        return strdup(compiled);
    }

    const char *xdg = getenv("XDG_DATA_HOME");
    if (xdg != NULL && xdg[0] != '\0') {
        char *p = join_path(xdg, "/tamlinux/knowledge");
        if (knowledge_ok(p)) {
            return p;
        }
        free(p);
    }

    const char *home = getenv("HOME");
    if (home != NULL && home[0] != '\0') {
        char *p = join_path(home, "/.local/share/tamlinux/knowledge");
        if (knowledge_ok(p)) {
            return p;
        }
        free(p);
    }

    const char *dirs = getenv("XDG_DATA_DIRS");
    if (dirs != NULL && dirs[0] != '\0') {
        const char *s = dirs;
        while (*s != '\0') {
            const char *colon = strchr(s, ':');
            size_t len = colon ? (size_t)(colon - s) : strlen(s);
            if (len > 0) {
                char *base = malloc(len + 1);
                memcpy(base, s, len);
                base[len] = '\0';
                char *p = join_path(base, "/tamlinux/knowledge");
                free(base);
                if (knowledge_ok(p)) {
                    return p;
                }
                free(p);
            }
            if (colon == NULL) {
                break;
            }
            s = colon + 1;
        }
    }

    return NULL;
}

static inline char *find_knowledge_root(void)
{
    return tam_find_knowledge_root();
}

static int knowledge_file_exists(const char *root, const char *rel)
{
    char *p = safe_join(root, rel);
    int ok = p != NULL && tam_fs_file_exists(p);
    free(p);
    return ok;
}

static void resolve_page_path(const char *root, const char *page, char *out,
                              size_t cap)
{
    static const char suffix[] = ".html";
    if (strstr(page, ".html") != NULL) {
        snprintf(out, cap, "%s", page);
        return;
    }
    char cand[600];
    int n = snprintf(cand, sizeof(cand), "%s%s", page, suffix);
    if (n > 0 && (size_t)n < sizeof(cand) && knowledge_file_exists(root, cand)) {
        snprintf(out, cap, "%s", cand);
        return;
    }
    n = snprintf(cand, sizeof(cand), "%s/index.html", page);
    if (n > 0 && (size_t)n < sizeof(cand) && knowledge_file_exists(root, cand)) {
        snprintf(out, cap, "%s", cand);
        return;
    }
    snprintf(out, cap, "%s%s", page, suffix);
}

/* Resolve a page name, keeping any "?level=NAME" depth selection. */
static void resolve_page(const char *root, const char *page, char *out,
                         size_t cap)
{
    const char *q = strchr(page, '?');
    if (q == NULL) {
        resolve_page_path(root, page, out, cap);
        return;
    }
    char base[600];
    size_t n = (size_t)(q - page);
    if (n >= sizeof(base)) {
        n = sizeof(base) - 1;
    }
    memcpy(base, page, n);
    base[n] = '\0';
    char path[600];
    resolve_page_path(root, base, path, sizeof(path));
    snprintf(out, cap, "%s%s", path, q);
}

typedef struct {
    char *root;
    int   width;
} corpus_ctx_t;

/* Read a corpus page and prepare it at its selected depth. `href` may carry
 * "?level=NAME". Returns NULL when the page is missing, unreadable, or does
 * not offer the selected level. */
static char *read_corpus_page(const char *root, const char *href,
                              size_t *out_len)
{
    unsigned level = 0;
    char *rel = tam_depth_split_href(href, &level);
    if (rel == NULL) {
        return NULL;
    }
    char *path = safe_join(root, rel);
    if (path == NULL) {
        free(rel);
        return NULL;
    }
    size_t len = 0;
    char *src = tam_fs_read_file(path, &len);
    free(path);
    if (src == NULL) {
        free(rel);
        return NULL;
    }
    char *html = tam_depth_apply(src, len, rel, level, out_len, NULL);
    free(src);
    free(rel);
    return html;
}

tam_document_t *tam_topics_document(const char *root, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }
    tam_str_t html = tam_str_new("<!DOCTYPE html>\n<html>\n<head>\n");
    html = tam_str_cat(html, "<title>Available Explanation Topics</title>\n</head>\n<body>\n");
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Topics with a %s level</h1>\n", tam_level_name(level));
    } else {
        html = tam_str_cat(html, "<h1>Available Explanation Topics</h1>\n");
    }
    html = tam_str_cat(html,
        "<p>Select a topic with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to view its explanation.<br>\n"
        "Press <kbd>`</kbd> to return to the guide index, or <kbd>q</kbd> to exit.</p>\n"
        "<table>\n"
        "<tr><th>Topic</th><th>Summary</th></tr>\n");
    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (level != 0 && !(e->levels & level)) {
            continue;
        }
        if (level != 0) {
            html = tam_str_cat_printf(html,
                "<tr><td><a href=\"%s?level=%s\">%s</a></td><td>%s</td></tr>\n",
                e->path, tam_level_name(level), e->topic, e->summary);
        } else {
            html = tam_str_cat_printf(html,
                "<tr><td><a href=\"%s\">%s</a></td><td>%s</td></tr>\n",
                e->path, e->topic, e->summary);
        }
    }
    html = tam_str_cat(html, "</table>\n</body>\n</html>\n");
    tam_index_free(idx);

    tam_document_t *doc = tam_render_html(html, strlen(html), width);
    tam_str_free(html);
    return doc;
}

static tam_document_t *corpus_load(void *vctx, const char *href)
{
    corpus_ctx_t *ctx = vctx;
    if (href == NULL || href[0] == '\0') {
        return NULL;
    }
    if (strncmp(href, "man:", 4) == 0) {
        return tam_man_load(href + 4, ctx->width);
    }
    if (strncmp(href, "tam:topics", 10) == 0) {
        unsigned level = 0;
        const char *q = strstr(href, "?level=");
        if (q != NULL) {
            level = tam_level_parse(q + 7);
        }
        return tam_topics_document(ctx->root, level, ctx->width);
    }
    if (ctx->root == NULL) {
        return NULL;
    }
    size_t len = 0;
    char *src = read_corpus_page(ctx->root, href, &len);
    if (src == NULL) {
        return NULL;
    }
    tam_document_t *doc = tam_render_html(src, len, ctx->width);
    free(src);
    return doc;
}

static int ensure_dir_exists(const char *dir)
{
    if (dir == NULL || dir[0] == '\0') {
        return -1;
    }
    char tmp[4096];
    size_t len = strlen(dir);
    if (len >= sizeof(tmp)) {
        return -1;
    }
    memcpy(tmp, dir, len + 1);
    for (char *p = tmp + 1; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\0';
            if (tmp[0] != '\0') {
                if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                    return -1;
                }
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

static int record_welcome_seen(void)
{
    const char *xdg = getenv("XDG_STATE_HOME");
    const char *home = getenv("HOME");
    char *dir = NULL;
    char *path = NULL;

    if (xdg != NULL && xdg[0] != '\0') {
        dir = join_path(xdg, "/tamlinux");
        path = join_path(xdg, "/tamlinux/welcome_seen");
    } else if (home != NULL && home[0] != '\0') {
        dir = join_path(home, "/.local/state/tamlinux");
        path = join_path(home, "/.local/state/tamlinux/welcome_seen");
    }

    if (dir == NULL || path == NULL) {
        free(dir);
        free(path);
        return -1;
    }

    if (ensure_dir_exists(dir) != 0) {
        free(dir);
        free(path);
        return -1;
    }
    free(dir);

    time_t now = time(NULL);
    struct tm tm;
    gmtime_r(&now, &tm);
    char buf[64];
    size_t len = strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ\n", &tm);

    bool ok = tam_fs_write_atomic(path, buf, len, 0644);
    free(path);
    return ok ? 0 : -1;
}

static char *read_all_stdin(size_t *out_len)
{
    size_t cap = 8192;
    size_t len = 0;
    char *buf = malloc(cap);
    if (buf == NULL) {
        return NULL;
    }
    while (1) {
        if (len + 4096 >= cap) {
            cap *= 2;
            char *next = realloc(buf, cap);
            if (next == NULL) {
                free(buf);
                return NULL;
            }
            buf = next;
        }
        ssize_t n = read(STDIN_FILENO, buf + len, cap - len - 1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            free(buf);
            return NULL;
        }
        if (n == 0) {
            break;
        }
        len += (size_t)n;
    }
    buf[len] = '\0';
    *out_len = len;
    return buf;
}

static int view_document(const char *root, const char *href, const char *src,
                         size_t len, int pipe)
{
    int interactive = !pipe && isatty(STDOUT_FILENO);
    if (interactive && !isatty(STDIN_FILENO)) {
        int tty = open("/dev/tty", O_RDONLY);
        if (tty < 0) {
            interactive = 0;
        } else {
            close(tty);
        }
    }
    int width = interactive ? tam_terminal_width() : 80;

    tam_document_t *doc = tam_render_html(src, len, width);
    if (doc == NULL) {
        return TAM_EXIT_FAILURE;
    }

    int rc = 0;
    if (interactive) {
        corpus_ctx_t ctx;
        ctx.root = (char *)root;
        ctx.width = width;
        rc = tam_pager_run(doc, href, root != NULL ? corpus_load : NULL, &ctx);
    } else {
        char *text = tam_document_text(doc);
        if (text != NULL) {
            fputs(text, stdout);
            free(text);
        }
        tam_document_free(doc);
    }
    return rc;
}

static int view_knowledge_page(const char *root, const char *href, int pipe)
{
    unsigned level = 0;
    char *rel = tam_depth_split_href(href, &level);
    char *path = rel != NULL ? safe_join(root, rel) : NULL;
    free(rel);
    if (path == NULL || !tam_fs_file_exists(path)) {
        fprintf(stderr, TAM_COMMAND_NAME ": guide page not found: %s\n", href);
        free(path);
        return TAM_EXIT_FAILURE;
    }
    free(path);

    size_t len = 0;
    char *src = read_corpus_page(root, href, &len);
    if (src == NULL) {
        if (level != 0) {
            fprintf(stderr,
                    TAM_COMMAND_NAME ": guide page %s has no %s level\n",
                    href, tam_level_name(level));
        } else {
            fprintf(stderr, TAM_COMMAND_NAME ": cannot read guide page: %s\n",
                    href);
        }
        return TAM_EXIT_FAILURE;
    }

    int rc = view_document(root, href, src, len, pipe);
    free(src);
    return rc;
}

static int cmd_show(int n, char **args)
{
    int pipe = 0;
    const char *page = NULL;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (strcmp(args[i], "-") == 0) {
            if (page == NULL) {
                page = "-";
            } else {
                return too_many_arguments();
            }
        } else if (args[i][0] == '-') {
            return usage_error(args[i]);
        } else if (page == NULL) {
            page = args[i];
        } else {
            return too_many_arguments();
        }
    }

    if (page != NULL && strcmp(page, "-") == 0) {
        size_t len = 0;
        char *src = read_all_stdin(&len);
        if (src == NULL) {
            fprintf(stderr, TAM_COMMAND_NAME ": failed to read HTML from standard input\n");
            return TAM_EXIT_FAILURE;
        }
        char *root = find_knowledge_root();
        int rc = view_document(root, "<stdin>", src, len, pipe);
        free(src);
        free(root);
        return rc;
    }

    char *root = find_knowledge_root();
    if (root == NULL) {
        fprintf(stderr,
                TAM_COMMAND_NAME ": no offline guide installed "
                "(set TAM_KNOWLEDGE_DIR to a knowledge directory)\n");
        return TAM_EXIT_FAILURE;
    }

    if (page != NULL && (strcmp(page, "topics") == 0 || strncmp(page, "tam:topics", 10) == 0)) {
        unsigned level = 0;
        const char *q = strstr(page, "?level=");
        if (q != NULL) {
            level = tam_level_parse(q + 7);
        }
        int interactive = !pipe && isatty(STDOUT_FILENO);
        int width = interactive ? tam_terminal_width() : 80;
        tam_document_t *doc = tam_topics_document(root, level, width);
        if (doc == NULL) {
            free(root);
            return TAM_EXIT_FAILURE;
        }
        int rc = 0;
        if (interactive) {
            corpus_ctx_t ctx;
            ctx.root = root;
            ctx.width = width;
            rc = tam_pager_run(doc, "tam:topics", corpus_load, &ctx);
        } else {
            char *text = tam_document_text(doc);
            if (text != NULL) {
                fputs(text, stdout);
                free(text);
            }
            tam_document_free(doc);
        }
        free(root);
        return rc;
    }

    if (page != NULL && strncmp(page, "man:", 4) == 0) {
        int interactive = !pipe && isatty(STDOUT_FILENO);
        int width = interactive ? tam_terminal_width() : 80;
        tam_document_t *doc = tam_man_load(page + 4, width);
        if (doc == NULL) {
            fprintf(stderr, TAM_COMMAND_NAME ": no manual page found for '%s'\n", page + 4);
            free(root);
            return TAM_EXIT_FAILURE;
        }
        int rc = 0;
        if (interactive) {
            corpus_ctx_t ctx;
            ctx.root = root;
            ctx.width = width;
            rc = tam_pager_run(doc, page, corpus_load, &ctx);
        } else {
            char *text = tam_document_text(doc);
            if (text != NULL) {
                fputs(text, stdout);
                free(text);
            }
            tam_document_free(doc);
        }
        free(root);
        return rc;
    }

    char href[600];
    if (page == NULL) {
        snprintf(href, sizeof(href), "%s", "index.html");
    } else {
        resolve_page(root, page, href, sizeof(href));
    }

    int rc = view_knowledge_page(root, href, pipe);
    free(root);
    return rc;
}

static int cmd_welcome(int n, char **args)
{
    int pipe = 0;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (args[i][0] == '-') {
            return usage_error(args[i]);
        } else {
            return too_many_arguments();
        }
    }

    char *root = find_knowledge_root();
    if (root == NULL) {
        fprintf(stderr,
                TAM_COMMAND_NAME ": no offline guide installed "
                "(set TAM_KNOWLEDGE_DIR to a knowledge directory)\n");
        return TAM_EXIT_FAILURE;
    }

    int rc = view_knowledge_page(root, "welcome.html", pipe);
    if (rc == 0) {
        record_welcome_seen();
    }
    free(root);
    return rc;
}

static int level_usage_error(const char *bad)
{
    if (bad == NULL) {
        fprintf(stderr, TAM_COMMAND_NAME ": --level requires a value\n");
    } else {
        fprintf(stderr, TAM_COMMAND_NAME ": unknown level '%s'\n", bad);
    }
    fprintf(stderr, "Levels: beginner, intermediate, expert.\n");
    return TAM_EXIT_USAGE;
}

/* Open a resolved topic, at `level` when one was requested. */
static int explain_entry(const char *root, const tam_index_entry_t *entry,
                         unsigned level, int pipe)
{
    if (level == 0) {
        return view_knowledge_page(root, entry->path, pipe);
    }
    if (!(entry->levels & level)) {
        char offered[64];
        tam_level_format(entry->levels, ", ", offered, sizeof(offered));
        fprintf(stderr, TAM_COMMAND_NAME ": topic '%s' has no %s level "
                "(available: %s)\n", entry->topic, tam_level_name(level),
                offered);
        fprintf(stderr, "Run 'tam explain %s' to view it.\n", entry->topic);
        return TAM_EXIT_FAILURE;
    }
    char href[600];
    snprintf(href, sizeof(href), "%s?level=%s", entry->path,
             tam_level_name(level));
    return view_knowledge_page(root, href, pipe);
}

static void print_explain_usage(void)
{
    fputs(
        "tam explain - explain a curated topic or concept\n\n"
        "Usage:\n"
        "  tam explain TOPIC           explain a topic in the terminal\n"
        "  tam explain TOPIC --pipe    print topic explanation as plain text\n"
        "  tam explain --level LEVEL TOPIC  explain at depth (beginner/intermediate/expert)\n"
        "  tam explain topics          interactive list of topics with live hyperlinks\n"
        "  tam explain topics --pipe   print available topics as plain text\n"
        "  tam explain -               read topic keyword from standard input\n\n"
        "Run 'tam explain topics' to browse all available topics.\n"
        "Run 'tam explain <topic>' to view a specific explanation.\n"
        "Run 'tam show' to open the offline guide.\n",
        stdout);
}

static int cmd_explain(int n, char **args)
{
    int pipe = 0;
    int terse = 0;
    int verbose = 0;
    int from_stdin = 0;
    unsigned level = 0;
    char stdin_topic[512] = {0};
    char query_buf[512] = {0};
    size_t qlen = 0;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (strcmp(args[i], "--terse") == 0) {
            terse = 1;
        } else if (strcmp(args[i], "--verbose") == 0 || strcmp(args[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(args[i], "--level") == 0 ||
                   strncmp(args[i], "--level=", 8) == 0) {
            const char *value = NULL;
            if (args[i][7] == '=') {
                value = args[i] + 8;
            } else if (i + 1 < n) {
                value = args[++i];
            }
            if (value == NULL || value[0] == '\0') {
                return level_usage_error(NULL);
            }
            level = tam_level_parse(value);
            if (level == 0) {
                return level_usage_error(value);
            }
        } else if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0) {
            print_explain_usage();
            return 0;
        } else if (strcmp(args[i], "-") == 0) {
            if (from_stdin || qlen > 0) {
                return too_many_arguments();
            }
            from_stdin = 1;
        } else if (args[i][0] == '-') {
            return usage_error(args[i]);
        } else {
            if (from_stdin) {
                return too_many_arguments();
            }
            if (qlen > 0 && qlen + 1 < sizeof(query_buf)) {
                query_buf[qlen++] = ' ';
            }
            size_t alen = strlen(args[i]);
            if (qlen + alen < sizeof(query_buf)) {
                memcpy(query_buf + qlen, args[i], alen);
                qlen += alen;
                query_buf[qlen] = '\0';
            }
        }
    }

    if (from_stdin) {
        if (fgets(stdin_topic, sizeof(stdin_topic), stdin) == NULL) {
            fprintf(stderr, TAM_COMMAND_NAME ": failed to read topic from standard input\n");
            return TAM_EXIT_FAILURE;
        }
        size_t slen = strlen(stdin_topic);
        while (slen > 0 && (stdin_topic[slen - 1] == '\n' || stdin_topic[slen - 1] == '\r' ||
                            stdin_topic[slen - 1] == ' ' || stdin_topic[slen - 1] == '\t')) {
            stdin_topic[--slen] = '\0';
        }
        snprintf(query_buf, sizeof(query_buf), "%s", stdin_topic);
        qlen = strlen(query_buf);
    }

    if (qlen == 0 || strcmp(query_buf, "help") == 0) {
        if (level == 0 && !terse && !verbose) {
            print_explain_usage();
            return 0;
        }
        strcpy(query_buf, "topics");
        qlen = 6;
    }

    char *root = tam_find_knowledge_root();
    if (root == NULL) {
        fprintf(stderr,
                TAM_COMMAND_NAME ": no offline guide installed "
                "(set TAM_KNOWLEDGE_DIR to a knowledge directory)\n");
        return TAM_EXIT_FAILURE;
    }

    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        fprintf(stderr, TAM_COMMAND_NAME ": failed to load guide index\n");
        free(root);
        return TAM_EXIT_FAILURE;
    }

    if (strcmp(query_buf, "topics") == 0) {
        int interactive = !pipe && !terse && !verbose && isatty(STDOUT_FILENO);
        if (interactive && !isatty(STDIN_FILENO)) {
            int tty = open("/dev/tty", O_RDONLY);
            if (tty < 0) {
                interactive = 0;
            } else {
                close(tty);
            }
        }

        if (interactive) {
            int width = tam_terminal_width();
            tam_document_t *doc = tam_topics_document(root, level, width);
            if (doc == NULL) {
                tam_index_free(idx);
                free(root);
                return TAM_EXIT_FAILURE;
            }
            corpus_ctx_t ctx;
            ctx.root = root;
            ctx.width = width;
            char initial_href[64];
            if (level != 0) {
                snprintf(initial_href, sizeof(initial_href), "tam:topics?level=%s", tam_level_name(level));
            } else {
                snprintf(initial_href, sizeof(initial_href), "tam:topics");
            }
            int rc = tam_pager_run(doc, initial_href, corpus_load, &ctx);
            tam_index_free(idx);
            free(root);
            return rc;
        }

        tam_index_print_catalog_ex(stdout, idx, level, terse != 0, verbose != 0);
        tam_index_free(idx);
        free(root);
        return 0;
    }

    /* 1. Exact alias lookup */
    const tam_index_entry_t *entry = tam_index_lookup_alias(idx, query_buf);
    if (entry != NULL) {
        int rc = explain_entry(root, entry, level, pipe);
        tam_index_free(idx);
        free(root);
        return rc;
    }

    /* 2. Multi-word / fuzzy search fallback (if not containing command separator ;) */
    if (strchr(query_buf, ';') == NULL) {
        tam_search_result_t results[8];
        size_t found = tam_index_search(idx, query_buf, results, 8);
        if (found > 0 && results[0].score > 0) {
            int rc = explain_entry(root, results[0].entry, level, pipe);
            tam_index_free(idx);
            free(root);
            return rc;
        }
    }

    /* 3. Miss banner */
    tam_index_print_miss(stderr, idx, query_buf);
    tam_index_free(idx);
    free(root);
    return TAM_EXIT_FAILURE;
}

static int cmd_search(int n, char **args)
{
    int json_mode = 0;
    char query_buf[512] = {0};
    size_t qlen = 0;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            /* Piped plain text mode is the default when outputting text */
        } else if (strcmp(args[i], "--json") == 0) {
            json_mode = 1;
        } else if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0) {
            fputs(
                "tam search - search the Tamlinux offline guide\n\n"
                "Usage:\n"
                "  tam search TERM...         search the guide for topics or keywords\n"
                "  tam search --json TERM...  output search results as JSON\n"
                "  tam search --pipe TERM...  print search results as plain text\n",
                stdout);
            return 0;
        } else if (args[i][0] == '-') {
            return usage_error(args[i]);
        } else {
            if (qlen > 0 && qlen + 1 < sizeof(query_buf)) {
                query_buf[qlen++] = ' ';
            }
            size_t alen = strlen(args[i]);
            if (qlen + alen < sizeof(query_buf)) {
                memcpy(query_buf + qlen, args[i], alen);
                qlen += alen;
                query_buf[qlen] = '\0';
            }
        }
    }

    if (qlen == 0) {
        fputs(
            "tam search - search the Tamlinux offline guide\n\n"
            "Usage:\n"
            "  tam search TERM...         search the guide for topics or keywords\n"
            "  tam search --json TERM...  output search results as JSON\n"
            "  tam search --pipe TERM...  print search results as plain text\n",
            stdout);
        return 0;
    }

    char *root = find_knowledge_root();
    if (root == NULL) {
        fprintf(stderr,
                TAM_COMMAND_NAME ": no offline guide installed "
                "(set TAM_KNOWLEDGE_DIR to a knowledge directory)\n");
        return TAM_EXIT_FAILURE;
    }

    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        fprintf(stderr, TAM_COMMAND_NAME ": failed to load guide index\n");
        free(root);
        return TAM_EXIT_FAILURE;
    }

    tam_search_result_t results[32];
    size_t found = tam_index_search(idx, query_buf, results, 32);

    if (found == 0 || results[0].score <= 0) {
        tam_index_print_miss(stderr, idx, query_buf);
        tam_index_free(idx);
        free(root);
        return TAM_EXIT_FAILURE;
    }

    if (json_mode) {
        printf("[\n");
        for (size_t i = 0; i < found; i++) {
            const tam_index_entry_t *e = results[i].entry;
            char levels[64];
            tam_level_format(e->levels, "\", \"", levels, sizeof(levels));
            printf("  {\n");
            printf("    \"topic\": \"%s\",\n", e->topic);
            printf("    \"page\": \"%s\",\n", e->path);
            printf("    \"title\": \"%s\",\n", e->title);
            printf("    \"summary\": \"%s\",\n", e->summary);
            printf("    \"levels\": [\"%s\"],\n", levels);
            printf("    \"track\": \"%s\",\n", e->track);
            printf("    \"prereqs\": [");
            for (size_t k = 0; k < e->num_prereqs; k++) {
                printf("%s\"%s\"", k > 0 ? ", " : "", e->prereqs[k]);
            }
            printf("],\n");
            printf("    \"score\": %d\n", results[i].score);
            printf("  }%s\n", (i + 1 < found) ? "," : "");
        }
        printf("]\n");
    } else {
        printf("tam search - search the Tamlinux guide\n\n");
        printf("Matches for '%s':\n", query_buf);
        for (size_t i = 0; i < found; i++) {
            const tam_index_entry_t *e = results[i].entry;
            char levels[64];
            tam_level_format(e->levels, ", ", levels, sizeof(levels));
            printf("  %-12s %s (%s)\n", e->topic, e->title, e->path);
            printf("               %s\n", e->summary);
            printf("               Depth: %s; track: %s\n", levels, e->track);
        }
        printf("\nRun 'tam explain [--level LEVEL] <topic>' or 'tam show <page>' to view.\n");
    }

    tam_index_free(idx);
    free(root);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc <= 1) {
        print_intro();
        return 0;
    }

    const char *arg = argv[1];
    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0 ||
        strcmp(arg, "help") == 0) {
        if (argc > 2) {
            return too_many_arguments();
        }
        print_help();
        return 0;
    }
    if (strcmp(arg, "-V") == 0 || strcmp(arg, "--version") == 0) {
        if (argc > 2) {
            return too_many_arguments();
        }
        print_version();
        return 0;
    }
    if (strcmp(arg, "show") == 0) {
        return cmd_show(argc - 2, argv + 2);
    }
    if (strcmp(arg, "welcome") == 0) {
        return cmd_welcome(argc - 2, argv + 2);
    }
    if (strcmp(arg, "search") == 0) {
        return cmd_search(argc - 2, argv + 2);
    }
    if (strcmp(arg, "explain") == 0) {
        return cmd_explain(argc - 2, argv + 2);
    }
    if (strcmp(arg, "install") == 0) {
        return cmd_install(argc - 2, argv + 2);
    }
    if (strcmp(arg, "man") == 0) {
        return tam_cmd_man(argc - 2, argv + 2);
    }
    if (strcmp(arg, "ask") == 0) {
        return tam_cmd_ask(argc - 2, argv + 2);
    }
    return usage_error(arg);
}
