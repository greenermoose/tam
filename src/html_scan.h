#ifndef TAM_HTML_SCAN_H
#define TAM_HTML_SCAN_H

#include <stddef.h>

/* The one HTML tag scanner, shared by the level filter (depth.c), the fold
 * transform (fold.c) and the renderer (render.c), so all three read a page
 * the same way. */

typedef struct {
    size_t start;    /* the '<' */
    size_t end;      /* one past the closing '>', or past "-->" for a comment;
                        the input length when the tag is not complete */
    size_t attrs;    /* first byte after the element name */
    char   name[16]; /* lower-case element name; "" for comments and <!...> */
    int    closing;  /* </name> */
    int    comment;  /* <!-- ... --> */
    int    complete; /* the closing '>' or "-->" was found */
} tam_tag_t;

/* Read the tag that starts at s[i] (which must be '<'), honouring quoted
 * attribute values. Returns t->complete. */
int tam_tag_read(const char *s, size_t n, size_t i, tam_tag_t *t);

/* Does t open (or, with closing, close) element `name` (lower case)? */
int tam_tag_is(const tam_tag_t *t, const char *name, int closing);

/* Find attribute `key` (case-insensitive) among the name[=value] pairs in
 * s[from..to), so text inside another attribute's value never matches.
 * Stores the raw value's offset and length (entities not decoded; a bare
 * attribute has an empty value). Returns 1 when present. */
int tam_html_attr(const char *s, size_t from, size_t to, const char *key,
                  size_t *value, size_t *vlen);

/* tam_html_attr over a tag's attributes, copied into out (truncated to fit). */
int tam_tag_attr(const char *s, const tam_tag_t *t, const char *key,
                 char *out, size_t cap);

#endif /* TAM_HTML_SCAN_H */
