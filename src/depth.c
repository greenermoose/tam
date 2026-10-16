#include <tam.h>

#include "depth.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static const struct {
    unsigned    bit;
    const char *name;
    const char *num;
} LEVELS[] = {
    { TAM_LEVEL_BEGINNER,     "beginner",     "1" },
    { TAM_LEVEL_INTERMEDIATE, "intermediate", "2" },
    { TAM_LEVEL_EXPERT,       "expert",       "3" },
};

#define NLEVELS (sizeof(LEVELS) / sizeof(LEVELS[0]))

unsigned tam_level_parse(const char *name)
{
    if (name == NULL) {
        return 0;
    }
    for (size_t i = 0; i < NLEVELS; i++) {
        if (strcasecmp(name, LEVELS[i].name) == 0 ||
            strcmp(name, LEVELS[i].num) == 0) {
            return LEVELS[i].bit;
        }
    }
    return 0;
}

int tam_level_parse_arg(const char *arg, unsigned *level)
{
    if (arg == NULL || level == NULL || arg[0] == '\0') {
        return 0;
    }
    if (strcasecmp(arg, "all") == 0 || strcmp(arg, "0") == 0) {
        *level = 0;
        return 1;
    }
    unsigned bit = tam_level_parse(arg);
    if (bit != 0) {
        *level = bit;
        return 1;
    }
    return 0;
}

unsigned tam_level_parse_list(const char *list)
{
    unsigned mask = 0;
    if (list == NULL) {
        return 0;
    }
    const char *s = list;
    while (*s != '\0') {
        while (*s == ',' || isspace((unsigned char)*s)) {
            s++;
        }
        const char *start = s;
        while (*s != '\0' && *s != ',' && !isspace((unsigned char)*s)) {
            s++;
        }
        size_t n = (size_t)(s - start);
        if (n > 0 && n < 32) {
            char word[32];
            memcpy(word, start, n);
            word[n] = '\0';
            if (strcasecmp(word, "all") == 0 || strcmp(word, "0") == 0) {
                mask |= TAM_LEVEL_ALL;
            } else {
                mask |= tam_level_parse(word);
            }
        }
    }
    return mask;
}

const char *tam_level_name(unsigned bit)
{
    for (size_t i = 0; i < NLEVELS; i++) {
        if (LEVELS[i].bit == bit) {
            return LEVELS[i].name;
        }
    }
    return NULL;
}

int tam_level_count(unsigned mask)
{
    int n = 0;
    for (size_t i = 0; i < NLEVELS; i++) {
        if (mask & LEVELS[i].bit) {
            n++;
        }
    }
    return n;
}

char *tam_level_format(unsigned mask, const char *sep, char *out, size_t cap)
{
    if (out == NULL || cap == 0) {
        return out;
    }
    out[0] = '\0';
    size_t used = 0;
    for (size_t i = 0; i < NLEVELS; i++) {
        if (!(mask & LEVELS[i].bit)) {
            continue;
        }
        int n = snprintf(out + used, cap - used, "%s%s",
                         used > 0 ? sep : "", LEVELS[i].name);
        if (n < 0 || (size_t)n >= cap - used) {
            break;
        }
        used += (size_t)n;
    }
    return out;
}

char *tam_depth_split_href(const char *href, unsigned *level)
{
    *level = 0;
    if (href == NULL) {
        return NULL;
    }
    const char *q = strchr(href, '?');
    if (q == NULL) {
        return strdup(href);
    }
    static const char key[] = "level=";
    if (strncmp(q + 1, key, sizeof(key) - 1) != 0) {
        return NULL;
    }
    unsigned bit = 0;
    if (!tam_level_parse_arg(q + sizeof(key), &bit)) {
        return NULL;
    }
    size_t n = (size_t)(q - href);
    char *path = malloc(n + 1);
    if (path == NULL) {
        return NULL;
    }
    memcpy(path, href, n);
    path[n] = '\0';
    *level = bit;
    return path;
}

/* ------------------------------------------------------------------------- */
/* Growable output buffer                                                     */
/* ------------------------------------------------------------------------- */

typedef struct {
    char  *data;
    size_t len;
    size_t cap;
    int    failed;
} buf_t;

static void buf_add(buf_t *b, const char *s, size_t n)
{
    if (b->failed) {
        return;
    }
    if (b->len + n + 1 > b->cap) {
        size_t cap = b->cap ? b->cap : 1024;
        while (b->len + n + 1 > cap) {
            cap *= 2;
        }
        char *next = realloc(b->data, cap);
        if (next == NULL) {
            b->failed = 1;
            return;
        }
        b->data = next;
        b->cap = cap;
    }
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static void buf_str(buf_t *b, const char *s)
{
    buf_add(b, s, strlen(s));
}

/* ------------------------------------------------------------------------- */
/* Minimal tag scanning                                                       */
/* ------------------------------------------------------------------------- */

/* Index of the '>' closing the tag that starts at src[i] ('<'), honouring
 * quoted attribute values; len when unterminated. */
static size_t tag_end(const char *src, size_t len, size_t i)
{
    char quote = 0;
    for (size_t j = i + 1; j < len; j++) {
        char c = src[j];
        if (quote) {
            if (c == quote) {
                quote = 0;
            }
        } else if (c == '"' || c == '\'') {
            quote = c;
        } else if (c == '>') {
            return j;
        }
    }
    return len;
}

/* Does the tag at src[i] open (or, with closing, close) element `name`? */
static int tag_is(const char *src, size_t len, size_t i, const char *name,
                  int closing)
{
    size_t j = i + 1;
    if (closing) {
        if (j >= len || src[j] != '/') {
            return 0;
        }
        j++;
    }
    size_t n = strlen(name);
    if (j + n > len || strncasecmp(src + j, name, n) != 0) {
        return 0;
    }
    if (j + n == len) {
        return 0;
    }
    char after = src[j + n];
    return after == '>' || after == '/' || isspace((unsigned char)after);
}

/* Copy attribute `attr` of the tag spanning src[start..end) into out.
 * Returns 1 when present. */
static int tag_attr(const char *src, size_t start, size_t end,
                    const char *attr, char *out, size_t cap)
{
    size_t alen = strlen(attr);
    for (size_t j = start; j + alen < end; j++) {
        if (strncasecmp(src + j, attr, alen) != 0) {
            continue;
        }
        char before = src[j - 1];
        if (!isspace((unsigned char)before)) {
            continue;
        }
        size_t k = j + alen;
        while (k < end && isspace((unsigned char)src[k])) {
            k++;
        }
        if (k >= end || src[k] != '=') {
            continue;
        }
        k++;
        while (k < end && isspace((unsigned char)src[k])) {
            k++;
        }
        char quote = 0;
        if (k < end && (src[k] == '"' || src[k] == '\'')) {
            quote = src[k++];
        }
        size_t v = k;
        while (k < end && (quote ? src[k] != quote
                                 : !isspace((unsigned char)src[k]))) {
            k++;
        }
        size_t n = k - v;
        if (n >= cap) {
            n = cap - 1;
        }
        memcpy(out, src + v, n);
        out[n] = '\0';
        return 1;
    }
    return 0;
}

/* Levels declared by <meta name="level" content="...">; 0 when absent. */
static unsigned declared_levels(const char *src, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (src[i] != '<') {
            continue;
        }
        if (tag_is(src, len, i, "body", 0)) {
            break;
        }
        if (!tag_is(src, len, i, "meta", 0)) {
            continue;
        }
        size_t end = tag_end(src, len, i);
        char name[32];
        char content[128];
        if (tag_attr(src, i + 1, end, "name", name, sizeof(name)) &&
            strcasecmp(name, "level") == 0 &&
            tag_attr(src, i + 1, end, "content", content, sizeof(content))) {
            return tam_level_parse_list(content);
        }
        i = end;
    }
    return 0;
}

/* Position just past the </section> matching the <section> whose tag ends
 * at `open_end`, counting nested sections. */
static size_t skip_section(const char *src, size_t len, size_t open_end)
{
    int depth = 1;
    size_t j = open_end + 1;
    while (j < len) {
        if (src[j] == '<') {
            size_t e = tag_end(src, len, j);
            if (tag_is(src, len, j, "section", 0)) {
                depth++;
            } else if (tag_is(src, len, j, "section", 1)) {
                depth--;
                if (depth == 0) {
                    return e < len ? e + 1 : len;
                }
            }
            j = e < len ? e + 1 : len;
            continue;
        }
        j++;
    }
    return len;
}

static void depth_line(buf_t *b, const char *path, unsigned levels,
                       unsigned level)
{
    buf_str(b, "\n<p>Depth: ");
    if (level == 0) {
        int first = 1;
        for (size_t i = 0; i < NLEVELS; i++) {
            if (!(levels & LEVELS[i].bit)) {
                continue;
            }
            buf_str(b, first ? "" : ", ");
            buf_str(b, "<a href=\"");
            buf_str(b, path);
            buf_str(b, "?level=");
            buf_str(b, LEVELS[i].name);
            buf_str(b, "\">");
            buf_str(b, LEVELS[i].name);
            buf_str(b, "</a>");
            first = 0;
        }
        buf_str(b, ". Every level is shown below; follow one to read "
                   "only that depth.</p>\n");
        return;
    }
    buf_str(b, tam_level_name(level));
    buf_str(b, ". Other depths: ");
    for (size_t i = 0; i < NLEVELS; i++) {
        if (!(levels & LEVELS[i].bit) || LEVELS[i].bit == level) {
            continue;
        }
        buf_str(b, "<a href=\"");
        buf_str(b, path);
        buf_str(b, "?level=");
        buf_str(b, LEVELS[i].name);
        buf_str(b, "\">");
        buf_str(b, LEVELS[i].name);
        buf_str(b, "</a>, ");
    }
    buf_str(b, "<a href=\"");
    buf_str(b, path);
    buf_str(b, "\">all levels</a>.</p>\n");
}

char *tam_depth_apply(const char *src, size_t len, const char *path,
                      unsigned level, size_t *out_len, unsigned *page_levels)
{
    unsigned levels = declared_levels(src, len);
    if (levels == 0) {
        levels = TAM_LEVEL_BEGINNER;
    }
    if (page_levels != NULL) {
        *page_levels = levels;
    }
    if (level != 0 && !(levels & level)) {
        return NULL;
    }

    int laddered = tam_level_count(levels) > 1 && path != NULL;
    int line_done = !laddered;
    buf_t b = {0};

    size_t i = 0;
    while (i < len) {
        if (src[i] == '<') {
            if (level != 0 && tag_is(src, len, i, "section", 0)) {
                size_t e = tag_end(src, len, i);
                char value[32];
                if (tag_attr(src, i + 1, e, "data-level", value,
                             sizeof(value))) {
                    unsigned bit = tam_level_parse(value);
                    if (bit != 0 && bit != level) {
                        i = skip_section(src, len, e);
                        continue;
                    }
                }
            } else if (!line_done && tag_is(src, len, i, "h1", 1)) {
                size_t e = tag_end(src, len, i);
                size_t stop = e < len ? e + 1 : len;
                buf_add(&b, src + i, stop - i);
                depth_line(&b, path, levels, level);
                line_done = 1;
                i = stop;
                continue;
            }
        }
        buf_add(&b, src + i, 1);
        i++;
    }

    if (b.data == NULL && !b.failed) {
        buf_add(&b, "", 0);
    }
    if (b.failed) {
        free(b.data);
        return NULL;
    }
    if (out_len != NULL) {
        *out_len = b.len;
    }
    return b.data;
}
