#define _POSIX_C_SOURCE 200809L
#include <tam.h>

#include "address.h"
#include "archwiki.h"
#include "documentation.h"
#include "html.h"
#include "man.h"
#include "pager.h"
#include "render.h"
#include "terminal.h"
#include "version.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#define TAM_EXIT_USAGE   2
#define TAM_EXIT_FAILURE 1
#define ARCHWIKI_DEFAULT_DIR "/usr/share/doc/arch-wiki/tam"
#define ARCHWIKI_RESULTS_PER_PAGE 60

extern char *tam_find_knowledge_root(void);

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

static const char *ci_strstr(const char *hay, const char *needle)
{
    if (hay == NULL || needle == NULL) return NULL;
    size_t nlen = strlen(needle);
    if (nlen == 0) return hay;
    for (const char *p = hay; *p != '\0'; p++) {
        if (strncasecmp(p, needle, nlen) == 0) return p;
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* Directory Resolution and Index Scanning                                    */
/* ------------------------------------------------------------------------- */

static int archwiki_dir_ok(const char *dir)
{
    if (dir == NULL || dir[0] == '\0') return 0;
    struct stat st;
    return stat(dir, &st) == 0 && S_ISDIR(st.st_mode);
}

char *tam_find_archwiki_root(void)
{
    const char *env = getenv("TAM_ARCHWIKI_DIR");
    if (env != NULL && env[0] != '\0') {
        return archwiki_dir_ok(env) ? strdup(env) : NULL;
    }
    char *path = tam_documentation_root("archwiki");
    if (archwiki_dir_ok(path)) return path;
    free(path);
    return NULL;
}

/* MediaWiki's offline dump hashes non-ASCII titles into 32-char lowercase hex
 * filenames; skip them from A-Z browse lists and keyword search. */
static int is_hex_hash_slug(const char *slug)
{
    size_t len = strlen(slug);
    if (len != 32) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = slug[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return 0;
    }
    return 1;
}

static char *slug_to_title(const char *slug)
{
    char *dec = uri_decode(slug);
    if (dec == NULL) return NULL;
    for (char *p = dec; *p != '\0'; p++) {
        if (*p == '_') *p = ' ';
    }
    return dec;
}

typedef struct {
    char *slug;
    char *title;
} archwiki_entry_t;

typedef struct {
    archwiki_entry_t *items;
    size_t count;
    size_t cap;
} archwiki_index_t;

static int cmp_archwiki_entry(const void *a, const void *b)
{
    const archwiki_entry_t *ea = a;
    const archwiki_entry_t *eb = b;
    return strcasecmp(ea->title, eb->title);
}

static void archwiki_index_free(archwiki_index_t *idx)
{
    if (idx == NULL) return;
    for (size_t i = 0; i < idx->count; i++) {
        free(idx->items[i].slug);
        free(idx->items[i].title);
    }
    free(idx->items);
    free(idx);
}

static void archwiki_index_scan_dir(const char *root, const char *rel_prefix,
                                    archwiki_index_t *idx, int depth)
{
    if (depth > 4) return;
    char dir_path[1024];
    if (rel_prefix[0] != '\0') {
        if (snprintf(dir_path, sizeof(dir_path), "%s/%s", root, rel_prefix) >= (int)sizeof(dir_path)) {
            return;
        }
    } else {
        if (snprintf(dir_path, sizeof(dir_path), "%s", root) >= (int)sizeof(dir_path)) {
            return;
        }
    }

    DIR *dp = opendir(dir_path);
    if (dp == NULL) return;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        const char *name = de->d_name;
        if (name[0] == '.') continue;

        char full_path[1024];
        if (snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, name) >= (int)sizeof(full_path)) {
            continue;
        }
        struct stat st;
        if (lstat(full_path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            char next_rel[512];
            int rn = rel_prefix[0] != '\0'
                ? snprintf(next_rel, sizeof(next_rel), "%s/%s", rel_prefix, name)
                : snprintf(next_rel, sizeof(next_rel), "%s", name);
            if (rn > 0 && (size_t)rn < sizeof(next_rel)) {
                archwiki_index_scan_dir(root, next_rel, idx, depth + 1);
            }
            continue;
        }
        if (!S_ISREG(st.st_mode)) continue;

        size_t len = strlen(name);
        if (len <= 5 || strcmp(name + len - 5, ".html") != 0) continue;

        char base_slug[512];
        size_t blen = len - 5;
        if (blen >= sizeof(base_slug)) continue;
        memcpy(base_slug, name, blen);
        base_slug[blen] = '\0';
        if (rel_prefix[0] == '\0' && is_hex_hash_slug(base_slug)) continue;

        char slug_buf[768];
        int sn = rel_prefix[0] != '\0'
            ? snprintf(slug_buf, sizeof(slug_buf), "%s/%s", rel_prefix, base_slug)
            : snprintf(slug_buf, sizeof(slug_buf), "%s", base_slug);
        if (sn <= 0 || (size_t)sn >= sizeof(slug_buf)) continue;

        char *slug = strdup(slug_buf);
        if (slug == NULL) continue;
        char *title = slug_to_title(slug);
        if (title == NULL) {
            free(slug);
            continue;
        }
        if (idx->count == idx->cap) {
            size_t ncap = idx->cap ? idx->cap * 2 : 128;
            archwiki_entry_t *nitems = realloc(idx->items, ncap * sizeof(*nitems));
            if (nitems == NULL) {
                free(slug);
                free(title);
                break;
            }
            idx->items = nitems;
            idx->cap = ncap;
        }
        idx->items[idx->count].slug = slug;
        idx->items[idx->count].title = title;
        idx->count++;
    }
    closedir(dp);
}

static archwiki_index_t *archwiki_index_load(const char *root)
{
    if (root == NULL) return NULL;
    archwiki_index_t *idx = calloc(1, sizeof(*idx));
    if (idx == NULL) return NULL;

    archwiki_index_scan_dir(root, "", idx, 0);

    if (idx->count > 1) {
        qsort(idx->items, idx->count, sizeof(*idx->items), cmp_archwiki_entry);
    }
    return idx;
}

/* ------------------------------------------------------------------------- */
/* Page Normalization & Resolution                                            */
/* ------------------------------------------------------------------------- */

static int normalize_archwiki_spec(const char *spec, char *out_slug, size_t slug_cap,
                                   char *out_frag, size_t frag_cap)
{
    if (spec == NULL || out_slug == NULL || slug_cap == 0) return 0;
    if (out_frag != NULL && frag_cap > 0) out_frag[0] = '\0';

    const char *s = spec;
    if (strncmp(s, "archwiki:", 9) == 0) s += 9;
    while (*s == ' ' || *s == '/') s++;
    if (*s == '\0') return 0;

    const char *hash = strchr(s, '#');
    size_t slen = hash ? (size_t)(hash - s) : strlen(s);
    while (slen > 0 && (s[slen - 1] == ' ' || s[slen - 1] == '/')) slen--;
    if (slen == 0) return 0;
    if (slen >= 5 && strncmp(s + slen - 5, ".html", 5) == 0) slen -= 5;
    if (slen == 0 || slen >= slug_cap) return 0;

    for (size_t i = 0; i < slen; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\\' || c < 0x20) return 0;
        if (c == '/' && i > 0 && out_slug[i - 1] == '/') return 0;
        out_slug[i] = (c == ' ') ? '_' : (char)c;
    }
    out_slug[slen] = '\0';
    if (out_slug[0] == '/' || out_slug[slen - 1] == '/' ||
        strcmp(out_slug, ".") == 0 || strcmp(out_slug, "..") == 0 ||
        strstr(out_slug, "..") != NULL) {
        return 0;
    }

    if (hash != NULL && hash[1] != '\0' && out_frag != NULL && frag_cap > 0) {
        snprintf(out_frag, frag_cap, "%s", hash + 1);
    }
    return 1;
}

static char *archwiki_resolve_page(const char *root, const char *slug,
                                   char *resolved_slug, size_t slug_cap)
{
    if (root == NULL || slug == NULL || slug[0] == '\0') return NULL;
    char path[1024];
    int n = snprintf(path, sizeof(path), "%s/%s.html", root, slug);
    if (n > 0 && (size_t)n < sizeof(path) && tam_fs_file_exists(path)) {
        if (resolved_slug != NULL && slug_cap > 0) {
            snprintf(resolved_slug, slug_cap, "%s", slug);
        }
        return strdup(path);
    }

    /* Case-insensitive fallback scan in target directory (supports subpages). */
    const char *slash = strrchr(slug, '/');
    char dir_path[1024];
    char target[300];
    if (slash != NULL) {
        size_t dlen = (size_t)(slash - slug);
        if (dlen == 0 || dlen >= 512) return NULL;
        snprintf(dir_path, sizeof(dir_path), "%s/%.*s", root, (int)dlen, slug);
        snprintf(target, sizeof(target), "%s.html", slash + 1);
    } else {
        snprintf(dir_path, sizeof(dir_path), "%s", root);
        snprintf(target, sizeof(target), "%s.html", slug);
    }

    DIR *dp = opendir(dir_path);
    if (dp == NULL) return NULL;

    char *found = NULL;
    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (strcasecmp(de->d_name, target) == 0) {
            n = snprintf(path, sizeof(path), "%s/%s", dir_path, de->d_name);
            if (n > 0 && (size_t)n < sizeof(path) && tam_fs_file_exists(path)) {
                found = strdup(path);
                if (resolved_slug != NULL && slug_cap > 0) {
                    size_t dlen = strlen(de->d_name);
                    size_t base_copy = dlen > 5 ? dlen - 5 : 0;
                    if (slash != NULL) {
                        size_t prefix_len = (size_t)(slash - slug);
                        snprintf(resolved_slug, slug_cap, "%.*s/%.*s",
                                 (int)prefix_len, slug, (int)base_copy, de->d_name);
                    } else {
                        if (base_copy >= slug_cap) base_copy = slug_cap - 1;
                        memcpy(resolved_slug, de->d_name, base_copy);
                        resolved_slug[base_copy] = '\0';
                    }
                }
            }
            break;
        }
    }
    closedir(dp);
    return found;
}

/* ------------------------------------------------------------------------- */
/* HTML Extraction & Link Rewriting                                           */
/* ------------------------------------------------------------------------- */

static char *extract_html_title(const char *html, const char *fallback_slug)
{
    const char *t1 = ci_strstr(html, "<title>");
    if (t1 != NULL) {
        t1 += 7;
        const char *t2 = ci_strstr(t1, "</title>");
        if (t2 != NULL && t2 > t1) {
            size_t len = (size_t)(t2 - t1);
            char *title = strndup(t1, len);
            if (title != NULL) {
                const char *suffix = " - ArchWiki";
                size_t slen = strlen(suffix);
                if (len > slen && strcmp(title + len - slen, suffix) == 0) {
                    title[len - slen] = '\0';
                }
                return title;
            }
        }
    }
    return slug_to_title(fallback_slug);
}

static void find_body_bounds(const char *html, size_t len,
                             const char **out_start, const char **out_end)
{
    const char *start = strstr(html, "<div class=\"mw-content-ltr mw-parser-output\"");
    if (start == NULL) start = strstr(html, "<div id=\"mw-content-text\"");
    if (start == NULL) start = ci_strstr(html, "<body");
    if (start != NULL) {
        const char *gt = strchr(start, '>');
        start = gt ? gt + 1 : html;
    } else {
        start = html;
    }

    const char *end = html + len;
    static const char *const end_markers[] = {
        "<div id=\"catlinks\"",
        "<div class=\"printfooter\"",
        "<footer",
        "</main>",
        "</body>",
    };
    for (size_t i = 0; i < sizeof(end_markers) / sizeof(end_markers[0]); i++) {
        const char *m = strstr(start, end_markers[i]);
        if (m != NULL && m < end) end = m;
    }

    *out_start = start;
    *out_end = end;
}

typedef struct {
    char *key;
    int exists;
} man_cache_entry_t;

typedef struct {
    man_cache_entry_t *items;
    size_t count;
    size_t cap;
} man_cache_t;

static void man_cache_free(man_cache_t *c)
{
    if (c == NULL) return;
    for (size_t i = 0; i < c->count; i++) free(c->items[i].key);
    free(c->items);
}

static int man_cache_exists(man_cache_t *c, const char *name, const char *sec)
{
    char key[176];
    snprintf(key, sizeof(key), "%s(%s)", name, sec);
    for (size_t i = 0; i < c->count; i++) {
        if (strcmp(c->items[i].key, key) == 0) return c->items[i].exists;
    }
    char *resolved = tam_man_resolve_path(name, sec);
    int exists = (resolved != NULL);
    free(resolved);
    if (c->count == c->cap) {
        size_t ncap = c->cap ? c->cap * 2 : 16;
        man_cache_entry_t *nitems = realloc(c->items, ncap * sizeof(*nitems));
        if (nitems == NULL) return exists;
        c->items = nitems;
        c->cap = ncap;
    }
    c->items[c->count].key = strdup(key);
    c->items[c->count].exists = exists;
    if (c->items[c->count].key != NULL) c->count++;
    return exists;
}

static char *rewrite_man_archlinux_href(const char *href, man_cache_t *cache)
{
    const char *p = NULL;
    if (strncmp(href, "https://man.archlinux.org/man/", 30) == 0) {
        p = href + 30;
    } else if (strncmp(href, "http://man.archlinux.org/man/", 29) == 0) {
        p = href + 29;
    } else {
        return NULL;
    }

    const char *slash = strrchr(p, '/');
    if (slash != NULL) p = slash + 1;
    if (*p == '\0') return NULL;

    const char *hash = strchr(p, '#');
    size_t len = hash ? (size_t)(hash - p) : strlen(p);
    if (len == 0 || len >= 160) return NULL;

    char buf[160];
    memcpy(buf, p, len);
    buf[len] = '\0';

    if (len > 5 && strcmp(buf + len - 5, ".html") == 0) {
        len -= 5;
        buf[len] = '\0';
    }
    if (len > 3 && strcmp(buf + len - 3, ".en") == 0) {
        len -= 3;
        buf[len] = '\0';
    }

    char *dot = strrchr(buf, '.');
    if (dot == NULL || dot == buf || dot[1] == '\0') return NULL;
    *dot = '\0';
    const char *name = buf;
    const char *sec = dot + 1;

    if (!isdigit((unsigned char)sec[0]) || strlen(sec) > 8) return NULL;
    for (const char *s = sec; *s; s++) {
        if (!isalnum((unsigned char)*s)) return NULL;
    }
    for (const char *s = name; *s; s++) {
        if (isspace((unsigned char)*s) || *s == '/') return NULL;
    }

    if (!man_cache_exists(cache, name, sec)) return NULL;

    char *out = malloc(4 + strlen(name) + 1 + strlen(sec) + 2);
    if (out != NULL) {
        sprintf(out, "man:%s(%s)", name, sec);
    }
    return out;
}

static tam_str_t rewrite_anchor_tag(tam_str_t out, const char *tag, size_t tag_len,
                                    man_cache_t *man_cache)
{
    /* Locate href="..." or href='...' inside <a ...> */
    const char *p = tag;
    const char *end = tag + tag_len;
    const char *href_attr = NULL;
    char quote = '\0';
    const char *val_start = NULL;
    const char *val_end = NULL;

    while (p < end) {
        if ((p == tag || isspace((unsigned char)p[-1])) &&
            (size_t)(end - p) >= 6 && strncasecmp(p, "href=", 5) == 0) {
            href_attr = p;
            const char *v = p + 5;
            if (v < end && (*v == '"' || *v == '\'')) {
                quote = *v++;
                val_start = v;
                val_end = memchr(v, quote, (size_t)(end - v));
                if (val_end != NULL) break;
            }
            href_attr = NULL;
        }
        p++;
    }

    if (href_attr == NULL || val_start == NULL || val_end == NULL) {
        return tam_str_cat_len(out, tag, tag_len);
    }

    size_t vlen = (size_t)(val_end - val_start);
    char *href = strndup(val_start, vlen);
    if (href == NULL) return tam_str_cat_len(out, tag, tag_len);

    char *rewritten = NULL;
    int drop_href = 0;

    if (href[0] == '#' ||
        strncmp(href, "archwiki:", 9) == 0 ||
        strncmp(href, "tam:archwiki:", 13) == 0 ||
        strncmp(href, "man:", 4) == 0) {
        rewritten = strdup(href);
    } else if (strncmp(href, "http://", 7) == 0 ||
               strncmp(href, "https://", 8) == 0) {
        rewritten = rewrite_man_archlinux_href(href, man_cache);
        if (rewritten == NULL) rewritten = strdup(href);
    } else {
        const char *page_part = href;
        while (strncmp(page_part, "../", 3) == 0) {
            if (strncmp(page_part, "../en/", 6) == 0) {
                page_part += 6;
                break;
            }
            page_part += 3;
        }
        if (strncmp(page_part, "./", 2) == 0) {
            page_part += 2;
        } else if (page_part[0] == '/') {
            drop_href = 1;
        }

        if (!drop_href) {
            const char *hash = strchr(page_part, '#');
            size_t plen = hash ? (size_t)(hash - page_part) : strlen(page_part);
            if (plen > 5 && strncmp(page_part + plen - 5, ".html", 5) == 0) {
                plen -= 5;
                size_t flen = hash ? strlen(hash) : 0;
                rewritten = malloc(9 + plen + flen + 1);
                if (rewritten != NULL) {
                    memcpy(rewritten, "archwiki:", 9);
                    memcpy(rewritten + 9, page_part, plen);
                    if (flen > 0) memcpy(rewritten + 9 + plen, hash, flen);
                    rewritten[9 + plen + flen] = '\0';
                }
            } else {
                drop_href = 1;
            }
        }
    }

    if (drop_href) {
        /* Omit href so non-local/special links render as plain text. */
        out = tam_str_cat_len(out, tag, (size_t)(href_attr - tag));
        out = tam_str_cat_len(out, val_end + 1, (size_t)(end - (val_end + 1)));
    } else if (rewritten != NULL) {
        out = tam_str_cat_len(out, tag, (size_t)(val_start - tag));
        out = tam_str_cat(out, rewritten);
        out = tam_str_cat_len(out, val_end, (size_t)(end - val_end));
    } else {
        out = tam_str_cat_len(out, tag, tag_len);
    }

    free(rewritten);
    free(href);
    return out;
}

static const char *skip_element_block(const char *p, const char *end, const char *close_tag)
{
    const char *gt = memchr(p, '>', (size_t)(end - p));
    if (gt == NULL) return end;
    const char *close = ci_strstr(gt + 1, close_tag);
    if (close == NULL || close >= end) return end;
    const char *cgt = memchr(close, '>', (size_t)(end - close));
    return cgt ? cgt + 1 : end;
}

static tam_str_t clean_archwiki_body(const char *start, const char *end)
{
    tam_str_t out = tam_str_new("");
    man_cache_t man_cache = {0};
    const char *p = start;

    while (p < end) {
        const char *lt = memchr(p, '<', (size_t)(end - p));
        if (lt == NULL) {
            out = tam_str_cat_len(out, p, (size_t)(end - p));
            break;
        }
        if (lt > p) {
            out = tam_str_cat_len(out, p, (size_t)(lt - p));
        }

        size_t rem = (size_t)(end - lt);
        if (rem >= 6 && strncasecmp(lt, "<style", 6) == 0 &&
            (lt[6] == '>' || isspace((unsigned char)lt[6]))) {
            p = skip_element_block(lt, end, "</style>");
            continue;
        }
        if (rem >= 7 && strncasecmp(lt, "<script", 7) == 0 &&
            (lt[7] == '>' || isspace((unsigned char)lt[7]))) {
            p = skip_element_block(lt, end, "</script>");
            continue;
        }
        if (rem >= 4 && strncasecmp(lt, "<svg", 4) == 0 &&
            (lt[4] == '>' || isspace((unsigned char)lt[4]))) {
            p = skip_element_block(lt, end, "</svg>");
            continue;
        }
        if (rem >= 7 && strncasecmp(lt, "<button", 7) == 0 &&
            (lt[7] == '>' || isspace((unsigned char)lt[7]))) {
            p = skip_element_block(lt, end, "</button>");
            continue;
        }
        if (rem >= 2 && (lt[1] == 'a' || lt[1] == 'A') &&
            (isspace((unsigned char)lt[2]) || lt[2] == '>')) {
            const char *gt = memchr(lt, '>', rem);
            if (gt == NULL) {
                out = tam_str_cat_len(out, lt, rem);
                break;
            }
            out = rewrite_anchor_tag(out, lt, (size_t)(gt - lt + 1), &man_cache);
            p = gt + 1;
            continue;
        }

        const char *gt = memchr(lt, '>', rem);
        if (gt == NULL) {
            out = tam_str_cat_len(out, lt, rem);
            break;
        }
        out = tam_str_cat_len(out, lt, (size_t)(gt - lt + 1));
        p = gt + 1;
    }
    man_cache_free(&man_cache);
    return out;
}

tam_document_t *tam_archwiki_load(const char *spec, int width)
{
    char slug[256], frag[256];
    if (!normalize_archwiki_spec(spec, slug, sizeof(slug), frag, sizeof(frag))) {
        return NULL;
    }

    char *root = tam_find_archwiki_root();
    if (root == NULL) return NULL;

    char resolved_slug[256];
    char *path = archwiki_resolve_page(root, slug, resolved_slug, sizeof(resolved_slug));
    free(root);
    if (path == NULL) return NULL;

    size_t len = 0;
    char *raw = tam_fs_read(path, 0, &len);
    free(path);
    if (raw == NULL) return NULL;

    char *title = extract_html_title(raw, resolved_slug);
    const char *body_start = NULL, *body_end = NULL;
    find_body_bounds(raw, len, &body_start, &body_end);

    tam_str_t cleaned = clean_archwiki_body(body_start, body_end);
    free(raw);

    const char *display_title = title ? title : resolved_slug;
    tam_str_t html = tam_page_begin(display_title, NULL, NULL, 0);
    html = tam_str_cat(html, "<h1>");
    html = tam_html_cat(html, display_title);
    html = tam_str_cat(html, "</h1>\n");
    html = tam_str_cat(html, cleaned ? cleaned : "");
    tam_str_free(cleaned);

    tam_document_t *doc = tam_page_render(html, width);
    free(title);
    if (doc != NULL) {
        char href[300];
        snprintf(href, sizeof(href), "archwiki:%s", resolved_slug);
        tam_document_ensure_location(doc, href);
        (void)frag; /* Fragment scrolling is handled by caller/pager when present. */
    }
    return doc;
}

/* ------------------------------------------------------------------------- */
/* Landing, Browse Index, and Keyword Search (tam:archwiki*)                  */
/* ------------------------------------------------------------------------- */

static tam_document_t *archwiki_html_document(const char *title, const char *body, int width)
{
    tam_str_t html = tam_page_begin(title ? title : "Arch Wiki", NULL, NULL, 0);
    html = tam_str_cat(html, "<h1>");
    html = tam_html_cat(html, title ? title : "Arch Wiki");
    html = tam_str_cat(html, "</h1>\n");
    html = tam_str_cat(html, body ? body : "");
    return tam_page_render(html, width);
}

static tam_document_t *archwiki_error_document(const char *title, const char *msg, int width)
{
    char *esc = tam_html_escape(msg);
    tam_str_t body = tam_str_new("<p>");
    body = tam_str_cat(body, esc ? esc : "Arch Wiki lookup failed.");
    body = tam_str_cat(body, "</p>\n<p><a href=\"tam:archwiki\">Return to Arch Wiki</a> · "
                             "<a href=\"index.html\">Tamlinux Guide</a></p>\n");
    free(esc);
    tam_document_t *doc = archwiki_html_document(title, body, width);
    tam_str_free(body);
    return doc;
}

static tam_str_t append_az_bar(tam_str_t body)
{
    body = tam_str_cat(body, "<p><a href=\"tam:archwiki:list\">All</a>");
    for (char c = 'a'; c <= 'z'; c++) {
        char upper = (char)toupper((unsigned char)c);
        body = tam_str_cat_printf(body,
            " <a href=\"tam:archwiki:list?letter=%c\">%c</a>", c, upper);
    }
    body = tam_str_cat(body, " <a href=\"tam:archwiki:list?letter=0\">0-9</a></p>\n");
    return body;
}

static tam_document_t *archwiki_landing_document(const char *root, int width)
{
    archwiki_index_t *idx = archwiki_index_load(root);
    size_t total = idx ? idx->count : 0;
    archwiki_index_free(idx);

    tam_str_t body = tam_str_new(
        "<p>Offline Arch Linux documentation from the <code>arch-wiki-docs</code> "
        "package. For complete flag references use <a href=\"tam:man\">Man pages</a>; "
        "for concise command examples use <a href=\"tam:tldr\">tldr pages</a>; "
        "for book-length GNU manuals use <a href=\"info:dir\">Info manuals</a>; "
        "for Tamlinux guides use <a href=\"tam:search\">Guide Search</a>.</p>\n");

    if (root == NULL) {
        body = tam_str_cat(body,
            "<p><strong>Note:</strong> Offline Arch Wiki pages were not found at "
            "<code>/usr/share/doc/arch-wiki/tam</code> or "
            "<code>/usr/share/doc/arch-wiki/html/en</code>. Install the "
            "Arch Wiki documentation package or generate the <code>tam</code> "
            "corpus from <code>arch-wiki-docs</code>.</p>\n");
    }

    body = tam_str_cat(body, "<h2>Starting points</h2>\n<table>\n");

    static const struct { const char *slug; const char *label; const char *desc; } starts[] = {
        {"Arch_Linux", "Arch Linux", "Overview and principles of Arch Linux"},
        {"Installation_guide", "Installation guide", "Official Arch Linux installation guide"},
        {"Pacman", "pacman", "Arch Linux package manager"},
        {"Systemd", "systemd", "System and service manager"},
        {"Hyprland", "Hyprland", "Dynamic tiling Wayland compositor"},
        {"Btrfs", "Btrfs", "Copy-on-write filesystem"},
        {"Network_configuration", "Network configuration", "Network setup and troubleshooting"},
        {"General_troubleshooting", "General troubleshooting", "Diagnosing and fixing system issues"},
    };
    int any_start = 0;
    for (size_t i = 0; i < sizeof(starts) / sizeof(starts[0]); i++) {
        char *p = archwiki_resolve_page(root, starts[i].slug, NULL, 0);
        if (p != NULL) {
            free(p);
            body = tam_str_cat_printf(body,
                "<tr><td><a href=\"archwiki:%s\">%s</a></td><td>%s</td></tr>\n",
                starts[i].slug, starts[i].label, starts[i].desc);
            any_start++;
        }
    }
    if (!any_start) {
        body = tam_str_cat(body, "<tr><td colspan=\"2\">Use Open an Arch Wiki page or Browse index below.</td></tr>\n");
    }
    body = tam_str_cat(body, "</table>\n<ul>\n"
        "<li><a href=\"tam:prompt:archwiki-open\">Open an Arch Wiki page</a> by title "
        "(for example <code>Pacman</code> or <code>Systemd</code>).</li>\n"
        "<li><a href=\"tam:prompt:archwiki-keyword\">Find by keyword</a> across Arch Wiki page titles.</li>\n");
    if (total > 0) {
        body = tam_str_cat_printf(body,
            "<li><a href=\"tam:archwiki:list\">Browse all %zu Arch Wiki pages</a> in the A-Z index.</li>\n",
            total);
    } else {
        body = tam_str_cat(body,
            "<li><a href=\"tam:archwiki:list\">Browse Arch Wiki pages</a> in the A-Z index.</li>\n");
    }
    body = tam_str_cat(body, "</ul>\n<h2>Browse index</h2>\n");
    body = append_az_bar(body);

    tam_document_t *doc = archwiki_html_document("Arch Wiki", body, width);
    tam_str_free(body);
    return doc;
}

static int entry_matches_letter(const archwiki_entry_t *e, const char *letter)
{
    if (letter == NULL || letter[0] == '\0') return 1;
    unsigned char first = (unsigned char)e->title[0];
    if (letter[0] == '0') return !isalpha(first);
    return tolower(first) == tolower((unsigned char)letter[0]);
}

static tam_document_t *archwiki_list_document(const char *root, const char *uri, int width)
{
    archwiki_index_t *idx = archwiki_index_load(root);
    if (idx == NULL) {
        return archwiki_error_document("Arch Wiki index",
            "Offline Arch Wiki pages are not available.", width);
    }

    char *letter = uri_parameter(uri, "letter");
    char *offset_str = uri_parameter(uri, "offset");
    size_t offset = 0;
    if (offset_str != NULL) {
        unsigned long v = strtoul(offset_str, NULL, 10);
        if (v <= 100000) offset = (size_t)v;
        free(offset_str);
    }

    char title[128];
    if (letter != NULL && letter[0] != '\0') {
        if (letter[0] == '0') snprintf(title, sizeof(title), "Arch Wiki index: 0-9");
        else snprintf(title, sizeof(title), "Arch Wiki index: %c",
                      toupper((unsigned char)letter[0]));
    } else {
        snprintf(title, sizeof(title), "Arch Wiki index");
    }

    tam_str_t body = tam_str_new("");

    size_t matched = 0;
    for (size_t i = 0; i < idx->count; i++) {
        if (entry_matches_letter(&idx->items[i], letter)) matched++;
    }

    if (matched == 0) {
        body = tam_str_cat(body, "<p>No Arch Wiki pages match this filter.</p>\n");
    } else {
        size_t shown_from = offset < matched ? offset + 1 : matched;
        size_t end_idx = offset + ARCHWIKI_RESULTS_PER_PAGE;
        if (end_idx > matched) end_idx = matched;
        body = tam_str_cat_printf(body,
            "<p>Showing %zu–%zu of %zu Arch Wiki pages.</p>\n<ul>\n",
            shown_from, end_idx, matched);

        size_t seen = 0;
        for (size_t i = 0; i < idx->count; i++) {
            if (!entry_matches_letter(&idx->items[i], letter)) continue;
            if (seen >= offset && seen < end_idx) {
                char *eslug = tam_html_escape(idx->items[i].slug);
                char *etitle = tam_html_escape(idx->items[i].title);
                body = tam_str_cat_printf(body,
                    "<li><a href=\"archwiki:%s\">%s</a></li>\n",
                    eslug ? eslug : "", etitle ? etitle : "");
                free(eslug);
                free(etitle);
            }
            seen++;
        }
        body = tam_str_cat(body, "</ul>\n");

        if (end_idx < matched) {
            tam_str_t next_uri = tam_str_new("tam:archwiki:list?");
            if (letter != NULL && letter[0] != '\0') {
                next_uri = tam_str_cat_printf(next_uri, "letter=%c&amp;", letter[0]);
            }
            next_uri = tam_str_cat_printf(next_uri, "offset=%zu", end_idx);
            body = tam_str_cat_printf(body,
                "<p><a href=\"%s\">Next page</a></p>\n", next_uri);
            tam_str_free(next_uri);
        }
    }

    body = append_az_bar(body);

    body = tam_str_cat(body,
        "<p><a href=\"tam:archwiki\">Return to Arch Wiki</a> · "
        "<a href=\"index.html\">Tamlinux Guide</a></p>\n");

    free(letter);
    archwiki_index_free(idx);
    tam_document_t *doc = archwiki_html_document(title, body, width);
    tam_str_free(body);
    return doc;
}

static int entry_matches_query(const archwiki_entry_t *e, const char *query)
{
    if (query == NULL || query[0] == '\0') return 0;
    char *copy = strdup(query);
    if (copy == NULL) return 0;
    int all_match = 1;
    char *saveptr = NULL;
    for (char *tok = strtok_r(copy, " \t\r\n", &saveptr);
         tok != NULL;
         tok = strtok_r(NULL, " \t\r\n", &saveptr)) {
        if (ci_strstr(e->title, tok) == NULL && ci_strstr(e->slug, tok) == NULL) {
            all_match = 0;
            break;
        }
    }
    free(copy);
    return all_match;
}

static tam_document_t *archwiki_keyword_document(const char *root, const char *query,
                                                 size_t offset, int width)
{
    if (query == NULL || query[0] == '\0') {
        return archwiki_error_document("Find an Arch Wiki page",
            "Enter one or more keywords to search Arch Wiki titles.", width);
    }

    archwiki_index_t *idx = archwiki_index_load(root);
    if (idx == NULL) {
        return archwiki_error_document("Find an Arch Wiki page",
            "Offline Arch Wiki pages are not available.", width);
    }

    char *eq = tam_html_escape(query);
    tam_str_t body = tam_str_new("");
    size_t matched = 0;
    for (size_t i = 0; i < idx->count; i++) {
        if (entry_matches_query(&idx->items[i], query)) matched++;
    }

    if (matched == 0) {
        body = tam_str_cat_printf(body,
            "<p>No Arch Wiki pages matched <code>%s</code>.</p>\n"
            "<p><a href=\"tam:prompt:archwiki-keyword\">Try another search</a> · "
            "<a href=\"tam:archwiki:list\">Browse all pages</a></p>\n",
            eq ? eq : "");
    } else {
        size_t end_idx = offset + ARCHWIKI_RESULTS_PER_PAGE;
        if (end_idx > matched) end_idx = matched;
        body = tam_str_cat_printf(body,
            "<p>Found %zu Arch Wiki page%s matching <code>%s</code>.</p>\n<ul>\n",
            matched, matched == 1 ? "" : "s", eq ? eq : "");

        size_t seen = 0;
        for (size_t i = 0; i < idx->count; i++) {
            if (!entry_matches_query(&idx->items[i], query)) continue;
            if (seen >= offset && seen < end_idx) {
                char *eslug = tam_html_escape(idx->items[i].slug);
                char *etitle = tam_html_escape(idx->items[i].title);
                body = tam_str_cat_printf(body,
                    "<li><a href=\"archwiki:%s\">%s</a></li>\n",
                    eslug ? eslug : "", etitle ? etitle : "");
                free(eslug);
                free(etitle);
            }
            seen++;
        }
        body = tam_str_cat(body, "</ul>\n");

        if (end_idx < matched) {
            char *enc = uri_encode(query);
            body = tam_str_cat_printf(body,
                "<p><a href=\"tam:archwiki:keyword?query=%s&amp;offset=%zu\">More results</a></p>\n",
                enc ? enc : "", end_idx);
            free(enc);
        }
    }

    body = tam_str_cat(body,
        "<p><a href=\"tam:archwiki\">Return to Arch Wiki</a></p>\n");

    char title[160];
    snprintf(title, sizeof(title), "Arch Wiki search: %s", query);
    free(eq);
    archwiki_index_free(idx);
    tam_document_t *doc = archwiki_html_document(title, body, width);
    tam_str_free(body);
    return doc;
}

static tam_document_t *archwiki_broken_document(const char *uri, int width)
{
    char *target = uri_parameter(uri, "target");
    char *from = uri_parameter(uri, "from");
    char *etarget = tam_html_escape(target && target[0] ? target : "unknown target");
    char *efrom = tam_html_escape(from && from[0] ? from : "");

    tam_str_t body = tam_str_new("");
    body = tam_str_cat_printf(body,
        "<p><strong>Broken internal link:</strong> The Arch Wiki link to "
        "<code>%s</code>", etarget ? etarget : "");
    if (from && from[0]) {
        body = tam_str_cat_printf(body, " (referenced from <code>%s</code>)",
                                  efrom ? efrom : "");
    }
    body = tam_str_cat(body,
        " could not be resolved in the offline Tamlinux Arch Wiki corpus.</p>\n"
        "<p>This link has been logged in the broken-links manifest so an AI agent "
        "can track down the missing page or mend it in a future patch.</p>\n"
        "<ul>\n");

    if (from && from[0]) {
        char *from_title = slug_to_title(from);
        char *efrom_title = tam_html_escape(from_title ? from_title : from);
        body = tam_str_cat_printf(body,
            "<li><a href=\"archwiki:%s\">Return to %s</a></li>\n",
            efrom ? efrom : "", efrom_title ? efrom_title : "");
        free(efrom_title);
        free(from_title);
    }

    if (target && target[0]) {
        /* Strip .html or #fragment for keyword search */
        char search_buf[256];
        snprintf(search_buf, sizeof(search_buf), "%s", target);
        char *hash = strchr(search_buf, '#');
        if (hash) *hash = '\0';
        size_t slen = strlen(search_buf);
        if (slen > 5 && strcmp(search_buf + slen - 5, ".html") == 0) {
            search_buf[slen - 5] = '\0';
        }
        for (char *p = search_buf; *p; p++) {
            if (*p == '_' || *p == '/') *p = ' ';
        }
        char *enc = uri_encode(search_buf);
        char *esearch = tam_html_escape(search_buf);
        body = tam_str_cat_printf(body,
            "<li><a href=\"tam:archwiki:keyword?query=%s\">Search Arch Wiki for %s</a></li>\n",
            enc ? enc : "", esearch ? esearch : "");
        free(esearch);
        free(enc);
    }

    body = tam_str_cat(body,
        "<li><a href=\"tam:archwiki\">Arch Wiki index</a></li>\n"
        "<li><a href=\"index.html\">Tamlinux Guide</a></li>\n"
        "</ul>\n");

    free(efrom);
    free(etarget);
    free(from);
    free(target);
    tam_document_t *doc = archwiki_html_document("Broken Arch Wiki link", body, width);
    tam_str_free(body);
    return doc;
}

tam_document_t *tam_archwiki_uri_document(const char *uri, int width)
{
    if (uri != NULL && strncmp(uri, "tam:archwiki:broken", 19) == 0) {
        return archwiki_broken_document(uri, width);
    }

    char *root = tam_find_archwiki_root();
    if (uri == NULL || strcmp(uri, "tam:archwiki") == 0) {
        tam_document_t *doc = archwiki_landing_document(root, width);
        free(root);
        return doc;
    }

    if (strncmp(uri, "tam:archwiki:list", 17) == 0) {
        tam_document_t *doc = archwiki_list_document(root, uri, width);
        free(root);
        return doc;
    }

    if (strncmp(uri, "tam:archwiki:open?", 18) == 0) {
        char *query = uri_parameter(uri, "query");
        if (query == NULL || query[0] == '\0') {
            free(query);
            free(root);
            return archwiki_error_document("Open an Arch Wiki page",
                "Enter a page title, such as Pacman or Arch Linux.", width);
        }
        tam_document_t *direct = tam_archwiki_load(query, width);
        if (direct != NULL) {
            free(query);
            free(root);
            return direct;
        }
        tam_document_t *fallback = archwiki_keyword_document(root, query, 0, width);
        free(query);
        free(root);
        return fallback;
    }

    if (strncmp(uri, "tam:archwiki:keyword?", 21) == 0) {
        char *query = uri_parameter(uri, "query");
        char *offset_str = uri_parameter(uri, "offset");
        size_t offset = 0;
        if (offset_str != NULL) {
            unsigned long v = strtoul(offset_str, NULL, 10);
            if (v <= 100000) offset = (size_t)v;
            free(offset_str);
        }
        tam_document_t *doc = archwiki_keyword_document(root, query, offset, width);
        free(query);
        free(root);
        return doc;
    }

    free(root);
    return archwiki_error_document("Arch Wiki", "Unsupported Arch Wiki lookup request.", width);
}

/* ------------------------------------------------------------------------- */
/* CLI Subcommand: tam archwiki                                               */
/* ------------------------------------------------------------------------- */

static void print_archwiki_help(void)
{
    fputs(
        "tam archwiki - browse offline Arch Wiki documentation with hyperlinked references\n\n"
        "Usage:\n"
        "  tam archwiki                   open the Arch Wiki discovery and index page\n"
        "  tam archwiki PAGE...           open an Arch Wiki page in the terminal pager\n"
        "  tam archwiki --list            browse the full A-Z index of Arch Wiki pages\n"
        "  tam archwiki --search KEYWORD  find Arch Wiki pages matching KEYWORD\n"
        "  tam archwiki [ARGS] --pipe     print formatted plain text without pager\n"
        "  tam archwiki --help            show this help and exit\n\n"
        "Examples:\n"
        "  tam archwiki                   browse and search offline Arch Wiki pages\n"
        "  tam archwiki Pacman            view the pacman Arch Wiki page\n"
        "  tam archwiki Arch Linux        view the Arch Linux page\n"
        "  tam archwiki Pacman --pipe     output the pacman page as plain text\n",
        stdout);
}

int tam_cmd_archwiki(int n, char **args)
{
    int pipe = 0;
    int list_mode = 0;
    const char *search_kw = NULL;
    char page_buf[200] = {0};
    size_t page_len = 0;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (strcmp(args[i], "--list") == 0 || strcmp(args[i], "-l") == 0) {
            list_mode = 1;
        } else if (strcmp(args[i], "--search") == 0 || strcmp(args[i], "--keyword") == 0) {
            if (i + 1 >= n || args[i + 1][0] == '\0') {
                fprintf(stderr, TAM_COMMAND_NAME ": %s requires a keyword\n", args[i]);
                return TAM_EXIT_USAGE;
            }
            search_kw = args[++i];
        } else if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0 ||
                   (page_len == 0 && strcmp(args[i], "help") == 0)) {
            print_archwiki_help();
            return 0;
        } else if (args[i][0] == '-') {
            fprintf(stderr, TAM_COMMAND_NAME ": unrecognized argument '%s'\n", args[i]);
            fprintf(stderr, "Run '%s archwiki --help' for available options.\n", TAM_COMMAND_NAME);
            return TAM_EXIT_USAGE;
        } else {
            if (page_len > 0 && page_len + 1 < sizeof(page_buf)) {
                page_buf[page_len++] = '_';
            }
            size_t alen = strlen(args[i]);
            if (page_len + alen >= sizeof(page_buf)) {
                fprintf(stderr, TAM_COMMAND_NAME ": page title too long\n");
                return TAM_EXIT_USAGE;
            }
            memcpy(page_buf + page_len, args[i], alen);
            page_len += alen;
            page_buf[page_len] = '\0';
        }
    }

    int interactive = !pipe && isatty(STDOUT_FILENO);
    int width = interactive ? tam_terminal_width() : 80;

    if (list_mode || search_kw != NULL || page_len == 0) {
        char uri[256];
        if (search_kw != NULL) {
            char *enc = uri_encode(search_kw);
            snprintf(uri, sizeof(uri), "tam:archwiki:keyword?query=%s", enc ? enc : "");
            free(enc);
        } else if (list_mode) {
            snprintf(uri, sizeof(uri), "tam:archwiki:list");
        } else {
            snprintf(uri, sizeof(uri), "tam:archwiki");
        }

        tam_document_t *doc = tam_archwiki_uri_document(uri, width);
        if (doc == NULL) return TAM_EXIT_FAILURE;
        tam_document_ensure_location(doc, uri);
        int rc = 0;
        if (interactive) {
            char *kroot = tam_find_knowledge_root();
            tam_loader_ctx_t ctx = {.root = kroot};
            rc = tam_reader_run(doc, uri, tam_address_load, &ctx);
            free(kroot);
        } else {
            char *text = tam_document_text(doc);
            if (text != NULL) { fputs(text, stdout); free(text); }
            tam_document_free(doc);
        }
        return rc;
    }

    tam_document_t *doc = tam_archwiki_load(page_buf, width);
    if (doc == NULL) {
        fprintf(stderr, TAM_COMMAND_NAME ": no Arch Wiki page found for '%s'\n", page_buf);
        return TAM_EXIT_FAILURE;
    }

    char initial_href[220];
    snprintf(initial_href, sizeof(initial_href), "archwiki:%s", page_buf);

    int rc = 0;
    if (interactive) {
        char *kroot = tam_find_knowledge_root();
        tam_loader_ctx_t ctx = {.root = kroot};
        rc = tam_reader_run(doc, initial_href, tam_address_load, &ctx);
        free(kroot);
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
