#include <tam.h>

#include "depth.h"
#include "version.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static const struct {
    unsigned    bit;
    const char *name;
    const char *num;
    const char *label;
} LEVELS[] = {
    { TAM_LEVEL_BEGINNER,     "beginner",     "1", "Beginner" },
    { TAM_LEVEL_INTERMEDIATE, "intermediate", "2", "Intermediate" },
    { TAM_LEVEL_EXPERT,       "expert",       "3", "Expert" },
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

const char *tam_level_label(unsigned bit)
{
    for (size_t i = 0; i < NLEVELS; i++) {
        if (LEVELS[i].bit == bit) {
            return LEVELS[i].label;
        }
    }
    return "All";
}

int tam_level_number(unsigned bit)
{
    for (size_t i = 0; i < NLEVELS; i++) {
        if (LEVELS[i].bit == bit) {
            return (int)i + 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/* The reader's lens                                                          */
/* ------------------------------------------------------------------------- */

static int      g_lens_known;
static unsigned g_lens;

static int env_level(unsigned *level)
{
    const char *env = getenv("TAM_LEVEL");
    return env != NULL && tam_level_parse_arg(env, level);
}

unsigned tam_lens_get(void)
{
    if (g_lens_known) {
        return g_lens;
    }
    g_lens_known = 1;
    g_lens = 0;
    if (env_level(&g_lens)) {
        return g_lens;
    }
    char *path = tam_state_path("level", false);
    if (path == NULL) {
        return g_lens;
    }
    size_t len = 0;
    char *text = tam_fs_read_file(path, &len);
    free(path);
    if (text == NULL) {
        return g_lens;
    }
    while (len > 0 && isspace((unsigned char)text[len - 1])) {
        text[--len] = '\0';
    }
    unsigned saved = 0;
    if (tam_level_parse_arg(text, &saved)) {
        g_lens = saved;
    }
    free(text);
    return g_lens;
}

void tam_lens_override(unsigned level)
{
    g_lens_known = 1;
    g_lens = level;
}

void tam_lens_set(unsigned level)
{
    g_lens_known = 1;
    g_lens = level;
    unsigned ignored;
    if (env_level(&ignored)) {
        return;
    }
    char *path = tam_state_path("level", true);
    if (path == NULL) {
        return;
    }
    char line[32];
    int n = snprintf(line, sizeof(line), "%s\n",
                     level == 0 ? "all" : tam_level_name(level));
    (void)tam_fs_write_atomic(path, line, (size_t)n, 0644);
    free(path);
}

int tam_href_take_level(char *href, unsigned *level)
{
    if (href == NULL) {
        return 0;
    }
    char *q = strchr(href, '?');
    while (q != NULL) {
        char *param = q + 1;
        char *end = param + strcspn(param, "&#");
        if (strncmp(param, "level=", 6) == 0) {
            char value[32];
            size_t n = (size_t)(end - (param + 6));
            if (n >= sizeof(value)) {
                n = sizeof(value) - 1;
            }
            memcpy(value, param + 6, n);
            value[n] = '\0';
            unsigned bit = 0;
            int ok = tam_level_parse_arg(value, &bit);
            /* Drop "level=..." with the separator before it; when it was the
             * first parameter, the next one (if any) takes over the '?'. */
            if (*end == '&') {
                memmove(param, end + 1, strlen(end + 1) + 1);
            } else {
                memmove(q, end, strlen(end) + 1);
            }
            if (ok && level != NULL) {
                *level = bit;
            }
            return ok;
        }
        if (*end != '&') {
            break;
        }
        q = end;
    }
    return 0;
}

int tam_href_put_level(const char *href, unsigned level, char *out,
                       size_t cap)
{
    if (href == NULL) {
        return 0;
    }
    if (level == 0 || tam_level_name(level) == NULL) {
        return snprintf(out, cap, "%s", href) < (int)cap;
    }
    size_t base = strcspn(href, "#");
    const char *sep = memchr(href, '?', base) != NULL ? "&" : "?";
    int n = snprintf(out, cap, "%.*s%slevel=%s%s", (int)base, href, sep,
                     tam_level_name(level), href + base);
    return n > 0 && (size_t)n < cap;
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
unsigned tam_depth_declared(const char *src, size_t len)
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

/* Append the text of the first h2-h4 heading in src[from..to), tags
 * stripped and whitespace collapsed. Entities pass through unchanged. */
static void heading_text(buf_t *b, const char *src, size_t from, size_t to)
{
    static const char *const heads[] = { "h2", "h3", "h4" };
    for (size_t i = from; i < to; i++) {
        if (src[i] != '<') {
            continue;
        }
        const char *h = NULL;
        for (size_t k = 0; k < sizeof(heads) / sizeof(heads[0]); k++) {
            if (tag_is(src, to, i, heads[k], 0)) {
                h = heads[k];
            }
        }
        if (h == NULL) {
            continue;
        }
        size_t j = tag_end(src, to, i) + 1;
        int space = 0;
        int any = 0;
        while (j < to && !tag_is(src, to, j, h, 1)) {
            if (src[j] == '<') {
                j = tag_end(src, to, j) + 1;
                continue;
            }
            if (isspace((unsigned char)src[j])) {
                space = any;
            } else {
                if (space) {
                    buf_add(b, " ", 1);
                }
                buf_add(b, src + j, 1);
                space = 0;
                any = 1;
            }
            j++;
        }
        return;
    }
}

static void level_link(buf_t *b, const char *path, const char *name,
                       const char *text)
{
    buf_str(b, "<a href=\"");
    buf_str(b, path);
    buf_str(b, "?level=");
    buf_str(b, name);
    buf_str(b, "\">");
    buf_str(b, text);
    buf_str(b, "</a>");
}

/* Level bar of a laddered page:
 *   Level: [All 0] | Beginner 1 | Intermediate 2 | Expert 3 -   keys 0-3
 * Offered levels are links, the lens is in brackets and a level the page
 * lacks is plain text followed by " -" (the lens too). */
static void level_bar(buf_t *b, const char *path, unsigned levels,
                      unsigned level)
{
    char item[32];
    buf_str(b, "\n<p>Level: ");
    for (size_t i = 0; i <= NLEVELS; i++) {
        unsigned bit = i == 0 ? 0 : LEVELS[i - 1].bit;
        snprintf(item, sizeof(item), "%s %d", tam_level_label(bit), (int)i);
        if (i > 0) {
            buf_str(b, " | ");
        }
        if (bit == level) {
            buf_str(b, "[");
            buf_str(b, item);
            buf_str(b, bit != 0 && !(levels & bit) ? "] -" : "]");
        } else if (bit != 0 && !(levels & bit)) {
            buf_str(b, item);
            buf_str(b, " -");
        } else {
            level_link(b, path, bit == 0 ? "all" : LEVELS[i - 1].name, item);
        }
    }
    buf_str(b, "&nbsp;&nbsp;&nbsp;keys 0-3</p>\n");
}

/* "Written for: Beginner" or, when the lens is not offered, the banner that
 * says so and where to read at the lens level instead. */
static void written_for(buf_t *b, unsigned levels, unsigned level,
                        const char *related)
{
    char names[64] = {0};
    for (size_t i = 0; i < NLEVELS; i++) {
        if (levels & LEVELS[i].bit) {
            if (names[0] != '\0') {
                strncat(names, ", ", sizeof(names) - strlen(names) - 1);
            }
            strncat(names, LEVELS[i].label, sizeof(names) - strlen(names) - 1);
        }
    }
    buf_str(b, "\n<p>Written for: ");
    buf_str(b, names);
    if (level == 0 || (levels & level)) {
        buf_str(b, "</p>\n");
        return;
    }
    buf_str(b, ". This page has no ");
    buf_str(b, tam_level_label(level));
    buf_str(b, " version.</p>\n<p>At ");
    buf_str(b, tam_level_label(level));
    buf_str(b, ": ");
    if (related != NULL && related[0] != '\0') {
        buf_str(b, related);
    } else {
        buf_str(b, "<a href=\"");
        buf_str(b, tam_level_name(level));
        buf_str(b, ".html\">the ");
        buf_str(b, tam_level_name(level));
        buf_str(b, " path</a>");
    }
    buf_str(b, "</p>\n");
}

static void rev_line(buf_t *b, int rev, const char *updated)
{
    char rev_buf[128];
    snprintf(rev_buf, sizeof(rev_buf), "\n<p>Rev %d Last updated %s</p>\n",
             rev, updated);
    buf_str(b, rev_buf);
}

char *tam_depth_apply_rev(const char *src, size_t len, const char *path,
                          unsigned level, int rev, const char *updated,
                          const char *related, size_t *out_len,
                          tam_depth_info_t *info)
{
    unsigned levels = tam_depth_declared(src, len);
    /* A page without the lens level is shown whole under its banner, so the
     * reader is never left with an empty page. */
    unsigned filter = (levels & level) ? level : 0;
    int line_done = levels == 0 || path == NULL;
    int has_rev = (rev > 0 && updated != NULL && updated[0] != '\0');
    int hidden = 0;
    buf_t b = {0};

    size_t i = 0;
    while (i < len) {
        if (src[i] == '<') {
            if (filter != 0 && tag_is(src, len, i, "section", 0)) {
                size_t e = tag_end(src, len, i);
                char value[32];
                if (tag_attr(src, i + 1, e, "data-level", value,
                             sizeof(value))) {
                    unsigned bit = tam_level_parse(value);
                    if (bit != 0 && bit != filter) {
                        size_t next = skip_section(src, len, e);
                        char mark[64];
                        snprintf(mark, sizeof(mark), "\n<p>-- %d %s: ",
                                 tam_level_number(bit), tam_level_label(bit));
                        buf_str(&b, mark);
                        heading_text(&b, src, e + 1, next);
                        snprintf(mark, sizeof(mark),
                                 " (hidden; press %d or 0) --</p>\n",
                                 tam_level_number(bit));
                        buf_str(&b, mark);
                        hidden++;
                        i = next;
                        continue;
                    }
                }
            } else if (!line_done && tag_is(src, len, i, "h1", 1)) {
                size_t e = tag_end(src, len, i);
                size_t stop = e < len ? e + 1 : len;
                buf_add(&b, src + i, stop - i);
                if (tam_level_count(levels) > 1) {
                    level_bar(&b, path, levels, level);
                    if (level != 0 && filter == 0) {
                        written_for(&b, levels, level, related);
                    }
                } else {
                    written_for(&b, levels, level, related);
                }
                line_done = 1;
                i = stop;
                continue;
            } else if (has_rev && tag_is(src, len, i, "body", 1)) {
                rev_line(&b, rev, updated);
                has_rev = 0;
            }
        }
        buf_add(&b, src + i, 1);
        i++;
    }

    if (has_rev) {
        rev_line(&b, rev, updated);
    }

    if (b.data == NULL && !b.failed) {
        buf_add(&b, "", 0);
    }
    if (b.failed) {
        free(b.data);
        return NULL;
    }
    if (info != NULL) {
        info->levels = levels;
        info->hidden = hidden;
    }
    if (out_len != NULL) {
        *out_len = b.len;
    }
    return b.data;
}
