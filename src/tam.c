#include <tam.h>

#include "ask.h"
#include "depth.h"
#include "fold.h"
#include "address.h"
#include "html.h"
#include "discovery.h"
#include "index.h"
#include "info.h"
#include "upgrade.h"
#include "install.h"
#include "man.h"
#include "pager.h"
#include "render.h"
#include "tldr.h"
#include "views.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <locale.h>
#include <pwd.h>

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

/* Tamlinux is deployed per user (D181): its version and cultivar name sit in
 * that user's ~/.config/tamlinux. Under pkexec the user is the one who ran
 * pkexec (PKEXEC_UID), not root. */
static const struct passwd *cultivar_passwd(void)
{
    const char *pk = getenv("PKEXEC_UID");
    if (geteuid() == 0 && pk != NULL && pk[0] != '\0') {
        char *end = NULL;
        unsigned long uid = strtoul(pk, &end, 10);
        if (end != pk && *end == '\0') {
            const struct passwd *pw = getpwuid((uid_t)uid);
            if (pw != NULL) {
                return pw;
            }
        }
    }
    return getpwuid(geteuid());
}

bool cultivar_for_root(void)
{
    const char *pk = getenv("PKEXEC_UID");
    return geteuid() == 0 && (pk == NULL || pk[0] == '\0');
}

char *cultivar_user(void)
{
    const struct passwd *pw = cultivar_passwd();
    return strdup(pw != NULL && pw->pw_name != NULL ? pw->pw_name : "this user");
}

char *cultivar_path(const char *leaf)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    char *config = NULL;

    if (xdg != NULL && xdg[0] != '\0' && getenv("PKEXEC_UID") == NULL) {
        config = strdup(xdg);
    } else {
        const struct passwd *pw = cultivar_passwd();
        const char *home = getenv("HOME");
        if (getenv("PKEXEC_UID") != NULL && pw != NULL && pw->pw_dir != NULL) {
            home = pw->pw_dir;
        }
        if (home != NULL && home[0] != '\0') {
            config = tam_path_join(home, "/.config");
        }
    }
    if (config == NULL) {
        return NULL;
    }
    char *dir = tam_path_join(config, "/tamlinux/");
    free(config);
    if (dir == NULL) {
        return NULL;
    }
    char *path = tam_path_join(dir, leaf);
    free(dir);
    return path;
}

static char *cultivar_file(const char *leaf)
{
    char *path = cultivar_path(leaf);
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

/* The version file wins over $TAMLINUX_VERSION: activation rewrites the file,
 * while a long-running process keeps the value it started with. */
char *cultivar_version(void)
{
    char *from_file = cultivar_file("version");
    if (from_file != NULL) {
        return from_file;
    }

    const char *env = getenv("TAMLINUX_VERSION");
    if (env != NULL && env[0] != '\0') {
        return strdup(env);
    }
    return NULL;
}

char *cultivar_name(void)
{
    return cultivar_file("cultivar");
}

/* "Incipiens 0.4.5", or the bare version when no name is installed. */
char *cultivar_label(void)
{
    char *version = cultivar_version();
    if (version == NULL) {
        return NULL;
    }
    char *name = cultivar_name();
    if (name == NULL) {
        return version;
    }
    size_t len = strlen(name) + 1 + strlen(version) + 1;
    char *label = malloc(len);
    if (label != NULL) snprintf(label, len, "%s %s", name, version);
    free(name);
    free(version);
    return label;
}

char *tam_corpus_version(const char *root)
{
    if (root == NULL || root[0] == '\0') {
        return NULL;
    }
    char *path = tam_path_join(root, "/version");
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
    char *path = tam_path_join(root, "/.build_id");
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
            "Usage: tam <subcommand> [arguments] [--options]\n\n"
            "Essential Subcommands:\n"
            "  tam welcome                   first-run orientation\n"
            "  tam show [PAGE]               open guide or specific page\n"
            "  tam inspect                   inspect system components\n\n"
            "  tam upgrade                   check and install available versions\n"
            "  tam switch STAGE              select a source stage or roll back\n\n"
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
            "  tam <subcommand> [arguments] [--options]\n\n"
            "Guide and knowledge:\n"
            "  tam welcome [--pipe]\n"
            "      first-run orientation and welcome guide (aliases: hello, hi)\n"
            "  tam show [PAGE|-] [--pipe]\n"
            "      open the offline guide, a specific page, or render HTML from stdin\n"
            "  tam search [--json|--pipe] TERM...\n"
            "      search guide topics and keywords; results open in the pager\n"
            "  tam explain [TOPIC] [--level LEVEL] [--pipe]\n"
            "      explain a concept or browse the catalog at beginner, intermediate, or expert level\n"
            "  tam explain topics [--pipe] [--terse|--verbose] [--level LEVEL]\n"
            "      interactive or plain-text catalog of all available topics\n"
            "  tam man [SECTION] PAGE [--pipe]\n"
            "      hyperlinked manual page viewer with interactive cross-reference navigation\n"
            "  tam info [MANUAL [NODE]] [--pipe]\n"
            "      browse GNU Info manuals with hyperlinked menus and cross-references\n"
            "  tam tldr [COMMAND...] [--platform OS] [--pipe]\n"
            "      browse or view concise tldr command-example pages with hyperlinked references\n"
            "  tam ask [--json] [--model MODEL] [--server URL] QUESTION\n"
            "      query a local AI model for answers from the guide to any question\n\n"
            "  tam upgrade [--check|--apply] [--channel STAGE] [--json]\n"
            "  tam switch STAGE [--version X.Y.Z --corpus-version A.B.C]\n"
            "      discover, build and switch pinned tam source generations\n\n"
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
            "  TAM_TLDR_DIR              override path to offline curated tldr directory\n"
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
        "  tam <subcommand> [arguments] [--options]\n\n"
        "  tam welcome               first-run orientation\n\n"
        "Guide subcommands to try:\n"
        "  tam show [PAGE]           open the guide or a specific page\n"
        "  tam explain [TOPIC]       browse all topics or show a topic\n"
        "  tam man [SECTION] PAGE    view hyperlinked manual page\n"
        "  tam info [MANUAL [NODE]]  browse GNU Info manuals\n"
        "  tam tldr [COMMAND]        browse or view command example pages\n"
        "  tam ask QUESTION          ask a local AI model (if available)\n\n"
        "System subcommands to try:\n"
        "  tam inspect               inspect system components\n"
        "  tam check                 check system health\n\n"
        "  tam upgrade / switch      check, build, or recover source stages\n\n"
        "Options to know:\n"
        "  --help, -h                show this help overview\n"
        "  --pipe                    output as plain text for piping\n"
        "  --version, -V             show version info\n\n"
        "Run 'tam --verbose' for a complete list of commands and more detail.\n",
        TAM_COMMAND_VERSION, dev_buf, k_buf);
    free(k_ver);
    free(k_build);
    free(k_root);
}

static void print_version(void)
{
    char *cultivar = cultivar_label();
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

/* A corpus-relative page path under root; never a URL or a path that
 * leaves the corpus. */
static char *corpus_join(const char *root, const char *href)
{
    if (href == NULL || strstr(href, "://") != NULL) {
        return NULL;
    }
    return tam_path_join_under(root, href);
}

static int knowledge_ok(const char *dir)
{
    if (dir == NULL || dir[0] == '\0') {
        return 0;
    }
    char *p = tam_path_join(dir, "/index.html");
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
        char *p = tam_path_join(xdg, "/tamlinux/knowledge");
        if (knowledge_ok(p)) {
            return p;
        }
        free(p);
    }

    const char *home = getenv("HOME");
    if (home != NULL && home[0] != '\0') {
        char *p = tam_path_join(home, "/.local/share/tamlinux/knowledge");
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
                char *p = tam_path_join(base, "/tamlinux/knowledge");
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
    char *p = corpus_join(root, rel);
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

/* Resolve a page name, keeping any "?level=NAME" level selection. */
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

static int shared_prereqs(const tam_index_entry_t *a, const tam_index_entry_t *b)
{
    int n = 0;
    for (size_t i = 0; i < a->num_prereqs; i++) {
        for (size_t j = 0; j < b->num_prereqs; j++) {
            if (strcmp(a->prereqs[i], b->prereqs[j]) == 0) {
                n++;
            }
        }
    }
    return n;
}


/* How `e` relates to `page` through the index's parent links: 0 for a
 * child, 1 for a sibling, 2 for the parent, -1 for any other page. */
static int parent_relation(const tam_index_entry_t *page,
                           const tam_index_entry_t *e)
{
    if (e->parent != NULL && strcmp(e->parent, page->path) == 0) {
        return 0;
    }
    if (page->parent != NULL && e->parent != NULL &&
        strcmp(e->parent, page->parent) == 0) {
        return 1;
    }
    if (page->parent != NULL && strcmp(page->parent, e->path) == 0) {
        return 2;
    }
    return -1;
}

/* Links to at most three relatives of `page` that offer `level`: its
 * children, then its siblings, then its parent (by the index's parent
 * links), each ranked by shared prerequisites, then by title. The level path
 * page is left out, since the level line already links it. NULL when there
 * are none, so the level line links only the level path. */
static char *related_at_level(const tam_index_t *idx,
                              const tam_index_entry_t *page, unsigned level)
{
    enum { MAX_RELATED = 3 };
    char level_path[32];
    snprintf(level_path, sizeof(level_path), "%s.html", tam_level_name(level));
    const tam_index_entry_t *best[MAX_RELATED];
    int best_rel[MAX_RELATED];
    int best_score[MAX_RELATED];
    size_t nbest = 0;
    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (e == page || !(e->levels & level) ||
            strcmp(e->path, level_path) == 0) {
            continue;
        }
        int rel = parent_relation(page, e);
        if (rel < 0) {
            continue;
        }
        int score = shared_prereqs(page, e);
        size_t at = nbest;
        while (at > 0 && (rel < best_rel[at - 1] ||
                          (rel == best_rel[at - 1] &&
                           (score > best_score[at - 1] ||
                            (score == best_score[at - 1] &&
                             strcasecmp(e->title, best[at - 1]->title) < 0))))) {
            at--;
        }
        if (at >= MAX_RELATED) {
            continue;
        }
        size_t last = nbest < MAX_RELATED ? nbest : MAX_RELATED - 1;
        for (size_t k = last; k > at; k--) {
            best[k] = best[k - 1];
            best_rel[k] = best_rel[k - 1];
            best_score[k] = best_score[k - 1];
        }
        best[at] = e;
        best_rel[at] = rel;
        best_score[at] = score;
        if (nbest < MAX_RELATED) {
            nbest++;
        }
    }
    if (nbest == 0) {
        return NULL;
    }
    tam_str_t html = tam_str_new("");
    for (size_t k = 0; k < nbest; k++) {
        char *title = tam_html_escape(best[k]->title);
        /* Titles may hold commas, so links are separated by bars. */
        html = tam_str_cat_printf(html, "%s<a href=\"%s\">%s</a>",
                                  k > 0 ? " | " : "", best[k]->path, title);
        free(title);
    }
    char *out = strdup(html);
    tam_str_free(html);
    return out;
}

/* Read a corpus page and prepare it at its selected level. `href` may carry
 * "?level=NAME". A page without that level is shown whole under a banner.
 * Returns NULL when the page is missing or unreadable. */
static char *read_corpus_page(const char *root, const char *href,
                              size_t *out_len, tam_depth_info_t *info)
{
    unsigned level = 0;
    char *rel = tam_depth_split_href(href, &level);
    if (rel == NULL) {
        return NULL;
    }
    char *path = corpus_join(root, rel);
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
    int rev = 0;
    char updated[64] = {0};
    char *related = NULL;
    tam_index_t *idx = tam_index_load(root);
    if (idx != NULL) {
        const tam_index_entry_t *e = tam_index_lookup_path(idx, rel);
        if (e != NULL && e->rev > 0 && e->updated != NULL && e->updated[0] != '\0') {
            rev = e->rev;
            snprintf(updated, sizeof(updated), "%s", e->updated);
        }
        if (e != NULL && level != 0 && !(tam_depth_declared(src, len) & level)) {
            related = related_at_level(idx, e, level);
        }
        tam_index_free(idx);
    }
    char *numbered = tam_fold_number(src, len);
    if (numbered == NULL) { free(related); free(src); free(rel); return NULL; }
    char *html = tam_depth_apply_rev(numbered, strlen(numbered), rel, level, rev,
                                     updated[0] ? updated : NULL, related,
                                     out_len, info);
    free(numbered);
    free(related);
    free(src);
    free(rel);
    return html;
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


tam_document_t *tam_topic_guide_document(const char *root, const char *topic, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }
    const tam_index_entry_t *matches[64];
    size_t num_matches = tam_index_find_topic_pages(idx, topic, level, matches, 64);
    /* No page at the reader's level: list every level under a banner rather
     * than fail. */
    unsigned missing = 0;
    if (num_matches == 0 && level != 0) {
        num_matches = tam_index_find_topic_pages(idx, topic, 0, matches, 64);
        missing = level;
        level = 0;
    }
    if (num_matches == 0) {
        tam_index_free(idx);
        return NULL;
    }

    char *esc_topic = tam_html_escape(topic);
    static const tam_crumb_t crumbs[] = {
        { "index.html", "Guide" }, { "tam:topics", "Topics" }, { NULL, "Topic Guide" }
    };
    tam_str_t title = tam_str_cat(tam_str_new("Topic Guide: "), topic);
    tam_str_t html = tam_page_begin(title, NULL, crumbs, 3);
    tam_str_free(title);
    unsigned lens = missing != 0 ? missing : level;
    if (lens != 0) {
        char *line = tam_level_lines(missing != 0 ? TAM_LEVEL_ALL : lens, lens,
                                     missing != 0, NULL);
        if (line != NULL) {
            html = tam_str_cat(html, line);
            free(line);
        }
    }
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
        char *esc_title = tam_html_escape(e->title);
        char *esc_summary = tam_html_escape(e->summary);
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
            snprintf(levels_buf, sizeof(levels_buf), " | Levels: %s", lvl_names);
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
    html = tam_str_cat(html, "</dl>\n");
    tam_index_free(idx);
    return tam_page_render(html, width);
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

    static const tam_crumb_t crumbs[] = { { "index.html", "Guide" }, { NULL, "Keyword Index" } };
    tam_str_t html = tam_page_begin("Keyword Index", NULL, crumbs, 2);
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Keyword Index (%s)</h1>\n", tam_level_name(level));
    } else {
        html = tam_str_cat(html, "<h1>Keyword Index</h1>\n");
    }
    html = tam_str_cat(html,
        "<p>Every word <code>tam explain</code> recognizes and what it leads to.<br>\n"
        "Select a keyword with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to see its topic guide. "
        "Press <kbd>`</kbd> for the guide index, or <kbd>q</kbd> to quit.</p>\n"
        "<table>\n"
        "<tr><th>Keyword</th><th>Pages</th><th>Covers</th></tr>\n");

    for (size_t i = 0; i < list->count; i++) {
        char *enc_kw = url_encode(list->items[i].keyword);
        char *esc_kw = tam_html_escape(list->items[i].keyword);
        char *esc_sum = tam_html_escape(list->items[i].primary_summary);
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
    html = tam_str_cat(html, "</table>\n");
    tam_keyword_list_free(list);
    tam_index_free(idx);
    return tam_page_render(html, width);
}

tam_document_t *tam_topics_document(const char *root, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }

    static const tam_crumb_t crumbs[] = { { "index.html", "Guide" }, { NULL, "Topics" } };
    tam_str_t html = tam_page_begin("Topics A to Z", NULL, crumbs, 2);
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Topics A to Z (%s)</h1>\n",
                                  tam_level_name(level));
    } else {
        html = tam_str_cat(html, "<h1>Topics A to Z</h1>\n");
    }
    html = tam_str_cat(html,
        "<p>Every topic tam can explain, one line each.<br>\n"
        "Select a topic with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to read it. "
        "Press <kbd>`</kbd> for the guide index, or <kbd>q</kbd> to quit.</p>\n"
        "<table>\n"
        "<tr><th>Topic</th><th>Covers</th></tr>\n");

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (level != 0 && !(e->levels & level)) {
            continue;
        }
        if (level != 0) {
            html = tam_str_cat_printf(html, "<tr><td><a href=\"%s?level=%s\">",
                                      e->path, tam_level_name(level));
        } else {
            html = tam_str_cat_printf(html, "<tr><td><a href=\"%s\">", e->path);
        }
        html = tam_html_cat(html, e->topic);
        html = tam_str_cat(html, "</a></td><td>");
        html = tam_html_cat(html, e->summary);
        html = tam_str_cat(html, "</td></tr>\n");
    }
    html = tam_str_cat(html, "</table>\n");
    tam_index_free(idx);
    return tam_page_render(html, width);
}

tam_document_t *tam_corpus_page_document(const tam_loader_ctx_t *ctx,
                                         const char *href, int content_width)
{
    if (ctx == NULL || href == NULL || href[0] == '\0') {
        return NULL;
    }
    if (ctx->root == NULL && ctx->stdin_html == NULL) {
        return NULL;
    }
    char *open = NULL;
    char *bare = tam_fold_split_href(href, &open);
    if (bare == NULL) { free(open); return NULL; }
    if (ctx->root == NULL && strcmp(bare, "<stdin>") != 0) {
        free(bare); free(open); return NULL;
    }
    size_t len = 0;
    tam_depth_info_t info = {0};
    char *src = strcmp(bare, "<stdin>") == 0 && ctx->stdin_html != NULL
        ? tam_fold_number(ctx->stdin_html, ctx->stdin_len)
        : read_corpus_page(ctx->root, bare, &len, &info);
    free(bare);
    if (src == NULL) {
        free(open);
        return NULL;
    }
    tam_document_t *doc = tam_fold_render(src, strlen(src), content_width, open, 0);
    free(open);
    free(src);
    if (doc != NULL) {
        doc->levels = info.levels;
        doc->hidden = info.hidden;
    }
    return doc;
}

static int record_welcome_seen(void)
{
    char *path = tam_xdg_path(TAM_XDG_STATE, "tam", "welcome_seen", true);
    if (path == NULL) {
        return -1;
    }

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
                         size_t len, int pipe, const tam_depth_info_t *info)
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

    char *numbered = info == NULL ? tam_fold_number(src, len) : NULL;
    tam_document_t *doc = (info == NULL && numbered == NULL) ? NULL :
        tam_fold_render(numbered ? numbered : src, numbered ? strlen(numbered) : len,
                        width, NULL, !interactive);
    free(numbered);
    if (doc == NULL) {
        return TAM_EXIT_FAILURE;
    }
    if (info != NULL) {
        doc->levels = info->levels;
        doc->hidden = info->hidden;
    }

    int rc = 0;
    if (interactive) {
        tam_loader_ctx_t ctx = { .root = (char *)root,
            .stdin_html = info == NULL ? src : NULL, .stdin_len = len };
        rc = tam_pager_run(doc, href, tam_address_load, &ctx);
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
    char *path = rel != NULL ? corpus_join(root, rel) : NULL;
    free(rel);
    if (path == NULL || !tam_fs_file_exists(path)) {
        fprintf(stderr, TAM_COMMAND_NAME ": guide page not found: %s\n", href);
        free(path);
        return TAM_EXIT_FAILURE;
    }
    free(path);

    size_t len = 0;
    tam_depth_info_t info = {0};
    char *src = read_corpus_page(root, href, &len, &info);
    if (src == NULL) {
        fprintf(stderr, TAM_COMMAND_NAME ": cannot read guide page: %s\n",
                href);
        return TAM_EXIT_FAILURE;
    }

    int rc = view_document(root, href, src, len, pipe, &info);
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
    fprintf(stderr, "Levels: all (0), beginner (1), intermediate (2), expert (3).\n");
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
            if (!tam_level_parse_arg(value, &level)) {
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

    if (page != NULL && strncmp(page, "page:", 5) == 0) {
        page += 5;
        if (page[0] == '\0') {
            fprintf(stderr, TAM_COMMAND_NAME ": missing guide page after 'page:'\n");
            return TAM_EXIT_USAGE;
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
        int rc = view_document(root, "<stdin>", src, len, pipe, NULL);
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

    if (page != NULL && strcmp(page, "topics") == 0) {
        page = "tam:topics";
    }
    tam_addr_kind_t kind = tam_addr_kind(page);
    if (kind == TAM_ADDR_MAN || kind == TAM_ADDR_INFO || kind == TAM_ADDR_TLDR ||
        kind == TAM_ADDR_VIEW) {
        /* --level applies to a view that takes the lens and names none. */
        char addr[600];
        char named[32];
        snprintf(addr, sizeof(addr), "%s", page);
        if (level != 0 && tam_addr_takes_lens(addr) &&
            !tam_addr_param(addr, "level", named, sizeof(named))) {
            size_t len = strlen(addr);
            snprintf(addr + len, sizeof(addr) - len, "%clevel=%s",
                     strchr(addr, '?') != NULL ? '&' : '?', tam_level_name(level));
        }
        tam_loader_ctx_t ctx = {.root = root};
        int interactive = !pipe && isatty(STDOUT_FILENO);
        int width = interactive ? tam_terminal_width() : 80;
        tam_document_t *doc = tam_address_load(&ctx, addr, width);
        if (doc == NULL) {
            if (kind == TAM_ADDR_MAN)
                fprintf(stderr, TAM_COMMAND_NAME ": no manual page found for '%s'\n", page + 4);
            else if (kind == TAM_ADDR_INFO)
                fprintf(stderr, TAM_COMMAND_NAME ": no info manual or node found for '%s'\n", page + 5);
            else if (kind == TAM_ADDR_TLDR)
                fprintf(stderr, TAM_COMMAND_NAME ": no tldr page found for '%s'\n", page + 5);
            else
                fprintf(stderr, TAM_COMMAND_NAME ": could not open discovery page '%s'\n", page);
            free(root);
            return TAM_EXIT_FAILURE;
        }
        tam_document_ensure_location(doc, addr);
        int rc = 0;
        if (interactive) {
            rc = tam_pager_run(doc, addr, tam_address_load, &ctx);
        } else {
            char *text = tam_document_text(doc);
            if (text != NULL) { fputs(text, stdout); free(text); }
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
        "  tam explain --level LEVEL [TOPIC]  filter by level (0|all, 1|beginner, 2|intermediate, 3|expert)\n"
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
            if (!tam_level_parse_arg(value, &level)) {
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

        tam_loader_ctx_t ctx = {0};
        ctx.root = root;
        char initial_href[64];
        if (level != 0) {
            snprintf(initial_href, sizeof(initial_href), "tam:topics?level=%s", tam_level_name(level));
        } else {
            snprintf(initial_href, sizeof(initial_href), "tam:topics");
        }
        int rc = tam_pager_run(doc, initial_href, tam_address_load, &ctx);
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
            tam_loader_ctx_t ctx = {0};
            ctx.root = root;
                char initial_href[64];
            if (level != 0) {
                snprintf(initial_href, sizeof(initial_href), "tam:keywords?level=%s", tam_level_name(level));
            } else {
                snprintf(initial_href, sizeof(initial_href), "tam:keywords");
            }
            rc = tam_pager_run(doc, initial_href, tam_address_load, &ctx);
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
        tam_index_print_miss(stderr, idx, query_buf);
        tam_index_free(idx);
        free(root);
        return TAM_EXIT_FAILURE;
    }

    int rc = 0;
    if (interactive) {
        tam_loader_ctx_t ctx = {0};
        ctx.root = root;
        char initial_href[512];
        char *enc_q = url_encode(query_buf);
        if (level != 0) {
            snprintf(initial_href, sizeof(initial_href), "tam:explain?topic=%s&level=%s", enc_q ? enc_q : "", tam_level_name(level));
        } else {
            snprintf(initial_href, sizeof(initial_href), "tam:explain?topic=%s", enc_q ? enc_q : "");
        }
        free(enc_q);
        rc = tam_pager_run(doc, initial_href, tam_address_load, &ctx);
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
    int pipe = 0;
    char query_buf[512] = {0};
    size_t qlen = 0;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (strcmp(args[i], "--json") == 0) {
            json_mode = 1;
        } else if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0) {
            fputs(
                "tam search - search the Tamlinux offline guide\n\n"
                "Usage:\n"
                "  tam search TERM...         open matching guide pages in the pager\n"
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
            "  tam search TERM...         open matching guide pages in the pager\n"
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

    int interactive = !pipe && !json_mode && isatty(STDOUT_FILENO);
    if (interactive && !isatty(STDIN_FILENO)) {
        int tty = open("/dev/tty", O_RDONLY);
        if (tty < 0) {
            interactive = 0;
        } else {
            close(tty);
        }
    }

    if (interactive) {
        tam_index_free(idx);
        char initial_href[1600];
        char *enc_q = url_encode(query_buf);
        snprintf(initial_href, sizeof(initial_href), "tam:search?query=%s", enc_q ? enc_q : "");
        free(enc_q);
        tam_document_t *doc = tam_guide_search_document(root, query_buf, tam_terminal_width());
        if (doc == NULL) {
            free(root);
            return TAM_EXIT_FAILURE;
        }
        tam_document_ensure_location(doc, initial_href);
        tam_loader_ctx_t ctx = {0};
        ctx.root = root;
        int rc = tam_pager_run(doc, initial_href, tam_address_load, &ctx);
        free(root);
        return rc;
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
            printf("               Levels: %s; track: %s\n", levels, e->track);
        }
        printf("\nRun 'tam explain [--level LEVEL] <topic>' or 'tam show <page>' to view.\n");
    }

    tam_index_free(idx);
    free(root);
    return 0;
}

int main(int argc, char **argv)
{
    (void)setlocale(LC_CTYPE, "");
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
    if (strcmp(arg, "tldr") == 0) {
        return tam_cmd_tldr(argc - 2, argv + 2);
    }
    if (strcmp(arg, "ask") == 0) {
        return tam_cmd_ask(argc - 2, argv + 2);
    }
    if (strcmp(arg, "upgrade") == 0) {
        return tam_cmd_upgrade(argc - 2, argv + 2);
    }
    if (strcmp(arg, "switch") == 0) {
        return tam_cmd_switch(argc - 2, argv + 2);
    }
    return usage_error(arg);
}
