#include <tam.h>

#include "ask.h"
#include "depth.h"
#include "index.h"
#include "info.h"
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
#include <ctype.h>

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

char *tam_corpus_version(const char *root)
{
    if (root == NULL || root[0] == '\0') {
        return NULL;
    }
    char *path = join_path(root, "/version");
    if (path == NULL) {
        return NULL;
    }
    char *buf = tam_fs_read_file(path, NULL);
    free(path);
    if (buf != NULL) {
        trim_in_place(buf);
        if (buf[0] != '\0') {
            return buf;
        }
        free(buf);
    }
    return strdup(TAM_DEFAULT_CORPUS_VERSION);
}

char *tam_corpus_build_id(const char *root)
{
    if (root == NULL || root[0] == '\0') {
        return NULL;
    }
    char *path = join_path(root, "/.build_id");
    if (path == NULL) {
        return NULL;
    }
    char *buf = tam_fs_read_file(path, NULL);
    free(path);
    if (buf != NULL) {
        trim_in_place(buf);
        if (buf[0] != '\0') {
            return buf;
        }
        free(buf);
    }
    return NULL;
}

enum {
    HELP_STANDARD = 0,
    HELP_TERSE,
    HELP_VERBOSE
};

static void print_tam_help(int tier)
{
    char dev_buf[64] = "";
    if (TAM_BUILD[0] != '\0') {
        snprintf(dev_buf, sizeof(dev_buf), " (dev %s)", TAM_BUILD);
    }

    char *k_root = tam_find_knowledge_root();
    char *k_ver = tam_corpus_version(k_root);
    char *k_build = tam_corpus_build_id(k_root);
    char k_buf[128] = "";
    if (k_ver != NULL) {
        if (k_build != NULL && k_build[0] != '\0') {
            snprintf(k_buf, sizeof(k_buf), " showing corpus version %s (dev %s)", k_ver, k_build);
        } else {
            snprintf(k_buf, sizeof(k_buf), " showing corpus version %s", k_ver);
        }
    } else {
        snprintf(k_buf, sizeof(k_buf), " (corpus unavailable)");
    }

    if (tier == HELP_TERSE) {
        fputs(
            "tam - the Tamlinux command\n\n"
            "Usage: tam <subcommand> [--options] [arguments]\n\n"
            "Essential Subcommands:\n"
            "  tam welcome                   first-run orientation\n"
            "  tam show [PAGE]               open guide or specific page\n"
            "  tam info [MANUAL]             browse GNU Info manuals\n"
            "  tam inspect                   inspect system health\n\n"
            "Essential Options:\n"
            "  --help, -h | --pipe | --terse | --verbose | --version, -V\n",
            stdout);
        free(k_ver);
        free(k_build);
        free(k_root);
        return;
    }

    if (tier == HELP_VERBOSE) {
        printf(
            "tam - the Tamlinux command\n\n"
            "Understand, use, maintain, and improve your Tamlinux system.\n\n"
            "This is tam version %s%s%s\n\n"
            "Usage:\n"
            "  tam <subcommand> [--options] [arguments]\n\n"
            "Guide and knowledge:\n"
            "  tam welcome [--pipe]\n"
            "      first-run orientation and welcome guide (aliases: hello, hi)\n"
            "  tam show [PAGE|-] [--pipe]\n"
            "      open the offline guide, a specific page, or render HTML from stdin\n"
            "  tam search [--json] TERM...\n"
            "      search guide topics and keywords with ranked scoring\n"
            "  tam explain [TOPIC] [--level LEVEL] [--pipe]\n"
            "      explain a concept or browse the catalog at beginner, intermediate, or expert depth\n"
            "  tam explain topics [--pipe] [--terse|--verbose] [--level LEVEL]\n"
            "      interactive or plain-text catalog of all available topics\n"
            "  tam man [SECTION] PAGE [--pipe]\n"
            "      hyperlinked manual page viewer with interactive cross-reference navigation\n"
            "  tam info [MANUAL [NODE]] [--pipe]\n"
            "      browse GNU Info manuals with hyperlinked menus and cross-references\n"
            "  tam ask [--json] [--model MODEL] [--server URL] QUESTION\n"
            "      query a local AI model for answers from the guide to any question\n\n"
            "System subcommands:\n"
            "  tam inspect [--json|--check [ID]] [--terse]\n"
            "      inspect hardware, kernel, DRM, storage mounts, and privilege escalation\n"
            "  tam check [ID] [--json|--terse]\n"
            "      evaluate workstation health findings against the offline Check Contract\n\n"
            "Options:\n"
            "  --help, -h\n"
            "      display help overview (combine with --verbose for this exhaustive view)\n"
            "  --pipe\n"
            "      emit clean plain text without ANSI escape sequences or interactive pager\n"
            "  --terse\n"
            "      emit compact single-line or summary output\n"
            "  --verbose\n"
            "      show all subcommands, arguments, secondary verbs, and options\n"
            "  --version, -V\n"
            "      display command, libtam, and Tamlinux cultivar metadata\n\n"
            "Environment:\n"
            "  TAM_KNOWLEDGE_DIR         override path to offline HTML guide corpus\n"
            "  TAM_ASK_MODEL             default local LLM name (default: qwen2.5-coder:1.5b)\n"
            "  TAM_ASK_SERVER            default local inference URL (default: http://127.0.0.1:11434)\n\n"
            "Run 'tam <subcommand> --help' for options specific to any subcommand.\n",
            TAM_COMMAND_VERSION, dev_buf, k_buf);
        free(k_ver);
        free(k_build);
        free(k_root);
        return;
    }

    /* HELP_STANDARD */
    printf(
        "tam - the Tamlinux command\n\n"
        "Understand, use, maintain, and improve your Tamlinux system.\n\n"
        "This is tam version %s%s%s\n\n"
        "Usage:\n"
        "  tam <subcommand> [--options] [arguments]\n\n"
        "  tam welcome               first-run orientation\n\n"
        "Guide subcommands to try:\n"
        "  tam show [PAGE]           open the guide or a specific page\n"
        "  tam search KEYWORD...     search guide deterministically\n"
        "  tam explain [TOPIC]       browse all topics or show a topic\n"
        "  tam man [SECTION] PAGE    view hyperlinked manual page\n"
        "  tam info [MANUAL [NODE]]  browse GNU Info manuals\n"
        "  tam ask QUESTION          ask a local AI model (if available)\n\n"
        "System subcommands to try:\n"
        "  tam inspect               inspect system components\n"
        "  tam check                 check system health\n\n"
        "Options to know:\n"
        "  --help, -h                show this help overview\n"
        "  --pipe                    output as plain text for piping\n"
        "  --terse                   shorter output\n"
        "  --verbose                 longer output\n"
        "  --version, -V             show version info\n\n"
        "Run 'tam --verbose' for a complete list of commands and more detail.\n",
        TAM_COMMAND_VERSION, dev_buf, k_buf);
    free(k_ver);
    free(k_build);
    free(k_root);
}

static void print_version(void)
{
    char *cultivar = cultivar_version();
    char *k_root = tam_find_knowledge_root();
    char *k_ver = tam_corpus_version(k_root);
    char *k_build = tam_corpus_build_id(k_root);

    if (TAM_BUILD[0] != '\0') {
        printf(TAM_COMMAND_NAME " command %s (dev %s)\n", TAM_COMMAND_VERSION,
               TAM_BUILD);
    } else {
        printf(TAM_COMMAND_NAME " command %s\n", TAM_COMMAND_VERSION);
    }
    if (k_ver != NULL) {
        if (k_build != NULL && k_build[0] != '\0') {
            printf("knowledge corpus %s (dev %s)\n", k_ver, k_build);
        } else {
            printf("knowledge corpus %s\n", k_ver);
        }
    } else {
        printf("knowledge corpus: unavailable (no knowledge corpus found)\n");
    }
    printf("libtam %s\n", tam_version());
    if (cultivar != NULL) {
        printf("Tamlinux cultivar %s\n", cultivar);
        free(cultivar);
    } else {
        printf("Tamlinux cultivar: unavailable (no installed cultivar metadata)\n");
    }
    free(k_ver);
    free(k_build);
    free(k_root);
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
    tam_index_t *idx = tam_index_load(root);
    if (idx != NULL) {
        const tam_index_entry_t *entry = tam_index_lookup_alias(idx, page);
        if (entry != NULL && entry->path != NULL) {
            snprintf(out, cap, "%s", entry->path);
            tam_index_free(idx);
            return;
        }
        tam_index_free(idx);
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

static void url_decode_in_place(char *s)
{
    if (s == NULL) return;
    char *o = s;
    for (char *p = s; *p != '\0'; p++) {
        if (*p == '%' && p[1] != '\0' && p[2] != '\0') {
            int h1 = p[1];
            int h2 = p[2];
            int v1 = (h1 >= '0' && h1 <= '9') ? (h1 - '0') :
                     (h1 >= 'a' && h1 <= 'f') ? (h1 - 'a' + 10) :
                     (h1 >= 'A' && h1 <= 'F') ? (h1 - 'A' + 10) : -1;
            int v2 = (h2 >= '0' && h2 <= '9') ? (h2 - '0') :
                     (h2 >= 'a' && h2 <= 'f') ? (h2 - 'a' + 10) :
                     (h2 >= 'A' && h2 <= 'F') ? (h2 - 'A' + 10) : -1;
            if (v1 >= 0 && v2 >= 0) {
                *o++ = (char)((v1 << 4) | v2);
                p += 2;
                continue;
            }
        } else if (*p == '+') {
            *o++ = ' ';
            continue;
        }
        *o++ = *p;
    }
    *o = '\0';
}

static char *url_encode(const char *s)
{
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    char *out = malloc(len * 3 + 1);
    if (out == NULL) return NULL;
    char *o = out;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            *o++ = (char)c;
        } else {
            sprintf(o, "%%%02X", c);
            o += 3;
        }
    }
    *o = '\0';
    return out;
}

static char *html_escape(const char *s)
{
    if (s == NULL) return strdup("");
    size_t len = strlen(s);
    char *out = malloc(len * 6 + 1);
    if (out == NULL) return strdup("");
    char *o = out;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (c == '&') {
            memcpy(o, "&amp;", 5); o += 5;
        } else if (c == '<') {
            memcpy(o, "&lt;", 4); o += 4;
        } else if (c == '>') {
            memcpy(o, "&gt;", 4); o += 4;
        } else if (c == '"') {
            memcpy(o, "&quot;", 6); o += 6;
        } else {
            *o++ = c;
        }
    }
    *o = '\0';
    return out;
}

tam_document_t *tam_topic_guide_document(const char *root, const char *topic, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }
    const tam_index_entry_t *matches[64];
    size_t num_matches = tam_index_find_topic_pages(idx, topic, level, matches, 64);
    if (num_matches == 0) {
        tam_index_free(idx);
        return NULL;
    }

    char *esc_topic = html_escape(topic);
    tam_str_t html = tam_str_new("<!DOCTYPE html>\n<html>\n<head>\n");
    html = tam_str_cat_printf(html, "<title>Topic Guide: %s</title>\n</head>\n<body>\n", esc_topic);
    html = tam_str_cat(html, "<nav aria-label=\"Breadcrumb\">\n<p><a href=\"index.html\">Guide</a> / <a href=\"tam:topics\">Topics</a> / Topic Guide</p>\n</nav>\n");
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Topic Guide: %s (%s Level)</h1>\n", esc_topic, tam_level_name(level));
    } else {
        html = tam_str_cat_printf(html, "<h1>Topic Guide: %s</h1>\n", esc_topic);
    }
    html = tam_str_cat_printf(html,
        "<p>Topic guide to pages in the Tamlinux guide covering <strong>%s</strong>.<br>\n"
        "Select a page with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to view it.<br>\n"
        "Press <kbd>u</kbd> to go back, <kbd>`</kbd> to return to the guide index, or <kbd>q</kbd> to exit.</p>\n"
        "<dl>\n",
        esc_topic);
    free(esc_topic);

    for (size_t i = 0; i < num_matches; i++) {
        const tam_index_entry_t *e = matches[i];
        char *esc_title = html_escape(e->title);
        char *esc_summary = html_escape(e->summary);
        char track_cap[32] = {0};
        if (e->track != NULL && e->track[0] != '\0') {
            snprintf(track_cap, sizeof(track_cap), "%s", e->track);
            track_cap[0] = toupper((unsigned char)track_cap[0]);
        } else {
            snprintf(track_cap, sizeof(track_cap), "Use");
        }
        char levels_buf[128] = {0};
        if (e->levels != 0) {
            char lvl_names[64];
            tam_level_format(e->levels, ", ", lvl_names, sizeof(lvl_names));
            snprintf(levels_buf, sizeof(levels_buf), " | Depth: %s", lvl_names);
        }

        if (level != 0) {
            html = tam_str_cat_printf(html,
                "<dt><a href=\"%s?level=%s\">%s</a> [%s%s]</dt>\n<dd>%s</dd>\n",
                e->path, tam_level_name(level), esc_title, track_cap, levels_buf, esc_summary);
        } else {
            html = tam_str_cat_printf(html,
                "<dt><a href=\"%s\">%s</a> [%s%s]</dt>\n<dd>%s</dd>\n",
                e->path, esc_title, track_cap, levels_buf, esc_summary);
        }
        free(esc_title);
        free(esc_summary);
    }
    html = tam_str_cat(html, "</dl>\n</body>\n</html>\n");
    tam_index_free(idx);

    tam_document_t *doc = tam_render_html(html, strlen(html), width);
    tam_str_free(html);
    return doc;
}

tam_document_t *tam_topic_keywords_document(const char *root, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }
    tam_keyword_list_t *list = tam_index_get_keywords(idx, level);
    if (list == NULL) {
        tam_index_free(idx);
        return NULL;
    }

    tam_str_t html = tam_str_new("<!DOCTYPE html>\n<html>\n<head>\n");
    html = tam_str_cat(html, "<title>Tamlinux Topic Keywords</title>\n</head>\n<body>\n");
    html = tam_str_cat(html, "<nav aria-label=\"Breadcrumb\">\n<p><a href=\"index.html\">Guide</a> / Topic Keywords</p>\n</nav>\n");
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Topics with a %s level:</h1>\n", tam_level_name(level));
    } else {
        html = tam_str_cat(html, "<h1>Available topics: Topic Keywords</h1>\n");
    }
    html = tam_str_cat(html,
        "<p>Available topics: select a topic keyword with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to view its topic guide.<br>\n"
        "Press <kbd>`</kbd> to return to the guide index, or <kbd>q</kbd> to exit.</p>\n"
        "<table>\n"
        "<tr><th>Topic Keyword</th><th>Pages</th><th>Summary / Coverage</th></tr>\n");

    for (size_t i = 0; i < list->count; i++) {
        char *enc_kw = url_encode(list->items[i].keyword);
        char *esc_kw = html_escape(list->items[i].keyword);
        char *esc_sum = html_escape(list->items[i].primary_summary);
        size_t cnt = list->items[i].count;

        if (level != 0) {
            html = tam_str_cat_printf(html,
                "<tr><td><a href=\"tam:explain?topic=%s&amp;level=%s\">%s</a></td><td>%zu page%s</td><td>%s</td></tr>\n",
                enc_kw ? enc_kw : "", tam_level_name(level), esc_kw, cnt, (cnt == 1 ? "" : "s"), esc_sum);
        } else {
            html = tam_str_cat_printf(html,
                "<tr><td><a href=\"tam:explain?topic=%s\">%s</a></td><td>%zu page%s</td><td>%s</td></tr>\n",
                enc_kw ? enc_kw : "", esc_kw, cnt, (cnt == 1 ? "" : "s"), esc_sum);
        }
        free(enc_kw);
        free(esc_kw);
        free(esc_sum);
    }
    html = tam_str_cat(html, "</table>\n</body>\n</html>\n");
    tam_keyword_list_free(list);
    tam_index_free(idx);

    tam_document_t *doc = tam_render_html(html, strlen(html), width);
    tam_str_free(html);
    return doc;
}

tam_document_t *tam_topics_document(const char *root, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }

    tam_str_t html = tam_str_new("<!DOCTYPE html>\n<html>\n<head>\n");
    html = tam_str_cat(html, "<title>Available Explanation Topics</title>\n</head>\n<body>\n");
    html = tam_str_cat(html, "<nav aria-label=\"Breadcrumb\">\n<p><a href=\"index.html\">Guide</a> / Topics</p>\n</nav>\n");
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Topics with a %s level</h1>\n",
                                  tam_level_name(level));
    } else {
        html = tam_str_cat(html, "<h1>Available Explanation Topics</h1>\n");
    }
    html = tam_str_cat(html,
        "<p>Available topics: select a topic with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to view its explanation.<br>\n"
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
    if (strncmp(href, "info:", 5) == 0) {
        return tam_info_load(href + 5, ctx->width);
    }
    if (strncmp(href, "tam:explain", 11) == 0) {
        unsigned level = 0;
        const char *lq = strstr(href, "level=");
        if (lq != NULL) {
            level = tam_level_parse(lq + 6);
        }
        const char *tq = strstr(href, "topic=");
        char topic[256] = {0};
        if (tq != NULL) {
            tq += 6;
            const char *end = strchr(tq, '&');
            size_t tlen = end ? (size_t)(end - tq) : strlen(tq);
            if (tlen >= sizeof(topic)) tlen = sizeof(topic) - 1;
            memcpy(topic, tq, tlen);
            topic[tlen] = '\0';
            url_decode_in_place(topic);
        } else if (href[11] == ':') {
            snprintf(topic, sizeof(topic), "%s", href + 12);
            url_decode_in_place(topic);
        }
        if (topic[0] != '\0') {
            return tam_topic_guide_document(ctx->root, topic, level, ctx->width);
        }
        return tam_topic_keywords_document(ctx->root, level, ctx->width);
    }
    if (strncmp(href, "tam:keywords", 12) == 0) {
        unsigned level = 0;
        const char *q = strstr(href, "?level=");
        if (q != NULL) {
            level = tam_level_parse(q + 7);
        }
        return tam_topic_keywords_document(ctx->root, level, ctx->width);
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

static int cmd_show(int n, char **args)
{
    int pipe = 0;
    const char *page = NULL;
    unsigned level = 0;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
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
        if (level == 0) {
            const char *q = strstr(page, "?level=");
            if (q != NULL) {
                level = tam_level_parse(q + 7);
            }
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
            char initial_href[64];
            if (level != 0) {
                snprintf(initial_href, sizeof(initial_href), "tam:topics?level=%s", tam_level_name(level));
            } else {
                snprintf(initial_href, sizeof(initial_href), "tam:topics");
            }
            rc = tam_pager_run(doc, initial_href, corpus_load, &ctx);
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

    if (page != NULL && strncmp(page, "info:", 5) == 0) {
        int interactive = !pipe && isatty(STDOUT_FILENO);
        int width = interactive ? tam_terminal_width() : 80;
        tam_document_t *doc = tam_info_load(page + 5, width);
        if (doc == NULL) {
            fprintf(stderr, TAM_COMMAND_NAME ": no info manual or node found for '%s'\n", page + 5);
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
        if (level != 0) {
            snprintf(href, sizeof(href), "index.html?level=%s", tam_level_name(level));
        } else {
            snprintf(href, sizeof(href), "%s", "index.html");
        }
    } else {
        resolve_page(root, page, href, sizeof(href));
        if (level != 0 && strchr(href, '?') == NULL) {
            size_t hlen = strlen(href);
            snprintf(href + hlen, sizeof(href) - hlen, "?level=%s", tam_level_name(level));
        }
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

static void print_explain_usage(void)
{
    fputs(
        "tam explain - display a topic guide to concepts in the Tamlinux guide\n\n"
        "Usage:\n"
        "  tam explain                 browse all topic keywords in the pager\n"
        "  tam explain TOPIC           display a topic guide for TOPIC in the pager\n"
        "  tam explain TOPIC --pipe    print topic guide as plain text\n"
        "  tam explain --level LEVEL [TOPIC]  filter by depth (beginner/intermediate/expert)\n"
        "  tam explain topics          interactive list of topics with live hyperlinks\n"
        "  tam explain topics --pipe   print available topics as plain text\n"
        "  tam explain -               read topic keyword from standard input\n\n"
        "Run 'tam explain' to browse all available topic keywords.\n"
        "Run 'tam explain <topic>' to view a specific topic guide.\n"
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

    if (strcmp(query_buf, "help") == 0) {
        print_explain_usage();
        return 0;
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

    int interactive = !pipe && !terse && !verbose && isatty(STDOUT_FILENO);
    if (interactive && !isatty(STDIN_FILENO)) {
        int tty = open("/dev/tty", O_RDONLY);
        if (tty < 0) {
            interactive = 0;
        } else {
            close(tty);
        }
    }
    int width = interactive ? tam_terminal_width() : 80;

    /* Case A1: tam explain topics -> display topics document */
    if (strcmp(query_buf, "topics") == 0) {
        if (!interactive) {
            tam_index_print_catalog_ex(stdout, idx, level, terse != 0, verbose != 0);
            tam_index_free(idx);
            free(root);
            return 0;
        }

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

    /* Case A2: Bare tam explain -> display topic keywords list */
    if (qlen == 0) {
        if (terse || verbose) {
            tam_index_print_catalog_ex(stdout, idx, level, terse != 0, verbose != 0);
            tam_index_free(idx);
            free(root);
            return 0;
        }

        tam_document_t *doc = tam_topic_keywords_document(root, level, width);
        if (doc == NULL) {
            tam_index_free(idx);
            free(root);
            return TAM_EXIT_FAILURE;
        }

        int rc = 0;
        if (interactive) {
            corpus_ctx_t ctx;
            ctx.root = root;
            ctx.width = width;
            char initial_href[64];
            if (level != 0) {
                snprintf(initial_href, sizeof(initial_href), "tam:keywords?level=%s", tam_level_name(level));
            } else {
                snprintf(initial_href, sizeof(initial_href), "tam:keywords");
            }
            rc = tam_pager_run(doc, initial_href, corpus_load, &ctx);
        } else {
            char *text = tam_document_text(doc);
            if (text != NULL) {
                fputs(text, stdout);
                free(text);
            }
            tam_document_free(doc);
        }
        tam_index_free(idx);
        free(root);
        return rc;
    }

    /* Case B: Topic specified -> always display topic guide in pager (or text if pipe) */
    tam_document_t *doc = tam_topic_guide_document(root, query_buf, level, width);
    if (doc == NULL) {
        if (level != 0) {
            const tam_index_entry_t *unfiltered[16];
            size_t any_found = tam_index_find_topic_pages(idx, query_buf, 0, unfiltered, 16);
            if (any_found > 0) {
                char offered[64];
                tam_level_format(unfiltered[0]->levels, ", ", offered, sizeof(offered));
                fprintf(stderr, TAM_COMMAND_NAME ": topic '%s' has no %s level (available: %s)\n",
                        query_buf, tam_level_name(level), offered);
                fprintf(stderr, "Run 'tam explain %s' to view it.\n", query_buf);
                tam_index_free(idx);
                free(root);
                return TAM_EXIT_FAILURE;
            }
        }
        tam_index_print_miss(stderr, idx, query_buf);
        tam_index_free(idx);
        free(root);
        return TAM_EXIT_FAILURE;
    }

    int rc = 0;
    if (interactive) {
        corpus_ctx_t ctx;
        ctx.root = root;
        ctx.width = width;
        char initial_href[512];
        char *enc_q = url_encode(query_buf);
        if (level != 0) {
            snprintf(initial_href, sizeof(initial_href), "tam:explain?topic=%s&level=%s", enc_q ? enc_q : "", tam_level_name(level));
        } else {
            snprintf(initial_href, sizeof(initial_href), "tam:explain?topic=%s", enc_q ? enc_q : "");
        }
        free(enc_q);
        rc = tam_pager_run(doc, initial_href, corpus_load, &ctx);
    } else {
        char *text = tam_document_text(doc);
        if (text != NULL) {
            fputs(text, stdout);
            free(text);
        }
        tam_document_free(doc);
    }
    tam_index_free(idx);
    free(root);
    return rc;
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
        print_tam_help(HELP_STANDARD);
        return 0;
    }

    const char *arg = argv[1];

    if (strcmp(arg, "--terse") == 0) {
        if (argc == 2) {
            print_tam_help(HELP_TERSE);
            return 0;
        }
        if (argc == 3 && (strcmp(argv[2], "--help") == 0 ||
                          strcmp(argv[2], "-h") == 0 ||
                          strcmp(argv[2], "help") == 0)) {
            print_tam_help(HELP_TERSE);
            return 0;
        }
        return too_many_arguments();
    }

    if (strcmp(arg, "--verbose") == 0) {
        if (argc == 2) {
            print_tam_help(HELP_VERBOSE);
            return 0;
        }
        if (argc == 3 && (strcmp(argv[2], "--help") == 0 ||
                          strcmp(argv[2], "-h") == 0 ||
                          strcmp(argv[2], "help") == 0)) {
            print_tam_help(HELP_VERBOSE);
            return 0;
        }
        return too_many_arguments();
    }

    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0 ||
        strcmp(arg, "help") == 0) {
        if (argc == 2) {
            print_tam_help(HELP_STANDARD);
            return 0;
        }
        if (argc == 3) {
            if (strcmp(argv[2], "--terse") == 0) {
                print_tam_help(HELP_TERSE);
                return 0;
            }
            if (strcmp(argv[2], "--verbose") == 0) {
                print_tam_help(HELP_VERBOSE);
                return 0;
            }
        }
        return too_many_arguments();
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
    if (strcmp(arg, "welcome") == 0 || strcmp(arg, "hello") == 0 ||
        strcmp(arg, "hi") == 0) {
        return cmd_welcome(argc - 2, argv + 2);
    }
    if (strcmp(arg, "search") == 0) {
        return cmd_search(argc - 2, argv + 2);
    }
    if (strcmp(arg, "explain") == 0) {
        return cmd_explain(argc - 2, argv + 2);
    }
    if (strcmp(arg, "inspect") == 0) {
        return cmd_inspect(argc - 2, argv + 2);
    }
    if (strcmp(arg, "check") == 0) {
        return cmd_check(argc - 2, argv + 2);
    }
    if (strcmp(arg, "install") == 0) {
        return cmd_install(argc - 2, argv + 2);
    }
    if (strcmp(arg, "man") == 0) {
        return tam_cmd_man(argc - 2, argv + 2);
    }
    if (strcmp(arg, "info") == 0) {
        return tam_cmd_info(argc - 2, argv + 2);
    }
    if (strcmp(arg, "ask") == 0) {
        return tam_cmd_ask(argc - 2, argv + 2);
    }
    return usage_error(arg);
}
