#include <tam.h>

#include "depth.h"
#include "man.h"
#include "pager.h"
#include "render.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAM_COMMAND_NAME "tam"
#define TAM_EXIT_USAGE 2
#define TAM_EXIT_FAILURE 1

static int is_name_char(char c)
{
    return isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.' || c == '+';
}

static int is_sec_char(char c)
{
    return isalnum((unsigned char)c);
}

static int has_suffix(const char *str, const char *suffix)
{
    size_t slen = strlen(str);
    size_t xlen = strlen(suffix);
    return slen >= xlen && strcmp(str + slen - xlen, suffix) == 0;
}

char *tam_man_linkify_html(const char *src)
{
    if (src == NULL) {
        return NULL;
    }
    size_t cap = strlen(src) * 2 + 1024;
    char *out = malloc(cap);
    if (out == NULL) {
        return NULL;
    }
    size_t out_len = 0;

    int in_a_tag = 0;
    const char *p = src;

    while (*p != '\0') {
        if (!in_a_tag && strncasecmp(p, "<a ", 3) == 0) {
            in_a_tag = 1;
        } else if (!in_a_tag && strncasecmp(p, "<a>", 3) == 0) {
            in_a_tag = 1;
        } else if (in_a_tag && strncasecmp(p, "</a>", 4) == 0) {
            in_a_tag = 0;
            memcpy(out + out_len, p, 4);
            out_len += 4;
            p += 4;
            continue;
        }

        if (in_a_tag) {
            out[out_len++] = *p++;
            continue;
        }

        /* Check for <b>NAME</b>(SEC) or <i>NAME</i>(SEC) */
        if (strncmp(p, "<b>", 3) == 0 || strncmp(p, "<i>", 3) == 0) {
            int is_it = (p[1] == 'i');
            const char *close_tag = is_it ? "</i>" : "</b>";
            const char *nstart = p + 3;
            const char *nend = nstart;
            while (*nend != '\0' && is_name_char(*nend)) {
                nend++;
            }
            size_t name_len = (size_t)(nend - nstart);

            /* Pattern A: <b>NAME</b>(SEC) or <b>NAME</b> (SEC) */
            if (name_len > 0 && name_len <= 64 && strncmp(nend, close_tag, 4) == 0) {
                const char *after_close = nend + 4;
                while (*after_close == ' ') {
                    after_close++;
                }
                if (*after_close == '(') {
                    const char *sec_start = after_close + 1;
                    const char *sec_end = sec_start;
                    if (isdigit((unsigned char)*sec_end)) {
                        while (*sec_end != '\0' && is_sec_char(*sec_end)) {
                            sec_end++;
                        }
                        if (*sec_end == ')' && (sec_end - sec_start) <= 8) {
                            char name[65];
                            memcpy(name, nstart, name_len);
                            name[name_len] = '\0';

                            char sec[16];
                            size_t slen = (size_t)(sec_end - sec_start);
                            memcpy(sec, sec_start, slen);
                            sec[slen] = '\0';

                            size_t match_len = (size_t)(sec_end + 1 - p);

                            int hlen = snprintf(out + out_len, cap - out_len,
                                                "<a href=\"man:%s(%s)\">", name, sec);
                            if (hlen > 0 && (size_t)hlen < cap - out_len) {
                                out_len += (size_t)hlen;
                            }
                            memcpy(out + out_len, p, match_len);
                            out_len += match_len;
                            memcpy(out + out_len, "</a>", 4);
                            out_len += 4;

                            p += match_len;
                            continue;
                        }
                    }
                }
            }

            /* Pattern B: <b>NAME(SEC)</b> */
            if (name_len > 0 && name_len <= 64 && *nend == '(') {
                const char *sec_start = nend + 1;
                const char *sec_end = sec_start;
                if (isdigit((unsigned char)*sec_end)) {
                    while (*sec_end != '\0' && is_sec_char(*sec_end)) {
                        sec_end++;
                    }
                    if (*sec_end == ')' && (sec_end - sec_start) <= 8 &&
                        strncmp(sec_end + 1, close_tag, 4) == 0) {
                        char name[65];
                        memcpy(name, nstart, name_len);
                        name[name_len] = '\0';

                        char sec[16];
                        size_t slen = (size_t)(sec_end - sec_start);
                        memcpy(sec, sec_start, slen);
                        sec[slen] = '\0';

                        size_t match_len = (size_t)(sec_end + 1 + 4 - p);

                        int hlen = snprintf(out + out_len, cap - out_len,
                                            "<a href=\"man:%s(%s)\">", name, sec);
                        if (hlen > 0 && (size_t)hlen < cap - out_len) {
                            out_len += (size_t)hlen;
                        }
                        memcpy(out + out_len, p, match_len);
                        out_len += match_len;
                        memcpy(out + out_len, "</a>", 4);
                        out_len += 4;

                        p += match_len;
                        continue;
                    }
                }
            }
        }

        out[out_len++] = *p++;
    }

    out[out_len] = '\0';
    return out;
}

static char *locate_man_file(const char *sec, const char *name)
{
    char cmd[512];
    if (sec != NULL && sec[0] != '\0') {
        snprintf(cmd, sizeof(cmd), "man -w '%s' '%s' 2>/dev/null", sec, name);
    } else {
        snprintf(cmd, sizeof(cmd), "man -w '%s' 2>/dev/null", name);
    }
    FILE *fp = popen(cmd, "r");
    if (fp != NULL) {
        char path[1024];
        if (fgets(path, sizeof(path), fp) != NULL) {
            pclose(fp);
            size_t len = strlen(path);
            while (len > 0 && (path[len - 1] == '\n' || path[len - 1] == '\r' || path[len - 1] == ' ')) {
                path[--len] = '\0';
            }
            if (len > 0 && access(path, R_OK) == 0) {
                return strdup(path);
            }
        } else {
            pclose(fp);
        }
    }

    /* Fallback for development installs or repository checkout */
    const char *s = (sec != NULL && sec[0] != '\0') ? sec : "1";
    char cand[1024];

    char *kroot = tam_find_knowledge_root();
    if (kroot != NULL) {
        snprintf(cand, sizeof(cand), "%s/../../man/man%s/%s.%s", kroot, s, name, s);
        free(kroot);
        if (access(cand, R_OK) == 0) {
            return strdup(cand);
        }
    }

    snprintf(cand, sizeof(cand), "man/%s.%s", name, s);
    if (access(cand, R_OK) == 0) {
        return strdup(cand);
    }

    return NULL;
}

char *tam_man_resolve_path(const char *name, const char *sec)
{
    return locate_man_file(sec, name);
}

static char *render_man_to_html(const char *path)
{
    char cmd[2048];
    if (has_suffix(path, ".gz")) {
        snprintf(cmd, sizeof(cmd), "gzip -dc '%s' | groff -mandoc -Thtml -P -D/tmp 2>/dev/null", path);
    } else if (has_suffix(path, ".bz2")) {
        snprintf(cmd, sizeof(cmd), "bzip2 -dc '%s' | groff -mandoc -Thtml -P -D/tmp 2>/dev/null", path);
    } else if (has_suffix(path, ".xz")) {
        snprintf(cmd, sizeof(cmd), "xz -dc '%s' | groff -mandoc -Thtml -P -D/tmp 2>/dev/null", path);
    } else if (has_suffix(path, ".zst")) {
        snprintf(cmd, sizeof(cmd), "zstd -dc '%s' | groff -mandoc -Thtml -P -D/tmp 2>/dev/null", path);
    } else {
        snprintf(cmd, sizeof(cmd), "groff -mandoc -Thtml -P -D/tmp '%s' 2>/dev/null", path);
    }

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        return NULL;
    }

    size_t cap = 32768;
    size_t len = 0;
    char *buf = malloc(cap);
    if (buf == NULL) {
        pclose(fp);
        return NULL;
    }

    char chunk[4096];
    size_t n;
    while ((n = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        if (len + n >= cap) {
            cap = (cap + n) * 2;
            char *next = realloc(buf, cap);
            if (next == NULL) {
                free(buf);
                pclose(fp);
                return NULL;
            }
            buf = next;
        }
        memcpy(buf + len, chunk, n);
        len += n;
    }
    pclose(fp);
    buf[len] = '\0';
    if (len == 0) {
        free(buf);
        return NULL;
    }
    return buf;
}

tam_document_t *tam_man_load(const char *spec, int width)
{
    if (spec == NULL || spec[0] == '\0') {
        return NULL;
    }
    const char *s = spec;
    if (strncmp(s, "man:", 4) == 0) {
        s += 4;
    }
    char name[65] = {0};
    char sec[16] = {0};

    const char *paren = strchr(s, '(');
    if (paren != NULL) {
        size_t nlen = (size_t)(paren - s);
        if (nlen >= sizeof(name)) {
            nlen = sizeof(name) - 1;
        }
        memcpy(name, s, nlen);
        name[nlen] = '\0';

        const char *rparen = strchr(paren, ')');
        if (rparen != NULL && rparen > paren + 1) {
            size_t slen = (size_t)(rparen - paren - 1);
            if (slen >= sizeof(sec)) {
                slen = sizeof(sec) - 1;
            }
            memcpy(sec, paren + 1, slen);
            sec[slen] = '\0';
        }
    } else {
        snprintf(name, sizeof(name), "%s", s);
    }

    for (char *c = name; *c != '\0'; c++) {
        if (!is_name_char(*c)) {
            return NULL;
        }
    }
    for (char *c = sec; *c != '\0'; c++) {
        if (!is_sec_char(*c)) {
            return NULL;
        }
    }

    char *path = locate_man_file(sec[0] ? sec : NULL, name);
    if (path == NULL) {
        return NULL;
    }

    char *raw_html = render_man_to_html(path);
    free(path);
    if (raw_html == NULL) {
        return NULL;
    }

    char *linkified = tam_man_linkify_html(raw_html);
    free(raw_html);
    if (linkified == NULL) {
        return NULL;
    }

    tam_document_t *doc = tam_render_html(linkified, strlen(linkified), width);
    free(linkified);
    return doc;
}

static void print_man_help(void)
{
    fputs(
        "tam man - view manual pages with hyperlinked cross-references\n\n"
        "Usage:\n"
        "  tam man PAGE              open manual page in the terminal\n"
        "  tam man SECTION PAGE      open manual page from a specific section\n"
        "  tam man PAGE --pipe       print manual page as plain text\n"
        "  tam man --help            show this help and exit\n\n"
        "Examples:\n"
        "  tam man grep              view grep(1) manual page\n"
        "  tam man 2 read            view read(2) system call\n"
        "  tam man systemd.unit      view systemd.unit(5) configuration\n"
        "  tam man grep --pipe       pipe manual page text\n\n"
        "Cross-references (such as in SEE ALSO) are rendered as live links.\n"
        "Press Tab/Shift-Tab to select links and Enter to follow them.\n",
        stdout);
}

typedef struct {
    char *root;
    int   width;
} man_loader_ctx_t;

static tam_document_t *man_unified_load(void *vctx, const char *href)
{
    man_loader_ctx_t *ctx = vctx;
    if (href == NULL || href[0] == '\0') {
        return NULL;
    }
    if (strncmp(href, "man:", 4) == 0) {
        return tam_man_load(href + 4, ctx->width);
    }
    if (ctx->root != NULL) {
        if (strncmp(href, "tam:topics", 10) == 0) {
            unsigned level = 0;
            const char *q = strstr(href, "?level=");
            if (q != NULL) {
                level = tam_level_parse(q + 7);
            }
            return tam_topics_document(ctx->root, level, ctx->width);
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", ctx->root, href);
        size_t len = 0;
        char *src = tam_fs_read_file(path, &len);
        if (src != NULL) {
            tam_document_t *doc = tam_render_html(src, len, ctx->width);
            free(src);
            return doc;
        }
    }
    return tam_man_load(href, ctx->width);
}

int tam_cmd_man(int n, char **args)
{
    int pipe = 0;
    const char *sec = NULL;
    const char *page = NULL;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0 ||
                   strcmp(args[i], "help") == 0) {
            print_man_help();
            return 0;
        } else if (args[i][0] == '-') {
            fprintf(stderr, TAM_COMMAND_NAME ": unrecognized argument '%s'\n", args[i]);
            fprintf(stderr, "Run '%s man --help' for available options.\n", TAM_COMMAND_NAME);
            return TAM_EXIT_USAGE;
        } else if (page == NULL) {
            page = args[i];
        } else if (sec == NULL) {
            if (isdigit((unsigned char)page[0])) {
                sec = page;
                page = args[i];
            } else if (isdigit((unsigned char)args[i][0])) {
                sec = args[i];
            } else {
                fprintf(stderr, TAM_COMMAND_NAME ": too many arguments\n");
                return TAM_EXIT_USAGE;
            }
        } else {
            fprintf(stderr, TAM_COMMAND_NAME ": too many arguments\n");
            return TAM_EXIT_USAGE;
        }
    }

    if (page == NULL) {
        print_man_help();
        return 0;
    }

    char spec[128];
    if (sec != NULL && sec[0] != '\0') {
        snprintf(spec, sizeof(spec), "%s(%s)", page, sec);
    } else {
        snprintf(spec, sizeof(spec), "%s", page);
    }

    int interactive = !pipe && isatty(STDOUT_FILENO);
    int width = interactive ? tam_terminal_width() : 80;

    tam_document_t *doc = tam_man_load(spec, width);
    if (doc == NULL) {
        if (sec != NULL && sec[0] != '\0') {
            fprintf(stderr, TAM_COMMAND_NAME ": no manual page found for '%s' in section %s\n",
                    page, sec);
        } else {
            fprintf(stderr, TAM_COMMAND_NAME ": no manual page found for '%s'\n", page);
        }
        return TAM_EXIT_FAILURE;
    }

    int rc = 0;
    if (interactive) {
        man_loader_ctx_t ctx;
        ctx.root = tam_find_knowledge_root();
        ctx.width = width;
        char initial_href[160];
        snprintf(initial_href, sizeof(initial_href), "man:%s", spec);
        rc = tam_pager_run(doc, initial_href, man_unified_load, &ctx);
        if (ctx.root != NULL) {
            free(ctx.root);
        }
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
