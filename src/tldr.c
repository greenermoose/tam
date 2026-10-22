#define _POSIX_C_SOURCE 200809L
#include <tam.h>

#include "tldr.h"
#include "depth.h"
#include "discovery.h"
#include "html.h"
#include "index.h"
#include "info.h"
#include "man.h"
#include "pager.h"
#include "render.h"
#include "terminal.h"
#include "version.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define TAM_EXIT_USAGE   2
#define TAM_EXIT_FAILURE 1
#define TLDR_RESULTS_PER_PAGE 60

extern char *tam_find_knowledge_root(void);
extern tam_document_t *tam_topics_document(const char *root, unsigned level, int width);

/* ------------------------------------------------------------------------- */
/* String and URI Helpers                                                     */
/* ------------------------------------------------------------------------- */

static char *uri_decode(const char *src)
{
    if (src == NULL) return NULL;
    size_t n = strlen(src);
    char *out = malloc(n + 1);
    if (out == NULL) return NULL;
    size_t r = 0, w = 0;
    while (r < n) {
        if (src[r] == '%' && r + 2 < n &&
            isxdigit((unsigned char)src[r + 1]) &&
            isxdigit((unsigned char)src[r + 2])) {
            char hex[3] = {src[r + 1], src[r + 2], '\0'};
            out[w++] = (char)strtoul(hex, NULL, 16);
            r += 3;
        } else if (src[r] == '+') {
            out[w++] = ' ';
            r++;
        } else {
            out[w++] = src[r++];
        }
    }
    out[w] = '\0';
    return out;
}

static char *uri_encode(const char *src)
{
    if (src == NULL) return NULL;
    static const char hex[] = "0123456789ABCDEF";
    size_t n = strlen(src);
    char *out = malloc(n * 3 + 1);
    if (out == NULL) return NULL;
    size_t w = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out[w++] = (char)c;
        } else {
            out[w++] = '%';
            out[w++] = hex[c >> 4];
            out[w++] = hex[c & 15];
        }
    }
    out[w] = '\0';
    return out;
}

static char *uri_parameter(const char *uri, const char *key)
{
    if (uri == NULL || key == NULL) return NULL;
    char needle[48];
    snprintf(needle, sizeof(needle), "%s=", key);
    size_t klen = strlen(needle);
    const char *p = strchr(uri, '?');
    if (p == NULL) return NULL;
    p++;
    while (*p != '\0') {
        const char *end = strchr(p, '&');
        size_t n = end ? (size_t)(end - p) : strlen(p);
        if (n >= klen && strncmp(p, needle, klen) == 0) {
            char *raw = strndup(p + klen, n - klen);
            char *dec = uri_decode(raw);
            free(raw);
            return dec;
        }
        if (end == NULL) break;
        p = end + 1;
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* Directory Resolution and Command Name Normalization                        */
/* ------------------------------------------------------------------------- */

static int tldr_dir_ok(const char *dir)
{
    if (dir == NULL || dir[0] == '\0') return 0;
    char path[1024];
    snprintf(path, sizeof(path), "%s/index.tsv", dir);
    if (tam_fs_file_exists(path)) return 1;
    snprintf(path, sizeof(path), "%s/pages/common/tar.md", dir);
    if (tam_fs_file_exists(path)) return 1;
    return 0;
}

#ifndef TAM_DATADIR
#define TAM_DATADIR "/usr/local/share/tamlinux/knowledge"
#endif

char *tam_find_tldr_root(const char *knowledge_root)
{
    const char *env = getenv("TAM_TLDR_DIR");
    if (env != NULL && env[0] != '\0') {
        return tldr_dir_ok(env) ? strdup(env) : NULL;
    }

    if (knowledge_root != NULL && knowledge_root[0] != '\0') {
        char cand[1024];
        snprintf(cand, sizeof(cand), "%s/tldr", knowledge_root);
        if (tldr_dir_ok(cand)) return strdup(cand);
    }

    const char *kenv = getenv("TAM_KNOWLEDGE_DIR");
    if (kenv != NULL && kenv[0] != '\0') {
        char cand[1024];
        snprintf(cand, sizeof(cand), "%s/tldr", kenv);
        if (tldr_dir_ok(cand)) return strdup(cand);
    }

    const char *compiled = getenv("TAM_DATADIR");
    if (compiled == NULL || compiled[0] == '\0') {
        compiled = TAM_DATADIR;
    }
    if (compiled != NULL && compiled[0] != '\0') {
        char cand[1024];
        snprintf(cand, sizeof(cand), "%s/tldr", compiled);
        if (tldr_dir_ok(cand)) return strdup(cand);
    }

    char *owned_kroot = tam_find_knowledge_root();
    if (owned_kroot != NULL) {
        char cand[1024];
        snprintf(cand, sizeof(cand), "%s/tldr", owned_kroot);
        free(owned_kroot);
        if (tldr_dir_ok(cand)) return strdup(cand);
    }

    if (tldr_dir_ok("knowledge/tldr")) {
        return strdup("knowledge/tldr");
    }
    return NULL;
}

static int is_valid_cmd_char(char c)
{
    return isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' ||
           c == '+' || c == ':' || c == '[' || c == '@';
}

/* Normalize a command specification per tldr Client Specification v2.3:
 * - Strips optional "tldr:" prefix
 * - Extracts optional "<platform>/" prefix into out_platform
 * - Converts spaces to '-' and uppercase ASCII to lowercase */
static int normalize_tldr_spec(const char *spec, char *out_platform, size_t plat_cap,
                               char *out_name, size_t name_cap)
{
    if (spec == NULL) return 0;
    out_platform[0] = '\0';
    out_name[0] = '\0';

    const char *s = spec;
    if (strncmp(s, "tldr:", 5) == 0) {
        s += 5;
    }
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0') return 0;

    const char *slash = strchr(s, '/');
    if (slash != NULL && strchr(s, ' ') == NULL) {
        size_t plen = (size_t)(slash - s);
        if (plen == 0 || plen >= plat_cap) return 0;
        for (size_t i = 0; i < plen; i++) {
            char c = (char)tolower((unsigned char)s[i]);
            if (!isalnum((unsigned char)c) && c != '-' && c != '_') return 0;
            out_platform[i] = c;
        }
        out_platform[plen] = '\0';
        s = slash + 1;
    }

    size_t w = 0;
    int prev_dash = 0;
    for (const char *p = s; *p != '\0'; p++) {
        if (*p == ' ' || *p == '\t') {
            if (w > 0 && !prev_dash && w + 1 < name_cap) {
                out_name[w++] = '-';
                prev_dash = 1;
            }
            continue;
        }
        char c = (char)tolower((unsigned char)*p);
        if (!is_valid_cmd_char(c)) return 0;
        if (w + 1 >= name_cap) return 0;
        out_name[w++] = c;
        prev_dash = (c == '-');
    }
    while (w > 0 && out_name[w - 1] == '-') w--;
    out_name[w] = '\0';
    return w > 0;
}

static int tldr_page_exists(const char *tldr_root, const char *platform,
                            const char *name, char *out_path, size_t path_cap)
{
    if (tldr_root == NULL || platform == NULL || name == NULL) return 0;
    char path[1024];
    int n = snprintf(path, sizeof(path), "%s/pages/%s/%s.md", tldr_root, platform, name);
    if (n <= 0 || (size_t)n >= sizeof(path)) return 0;
    if (!tam_fs_file_exists(path)) return 0;
    if (out_path != NULL && path_cap > 0) {
        snprintf(out_path, path_cap, "%s", path);
    }
    return 1;
}

static int tldr_resolve_page(const char *tldr_root, const char *req_platform,
                             const char *name, char *resolved_platform,
                             size_t plat_cap, char *out_path, size_t path_cap)
{
    static const char *const default_platforms[] = {"linux", "common"};
    if (tldr_root == NULL || name == NULL || name[0] == '\0') return 0;

    if (req_platform != NULL && req_platform[0] != '\0') {
        if (tldr_page_exists(tldr_root, req_platform, name, out_path, path_cap)) {
            snprintf(resolved_platform, plat_cap, "%s", req_platform);
            return 1;
        }
        return 0;
    }

    for (size_t i = 0; i < sizeof(default_platforms) / sizeof(default_platforms[0]); i++) {
        if (tldr_page_exists(tldr_root, default_platforms[i], name, out_path, path_cap)) {
            snprintf(resolved_platform, plat_cap, "%s", default_platforms[i]);
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Markdown-to-HTML Conversion & Hyperlinking                                 */
/* ------------------------------------------------------------------------- */

/* Format a tldr command example line (`...`):
 * - Unescapes \{\{ and \}\}
 * - Formats {{[-s|--long]}} option placeholders as [-s|--long]
 * - Formats {{placeholder}} (including nested single braces like {{stash@{0}}})
 *   as <em>placeholder</em> */
static tam_str_t format_tldr_code_inner(tam_str_t out, const char *code, size_t len)
{
    size_t i = 0;
    while (i < len) {
        if (i + 3 < len && code[i] == '\\' && code[i + 1] == '{' &&
            code[i + 2] == '\\' && code[i + 3] == '{') {
            out = tam_str_cat(out, "{{");
            i += 4;
            continue;
        }
        if (i + 3 < len && code[i] == '\\' && code[i + 1] == '}' &&
            code[i + 2] == '\\' && code[i + 3] == '}') {
            out = tam_str_cat(out, "}}");
            i += 4;
            continue;
        }
        if (i + 1 < len && code[i] == '{' && code[i + 1] == '{') {
            size_t start = i + 2;
            size_t end = start;
            int found = 0;
            while (end + 1 < len) {
                if (code[end] == '}' && code[end + 1] == '}') {
                    /* Support inner single braces like {{stash@{0}}} */
                    if (end + 2 < len && code[end + 2] == '}') {
                        end++;
                    }
                    found = 1;
                    break;
                }
                end++;
            }
            if (found) {
                size_t plen = end - start;
                if (plen >= 3 && code[start] == '[' && code[end - 1] == ']' &&
                    memchr(code + start, '|', plen) != NULL) {
                    /* Option placeholder {{[-s|--long]}}: display both forms [-s|--long] */
                    out = tam_html_cat_len(out, code + start, plen);
                } else {
                    out = tam_str_cat(out, "<em>");
                    out = tam_html_cat_len(out, code + start, plen);
                    out = tam_str_cat(out, "</em>");
                }
                i = end + 2;
                continue;
            }
        }
        size_t next = i + 1;
        while (next < len) {
            if (next + 1 < len && code[next] == '{' && code[next + 1] == '{') break;
            if (next + 3 < len && code[next] == '\\' &&
                (code[next + 1] == '{' || code[next + 1] == '}')) break;
            next++;
        }
        out = tam_html_cat_len(out, code + i, next - i);
        i = next;
    }
    return out;
}

/* Check if a command example is itself a `tldr ...` invocation (used in subcommand
 * overview pages like `git` or `pacman`, or alias pages) and return target spec. */
static int detect_tldr_invocation(const char *code, size_t len,
                                  const char *tldr_root,
                                  char *out_href, size_t href_cap)
{
    char buf[256];
    if (len >= sizeof(buf)) return 0;
    memcpy(buf, code, len);
    buf[len] = '\0';
    if (strncmp(buf, "tldr ", 5) != 0) return 0;
    if (strstr(buf, "{{") != NULL) return 0;

    const char *p = buf + 5;
    while (*p == ' ') p++;
    /* Skip optional -p/--platform flag if present in `tldr -p linux cmd` */
    if ((strncmp(p, "-p ", 3) == 0 || strncmp(p, "--platform ", 11) == 0)) {
        p = strchr(p, ' ');
        if (p == NULL) return 0;
        while (*p == ' ') p++;
        p = strchr(p, ' ');
        if (p == NULL) return 0;
        while (*p == ' ') p++;
    }
    char plat[32], norm[128], path[1024];
    if (!normalize_tldr_spec(p, plat, sizeof(plat), norm, sizeof(norm))) return 0;
    if (!tldr_resolve_page(tldr_root, NULL, norm, plat, sizeof(plat), path, sizeof(path))) {
        return 0;
    }
    snprintf(out_href, href_cap, "tldr:%s", norm);
    return 1;
}

/* Format inline text (in blockquotes or bullet descriptions), converting
 * `backticked` spans into <code>...</code> (or hyperlinks when linkify_cmds is set),
 * and stripping <...> around URLs. */
static tam_str_t format_inline_markdown(tam_str_t out, const char *line,
                                        const char *tldr_root, int linkify_cmds)
{
    size_t len = strlen(line);
    size_t i = 0;
    while (i < len) {
        if (line[i] == '`') {
            const char *close = strchr(line + i + 1, '`');
            if (close != NULL) {
                size_t clen = (size_t)(close - (line + i + 1));
                char token[160];
                if (linkify_cmds && clen > 0 && clen < sizeof(token)) {
                    memcpy(token, line + i + 1, clen);
                    token[clen] = '\0';
                    /* Strip any args if "alias of `cmd -flag`" */
                    char *sp_flag = strstr(token, " -");
                    char lookup_tok[160];
                    if (sp_flag != NULL) {
                        size_t blen = (size_t)(sp_flag - token);
                        memcpy(lookup_tok, token, blen);
                        lookup_tok[blen] = '\0';
                    } else {
                        snprintf(lookup_tok, sizeof(lookup_tok), "%s", token);
                    }
                    char plat[32], norm[128], path[1024];
                    if (normalize_tldr_spec(lookup_tok, plat, sizeof(plat), norm, sizeof(norm))) {
                        if (tldr_resolve_page(tldr_root, NULL, norm, plat, sizeof(plat), path, sizeof(path))) {
                            out = tam_str_cat_printf(out, "<a href=\"tldr:%s\"><code>", norm);
                            out = tam_html_cat_len(out, line + i + 1, clen);
                            out = tam_str_cat(out, "</code></a>");
                            i = (size_t)(close - line) + 1;
                            continue;
                        }
                        char *man_path = tam_man_resolve_path(norm, NULL);
                        if (man_path != NULL) {
                            free(man_path);
                            out = tam_str_cat_printf(out, "<a href=\"man:%s\"><code>", norm);
                            out = tam_html_cat_len(out, line + i + 1, clen);
                            out = tam_str_cat(out, "</code></a>");
                            i = (size_t)(close - line) + 1;
                            continue;
                        }
                    }
                }
                out = tam_str_cat(out, "<code>");
                out = format_tldr_code_inner(out, line + i + 1, clen);
                out = tam_str_cat(out, "</code>");
                i = (size_t)(close - line) + 1;
                continue;
            }
        }
        if (line[i] == '<' && (strncmp(line + i + 1, "http://", 7) == 0 ||
                               strncmp(line + i + 1, "https://", 8) == 0)) {
            const char *close = strchr(line + i + 1, '>');
            if (close != NULL) {
                size_t ulen = (size_t)(close - (line + i + 1));
                out = tam_html_cat_len(out, line + i + 1, ulen);
                i = (size_t)(close - line) + 1;
                continue;
            }
        }
        size_t next = i + 1;
        while (next < len && line[next] != '`' && line[next] != '<') next++;
        out = tam_html_cat_len(out, line + i, next - i);
        i = next;
    }
    return out;
}

static tam_document_t *render_tldr_markdown(const char *tldr_root,
                                            const char *platform,
                                            const char *name,
                                            const char *md,
                                            int width)
{
    char *esc_name = tam_html_escape(name);
    char *esc_plat = tam_html_escape(platform);
    tam_str_t html = tam_page_begin(name, NULL, NULL, 0);

    int seen_h1 = 0;
    int in_dl = 0;

    const char *p = md;
    while (*p != '\0') {
        const char *eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        while (len > 0 && (p[len - 1] == '\r' || p[len - 1] == ' ' || p[len - 1] == '\t')) {
            len--;
        }
        char *line = strndup(p, len);
        if (line == NULL) break;

        const char *s = line;
        while (*s == ' ' || *s == '\t') s++;

        if (s[0] == '\0') {
            free(line);
            if (eol == NULL) break;
            p = eol + 1;
            continue;
        }

        if (!seen_h1 && strncmp(s, "# ", 2) == 0) {
            seen_h1 = 1;
            char *h1_esc = tam_html_escape(s + 2);
            html = tam_str_cat_printf(html, "<h1>%s (%s)</h1>\n",
                                      h1_esc ? h1_esc : name,
                                      esc_plat ? esc_plat : platform);
            free(h1_esc);
        } else if (s[0] == '>') {
            if (in_dl) {
                html = tam_str_cat(html, "</dl>\n");
                in_dl = 0;
            }
            const char *q = s + 1;
            while (*q == ' ' || *q == '\t') q++;
            int linkify = (strncasecmp(q, "See also:", 9) == 0 ||
                           strstr(q, "alias of") != NULL);
            html = tam_str_cat(html, "<p>");
            html = format_inline_markdown(html, q, tldr_root, linkify);
            html = tam_str_cat(html, "</p>\n");
        } else if (strncmp(s, "- ", 2) == 0) {
            if (!in_dl) {
                html = tam_str_cat(html, "<dl>\n");
                in_dl = 1;
            }
            html = tam_str_cat(html, "<dt>• ");
            html = format_inline_markdown(html, s + 2, tldr_root, 0);
            html = tam_str_cat(html, "</dt>\n");
        } else if (s[0] == '`' && len >= 2 && s[strlen(s) - 1] == '`') {
            if (!in_dl) {
                html = tam_str_cat(html, "<dl>\n");
                in_dl = 1;
            }
            size_t clen = strlen(s) - 2;
            const char *code = s + 1;
            char sub_href[180];
            html = tam_str_cat(html, "<pre>");
            if (detect_tldr_invocation(code, clen, tldr_root, sub_href, sizeof(sub_href))) {
                html = tam_str_cat_printf(html, "<a href=\"%s\"><code>", sub_href);
                html = format_tldr_code_inner(html, code, clen);
                html = tam_str_cat(html, "</code></a>");
            } else {
                html = tam_str_cat(html, "<code>");
                html = format_tldr_code_inner(html, code, clen);
                html = tam_str_cat(html, "</code>");
            }
            html = tam_str_cat(html, "</pre>\n");
        } else {
            if (in_dl) {
                html = tam_str_cat(html, "</dl>\n");
                in_dl = 0;
            }
            html = tam_str_cat(html, "<p>");
            html = format_inline_markdown(html, s, tldr_root, 0);
            html = tam_str_cat(html, "</p>\n");
        }

        free(line);
        if (eol == NULL) break;
        p = eol + 1;
    }

    if (in_dl) {
        html = tam_str_cat(html, "</dl>\n");
    }

    /* Cross-links to alternate platform variant and full manual page */
    const char *alt_plat = (strcmp(platform, "linux") == 0) ? "common" : "linux";
    int has_alt = tldr_page_exists(tldr_root, alt_plat, name, NULL, 0);

    char man_target[128] = {0};
    char *man_path = tam_man_resolve_path(name, NULL);
    if (man_path != NULL) {
        free(man_path);
        snprintf(man_target, sizeof(man_target), "%s", name);
    } else {
        const char *dash = strchr(name, '-');
        if (dash != NULL) {
            char base[64];
            size_t blen = (size_t)(dash - name);
            if (blen > 0 && blen < sizeof(base)) {
                memcpy(base, name, blen);
                base[blen] = '\0';
                man_path = tam_man_resolve_path(base, NULL);
                if (man_path != NULL) {
                    free(man_path);
                    snprintf(man_target, sizeof(man_target), "%s", base);
                }
            }
        }
    }

    html = tam_str_cat(html, "<hr>\n<p>");
    if (man_target[0] != '\0') {
        char *esc_man = tam_html_escape(man_target);
        html = tam_str_cat_printf(html,
            "Full manual: <a href=\"man:%s\">%s</a> · ",
            esc_man ? esc_man : man_target, esc_man ? esc_man : man_target);
        free(esc_man);
    }
    if (has_alt) {
        html = tam_str_cat_printf(html,
            "Other platform variant: <a href=\"tldr:%s/%s\">%s/%s</a> · ",
            alt_plat, esc_name ? esc_name : name, alt_plat, esc_name ? esc_name : name);
    }
    html = tam_str_cat(html,
        "<a href=\"tam:tldr\">tldr index</a> · "
        "<a href=\"index.html\">Tamlinux Guide</a></p>\n");

    free(esc_name);
    free(esc_plat);
    return tam_page_render(html, width);
}

tam_document_t *tam_tldr_load(const char *spec, int width)
{
    char req_plat[32], name[128];
    if (!normalize_tldr_spec(spec, req_plat, sizeof(req_plat), name, sizeof(name))) {
        return NULL;
    }

    char *tldr_root = tam_find_tldr_root(NULL);
    if (tldr_root == NULL) {
        return NULL;
    }

    char plat[32], path[1024];
    if (!tldr_resolve_page(tldr_root, req_plat[0] ? req_plat : NULL, name,
                           plat, sizeof(plat), path, sizeof(path))) {
        free(tldr_root);
        return NULL;
    }

    size_t len = 0;
    char *md = tam_fs_read_file(path, &len);
    if (md == NULL) {
        free(tldr_root);
        return NULL;
    }

    tam_document_t *doc = render_tldr_markdown(tldr_root, plat, name, md, width);
    free(md);
    free(tldr_root);
    return doc;
}

/* ------------------------------------------------------------------------- */
/* Index Loading, Browsing & Keyword Search (tam:tldr*)                       */
/* ------------------------------------------------------------------------- */

typedef struct {
    char *name;
    char *platform;
    char *summary;
} tldr_entry_t;

typedef struct {
    tldr_entry_t *entries;
    size_t count;
} tldr_index_t;

static void tldr_index_free(tldr_index_t *idx)
{
    if (idx == NULL) return;
    for (size_t i = 0; i < idx->count; i++) {
        free(idx->entries[i].name);
        free(idx->entries[i].platform);
        free(idx->entries[i].summary);
    }
    free(idx->entries);
    free(idx);
}

static tldr_index_t *tldr_index_load(const char *tldr_root)
{
    if (tldr_root == NULL) return NULL;
    char path[1024];
    snprintf(path, sizeof(path), "%s/index.tsv", tldr_root);
    size_t len = 0;
    char *raw = tam_fs_read_file(path, &len);
    if (raw == NULL) return NULL;

    tldr_index_t *idx = calloc(1, sizeof(*idx));
    if (idx == NULL) {
        free(raw);
        return NULL;
    }

    size_t cap = 256;
    idx->entries = calloc(cap, sizeof(*idx->entries));
    if (idx->entries == NULL) {
        free(idx);
        free(raw);
        return NULL;
    }

    const char *p = raw;
    int first_line = 1;
    while (*p != '\0') {
        const char *eol = strchr(p, '\n');
        size_t l = eol ? (size_t)(eol - p) : strlen(p);
        while (l > 0 && p[l - 1] == '\r') l--;
        if (first_line) {
            first_line = 0;
            if (l >= 5 && strncmp(p, "name\t", 5) == 0) {
                if (eol == NULL) break;
                p = eol + 1;
                continue;
            }
        }
        if (l > 0) {
            const char *t1 = memchr(p, '\t', l);
            if (t1 != NULL) {
                size_t rem = l - (size_t)(t1 + 1 - p);
                const char *t2 = memchr(t1 + 1, '\t', rem);
                if (t2 != NULL) {
                    if (idx->count == cap) {
                        cap *= 2;
                        tldr_entry_t *grown = realloc(idx->entries, cap * sizeof(*grown));
                        if (grown == NULL) break;
                        idx->entries = grown;
                    }
                    tldr_entry_t *e = &idx->entries[idx->count++];
                    e->name = strndup(p, (size_t)(t1 - p));
                    e->platform = strndup(t1 + 1, (size_t)(t2 - (t1 + 1)));
                    e->summary = strndup(t2 + 1, (size_t)((p + l) - (t2 + 1)));
                }
            }
        }
        if (eol == NULL) break;
        p = eol + 1;
    }

    free(raw);
    return idx;
}

static int contains_icase(const char *haystack, const char *needle)
{
    if (haystack == NULL || needle == NULL || needle[0] == '\0') return 0;
    size_t nlen = strlen(needle);
    for (const char *h = haystack; *h != '\0'; h++) {
        if (strncasecmp(h, needle, nlen) == 0) return 1;
    }
    return 0;
}

static int score_tldr_entry(const tldr_entry_t *e, const char *query)
{
    if (e == NULL || query == NULL || query[0] == '\0') return 0;
    char qbuf[160];
    snprintf(qbuf, sizeof(qbuf), "%s", query);

    /* Exact or normalized match on full query */
    char plat[32], norm[128];
    if (normalize_tldr_spec(query, plat, sizeof(plat), norm, sizeof(norm))) {
        if (strcasecmp(e->name, norm) == 0) {
            return (strcmp(e->platform, "linux") == 0) ? 1000 : 950;
        }
        if (strncasecmp(e->name, norm, strlen(norm)) == 0) {
            return 500;
        }
    }

    /* Multi-token check: all tokens must appear in name or summary */
    int total = 0;
    char *save = NULL;
    char *tok = strtok_r(qbuf, " \t", &save);
    if (tok == NULL) return 0;
    while (tok != NULL) {
        int tok_score = 0;
        if (strcasecmp(e->name, tok) == 0) {
            tok_score = 300;
        } else if (strncasecmp(e->name, tok, strlen(tok)) == 0) {
            tok_score = 150;
        } else if (contains_icase(e->name, tok)) {
            tok_score = 80;
        } else if (contains_icase(e->summary, tok)) {
            tok_score = 30;
        } else {
            return 0;
        }
        total += tok_score;
        tok = strtok_r(NULL, " \t", &save);
    }
    return total;
}

static tam_document_t *tldr_html_document(const char *title, const char *body,
                                          int width)
{
    tam_str_t html = tam_page_begin(title ? title : "tldr pages", NULL, NULL, 0);
    html = tam_str_cat(html, "<h1>");
    html = tam_html_cat(html, title ? title : "tldr pages");
    html = tam_str_cat(html, "</h1>\n");
    html = tam_str_cat(html, body ? body : "");
    return tam_page_render(html, width);
}

static tam_str_t append_az_bar(tam_str_t body, const char *platform)
{
    body = tam_str_cat(body, "<p>Browse by letter: <a href=\"tam:tldr:list\">All</a>");
    for (char c = 'a'; c <= 'z'; c++) {
        char upper = (char)toupper((unsigned char)c);
        if (platform != NULL && platform[0] != '\0') {
            body = tam_str_cat_printf(body,
                " <a href=\"tam:tldr:list?letter=%c&amp;platform=%s\">%c</a>",
                c, platform, upper);
        } else {
            body = tam_str_cat_printf(body,
                " <a href=\"tam:tldr:list?letter=%c\">%c</a>", c, upper);
        }
    }
    body = tam_str_cat(body, " <a href=\"tam:tldr:list?letter=0\">0-9</a></p>\n");
    return body;
}

static tam_document_t *tldr_landing_document(const char *tldr_root, int width)
{
    tldr_index_t *idx = tldr_index_load(tldr_root);
    size_t total = idx ? idx->count : 0;
    tldr_index_free(idx);

    tam_str_t body = tam_str_new(
        "<p>Concise, example-first command cheat sheets from the upstream "
        "<code>tldr-pages</code> project, curated for Tamlinux and available "
        "offline. For complete flag references use <a href=\"tam:man\">Man pages</a>; "
        "for book-length GNU manuals use <a href=\"info:dir\">Info manuals</a>; "
        "for conceptual guides use <a href=\"tam:search\">Guide Search</a>.</p>\n"
        "<ul>\n"
        "<li><a href=\"tam:prompt:tldr-open\">Open a tldr page</a> by command name (for example <code>tar</code> or <code>git commit</code>).</li>\n"
        "<li><a href=\"tam:prompt:tldr-keyword\">Find by keyword</a> across command names and descriptions.</li>\n");
    if (total > 0) {
        body = tam_str_cat_printf(body,
            "<li><a href=\"tam:tldr:list\">Browse all %zu curated tldr pages</a> in the directory index.</li>\n",
            total);
    } else {
        body = tam_str_cat(body,
            "<li><a href=\"tam:tldr:list\">Browse all curated tldr pages</a> in the directory index.</li>\n");
    }
    body = tam_str_cat(body, "</ul>\n<h2>Browse index</h2>\n");
    body = append_az_bar(body, NULL);
    body = tam_str_cat(body,
        "<p>Filter by section: "
        "<a href=\"tam:tldr:list?platform=linux\">Linux commands</a> · "
        "<a href=\"tam:tldr:list?platform=common\">Common commands</a></p>\n"
        "<h2>Starting points</h2>\n<ul>\n");

    static const struct { const char *name; const char *desc; } starts[] = {
        {"tar", "Archiving utility"},
        {"grep", "Find patterns in files using regular expressions"},
        {"find", "Find files or directories under a directory tree"},
        {"git", "Distributed version control system"},
        {"systemctl", "Control the systemd system and service manager"},
        {"pacman", "Arch Linux package manager utility"},
        {"btrfs", "Btrfs filesystem management"},
        {"hyprctl", "Control the Hyprland Wayland compositor"},
    };
    int any_start = 0;
    for (size_t i = 0; i < sizeof(starts) / sizeof(starts[0]); i++) {
        char plat[32], path[1024];
        if (tldr_resolve_page(tldr_root, NULL, starts[i].name, plat, sizeof(plat), path, sizeof(path))) {
            body = tam_str_cat_printf(body,
                "<li><a href=\"tldr:%s\">%s</a> — %s</li>\n",
                starts[i].name, starts[i].name, starts[i].desc);
            any_start++;
        }
    }
    if (!any_start) {
        body = tam_str_cat(body, "<li>Use Open a tldr page or Browse index above.</li>\n");
    }
    body = tam_str_cat(body, "</ul>\n");

    tam_document_t *doc = tldr_html_document("tldr pages", body, width);
    tam_str_free(body);
    return doc;
}

static tam_document_t *tldr_error_document(const char *title, const char *msg, int width)
{
    char *esc = tam_html_escape(msg);
    tam_str_t body = tam_str_new("<p>");
    body = tam_str_cat(body, esc ? esc : "tldr lookup failed.");
    body = tam_str_cat(body, "</p>\n<p><a href=\"tam:tldr\">Return to tldr pages</a> · "
                             "<a href=\"index.html\">Tamlinux Guide</a></p>\n");
    free(esc);
    tam_document_t *doc = tldr_html_document(title, body, width);
    tam_str_free(body);
    return doc;
}

static tam_document_t *tldr_list_document(const char *tldr_root, const char *uri, int width)
{
    tldr_index_t *idx = tldr_index_load(tldr_root);
    if (idx == NULL) {
        return tldr_error_document("tldr index",
            "The curated tldr index is unavailable.", width);
    }

    char *letter = uri_parameter(uri, "letter");
    char *platform = uri_parameter(uri, "platform");
    char *offset_str = uri_parameter(uri, "offset");
    size_t offset = 0;
    if (offset_str != NULL) {
        unsigned long v = strtoul(offset_str, NULL, 10);
        if (v <= 100000) offset = (size_t)v;
        free(offset_str);
    }

    char title[128];
    if (letter != NULL && letter[0] != '\0' && platform != NULL && platform[0] != '\0') {
        snprintf(title, sizeof(title), "tldr index: %s (%c)", platform,
                 toupper((unsigned char)letter[0]));
    } else if (letter != NULL && letter[0] != '\0') {
        if (letter[0] == '0') {
            snprintf(title, sizeof(title), "tldr index: 0-9");
        } else {
            snprintf(title, sizeof(title), "tldr index: %c",
                     toupper((unsigned char)letter[0]));
        }
    } else if (platform != NULL && platform[0] != '\0') {
        snprintf(title, sizeof(title), "tldr index: %s", platform);
    } else {
        snprintf(title, sizeof(title), "tldr index");
    }

    tam_str_t body = tam_str_new("");
    body = append_az_bar(body, platform);

    /* Count matching entries and deduplicate linux+common when platform is not filtered */
    size_t matched = 0;
    size_t shown = 0;
    body = tam_str_cat(body, "<table><tr><th>Command</th><th>Section</th><th>Description</th></tr>\n");

    for (size_t i = 0; i < idx->count; i++) {
        const tldr_entry_t *e = &idx->entries[i];
        if (platform != NULL && platform[0] != '\0' &&
            strcasecmp(e->platform, platform) != 0) {
            continue;
        }
        if (letter != NULL && letter[0] != '\0') {
            char c0 = (char)tolower((unsigned char)e->name[0]);
            if (letter[0] == '0') {
                if (!isdigit((unsigned char)c0)) continue;
            } else if (c0 != tolower((unsigned char)letter[0])) {
                continue;
            }
        }
        if (matched >= offset && shown < TLDR_RESULTS_PER_PAGE) {
            char *n_esc = tam_html_escape(e->name);
            char *p_esc = tam_html_escape(e->platform);
            char *s_esc = tam_html_escape(e->summary);
            body = tam_str_cat_printf(body,
                "<tr><td><a href=\"tldr:%s/%s\">%s</a></td><td>%s</td><td>%s</td></tr>\n",
                p_esc ? p_esc : "", n_esc ? n_esc : "", n_esc ? n_esc : "",
                p_esc ? p_esc : "", s_esc ? s_esc : "");
            free(n_esc); free(p_esc); free(s_esc);
            shown++;
        }
        matched++;
    }
    body = tam_str_cat(body, "</table>\n");

    if (matched == 0) {
        body = tam_str_cat(body, "<p>No curated tldr pages matched this filter.</p>\n");
    } else {
        size_t end_num = offset + shown;
        body = tam_str_cat_printf(body, "<p>Showing %zu–%zu of %zu pages.",
                                  offset + 1, end_num, matched);
        if (offset + shown < matched) {
            tam_str_t next_uri = tam_str_new("tam:tldr:list?");
            if (letter != NULL && letter[0] != '\0') {
                next_uri = tam_str_cat_printf(next_uri, "letter=%c&amp;", letter[0]);
            }
            if (platform != NULL && platform[0] != '\0') {
                char *enc_p = uri_encode(platform);
                next_uri = tam_str_cat_printf(next_uri, "platform=%s&amp;", enc_p ? enc_p : "");
                free(enc_p);
            }
            next_uri = tam_str_cat_printf(next_uri, "offset=%zu", offset + TLDR_RESULTS_PER_PAGE);
            body = tam_str_cat_printf(body, " <a href=\"%s\">More pages</a>", next_uri);
            tam_str_free(next_uri);
        }
        body = tam_str_cat(body, "</p>\n");
    }
    body = tam_str_cat(body,
        "<p><a href=\"tam:prompt:tldr-open\">Open a tldr page</a> · "
        "<a href=\"tam:prompt:tldr-keyword\">Find by keyword</a> · "
        "<a href=\"tam:tldr\">Return to tldr pages</a></p>\n");

    free(letter);
    free(platform);
    tldr_index_free(idx);

    tam_document_t *doc = tldr_html_document(title, body, width);
    tam_str_free(body);
    return doc;
}

typedef struct {
    const tldr_entry_t *entry;
    int score;
} scored_tldr_t;

static int cmp_scored_tldr(const void *a, const void *b)
{
    const scored_tldr_t *sa = a;
    const scored_tldr_t *sb = b;
    if (sb->score != sa->score) return sb->score - sa->score;
    return strcmp(sa->entry->name, sb->entry->name);
}

static tam_document_t *tldr_keyword_document(const char *tldr_root, const char *query,
                                             size_t offset, int width)
{
    if (query == NULL || query[0] == '\0') {
        return tldr_error_document("Find a tldr page by keyword",
            "Enter a command name or keyword to search the curated tldr index.", width);
    }

    tldr_index_t *idx = tldr_index_load(tldr_root);
    if (idx == NULL) {
        return tldr_error_document("Find a tldr page by keyword",
            "The curated tldr index could not be loaded.", width);
    }

    scored_tldr_t *hits = calloc(idx->count ? idx->count : 1, sizeof(*hits));
    size_t nhits = 0;
    if (hits != NULL) {
        for (size_t i = 0; i < idx->count; i++) {
            int s = score_tldr_entry(&idx->entries[i], query);
            if (s > 0) {
                hits[nhits].entry = &idx->entries[i];
                hits[nhits].score = s;
                nhits++;
            }
        }
        qsort(hits, nhits, sizeof(*hits), cmp_scored_tldr);
    }

    char *esc_q = tam_html_escape(query);
    char title[180];
    snprintf(title, sizeof(title), "tldr keyword: %s", query);

    tam_str_t body = tam_str_new("");
    body = tam_str_cat_printf(body, "<p>Matches for <strong>%s</strong>:</p>\n",
                              esc_q ? esc_q : "");
    free(esc_q);

    if (nhits == 0) {
        body = tam_str_cat(body,
            "<p>No curated tldr pages matched this keyword.</p>\n");
    } else {
        body = tam_str_cat(body, "<table><tr><th>Command</th><th>Section</th><th>Description</th></tr>\n");
        size_t shown = 0;
        for (size_t i = offset; i < nhits && shown < TLDR_RESULTS_PER_PAGE; i++) {
            const tldr_entry_t *e = hits[i].entry;
            char *n_esc = tam_html_escape(e->name);
            char *p_esc = tam_html_escape(e->platform);
            char *s_esc = tam_html_escape(e->summary);
            body = tam_str_cat_printf(body,
                "<tr><td><a href=\"tldr:%s/%s\">%s</a></td><td>%s</td><td>%s</td></tr>\n",
                p_esc ? p_esc : "", n_esc ? n_esc : "", n_esc ? n_esc : "",
                p_esc ? p_esc : "", s_esc ? s_esc : "");
            free(n_esc); free(p_esc); free(s_esc);
            shown++;
        }
        body = tam_str_cat(body, "</table>\n");
        if (offset + shown < nhits) {
            char *enc_q = uri_encode(query);
            body = tam_str_cat_printf(body,
                "<p><a href=\"tam:tldr:keyword?query=%s&amp;offset=%zu\">More results</a></p>\n",
                enc_q ? enc_q : "", offset + TLDR_RESULTS_PER_PAGE);
            free(enc_q);
        }
    }

    body = tam_str_cat(body,
        "<p><a href=\"tam:prompt:tldr-keyword\">Search again</a> · "
        "<a href=\"tam:prompt:tldr-open\">Open a tldr page</a> · "
        "<a href=\"tam:tldr\">Return to tldr pages</a></p>\n");

    free(hits);
    tldr_index_free(idx);

    tam_document_t *doc = tldr_html_document(title, body, width);
    tam_str_free(body);
    return doc;
}

tam_document_t *tam_tldr_uri_document(const char *uri, int width)
{
    char *tldr_root = tam_find_tldr_root(NULL);
    if (uri == NULL || strcmp(uri, "tam:tldr") == 0) {
        tam_document_t *doc = tldr_landing_document(tldr_root, width);
        free(tldr_root);
        return doc;
    }

    if (strncmp(uri, "tam:tldr:list", 13) == 0) {
        tam_document_t *doc = tldr_list_document(tldr_root, uri, width);
        free(tldr_root);
        return doc;
    }

    if (strncmp(uri, "tam:tldr:open?", 14) == 0) {
        char *query = uri_parameter(uri, "query");
        if (query == NULL || query[0] == '\0') {
            free(query);
            free(tldr_root);
            return tldr_error_document("Open a tldr page",
                "Enter a command name, such as tar or git commit.", width);
        }
        tam_document_t *direct = tam_tldr_load(query, width);
        if (direct != NULL) {
            char plat[32], norm[128], href[180];
            if (normalize_tldr_spec(query, plat, sizeof(plat), norm, sizeof(norm))) {
                snprintf(href, sizeof(href), "tldr:%s", norm);
                tam_document_ensure_location(direct, href);
            }
            free(query);
            free(tldr_root);
            return direct;
        }
        tam_document_t *fallback = tldr_keyword_document(tldr_root, query, 0, width);
        free(query);
        free(tldr_root);
        return fallback;
    }

    if (strncmp(uri, "tam:tldr:keyword?", 17) == 0) {
        char *query = uri_parameter(uri, "query");
        char *offset_str = uri_parameter(uri, "offset");
        size_t offset = 0;
        if (offset_str != NULL) {
            unsigned long v = strtoul(offset_str, NULL, 10);
            if (v <= 100000) offset = (size_t)v;
            free(offset_str);
        }
        tam_document_t *doc = tldr_keyword_document(tldr_root, query, offset, width);
        free(query);
        free(tldr_root);
        return doc;
    }

    free(tldr_root);
    return tldr_error_document("tldr pages", "Unsupported tldr lookup request.", width);
}

/* ------------------------------------------------------------------------- */
/* CLI Subcommand: tam tldr                                                   */
/* ------------------------------------------------------------------------- */

static void print_tldr_help(void)
{
    fputs(
        "tam tldr - view concise command-example pages with hyperlinked references\n\n"
        "Usage:\n"
        "  tam tldr                       open the tldr discovery and index page\n"
        "  tam tldr COMMAND [SUBCOMMAND]  open a tldr page in the terminal pager\n"
        "  tam tldr --list                browse the full index of curated tldr pages\n"
        "  tam tldr --search KEYWORD      find tldr pages matching KEYWORD\n"
        "  tam tldr --platform OS CMD     open CMD from a specific section (linux, common)\n"
        "  tam tldr [ARGS] --pipe         print formatted plain text without pager\n"
        "  tam tldr --help                show this help and exit\n\n"
        "Examples:\n"
        "  tam tldr                       browse and search curated tldr pages\n"
        "  tam tldr tar                   view tar examples\n"
        "  tam tldr git commit            view git-commit examples\n"
        "  tam tldr --platform common cat view the POSIX/common cat page\n"
        "  tam tldr tar --pipe            output tar examples as plain text\n",
        stdout);
}

typedef struct {
    char *root;
} tldr_loader_ctx_t;

static tam_document_t *tldr_unified_load(void *vctx, const char *href,
                                         int content_width)
{
    tldr_loader_ctx_t *ctx = vctx;
    if (href == NULL || href[0] == '\0') return NULL;
    if (strncmp(href, "tldr:", 5) == 0) {
        return tam_tldr_load(href + 5, content_width);
    }
    if (strncmp(href, "tam:tldr", 8) == 0) {
        return tam_tldr_uri_document(href, content_width);
    }
    if (strncmp(href, "man:", 4) == 0) {
        if (href[4] == '\0') return tam_man_uri_document("tam:man", content_width);
        return tam_man_load(href + 4, content_width);
    }
    if (strncmp(href, "tam:man", 7) == 0) {
        return tam_man_uri_document(href, content_width);
    }
    if (strncmp(href, "info:", 5) == 0) {
        return tam_info_load(href + 5, content_width);
    }
    if (strncmp(href, "tam:search", 10) == 0) {
        return tam_guide_search_uri_document(ctx->root, href, content_width);
    }
    if (ctx->root != NULL) {
        if (strncmp(href, "tam:topics", 10) == 0) {
            unsigned level = 0;
            const char *q = strstr(href, "?level=");
            if (q != NULL) level = tam_level_parse(q + 7);
            return tam_topics_document(ctx->root, level, content_width);
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", ctx->root, href);
        size_t len = 0;
        char *src = tam_fs_read_file(path, &len);
        if (src != NULL) {
            tam_document_t *doc = tam_render_html(src, len, content_width);
            free(src);
            return doc;
        }
    }
    return NULL;
}

int tam_cmd_tldr(int n, char **args)
{
    int pipe = 0;
    int list_mode = 0;
    const char *platform = NULL;
    const char *search_kw = NULL;
    char cmd_buf[160] = {0};
    size_t cmd_len = 0;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0) {
            pipe = 1;
        } else if (strcmp(args[i], "-p") == 0) {
            if (i + 2 < n && (strcmp(args[i + 1], "linux") == 0 ||
                              strcmp(args[i + 1], "common") == 0)) {
                platform = args[++i];
            } else {
                pipe = 1;
            }
        } else if (strcmp(args[i], "--platform") == 0 ||
                   strcmp(args[i], "-L") == 0) {
            if (i + 1 >= n || args[i + 1][0] == '\0') {
                fprintf(stderr, TAM_COMMAND_NAME ": --platform requires a value\n");
                return TAM_EXIT_USAGE;
            }
            platform = args[++i];
        } else if (strncmp(args[i], "--platform=", 11) == 0) {
            platform = args[i] + 11;
            if (platform[0] == '\0') {
                fprintf(stderr, TAM_COMMAND_NAME ": --platform requires a value\n");
                return TAM_EXIT_USAGE;
            }
        } else if (strcmp(args[i], "--list") == 0 || strcmp(args[i], "-l") == 0) {
            list_mode = 1;
        } else if (strcmp(args[i], "--search") == 0 || strcmp(args[i], "--keyword") == 0) {
            if (i + 1 >= n || args[i + 1][0] == '\0') {
                fprintf(stderr, TAM_COMMAND_NAME ": %s requires a keyword\n", args[i]);
                return TAM_EXIT_USAGE;
            }
            search_kw = args[++i];
        } else if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0 ||
                   (cmd_len == 0 && strcmp(args[i], "help") == 0)) {
            print_tldr_help();
            return 0;
        } else if (args[i][0] == '-') {
            fprintf(stderr, TAM_COMMAND_NAME ": unrecognized argument '%s'\n", args[i]);
            fprintf(stderr, "Run '%s tldr --help' for available options.\n", TAM_COMMAND_NAME);
            return TAM_EXIT_USAGE;
        } else {
            if (cmd_len > 0 && cmd_len + 1 < sizeof(cmd_buf)) {
                cmd_buf[cmd_len++] = '-';
            }
            size_t alen = strlen(args[i]);
            if (cmd_len + alen >= sizeof(cmd_buf)) {
                fprintf(stderr, TAM_COMMAND_NAME ": command name too long\n");
                return TAM_EXIT_USAGE;
            }
            memcpy(cmd_buf + cmd_len, args[i], alen);
            cmd_len += alen;
            cmd_buf[cmd_len] = '\0';
        }
    }

    int interactive = !pipe && isatty(STDOUT_FILENO);
    int width = interactive ? tam_terminal_width() : 80;

    if (list_mode || search_kw != NULL || cmd_len == 0) {
        char uri[256];
        if (search_kw != NULL) {
            char *enc = uri_encode(search_kw);
            snprintf(uri, sizeof(uri), "tam:tldr:keyword?query=%s", enc ? enc : "");
            free(enc);
        } else if (list_mode) {
            if (platform != NULL && platform[0] != '\0') {
                char *enc = uri_encode(platform);
                snprintf(uri, sizeof(uri), "tam:tldr:list?platform=%s", enc ? enc : "");
                free(enc);
            } else {
                snprintf(uri, sizeof(uri), "tam:tldr:list");
            }
        } else {
            snprintf(uri, sizeof(uri), "tam:tldr");
        }

        tam_document_t *doc = tam_tldr_uri_document(uri, width);
        if (doc == NULL) return TAM_EXIT_FAILURE;
        tam_document_ensure_location(doc, uri);
        int rc = 0;
        if (interactive) {
            tldr_loader_ctx_t ctx = {.root = tam_find_knowledge_root()};
            rc = tam_pager_run(doc, uri, tldr_unified_load, &ctx);
            free(ctx.root);
        } else {
            char *text = tam_document_text(doc);
            if (text != NULL) { fputs(text, stdout); free(text); }
            tam_document_free(doc);
        }
        return rc;
    }

    char spec[200];
    if (platform != NULL && platform[0] != '\0') {
        snprintf(spec, sizeof(spec), "%s/%s", platform, cmd_buf);
    } else {
        snprintf(spec, sizeof(spec), "%s", cmd_buf);
    }

    tam_document_t *doc = tam_tldr_load(spec, width);
    if (doc == NULL) {
        if (platform != NULL && platform[0] != '\0') {
            fprintf(stderr, TAM_COMMAND_NAME ": no tldr page found for '%s' in section '%s'\n",
                    cmd_buf, platform);
        } else {
            fprintf(stderr, TAM_COMMAND_NAME ": no tldr page found for '%s'\n", cmd_buf);
        }
        return TAM_EXIT_FAILURE;
    }

    char initial_href[220];
    snprintf(initial_href, sizeof(initial_href), "tldr:%s", spec);
    tam_document_ensure_location(doc, initial_href);

    int rc = 0;
    if (interactive) {
        tldr_loader_ctx_t ctx = {.root = tam_find_knowledge_root()};
        rc = tam_pager_run(doc, initial_href, tldr_unified_load, &ctx);
        free(ctx.root);
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
