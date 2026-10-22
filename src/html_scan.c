#include "html_scan.h"

#include <ctype.h>
#include <string.h>
#include <strings.h>

int tam_tag_read(const char *s, size_t n, size_t i, tam_tag_t *t)
{
    memset(t, 0, sizeof(*t));
    t->start = i;
    t->end = n;
    t->attrs = i + 1;
    if (i + 4 <= n && memcmp(s + i, "<!--", 4) == 0) {
        t->comment = 1;
        for (size_t j = i + 4; j + 3 <= n; j++) {
            if (memcmp(s + j, "-->", 3) == 0) {
                t->end = j + 3;
                t->complete = 1;
                break;
            }
        }
        return t->complete;
    }
    size_t j = i + 1, k = 0;
    if (j < n && s[j] == '/') {
        t->closing = 1;
        j++;
    }
    /* A hyphen continues a name (custom elements), so <section-x> is not
     * <section>. */
    while (j < n && (isalnum((unsigned char)s[j]) || (k > 0 && s[j] == '-'))) {
        if (k + 1 < sizeof(t->name)) t->name[k++] = (char)tolower((unsigned char)s[j]);
        j++;
    }
    t->attrs = j;
    char quote = 0;
    for (; j < n; j++) {
        if (quote) {
            if (s[j] == quote) quote = 0;
        } else if (s[j] == '"' || s[j] == '\'') {
            quote = s[j];
        } else if (s[j] == '>') {
            t->end = j + 1;
            t->complete = 1;
            break;
        }
    }
    return t->complete;
}

int tam_tag_is(const tam_tag_t *t, const char *name, int closing)
{
    return !t->comment && t->closing == closing && strcmp(t->name, name) == 0;
}

int tam_html_attr(const char *s, size_t from, size_t to, const char *key,
                  size_t *value, size_t *vlen)
{
    size_t klen = strlen(key);
    size_t i = from;
    while (i < to) {
        while (i < to && (isspace((unsigned char)s[i]) || s[i] == '/')) i++;
        size_t a = i;
        while (i < to && !isspace((unsigned char)s[i]) && s[i] != '=' &&
               s[i] != '>' && s[i] != '/')
            i++;
        size_t z = i;
        if (z == a) {
            i++;
            continue;
        }
        while (i < to && isspace((unsigned char)s[i])) i++;
        size_t v = i, len = 0;
        if (i < to && s[i] == '=') {
            i++;
            while (i < to && isspace((unsigned char)s[i])) i++;
            char quote = (i < to && (s[i] == '"' || s[i] == '\'')) ? s[i++] : 0;
            v = i;
            while (i < to && (quote ? s[i] != quote
                                    : !isspace((unsigned char)s[i]) && s[i] != '>'))
                i++;
            len = i - v;
            if (quote && i < to) i++;
        }
        if (z - a == klen && strncasecmp(s + a, key, klen) == 0) {
            *value = v;
            *vlen = len;
            return 1;
        }
    }
    return 0;
}

int tam_tag_attr(const char *s, const tam_tag_t *t, const char *key,
                 char *out, size_t cap)
{
    size_t v, len;
    size_t to = t->complete && !t->comment ? t->end - 1 : t->end;
    if (cap == 0 || t->comment || !tam_html_attr(s, t->attrs, to, key, &v, &len))
        return 0;
    if (len >= cap) len = cap - 1;
    memcpy(out, s + v, len);
    out[len] = '\0';
    return 1;
}
