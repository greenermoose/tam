#include <tam.h>
#include "render.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* Block model (internal)                                                     */
/* ------------------------------------------------------------------------- */

enum {
    B_HEAD = 1,
    B_PARA,
    B_LIST,
    B_PRE,
    B_QUOTE,
    B_HR
};

typedef struct {
    int         kind;
    int         level;   /* heading level or list depth */
    int         ordered;
    int         number;
    tam_span_t *runs;
    size_t      nruns;
} tam_block_t;

typedef struct {
    tam_block_t *v;
    size_t       n;
    size_t       cap;
} blocklist_t;

typedef struct {
    tam_span_t *v;
    size_t      n;
    size_t      cap;
} spanlist_t;

typedef struct {
    tam_line_t *v;
    size_t      n;
    size_t      cap;
} linelist_t;

/* ------------------------------------------------------------------------- */
/* Small dynamic-array helpers                                               */
/* ------------------------------------------------------------------------- */

static int spanlist_append(spanlist_t *l, const char *text, size_t len,
                           unsigned style, const char *href)
{
    if (len == 0) {
        return 0;
    }
    if (l->n > 0) {
        tam_span_t *last = &l->v[l->n - 1];
        int same_href = (last->href == NULL && href == NULL) ||
                        (last->href != NULL && href != NULL &&
                         strcmp(last->href, href) == 0);
        if (last->style == style && same_href) {
            size_t old = strlen(last->text);
            char *nt = realloc(last->text, old + len + 1);
            if (nt == NULL) {
                return -1;
            }
            memcpy(nt + old, text, len);
            nt[old + len] = '\0';
            last->text = nt;
            return 0;
        }
    }
    if (l->n == l->cap) {
        size_t cap = l->cap ? l->cap * 2 : 8;
        tam_span_t *nv = realloc(l->v, cap * sizeof(*nv));
        if (nv == NULL) {
            return -1;
        }
        l->v = nv;
        l->cap = cap;
    }
    tam_span_t *s = &l->v[l->n++];
    s->text = malloc(len + 1);
    if (s->text == NULL) {
        l->n--;
        return -1;
    }
    memcpy(s->text, text, len);
    s->text[len] = '\0';
    s->style = style;
    s->href = href ? strdup(href) : NULL;
    return 0;
}

static void spanlist_free(spanlist_t *l)
{
    for (size_t i = 0; i < l->n; i++) {
        free(l->v[i].text);
        free(l->v[i].href);
    }
    free(l->v);
    l->v = NULL;
    l->n = 0;
    l->cap = 0;
}

static void spanlist_init(spanlist_t *l)
{
    l->v = NULL;
    l->n = 0;
    l->cap = 0;
}

static void linelist_grow(linelist_t *l)
{
    if (l->n == l->cap) {
        size_t cap = l->cap ? l->cap * 2 : 16;
        l->v = realloc(l->v, cap * sizeof(*l->v));
        l->cap = cap;
    }
}

static void linelist_push(linelist_t *ll, spanlist_t *sl)
{
    linelist_grow(ll);
    ll->v[ll->n].spans = sl->v;
    ll->v[ll->n].nspans = sl->n;
    ll->n++;
    spanlist_init(sl);
}

static void linelist_push_blank(linelist_t *ll)
{
    linelist_grow(ll);
    ll->v[ll->n].spans = NULL;
    ll->v[ll->n].nspans = 0;
    ll->n++;
}

static void linelist_blank(linelist_t *ll)
{
    if (ll->n == 0) {
        return;
    }
    if (ll->v[ll->n - 1].nspans == 0) {
        return;
    }
    linelist_push_blank(ll);
}

static void blocklist_add(blocklist_t *bl, int kind, int level, int ordered,
                          int number, tam_span_t *runs, size_t nruns)
{
    if (bl->n == bl->cap) {
        size_t cap = bl->cap ? bl->cap * 2 : 8;
        bl->v = realloc(bl->v, cap * sizeof(*bl->v));
        bl->cap = cap;
    }
    tam_block_t *b = &bl->v[bl->n++];
    b->kind = kind;
    b->level = level;
    b->ordered = ordered;
    b->number = number;
    b->runs = runs;
    b->nruns = nruns;
}

static void blocklist_free(blocklist_t *bl)
{
    for (size_t i = 0; i < bl->n; i++) {
        for (size_t j = 0; j < bl->v[i].nruns; j++) {
            free(bl->v[i].runs[j].text);
            free(bl->v[i].runs[j].href);
        }
        free(bl->v[i].runs);
    }
    free(bl->v);
    bl->v = NULL;
    bl->n = 0;
    bl->cap = 0;
}

/* ------------------------------------------------------------------------- */
/* Entities                                                                   */
/* ------------------------------------------------------------------------- */

static int utf8_encode(unsigned cp, char *out)
{
    if (cp < 0x80) {
        out[0] = (char)cp;
        return 1;
    }
    if (cp < 0x800) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    if (cp < 0x110000) {
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0;
}

static const struct {
    const char *name;
    unsigned    cp;
} entities[] = {
    {"amp", '&'},     {"lt", '<'},      {"gt", '>'},      {"quot", '"'},
    {"apos", '\''},   {"nbsp", 0xA0},   {"mdash", 0x2014}, {"ndash", 0x2013},
    {"hellip", 0x2026}, {"copy", 0xA9}, {"reg", 0xAE},
    {"lsquo", 0x2018}, {"rsquo", 0x2019}, {"ldquo", 0x201C}, {"rdquo", 0x201D},
    {NULL, 0}
};

static char *decode_entities(const char *s, size_t n, size_t *outlen)
{
    tam_str_t out = tam_str_new("");
    size_t i = 0;

    while (i < n) {
        if (s[i] == '&') {
            const char *semi = memchr(s + i, ';', n - i);
            if (semi != NULL && (size_t)(semi - (s + i)) <= 10) {
                size_t l = (size_t)(semi - (s + i + 1));
                char name[12];
                memcpy(name, s + i + 1, l);
                name[l] = '\0';
                unsigned cp = 0;
                int ok = 0;
                if (name[0] == '#') {
                    char *endp = NULL;
                    if (name[1] == 'x' || name[1] == 'X') {
                        cp = (unsigned)strtoul(name + 2, &endp, 16);
                    } else {
                        cp = (unsigned)strtoul(name + 1, &endp, 10);
                    }
                    if (endp != NULL && *endp == '\0') {
                        ok = 1;
                    }
                } else {
                    for (int k = 0; entities[k].name != NULL; k++) {
                        if (strcmp(entities[k].name, name) == 0) {
                            cp = entities[k].cp;
                            ok = 1;
                            break;
                        }
                    }
                }
                if (ok) {
                    char ub[4];
                    int ul = utf8_encode(cp, ub);
                    if (ul > 0) {
                        out = tam_str_cat_len(out, ub, (size_t)ul);
                        i = (size_t)(semi - s) + 1;
                        continue;
                    }
                }
            }
        }
        out = tam_str_cat_len(out, s + i, 1);
        i++;
    }

    char *res = strdup(out);
    if (outlen != NULL) {
        *outlen = strlen(res);
    }
    tam_str_free(out);
    return res;
}

/* ------------------------------------------------------------------------- */
/* Attribute extraction                                                       */
/* ------------------------------------------------------------------------- */

static char *extract_attr(const char *s, const char *end, const char *attr)
{
    size_t al = strlen(attr);
    const char *p = s;

    while (p + al <= end) {
        size_t k = 0;
        while (k < al && tolower((unsigned char)p[k]) == attr[k]) {
            k++;
        }
        if (k == al) {
            const char *q = p + al;
            if (q < end && (isalnum((unsigned char)*q) || *q == '-' || *q == '_')) {
                p++;
                continue;
            }
            while (q < end && isspace((unsigned char)*q)) {
                q++;
            }
            if (q < end && *q == '=') {
                q++;
                while (q < end && isspace((unsigned char)*q)) {
                    q++;
                }
                char quote = 0;
                if (q < end && (*q == '"' || *q == '\'')) {
                    quote = *q;
                    q++;
                }
                const char *start = q;
                if (quote) {
                    while (q < end && *q != quote) {
                        q++;
                    }
                } else {
                    while (q < end && !isspace((unsigned char)*q) && *q != '>') {
                        q++;
                    }
                }
                return decode_entities(start, (size_t)(q - start), NULL);
            }
        }
        p++;
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* Parser                                                                     */
/* ------------------------------------------------------------------------- */

typedef struct {
    int ordered;
    int counter;
} listctx_t;

typedef struct {
    blocklist_t blocks;

    int        kind;
    int        level;
    int        ordered;
    int        number;
    int        has_block;
    spanlist_t runs;

    unsigned   style;
    char      *href;

    listctx_t *lists;
    size_t     nlists;
    size_t     cap_lists;

    int        in_pre;
    int        in_title;
    tam_str_t  title;
    char       skip[16];
} parser_t;

static void list_push(parser_t *p, int ordered)
{
    if (p->nlists == p->cap_lists) {
        size_t cap = p->cap_lists ? p->cap_lists * 2 : 4;
        p->lists = realloc(p->lists, cap * sizeof(*p->lists));
        p->cap_lists = cap;
    }
    p->lists[p->nlists].ordered = ordered;
    p->lists[p->nlists].counter = 0;
    p->nlists++;
}

static void list_pop(parser_t *p)
{
    if (p->nlists > 0) {
        p->nlists--;
    }
}

static void parser_end_block(parser_t *p)
{
    if (!p->has_block) {
        return;
    }
    if (p->kind == B_HR || p->runs.n > 0) {
        blocklist_add(&p->blocks, p->kind, p->level, p->ordered, p->number,
                      p->runs.v, p->runs.n);
        spanlist_init(&p->runs);
    } else {
        spanlist_free(&p->runs);
    }
    p->has_block = 0;
    p->kind = 0;
}

static void parser_begin_block(parser_t *p, int kind, int level, int ordered,
                               int number)
{
    parser_end_block(p);
    p->kind = kind;
    p->level = level;
    p->ordered = ordered;
    p->number = number;
    p->has_block = 1;
}

static void parser_add_text(parser_t *p, const char *s, size_t n)
{
    size_t dl = 0;
    char *dec = decode_entities(s, n, &dl);
    if (dec == NULL) {
        return;
    }

    if (p->in_title) {
        p->title = tam_str_cat_len(p->title, dec, dl);
        free(dec);
        return;
    }

    if (p->in_pre) {
        if (!p->has_block) {
            parser_begin_block(p, B_PRE, 0, 0, 0);
        }
        spanlist_append(&p->runs, dec, dl, p->style | TAM_STYLE_CODE, p->href);
        free(dec);
        return;
    }

    int has_nonspace = 0;
    for (size_t i = 0; i < dl; i++) {
        if (!isspace((unsigned char)dec[i])) {
            has_nonspace = 1;
            break;
        }
    }
    if (!has_nonspace && !p->has_block) {
        free(dec);
        return;
    }
    if (!p->has_block) {
        parser_begin_block(p, B_PARA, 0, 0, 0);
    }

    size_t i = 0;
    size_t start = 0;
    while (i < dl) {
        if (isspace((unsigned char)dec[i])) {
            if (i > start) {
                spanlist_append(&p->runs, dec + start, i - start, p->style,
                                p->href);
            }
            spanlist_append(&p->runs, " ", 1, p->style, p->href);
            while (i + 1 < dl && isspace((unsigned char)dec[i + 1])) {
                i++;
            }
            start = i + 1;
        }
        i++;
    }
    if (dl > start) {
        spanlist_append(&p->runs, dec + start, dl - start, p->style, p->href);
    }
    free(dec);
}

static void parser_begin_list_item(parser_t *p)
{
    if (p->nlists == 0) {
        list_push(p, 0);
    }
    int idx = (int)p->nlists - 1;
    int ord = p->lists[idx].ordered;
    int num = 0;
    if (ord) {
        p->lists[idx].counter++;
        num = p->lists[idx].counter;
    }
    parser_begin_block(p, B_LIST, idx, ord, num);
}

static void parser_handle_tag(parser_t *p, const char *s, const char *end)
{
    int closing = 0;
    while (s < end && isspace((unsigned char)*s)) {
        s++;
    }
    if (s < end && *s == '/') {
        closing = 1;
        s++;
    }
    char name[16];
    size_t k = 0;
    while (s < end && k < 15 && isalnum((unsigned char)*s)) {
        name[k++] = (char)tolower((unsigned char)*s);
        s++;
    }
    name[k] = '\0';
    if (k == 0) {
        return;
    }

    if (strcmp(name, "a") == 0) {
        if (closing) {
            p->style &= ~TAM_STYLE_LINK;
            free(p->href);
            p->href = NULL;
        } else {
            char *h = extract_attr(s, end, "href");
            free(p->href);
            p->href = h;
            if (h != NULL) {
                p->style |= TAM_STYLE_LINK;
            } else {
                p->style &= ~TAM_STYLE_LINK;
            }
        }
        return;
    }
    if (strcmp(name, "code") == 0) {
        if (closing) {
            p->style &= ~TAM_STYLE_CODE;
        } else {
            p->style |= TAM_STYLE_CODE;
        }
        return;
    }
    if (strcmp(name, "strong") == 0 || strcmp(name, "b") == 0) {
        if (closing) {
            p->style &= ~TAM_STYLE_BOLD;
        } else {
            p->style |= TAM_STYLE_BOLD;
        }
        return;
    }
    if (strcmp(name, "em") == 0 || strcmp(name, "i") == 0) {
        if (closing) {
            p->style &= ~TAM_STYLE_ITALIC;
        } else {
            p->style |= TAM_STYLE_ITALIC;
        }
        return;
    }
    if (strcmp(name, "title") == 0) {
        p->in_title = closing ? 0 : 1;
        return;
    }
    if (strcmp(name, "script") == 0 || strcmp(name, "style") == 0) {
        if (!closing) {
            snprintf(p->skip, sizeof(p->skip), "%s", name);
        }
        return;
    }
    if (strcmp(name, "br") == 0) {
        if (p->has_block) {
            spanlist_append(&p->runs, "\n", 1, p->style, p->href);
        }
        return;
    }
    if (strcmp(name, "hr") == 0) {
        parser_begin_block(p, B_HR, 0, 0, 0);
        parser_end_block(p);
        return;
    }
    if (strcmp(name, "p") == 0) {
        if (closing) {
            parser_end_block(p);
        } else {
            parser_begin_block(p, B_PARA, 0, 0, 0);
        }
        return;
    }
    if (name[0] == 'h' && name[1] >= '1' && name[1] <= '3' && name[2] == '\0') {
        if (closing) {
            parser_end_block(p);
        } else {
            parser_begin_block(p, B_HEAD, name[1] - '0', 0, 0);
        }
        return;
    }
    if (strcmp(name, "ul") == 0) {
        parser_end_block(p);
        if (closing) {
            list_pop(p);
        } else {
            list_push(p, 0);
        }
        return;
    }
    if (strcmp(name, "ol") == 0) {
        parser_end_block(p);
        if (closing) {
            list_pop(p);
        } else {
            list_push(p, 1);
        }
        return;
    }
    if (strcmp(name, "li") == 0) {
        if (closing) {
            parser_end_block(p);
        } else {
            parser_begin_list_item(p);
        }
        return;
    }
    if (strcmp(name, "pre") == 0) {
        if (closing) {
            parser_end_block(p);
            p->in_pre = 0;
        } else {
            parser_begin_block(p, B_PRE, 0, 0, 0);
            p->in_pre = 1;
        }
        return;
    }
    if (strcmp(name, "blockquote") == 0) {
        if (closing) {
            parser_end_block(p);
        } else {
            parser_begin_block(p, B_QUOTE, 0, 0, 0);
        }
        return;
    }
}

static const char *find_sub(const char *hay, size_t hn, const char *needle,
                            size_t nn)
{
    if (nn == 0 || nn > hn) {
        return NULL;
    }
    for (size_t i = 0; i + nn <= hn; i++) {
        if (memcmp(hay + i, needle, nn) == 0) {
            return hay + i;
        }
    }
    return NULL;
}

static void parser_parse(parser_t *p, const char *src, size_t n)
{
    size_t i = 0;

    while (i < n) {
        if (p->skip[0] != '\0') {
            char close[24];
            int cl = snprintf(close, sizeof(close), "</%s", p->skip);
            const char *f = find_sub(src + i, n - i, close, (size_t)cl);
            if (f == NULL) {
                i = n;
                break;
            }
            const char *g = memchr(f, '>', n - (size_t)(f - src));
            if (g == NULL) {
                i = n;
                break;
            }
            p->skip[0] = '\0';
            i = (size_t)(g - src) + 1;
            continue;
        }

        if (src[i] == '<') {
            if (i + 4 <= n && memcmp(src + i, "<!--", 4) == 0) {
                const char *f = find_sub(src + i + 4, n - i - 4, "-->", 3);
                i = f ? (size_t)(f - src) + 3 : n;
                continue;
            }
            const char *g = memchr(src + i, '>', n - i);
            if (g == NULL) {
                parser_add_text(p, src + i, 1);
                i++;
                continue;
            }
            parser_handle_tag(p, src + i + 1, g);
            i = (size_t)(g - src) + 1;
            continue;
        }

        const char *lt = memchr(src + i, '<', n - i);
        size_t j = lt ? (size_t)(lt - src) : n;
        parser_add_text(p, src + i, j - i);
        i = j;
    }
}

/* ------------------------------------------------------------------------- */
/* Layout                                                                     */
/* ------------------------------------------------------------------------- */

static void start_line(spanlist_t *line, const char *prefix, size_t lead,
                       int first)
{
    spanlist_init(line);
    if (first) {
        spanlist_append(line, prefix, lead, 0, NULL);
        return;
    }
    char sp[160];
    size_t m = lead < sizeof(sp) ? lead : sizeof(sp) - 1;
    memset(sp, ' ', m);
    spanlist_append(line, sp, m, 0, NULL);
}

static void layout_flow(linelist_t *out, const tam_span_t *runs, size_t nruns,
                        int width, const char *prefix)
{
    size_t lead = strlen(prefix);
    if ((int)lead > width - 8) {
        lead = (size_t)(width > 8 ? width - 8 : 0);
    }

    spanlist_t line;
    int col = 0;

    start_line(&line, prefix, lead, 1);
    col = (int)lead;

    for (size_t r = 0; r < nruns; r++) {
        const char *t = runs[r].text;
        size_t tl = strlen(t);
        size_t i = 0;
        while (i < tl) {
            if (t[i] == '\n') {
                linelist_push(out, &line);
                start_line(&line, prefix, lead, 0);
                col = (int)lead;
                i++;
                continue;
            }
            if (isspace((unsigned char)t[i])) {
                i++;
                continue;
            }
            size_t ws = i;
            while (i < tl && !isspace((unsigned char)t[i]) && t[i] != '\n') {
                i++;
            }
            size_t wl = i - ws;
            int tight = strchr(",.;:!?%)]}", t[ws]) != NULL;
            int sep = (col > (int)lead && !tight) ? 1 : 0;
            if (col > (int)lead && col + sep + (int)wl > width) {
                linelist_push(out, &line);
                start_line(&line, prefix, lead, 0);
                col = (int)lead;
                sep = 0;
            }
            if (sep) {
                spanlist_append(&line, " ", 1, runs[r].style, runs[r].href);
                col++;
            }
            spanlist_append(&line, t + ws, wl, runs[r].style, runs[r].href);
            col += (int)wl;
        }
    }
    linelist_push(out, &line);
}

static void layout_pre(linelist_t *out, const tam_span_t *runs, size_t nruns)
{
    spanlist_t line;
    spanlist_init(&line);
    spanlist_append(&line, "  ", 2, 0, NULL);

    for (size_t r = 0; r < nruns; r++) {
        const char *t = runs[r].text;
        size_t tl = strlen(t);
        size_t start = 0;
        for (size_t i = 0; i <= tl; i++) {
            if (i == tl || t[i] == '\n') {
                if (i > start) {
                    spanlist_append(&line, t + start, i - start, runs[r].style,
                                    runs[r].href);
                }
                if (i == tl) {
                    break;
                }
                linelist_push(out, &line);
                spanlist_init(&line);
                spanlist_append(&line, "  ", 2, 0, NULL);
                start = i + 1;
            }
        }
    }
    linelist_push(out, &line);
}

static void block_prefix(const tam_block_t *b, char *buf, size_t cap)
{
    if (b->kind == B_LIST) {
        int indent = b->level * 2;
        if (indent > 20) {
            indent = 20;
        }
        if (b->ordered) {
            snprintf(buf, cap, "%*s%d. ", indent, "", b->number);
        } else {
            snprintf(buf, cap, "%*s- ", indent, "");
        }
    } else if (b->kind == B_QUOTE) {
        snprintf(buf, cap, "  ");
    } else {
        snprintf(buf, cap, "%s", "");
    }
}

static void layout_document(const blocklist_t *blocks, int width,
                            linelist_t *out)
{
    if (width < 16) {
        width = 16;
    }

    for (size_t i = 0; i < blocks->n; i++) {
        const tam_block_t *b = &blocks->v[i];
        if (i > 0) {
            const tam_block_t *prev = &blocks->v[i - 1];
            int tight = (b->kind == B_LIST && prev->kind == B_LIST &&
                         b->level == prev->level);
            if (!tight) {
                linelist_blank(out);
            }
        }

        if (b->kind == B_HR) {
            spanlist_t l;
            spanlist_init(&l);
            int n = width < 72 ? width : 72;
            if (n < 3) {
                n = 3;
            }
            char *dash = malloc((size_t)n + 1);
            memset(dash, '-', (size_t)n);
            dash[n] = '\0';
            spanlist_append(&l, dash, (size_t)n, 0, NULL);
            free(dash);
            linelist_push(out, &l);
            continue;
        }
        if (b->kind == B_PRE) {
            layout_pre(out, b->runs, b->nruns);
            continue;
        }
        char pfx[80];
        block_prefix(b, pfx, sizeof(pfx));
        layout_flow(out, b->runs, b->nruns, width, pfx);
    }
}

/* ------------------------------------------------------------------------- */
/* Public API                                                                 */
/* ------------------------------------------------------------------------- */

tam_document_t *tam_render_html(const char *src, size_t len, int width)
{
    parser_t p;
    memset(&p, 0, sizeof(p));
    p.title = tam_str_new("");

    parser_parse(&p, src, len);
    parser_end_block(&p);

    for (size_t i = 0; i < p.blocks.n; i++) {
        if (p.blocks.v[i].kind == B_HEAD) {
            for (size_t j = 0; j < p.blocks.v[i].nruns; j++) {
                p.blocks.v[i].runs[j].style |= TAM_STYLE_HEADING;
            }
        }
    }

    linelist_t out;
    memset(&out, 0, sizeof(out));
    layout_document(&p.blocks, width, &out);
    if (out.n == 0) {
        linelist_push_blank(&out);
    }

    tam_document_t *doc = malloc(sizeof(*doc));
    const char *title = p.title ? tam_str_trim(p.title, " \t\r\n") : NULL;
    doc->title = strdup((title != NULL && title[0] != '\0') ? title : "tam");
    doc->lines = out.v;
    doc->nlines = out.n;

    tam_str_free(p.title);
    blocklist_free(&p.blocks);
    free(p.lists);
    free(p.href);
    return doc;
}

void tam_document_free(tam_document_t *doc)
{
    if (doc == NULL) {
        return;
    }
    for (size_t i = 0; i < doc->nlines; i++) {
        for (size_t j = 0; j < doc->lines[i].nspans; j++) {
            free(doc->lines[i].spans[j].text);
            free(doc->lines[i].spans[j].href);
        }
        free(doc->lines[i].spans);
    }
    free(doc->lines);
    free(doc->title);
    free(doc);
}

char *tam_document_text(const tam_document_t *doc)
{
    tam_str_t s = tam_str_new("");

    for (size_t i = 0; i < doc->nlines; i++) {
        for (size_t j = 0; j < doc->lines[i].nspans; j++) {
            s = tam_str_cat(s, doc->lines[i].spans[j].text);
        }
        s = tam_str_cat(s, "\n");
    }

    const char **links = NULL;
    size_t nlinks = 0;
    for (size_t i = 0; i < doc->nlines; i++) {
        for (size_t j = 0; j < doc->lines[i].nspans; j++) {
            const char *h = doc->lines[i].spans[j].href;
            if (h == NULL) {
                continue;
            }
            int seen = 0;
            for (size_t k = 0; k < nlinks; k++) {
                if (strcmp(links[k], h) == 0) {
                    seen = 1;
                    break;
                }
            }
            if (!seen) {
                links = realloc(links, (nlinks + 1) * sizeof(*links));
                links[nlinks++] = h;
            }
        }
    }

    if (nlinks > 0) {
        s = tam_str_cat(s, "\nLinks:\n");
        for (size_t k = 0; k < nlinks; k++) {
            s = tam_str_cat_printf(s, "  [%zu] %s\n", k + 1, links[k]);
        }
    }

    char *res = strdup(s);
    tam_str_free(s);
    free(links);
    return res;
}
