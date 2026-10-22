#define _POSIX_C_SOURCE 200809L
#include "fold.h"
#include "html_scan.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct { char *s; size_t n, cap; int failed; } buf_t;
static void add(buf_t *b, const char *s, size_t n)
{
    if (b->failed) return;
    if (b->n + n + 1 > b->cap) {
        size_t cap = b->cap ? b->cap : 1024;
        while (cap < b->n + n + 1) cap *= 2;
        char *p = realloc(b->s, cap);
        if (!p) { b->failed = 1; return; }
        b->s = p; b->cap = cap;
    }
    if (n) memcpy(b->s + b->n, s, n);
    b->n += n; b->s[b->n] = 0;
}
static void str(buf_t *b, const char *s) { add(b, s, strlen(s)); }

/* One tag at s[i] from the shared scanner; comments and quoted '>' are
 * consumed as a unit. */
static tam_tag_t tag(const char *s, size_t n, size_t i)
{
    tam_tag_t t;
    tam_tag_read(s, n, i, &t);
    return t;
}
static int cls(const char *s, tam_tag_t t, const char *word)
{
    char value[256];
    if (!tam_tag_attr(s, &t, "class", value, sizeof(value))) return 0;
    for (const char *p = value; *p;) {
        while (isspace((unsigned char)*p)) p++;
        size_t n = strcspn(p, " \t\r\n");
        if (n == strlen(word) && !strncmp(p, word, n)) return 1;
        p += n;
    }
    return 0;
}
static int is_fold(const char *s, tam_tag_t t)
{
    return !t.closing && (!strcmp(t.name, "details") ||
           (!strcmp(t.name, "aside") && cls(s, t, "first-time")));
}
static tam_tag_t matching(const char *s, size_t n, tam_tag_t t)
{
    int depth = 1;
    for (size_t i = t.end; i < n;) {
        if (s[i] != '<') { i++; continue; }
        tam_tag_t next = tag(s, n, i);
        if (!strcmp(next.name, t.name)) depth += next.closing ? -1 : 1;
        if (!depth) return next;
        i = next.end;
    }
    return (tam_tag_t){ .start = n, .end = n };
}
char *tam_fold_number(const char *src, size_t len)
{
    buf_t b = {0}; unsigned id = 0;
    for (size_t i = 0; i < len;) {
        if (src[i] != '<') { add(&b, src + i++, 1); continue; }
        tam_tag_t t = tag(src, len, i);
        if (is_fold(src, t) && t.end > i && src[t.end - 1] == '>') {
            char num[64];
            snprintf(num, sizeof(num), " data-tam-fold=\"%u\"", ++id);
            add(&b, src + i, t.attrs - i); str(&b, num);
            add(&b, src + t.attrs, t.end - t.attrs);
        } else add(&b, src + i, t.end - i);
        i = t.end;
        if (!t.closing && (!strcmp(t.name, "script") || !strcmp(t.name, "style"))) {
            tam_tag_t end = matching(src, len, t);
            add(&b, src + i, end.end - i); i = end.end;
        }
    }
    if (b.failed) { free(b.s); return NULL; }
    if (!b.s) return strdup("");
    return b.s;
}
static int is_open(const char *set, unsigned id)
{
    const char *p = set ? set : "";
    while (*p) {
        char *end;
        unsigned long n = strtoul(p, &end, 10);
        if (end == p) return 0;
        if (n == id) return 1;
        if (*end != '.') break;
        p = end + 1;
    }
    return 0;
}
typedef struct {
    const char *s, *open;
    int pipe, failed;
    tam_fold_t *folds;
    size_t nfolds;
} transform_t;
static void answers_end(buf_t *out, buf_t *answers)
{
    if (answers->n) { str(out, "<h3>Answers</h3>"); add(out, answers->s, answers->n); }
    if (answers->failed) out->failed = 1;
    free(answers->s);
}
static void transform(transform_t *x, size_t from, size_t to, buf_t *out,
                      buf_t *answers, unsigned nesting)
{
    /* Bound recursion on hostile input, failing instead of leaking answers. */
    if (nesting > 128) { x->failed = 1; return; }
    const char *s = x->s;
    for (size_t i = from; i < to;) {
        if (s[i] != '<') { add(out, s + i++, 1); continue; }
        tam_tag_t t = tag(s, to, i);
        if (!t.closing && (!strcmp(t.name, "script") || !strcmp(t.name, "style"))) {
            tam_tag_t end = matching(s, to, t);
            add(out, s + i, end.end - i); i = end.end; continue;
        }
        if (!t.closing && !strcmp(t.name, "section")) {
            tam_tag_t end = matching(s, to, t);
            buf_t local = {0};
            add(out, s + i, t.end - i);
            transform(x, t.end, end.start, out, &local, nesting + 1);
            answers_end(out, &local);
            add(out, s + end.start, end.end - end.start);
            i = end.end; continue;
        }
        if (is_fold(s, t)) {
            tam_tag_t end = matching(s, to, t);
            size_t body = t.end;
            tam_tag_t summary = {0}, summary_end = {0};
            while (body < end.start) {
                if (isspace((unsigned char)s[body])) { body++; continue; }
                if (body + 4 <= end.start && !memcmp(s + body, "<!--", 4)) {
                    body = tag(s, end.start, body).end; continue;
                }
                break;
            }
            if (body < end.start && s[body] == '<') {
                summary = tag(s, end.start, body);
                if (strcmp(summary.name, "summary")) summary.name[0] = 0;
                else { summary_end = matching(s, end.start, summary); body = summary_end.end; }
            }
            const char *label = cls(s, t, "first-time") ? "New to this?" :
                cls(s, t, "solution") ? "Solution" : cls(s, t, "hint") ? "Hint" :
                cls(s, t, "q") && cls(s, t, "open") ? "Model answer" : "Answer";
            char value[32] = "0", line[160];
            tam_tag_attr(s, &t, "data-tam-fold", value, sizeof(value));
            unsigned id = (unsigned)strtoul(value, NULL, 10);
            int opened = is_open(x->open, id);
            tam_fold_t *next = realloc(x->folds, (x->nfolds + 1) * sizeof(*next));
            if (!next) { x->failed = 1; return; }
            x->folds = next;
            x->folds[x->nfolds++] = (tam_fold_t){ id, cls(s, t, "q"), opened };
            str(out, "<div"); add(out, s + t.attrs, t.end - t.attrs);
            str(out, "<p>");
            if (x->pipe) { snprintf(line, sizeof(line), "%u. ", id); str(out, line); }
            if (summary.name[0]) add(out, s + summary.end, summary_end.start - summary.end);
            else str(out, label);
            str(out, "</p>");
            if (!x->pipe) {
                /* The hint names the arrow keys; +, -, Enter, r and * also work. */
                snprintf(line, sizeof(line),
                         "<p><a href=\"tam:fold:%u\">[%c] %s (%s)</a></p>",
                         id, opened ? '-' : '+', label,
                         opened ? "&lt;- to hide" : "-&gt; to show");
                str(out, line);
            }
            buf_t content = {0}, nested_answers = {0};
            /* Traverse closed bodies too: * must include nested folds. */
            transform(x, body, end.start, &content, &nested_answers, nesting + 1);
            if (x->pipe) {
                snprintf(line, sizeof(line), "<p><strong>%u. %s</strong></p>", id, label);
                str(answers, line); add(answers, content.s ? content.s : "", content.n);
                add(answers, nested_answers.s, nested_answers.n);
            } else if (opened) add(out, content.s ? content.s : "", content.n);
            if (content.failed || nested_answers.failed) out->failed = 1;
            free(nested_answers.s);
            free(content.s);
            str(out, "</div>");
            i = end.end; continue;
        }
        if (x->pipe && t.closing && !strcmp(t.name, "body")) {
            answers_end(out, answers); *answers = (buf_t){0};
        }
        add(out, s + i, t.end - i); i = t.end;
    }
}
tam_document_t *tam_fold_render(const char *src, size_t len, int width,
                                const char *open, int pipe)
{
    transform_t x = { .s = src, .open = open, .pipe = pipe };
    buf_t out = {0}, answers = {0};
    transform(&x, 0, len, &out, &answers, 0);
    answers_end(&out, &answers);
    tam_document_t *doc = NULL;
    if (!out.failed && !x.failed) doc = tam_render_html(out.s ? out.s : "", out.n, width);
    free(out.s);
    if (doc) { doc->folds = x.folds; doc->nfolds = x.nfolds; }
    else free(x.folds);
    return doc;
}
char *tam_fold_split_href(const char *href, char **open)
{
    *open = NULL;
    char *copy = strdup(href);
    if (!copy) return NULL;
    char *q = strchr(copy, '?');
    if (!q) return copy;
    char *p = q + 1;
    while (*p) {
        char *end = p + strcspn(p, "&#");
        if (!strncmp(p, "fold=", 5)) {
            *open = strndup(p + 5, (size_t)(end - p - 5));
            if (*end == '&') memmove(p, end + 1, strlen(end + 1) + 1);
            else { char *cut = p == q + 1 ? q : p - 1; memmove(cut, end, strlen(end) + 1); }
            break;
        }
        if (*end != '&') break;
        p = end + 1;
    }
    return copy;
}
