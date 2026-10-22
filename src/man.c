#include <tam.h>

#include "depth.h"
#include "discovery.h"
#include "html.h"
#include "info.h"
#include "man.h"
#include "pager.h"
#include "render.h"
#include "tldr.h"

#include <ctype.h>
#include <dirent.h>
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

#define MAN_LOOKUP_LIMIT (96u * 1024u)
#define MAN_LOOKUP_TIMEOUT_MS 2500
#define MAN_RESULTS_PER_PAGE 24

typedef struct {
    char *text;
    size_t len;
    int status;
    int timed_out;
    int truncated;
    int unavailable;
} lookup_result_t;

/* man(1) tools run in the C locale and never page their output. */
#define MAN_TOOL_ENV "env", "LC_ALL=C", "PAGER=cat", "MANPAGER=cat"

/* Run a man(1) tool (argv without the environment prefix) with standard
 * input from /dev/null, a hard deadline and a strict output cap. User data is
 * always one literal argv element; no shell is involved. */
static lookup_result_t run_lookup(const char *const argv[])
{
    lookup_result_t result = {.status = -1};
    const char *full[16] = { MAN_TOOL_ENV };
    size_t n = 4;
    for (size_t i = 0; argv != NULL && argv[i] != NULL && n + 1 < sizeof(full) / sizeof(full[0]); i++)
        full[n++] = argv[i];
    full[n] = NULL;
    if (argv == NULL || argv[0] == NULL) {
        result.unavailable = 1;
        return result;
    }
    tam_exec_opts_t opts = { .timeout_ms = MAN_LOOKUP_TIMEOUT_MS,
                             .max_output = MAN_LOOKUP_LIMIT };
    tam_exec_result_t r = tam_exec_opts(full, &opts);
    free(r.stderr_buf);
    result.text = r.stdout_buf;
    result.len = r.stdout_len;
    result.status = r.exit_code;
    result.timed_out = r.timed_out;
    result.truncated = r.truncated;
    result.unavailable = r.stdout_buf == NULL || r.exit_code == 127;
    return result;
}

static char *uri_decode(const char *text)
{
    size_t n = strlen(text);
    char *out = malloc(n + 1);
    if (out == NULL) return NULL;
    size_t used = 0;
    for (size_t i = 0; i < n; i++) {
        if (text[i] == '%' && i + 2 < n && isxdigit((unsigned char)text[i + 1]) &&
            isxdigit((unsigned char)text[i + 2])) {
            char hex[3] = {text[i + 1], text[i + 2], 0};
            out[used++] = (char)strtoul(hex, NULL, 16);
            i += 2;
        } else if (text[i] == '+') out[used++] = ' ';
        else out[used++] = text[i];
    }
    out[used] = '\0';
    return out;
}

static char *uri_encode(const char *text)
{
    static const char hex[] = "0123456789ABCDEF";
    size_t n = strlen(text);
    char *out = malloc(n * 3 + 1);
    if (out == NULL) return NULL;
    size_t used = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)text[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') out[used++] = (char)c;
        else { out[used++] = '%'; out[used++] = hex[c >> 4]; out[used++] = hex[c & 15]; }
    }
    out[used] = '\0';
    return out;
}

static char *literal_apropos_pattern(const char *text)
{
    static const char special[] = ".^$*+?()[]{}|\\";
    size_t n = strlen(text);
    char *out = malloc(n * 2 + 1);
    if (out == NULL) return NULL;
    size_t used = 0;
    for (size_t i = 0; i < n; i++) {
        if (strchr(special, text[i]) != NULL) out[used++] = '\\';
        out[used++] = text[i];
    }
    out[used] = '\0';
    return out;
}

static tam_document_t *html_document(const char *title, const char *body, int width)
{
    const tam_crumb_t crumbs[] = {
        { "index.html", "Guide" }, { "tam:man", "Man pages" }, { NULL, title }
    };
    tam_str_t html = tam_page_begin(title, NULL, crumbs, 3);
    html = tam_str_cat(html, "<h1>");
    html = tam_html_cat(html, title);
    html = tam_str_cat(html, "</h1>");
    html = tam_str_cat(html, body);
    return tam_page_render(html, width);
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
    const char *argv[] = { "man", "-w", sec, name, NULL };
    if (sec == NULL || sec[0] == '\0') {
        argv[2] = name;
        argv[3] = NULL;
    }
    lookup_result_t r = run_lookup(argv);
    if (r.status == 0 && r.text != NULL) {
        char *path = r.text;
        path[strcspn(path, "\r\n")] = '\0';
        size_t len = strlen(path);
        while (len > 0 && path[len - 1] == ' ') path[--len] = '\0';
        if (len > 0 && access(path, R_OK) == 0) {
            char *found = strdup(path);
            free(r.text);
            return found;
        }
    }
    free(r.text);

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

/* A private scratch directory for groff's temporary files and any images
 * grohtml writes, removed after each render: under $XDG_RUNTIME_DIR, else
 * the cache directory, else /tmp. */
static char *groff_scratch_dir(void)
{
    char *base = tam_xdg_path(TAM_XDG_RUNTIME, "tam", NULL, true);
    if (base == NULL) base = tam_xdg_path(TAM_XDG_CACHE, "tam", NULL, true);
    char *tmpl = tam_path_join(base != NULL ? base : "/tmp", "groff-XXXXXX");
    free(base);
    if (tmpl != NULL && mkdtemp(tmpl) != NULL) return tmpl;
    free(tmpl);
    return NULL;
}

static void remove_scratch_dir(const char *dir)
{
    DIR *d = opendir(dir);
    if (d != NULL) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            char *p = tam_path_join(dir, e->d_name);
            if (p != NULL) unlink(p);
            free(p);
        }
        closedir(d);
    }
    rmdir(dir);
}

#define MAN_RENDER_TIMEOUT_MS 20000

/* Render a man page source file to HTML with groff. gzip is read in process;
 * bzip2, xz and zstd go through their decompressor. No shell is involved. */
static char *render_man_to_html(const char *path)
{
    static const struct { const char *suffix, *tool; } packers[] = {
        { ".bz2", "bzip2" }, { ".xz", "xz" }, { ".zst", "zstd" }
    };
    char *input = NULL;
    size_t input_len = 0;
    if (has_suffix(path, ".gz")) {
        input = tam_read_gzip_file(path, &input_len);
        if (input == NULL) return NULL;
    }
    for (size_t i = 0; input == NULL && i < sizeof(packers) / sizeof(packers[0]); i++) {
        if (!has_suffix(path, packers[i].suffix)) continue;
        const char *argv[] = { packers[i].tool, "-dc", path, NULL };
        tam_exec_result_t r = tam_exec(argv, MAN_RENDER_TIMEOUT_MS);
        free(r.stderr_buf);
        if (r.timed_out || r.truncated || r.stdout_len == 0) {
            free(r.stdout_buf);
            return NULL;
        }
        input = r.stdout_buf;
        input_len = r.stdout_len;
    }

    char *scratch = groff_scratch_dir();
    if (scratch == NULL) {
        free(input);
        return NULL;
    }
    char tmpdir_env[4200], image_dir[4200];
    snprintf(tmpdir_env, sizeof(tmpdir_env), "TMPDIR=%s", scratch);
    snprintf(image_dir, sizeof(image_dir), "-D%s", scratch);
    const char *argv[] = { "env", tmpdir_env, "groff", "-mandoc", "-Thtml", "-P",
                           image_dir, input != NULL ? NULL : path, NULL };
    tam_exec_opts_t opts = { .timeout_ms = MAN_RENDER_TIMEOUT_MS,
                             .input = input, .input_len = input_len };
    tam_exec_result_t r = tam_exec_opts(argv, &opts);
    remove_scratch_dir(scratch);
    free(scratch);
    free(input);
    free(r.stderr_buf);
    /* groff can warn and exit non-zero yet still produce the page. */
    if (r.timed_out || r.truncated || r.stdout_len == 0) {
        free(r.stdout_buf);
        return NULL;
    }
    return r.stdout_buf;
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
        if (nlen == 0 || nlen >= sizeof(name)) return NULL;
        memcpy(name, s, nlen);
        name[nlen] = '\0';

        const char *rparen = strchr(paren, ')');
        if (rparen == NULL || rparen == paren + 1 || rparen[1] != '\0') return NULL;
        size_t slen = (size_t)(rparen - paren - 1);
        if (slen >= sizeof(sec)) return NULL;
        memcpy(sec, paren + 1, slen);
        sec[slen] = '\0';
    } else {
        if (strlen(s) >= sizeof(name)) return NULL;
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

static char *uri_parameter(const char *uri, const char *key)
{
    char needle[48];
    snprintf(needle, sizeof(needle), "%s=", key);
    const char *p = strchr(uri, '?');
    if (p == NULL) return NULL;
    p++;
    while (*p != '\0') {
        const char *end = strchr(p, '&');
        size_t n = end ? (size_t)(end - p) : strlen(p);
        if (n >= strlen(needle) && strncmp(p, needle, strlen(needle)) == 0) {
            char *value = strndup(p + strlen(needle), n - strlen(needle));
            char *decoded = value ? uri_decode(value) : NULL;
            free(value);
            return decoded;
        }
        if (end == NULL) break;
        p = end + 1;
    }
    return NULL;
}

static int valid_section(const char *section)
{
    if (section == NULL || section[0] == '\0' || strlen(section) > 8) return 0;
    for (const char *p = section; *p; p++)
        if (!is_sec_char(*p)) return 0;
    return 1;
}

static int valid_lookup_text(const char *query)
{
    if (query == NULL || query[0] == '\0' || strlen(query) > 120) return 0;
    for (const unsigned char *p = (const unsigned char *)query; *p; p++)
        if (*p < 0x20 || *p == 0x7f) return 0;
    return 1;
}

static int parse_lookup_row(const char *line, char name[65], char section[16],
                            char **description)
{
    *description = NULL;
    const char *open = strchr(line, '(');
    const char *close = open ? strchr(open + 1, ')') : NULL;
    if (open == NULL || close == NULL || close == open + 1 ||
        (size_t)(close - open - 1) >= 16) return 0;
    const char *start = open;
    while (start > line && isspace((unsigned char)start[-1])) start--;
    const char *name_end = start;
    while (start > line && !isspace((unsigned char)start[-1]) && start[-1] != ',') start--;
    size_t nlen = (size_t)(name_end - start);
    size_t slen = (size_t)(close - open - 1);
    if (nlen == 0 || nlen >= 65) return 0;
    memcpy(name, start, nlen); name[nlen] = '\0';
    memcpy(section, open + 1, slen); section[slen] = '\0';
    if (!valid_section(section)) return 0;
    for (char *p = name; *p; p++) if (!is_name_char(*p)) return 0;
    const char *sep = strstr(close + 1, " - ");
    if (sep == NULL) sep = strstr(close + 1, ": ");
    if (sep != NULL) {
        sep += sep[0] == ':' ? 2 : 3;
        *description = strdup(sep);
    } else *description = strdup("");
    return *description != NULL;
}

static void append_lookup_rows(tam_str_t *body, const lookup_result_t *result,
                               const char *more_uri, size_t offset, size_t limit,
                               int *matched, int *more)
{
    *matched = 0;
    *more = 0;
    size_t row = 0;
    const char *p = result->text ? result->text : "";
    while (*p != '\0') {
        const char *end = strchr(p, '\n');
        size_t len = end ? (size_t)(end - p) : strlen(p);
        char *line = strndup(p, len);
        if (line != NULL) {
            char name[65], section[16];
            char *description = NULL;
            if (parse_lookup_row(line, name, section, &description)) {
                (*matched)++;
                if (row >= offset && row < offset + limit) {
                    char href[128];
                    snprintf(href, sizeof(href), "man:%s(%s)", name, section);
                    char *name_html = tam_html_escape(name);
                    char *sec_html = tam_html_escape(section);
                    char *desc_html = tam_html_escape(description);
                    *body = tam_str_cat_printf(*body,
                        "<tr><td><a href=\"%s\">%s(%s)</a></td><td>%s</td></tr>\n",
                        href, name_html ? name_html : "",
                        sec_html ? sec_html : "", desc_html ? desc_html : "");
                    free(name_html); free(sec_html); free(desc_html);
                }
                row++;
            }
            free(description);
            free(line);
        }
        if (end == NULL) break;
        p = end + 1;
    }
    *more = row > offset + limit || result->truncated;
    if (result->truncated)
        *body = tam_str_cat(*body, "<p>Output reached the safe lookup limit; results may be incomplete.</p>\n");
    if (*more && !result->truncated && more_uri != NULL) {
        *body = tam_str_cat_printf(*body,
            "<p><a href=\"%s&amp;offset=%zu\">More results</a></p>\n",
            more_uri ? more_uri : "tam:man", offset + limit);
    }
}

static tam_document_t *man_landing_document(int width)
{
    tam_str_t body = tam_str_new(
        "<p>Open installed manual pages, find them by keyword, or browse a section. "
        "Manual lookup is separate from searching the Guide with <a href=\"tam:search\">Guide Search</a>.</p>\n"
        "<ul>\n"
        "<li><a href=\"tam:prompt:man-open\">Open a page</a> by name, optionally with a section.</li>\n"
        "<li><a href=\"tam:prompt:man-keyword\">Find by keyword</a> in the installed manual index.</li>\n"
        "<li><a href=\"tam:prompt:man-section\">Browse a section</a> by section number or installed subsection.</li>\n"
        "</ul>\n<h2>Section guide</h2>\n"
        "<table><tr><th>Section</th><th>Contents</th></tr>\n"
        "<tr><td>1</td><td>Commands</td></tr><tr><td>2</td><td>System calls</td></tr>\n"
        "<tr><td>3</td><td>Library calls</td></tr><tr><td>4</td><td>Device files</td></tr>\n"
        "<tr><td>5</td><td>File formats</td></tr><tr><td>6</td><td>Games</td></tr>\n"
        "<tr><td>7</td><td>Miscellaneous and conventions</td></tr>\n"
        "<tr><td>8</td><td>Administration</td></tr><tr><td>9</td><td>Kernel routines (optional)</td></tr></table>\n"
        "<p>Installed subsection names are accepted too. Section contents differ by host.</p>\n"
        "<h2>Starting points</h2>\n<ul>\n");
    static const struct { const char *name; const char *section; } starts[] = {
        {"printf", "1"}, {"systemctl", "1"}, {"fstab", "5"}
    };
    int found = 0;
    for (size_t i = 0; i < sizeof(starts) / sizeof(starts[0]); i++) {
        char *path = tam_man_resolve_path(starts[i].name, starts[i].section);
        int installed = path != NULL;
        free(path);
        if (!installed) continue;
        body = tam_str_cat_printf(body,
            "<li><a href=\"man:%s(%s)\">%s(%s)</a></li>\n",
            starts[i].name, starts[i].section, starts[i].name, starts[i].section);
        found++;
    }
    if (found == 0)
        body = tam_str_cat(body, "<li>No curated manual pages are installed; use Open a page or Browse a section.</li>\n");
    body = tam_str_cat(body, "</ul>\n");
    tam_document_t *doc = html_document("Manual lookup", body, width);
    tam_str_free(body);
    return doc;
}

static tam_document_t *lookup_error_document(const char *title, const char *message,
                                             int width)
{
    char *escaped = tam_html_escape(message);
    tam_str_t body = tam_str_new("<p>");
    body = tam_str_cat(body, escaped ? escaped : "Lookup failed.");
    body = tam_str_cat(body, "</p><p><a href=\"tam:man\">Return to Man pages</a></p>");
    tam_document_t *doc = html_document(title, body, width);
    tam_str_free(body);
    free(escaped);
    return doc;
}

tam_document_t *tam_man_uri_document(const char *uri, int width)
{
    if (uri == NULL || strcmp(uri, "tam:man") == 0) return man_landing_document(width);
    if (strncmp(uri, "tam:man:open?", 13) == 0) {
        char *query = uri_parameter(uri, "query");
        if (!valid_lookup_text(query)) {
            free(query);
            return lookup_error_document("Open a manual page", "Enter a page name, optionally followed by a section.", width);
        }
        char spec[160];
        snprintf(spec, sizeof(spec), "%s", query);
        free(query);
        char *space = strchr(spec, ' ');
        char *page = NULL;
        char *section = NULL;
        if (space != NULL && strchr(spec, '(') == NULL) {
            *space++ = '\0';
            while (*space == ' ') space++;
            if (valid_section(spec)) { section = strdup(spec); page = strdup(space); }
            else if (valid_section(space)) { page = strdup(spec); section = strdup(space); }
        } else {
            char *open = strchr(spec, '(');
            if (open != NULL) {
                char *close = strchr(open + 1, ')');
                if (close != NULL) { *open++ = '\0'; *close = '\0'; page = strdup(spec); section = strdup(open); }
            } else page = strdup(spec);
        }
        if (page == NULL || page[0] == '\0' || (section != NULL && !valid_section(section))) {
            free(page); free(section);
            return lookup_error_document("Open a manual page", "Use PAGE or PAGE SECTION, for example printf 3.", width);
        }
        if (section == NULL) {
            const char *argv[] = {"apropos", "-e", "--", page, NULL};
            lookup_result_t r = run_lookup(argv);
            int count = 0, more = 0;
            tam_str_t body = tam_str_new("<table><tr><th>Page</th><th>Description</th></tr>\n");
            if (r.unavailable || r.timed_out) {
                tam_document_t *direct = r.unavailable ? tam_man_load(page, width) : NULL;
                free(r.text); tam_str_free(body);
                if (direct != NULL) { free(page); return direct; }
                const char *msg = r.unavailable ?
                    "The apropos tool is unavailable, and the page could not be opened directly." :
                    "Manual lookup timed out. Specify a section to open a page directly.";
                free(page);
                return lookup_error_document("Open a manual page", msg, width);
            }
            append_lookup_rows(&body, &r, NULL, 0, 50, &count, &more);
            if (count > 1) {
                body = tam_str_cat(body, "</table><p>Several installed sections match. Select the intended page.</p>");
                tam_document_t *doc = html_document("Choose a manual section", body, width);
                tam_str_free(body); free(r.text); free(page);
                return doc;
            }
            tam_str_free(body); free(r.text);
        }
        char target[160];
        if (section != NULL) snprintf(target, sizeof(target), "%s(%s)", page, section);
        else snprintf(target, sizeof(target), "%s", page);
        tam_document_t *doc = tam_man_load(target, width);
        if (doc != NULL) {
            char man_href[180];
            snprintf(man_href, sizeof(man_href), "man:%s", target);
            tam_document_ensure_location(doc, man_href);
        }
        free(page); free(section);
        if (doc != NULL) return doc;
        char msg[256]; snprintf(msg, sizeof(msg), "No installed manual page was found for %s.", target);
        return lookup_error_document("Manual page not found", msg, width);
    }
    if (strncmp(uri, "tam:man:keyword?", 16) == 0 ||
        strncmp(uri, "tam:man:section?", 16) == 0) {
        int section_mode = strncmp(uri, "tam:man:section?", 16) == 0;
        char *query = uri_parameter(uri, section_mode ? "section" : "query");
        if (!valid_lookup_text(query)) {
            free(query);
            return lookup_error_document(section_mode ? "Browse a manual section" : "Find a manual by keyword",
                section_mode ? "Enter a valid manual section." : "Enter a keyword to search the manual index.", width);
        }
        if (section_mode && !valid_section(query)) {
            free(query);
            return lookup_error_document("Browse a manual section", "Section names use letters and digits, up to eight characters.", width);
        }
        char *offset_text = uri_parameter(uri, "offset");
        size_t offset = 0;
        if (offset_text != NULL) {
            char *end = NULL; unsigned long parsed = strtoul(offset_text, &end, 10);
            if (end != offset_text && *end == '\0' && parsed <= 100000) offset = (size_t)parsed;
        }
        free(offset_text);
        char *pattern = section_mode ? NULL : literal_apropos_pattern(query);
        if (!section_mode && pattern == NULL) {
            free(query);
            return lookup_error_document("Manual keyword lookup", "Could not prepare the keyword query.", width);
        }
        const char *argv_keyword[] = {"apropos", "--", pattern, NULL};
        const char *argv_section[] = {"apropos", "-s", query, "--", ".", NULL};
        lookup_result_t r = run_lookup(section_mode ? argv_section : argv_keyword);
        free(pattern);
        char title[180]; snprintf(title, sizeof(title), section_mode ? "Manual section %s" : "Manual keyword: %s", query);
        if (r.unavailable || r.timed_out) {
            const char *msg = r.unavailable ? "The apropos tool is unavailable; the manual index cannot be searched." :
                "Manual index lookup timed out. Try a more specific query.";
            free(r.text); free(query);
            return lookup_error_document(title, msg, width);
        }
        if (r.status != 0 && r.status != 16 && !r.truncated && r.len == 0) {
            free(r.text); free(query);
            return lookup_error_document(title,
                "The manual index database is unavailable or could not be read.", width);
        }
        tam_str_t body = tam_str_new("<table><tr><th>Page</th><th>Description</th></tr>\n");
        int count = 0, more = 0;
        char *encoded_query = uri_encode(query);
        char more_uri[512];
        snprintf(more_uri, sizeof(more_uri), section_mode ?
            "tam:man:section?section=%s" : "tam:man:keyword?query=%s",
            encoded_query ? encoded_query : "");
        free(encoded_query);
        append_lookup_rows(&body, &r, more_uri, offset,
                           MAN_RESULTS_PER_PAGE, &count, &more);
        if (count == 0) {
            body = tam_str_cat(body, section_mode ?
                "</table><p>No installed manual pages were found in this section.</p>" :
                "</table><p>No manual entries matched this keyword.</p>");
        } else body = tam_str_cat(body, "</table>");
        body = tam_str_cat(body, "<p><a href=\"tam:prompt:man-keyword\">Find another keyword</a> · <a href=\"tam:prompt:man-section\">Browse another section</a></p>");
        tam_document_t *doc = html_document(title, body, width);
        tam_str_free(body); free(r.text); free(query);
        return doc;
    }
    return lookup_error_document("Manual lookup", "Unsupported manual lookup request.", width);
}

static void print_man_help(void)
{
    fputs(
        "tam man - view manual pages with hyperlinked cross-references\n\n"
        "Usage:\n"
        "  tam man                    open the manual discovery landing page\n"
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
        "Press ] / [ (or Tab / Shift-Tab) to select links and Enter to follow them.\n",
        stdout);
}

typedef struct {
    char *root;
} man_loader_ctx_t;

static tam_document_t *man_unified_load(void *vctx, const char *href,
                                        int content_width)
{
    man_loader_ctx_t *ctx = vctx;
    if (href == NULL || href[0] == '\0') {
        return NULL;
    }
    if (strncmp(href, "man:", 4) == 0) {
        return tam_man_load(href + 4, content_width);
    }
    if (strncmp(href, "info:", 5) == 0) {
        return tam_info_load(href + 5, content_width);
    }
    if (strncmp(href, "tldr:", 5) == 0) {
        return tam_tldr_load(href + 5, content_width);
    }
    if (strncmp(href, "tam:tldr", 8) == 0) {
        return tam_tldr_uri_document(href, content_width);
    }
    if (strncmp(href, "tam:man", 7) == 0) {
        return tam_man_uri_document(href, content_width);
    }
    if (strncmp(href, "tam:search", 10) == 0) {
        return tam_guide_search_uri_document(ctx->root, href, content_width);
    }
    if (ctx->root != NULL) {
        if (strncmp(href, "tam:topics", 10) == 0) {
            unsigned level = 0;
            const char *q = strstr(href, "?level=");
            if (q != NULL) {
                level = tam_level_parse(q + 7);
            }
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
        int interactive = !pipe && isatty(STDOUT_FILENO);
        int width = interactive ? tam_terminal_width() : 80;
        tam_document_t *doc = man_landing_document(width);
        if (doc == NULL) return TAM_EXIT_FAILURE;
        tam_document_ensure_location(doc, "tam:man");
        int rc = 0;
        if (interactive) {
            man_loader_ctx_t ctx = {.root = tam_find_knowledge_root()};
            rc = tam_pager_run(doc, "tam:man", man_unified_load, &ctx);
            free(ctx.root);
        } else {
            char *text = tam_document_text(doc);
            if (text != NULL) { fputs(text, stdout); free(text); }
            tam_document_free(doc);
        }
        return rc;
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

    char initial_href[160];
    snprintf(initial_href, sizeof(initial_href), "man:%s", spec);
    tam_document_ensure_location(doc, initial_href);

    int rc = 0;
    if (interactive) {
        man_loader_ctx_t ctx;
        ctx.root = tam_find_knowledge_root();
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
