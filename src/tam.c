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

#include "corpus.h"


enum {
    HELP_STANDARD = 0,
    HELP_TERSE,
    HELP_VERBOSE
};


static void print_tam_help(int tier);
static void print_version(void);
static int usage_error(const char *bad);
static int too_many_arguments(void);
static int record_welcome_seen(void);
static char *read_all_stdin(size_t *out_len);
static int view_document(const char *root, const char *href, const char *src,
                  size_t len, int pipe, const tam_depth_info_t *info);
static int view_knowledge_page(const char *root, const char *href, int pipe);
static int level_usage_error(const char *bad);
static int cmd_show(int n, char **args);
static int cmd_welcome(int n, char **args);
static void print_explain_usage(void);
static int cmd_explain(int n, char **args);
static int cmd_search(int n, char **args);

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
            "  tam hi                        first-run orientation\n"
            "  tam show [PAGE]               open guide or specific page\n"
            "  tam inspect                   inspect system components\n\n"
            "  tam upgrade                   check and install available versions\n"
            "  tam switch STAGE              select a source stage or roll back\n\n"
            "Essential Options:\n"
            "  --help, -h | --pipe | --terse | --verbose | --version, -v, -V\n",
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
            "  tam hi [--pipe]\n"
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
            "  --version, -v, -V\n"
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
        "  tam hi                    first-run orientation\n\n"
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
        "  --version, -v, -V         show version info\n\n"
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

    if (strcmp(arg, "-v") == 0 || strcmp(arg, "-V") == 0 || strcmp(arg, "--version") == 0) {
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
