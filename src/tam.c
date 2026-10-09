#include <tam.h>

#include "install.h"
#include "pager.h"
#include "render.h"

#include <errno.h>
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
        "  tam                  print a short introduction and exit\n"
        "  tam show             open the offline guide in the terminal\n"
        "  tam show PAGE        open a guide page (e.g. welcome, man, shell/pipelines)\n"
        "  tam show --pipe      print the guide as plain text\n"
        "  tam welcome          open the welcome orientation\n"
        "  tam welcome --pipe   print the welcome orientation as plain text\n"
        "  tam explain          list available explanation topics\n"
        "  tam explain TOPIC    explain a curated topic or concept\n"
        "  tam explain TOPIC --pipe  print the topic explanation as plain text\n"
        "  tam install          show installation framework commands\n"
        "  tam install inspect  inspect host and installation capabilities\n"
        "  tam --help           show this help and exit (same as: tam help)\n"
        "  tam --version        show version information and exit\n\n"
        "This build implements orientation, explanation, the offline guide, and\n"
        "installation inspection.\n",
        stdout);
}

static void print_version(void)
{
    char *cultivar = cultivar_version();

    printf(TAM_COMMAND_NAME " command %s\n", TAM_COMMAND_VERSION);
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

static char *find_knowledge_root(void)
{
    const char *env = getenv("TAM_KNOWLEDGE_DIR");
    if (env != NULL && knowledge_ok(env)) {
        return strdup(env);
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

    const char *compiled = getenv("TAM_DATADIR");
    if (compiled == NULL || compiled[0] == '\0') {
        compiled = TAM_DATADIR;
    }
    if (knowledge_ok(compiled)) {
        return strdup(compiled);
    }
    return NULL;
}

static int knowledge_file_exists(const char *root, const char *rel)
{
    char *p = safe_join(root, rel);
    int ok = p != NULL && tam_fs_file_exists(p);
    free(p);
    return ok;
}

static void resolve_page(const char *root, const char *page, char *out,
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

typedef struct {
    char *root;
    int   width;
} corpus_ctx_t;

static tam_document_t *corpus_load(void *vctx, const char *href)
{
    corpus_ctx_t *ctx = vctx;
    char *path = safe_join(ctx->root, href);
    if (path == NULL) {
        return NULL;
    }
    size_t len = 0;
    char *src = tam_fs_read_file(path, &len);
    free(path);
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

static int view_knowledge_page(const char *root, const char *href, int pipe)
{
    char *path = safe_join(root, href);
    if (path == NULL || !tam_fs_file_exists(path)) {
        fprintf(stderr, TAM_COMMAND_NAME ": guide page not found: %s\n", href);
        free(path);
        return TAM_EXIT_FAILURE;
    }
    size_t len = 0;
    char *src = tam_fs_read_file(path, &len);
    free(path);
    if (src == NULL) {
        fprintf(stderr, TAM_COMMAND_NAME ": cannot read guide page: %s\n", href);
        return TAM_EXIT_FAILURE;
    }

    int interactive = !pipe && isatty(STDOUT_FILENO) && isatty(STDIN_FILENO);
    int width = interactive ? tam_terminal_width() : 80;

    tam_document_t *doc = tam_render_html(src, len, width);
    free(src);

    int rc = 0;
    if (interactive) {
        corpus_ctx_t ctx;
        ctx.root = (char *)root;
        ctx.width = width;
        rc = tam_pager_run(doc, href, corpus_load, &ctx);
    } else {
        char *text = tam_document_text(doc);
        fputs(text, stdout);
        free(text);
        tam_document_free(doc);
    }
    return rc;
}

static int cmd_show(int n, char **args)
{
    int pipe = 0;
    const char *page = NULL;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (args[i][0] == '-') {
            return usage_error(args[i]);
        } else if (page == NULL) {
            page = args[i];
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

typedef struct {
    const char *keyword;
    const char *target_page;
} topic_mapping_t;

static const topic_mapping_t TOPIC_MAP[] = {
    /* Welcome & orientation */
    { "welcome",        "welcome.html" },
    { "orientation",    "welcome.html" },

    /* Install & maintenance */
    { "install",        "install/index.html" },
    { "installation",   "install/index.html" },
    { "inspect",        "install/index.html" },
    { "inspection",     "install/index.html" },
    { "maintain",       "install/index.html" },
    { "maintaining",    "install/index.html" },
    { "maintenance",    "install/index.html" },
    { "update",         "install/index.html" },
    { "updates",        "install/index.html" },
    { "recovery",       "install/index.html" },
    { "layers",         "install/index.html" },

    /* Shell pipelines & redirection */
    { "shell",          "shell/pipelines.html" },
    { "pipeline",       "shell/pipelines.html" },
    { "pipelines",      "shell/pipelines.html" },
    { "pipe",           "shell/pipelines.html" },
    { "pipes",          "shell/pipelines.html" },
    { "redirection",    "shell/pipelines.html" },
    { "redirect",       "shell/pipelines.html" },
    { "|",              "shell/pipelines.html" },
    { ">",              "shell/pipelines.html" },
    { ">>",             "shell/pipelines.html" },
    { "2>&1",           "shell/pipelines.html" },
    { "<",              "shell/pipelines.html" },
    { "grep",           "shell/pipelines.html" },
    { "ps",             "shell/pipelines.html" },
    { "wc",             "shell/pipelines.html" },
    { "cat",            "shell/pipelines.html" },

    /* Man pages */
    { "man",            "man/index.html" },
    { "manual",         "man/index.html" },
    { "manuals",        "man/index.html" },
    { "apropos",        "man/index.html" },
    { "whatis",         "man/index.html" },

    /* Guide index */
    { "guide",          "index.html" },
    { "index",          "index.html" },
    { "show",           "index.html" },
    { "browse",         "index.html" },

    { NULL, NULL }
};

static const char *match_curated_topic(const char *topic)
{
    if (topic == NULL || topic[0] == '\0') {
        return NULL;
    }
    char buf[256];
    size_t len = strlen(topic);
    if (len >= sizeof(buf)) {
        return NULL;
    }
    memcpy(buf, topic, len + 1);

    /* Strip matching outer quotes ('...' or "...") */
    while (len >= 2 && ((buf[0] == '\'' && buf[len - 1] == '\'') ||
                        (buf[0] == '"' && buf[len - 1] == '"'))) {
        memmove(buf, buf + 1, len - 2);
        buf[len - 2] = '\0';
        len -= 2;
    }

    if (len == 0) {
        return NULL;
    }

    for (size_t i = 0; TOPIC_MAP[i].keyword != NULL; i++) {
        if (strcasecmp(buf, TOPIC_MAP[i].keyword) == 0) {
            return TOPIC_MAP[i].target_page;
        }
    }
    return NULL;
}

static void print_explain_topics(void)
{
    fputs(
        "tam explain - explain a curated topic or concept\n\n"
        "Usage:\n"
        "  tam explain TOPIC           explain a topic in the terminal\n"
        "  tam explain TOPIC --pipe    print topic explanation as plain text\n\n"
        "Available topics:\n"
        "  welcome      Tamlinux welcome and orientation\n"
        "               Keywords: welcome, orientation\n"
        "  install      System installation, inspection, and maintenance\n"
        "               Keywords: install, installation, inspect, inspection,\n"
        "                         layers, maintain, maintaining, maintenance,\n"
        "                         recovery, update, updates\n"
        "  pipelines    Shell pipelines, redirection, and core filters\n"
        "               Keywords: pipelines, pipeline, pipe, pipes, shell,\n"
        "                         redirection, redirect, |, >, >>, <, 2>&1,\n"
        "                         cat, grep, ps, wc\n"
        "  man          Manual pages and system documentation\n"
        "               Keywords: man, manual, manuals, apropos, whatis\n"
        "  guide        Offline guide overview and index\n"
        "               Keywords: guide, index, show, browse\n\n"
        "Run 'tam explain <topic>' to view an explanation.\n"
        "Run 'tam show' to open the offline guide.\n",
        stdout);
}

static int cmd_explain(int n, char **args)
{
    int pipe = 0;
    const char *topic = NULL;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0) {
            print_explain_topics();
            return 0;
        } else if (args[i][0] == '-') {
            return usage_error(args[i]);
        } else if (topic == NULL) {
            topic = args[i];
        } else {
            return too_many_arguments();
        }
    }

    if (topic == NULL || strcmp(topic, "help") == 0) {
        print_explain_topics();
        return 0;
    }

    const char *target = match_curated_topic(topic);
    if (target == NULL) {
        fprintf(stderr,
                TAM_COMMAND_NAME ": no explanation found for '%s'\n"
                "Run '%s explain' to see available topics, or '%s show' to explore the offline guide.\n",
                topic, TAM_COMMAND_NAME, TAM_COMMAND_NAME);
        return TAM_EXIT_FAILURE;
    }

    char *root = find_knowledge_root();
    if (root == NULL) {
        fprintf(stderr,
                TAM_COMMAND_NAME ": no offline guide installed "
                "(set TAM_KNOWLEDGE_DIR to a knowledge directory)\n");
        return TAM_EXIT_FAILURE;
    }

    int rc = view_knowledge_page(root, target, pipe);
    free(root);
    return rc;
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
    if (strcmp(arg, "explain") == 0) {
        return cmd_explain(argc - 2, argv + 2);
    }
    if (strcmp(arg, "install") == 0) {
        return cmd_install(argc - 2, argv + 2);
    }
    return usage_error(arg);
}
