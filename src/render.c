#include <tam.h>
#include "display.h"
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
    B_HR,
    B_DT,
    B_DD,
    B_TABLE,
    B_META  /* <p class="level-line">: dimmed, right-aligned when it fits */
};

typedef struct {
    tam_span_t *runs;
    size_t      nruns;
    int         is_header;
} tam_table_cell_t;

typedef struct {
    tam_table_cell_t *cells;
    size_t            ncells;
    size_t            cap_cells;
    int               is_header_row;
} tam_table_row_t;

typedef struct {
    tam_table_row_t *rows;
    size_t           nrows;
    size_t           cap_rows;
    size_t           ncols;
} tam_table_t;

typedef struct {
    int          kind;
    int          level;   /* heading level or list depth */
    int          ordered;
    int          number;
    tam_span_t  *runs;
    size_t       nruns;
    tam_table_t *table;  /* non-NULL for B_TABLE */
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
    size_t      logical_block;
    size_t      logical_offset;
} linelist_t;

/* An in-page link target (id= on any element, or name= on <a>), recorded by
 * block so it survives width reflow. */
typedef struct {
    char  *id;
    size_t block;
} anchor_t;

typedef struct {
    anchor_t *v;
    size_t    n;
    size_t    cap;
} anchorlist_t;

typedef struct {
    blocklist_t  blocks;
    anchorlist_t anchors;
} layout_backing_t;

/* ------------------------------------------------------------------------- */
/* Small dynamic-array helpers                                               */
/* ------------------------------------------------------------------------- */

static int spanlist_append(spanlist_t *l, const char *text, size_t len,
                           unsigned style, const char *href, int link_id)
{
    if (len == 0) {
        return 0;
    }
    if (l->n > 0) {
        tam_span_t *last = &l->v[l->n - 1];
        int same_href = (last->href == NULL && href == NULL) ||
                        (last->href != NULL && href != NULL &&
                         strcmp(last->href, href) == 0);
        int same_link = (last->link_id == link_id);
        if (last->style == style && same_href && same_link) {
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
    s->link_id = link_id;
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
    ll->v[ll->n].logical_block = ll->logical_block;
    ll->v[ll->n].logical_offset = ll->logical_offset;
    ll->n++;
    spanlist_init(sl);
}

static void linelist_push_blank(linelist_t *ll)
{
    linelist_grow(ll);
    ll->v[ll->n].spans = NULL;
    ll->v[ll->n].nspans = 0;
    ll->v[ll->n].logical_block = ll->logical_block;
    ll->v[ll->n].logical_offset = ll->logical_offset;
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
    b->table = NULL;
}

static void table_free(tam_table_t *t)
{
    if (t == NULL) {
        return;
    }
    for (size_t r = 0; r < t->nrows; r++) {
        tam_table_row_t *row = &t->rows[r];
        for (size_t c = 0; c < row->ncells; c++) {
            tam_table_cell_t *cell = &row->cells[c];
            for (size_t k = 0; k < cell->nruns; k++) {
                free(cell->runs[k].text);
                free(cell->runs[k].href);
            }
            free(cell->runs);
        }
        free(row->cells);
    }
    free(t->rows);
    free(t);
}

static void blocklist_add_table(blocklist_t *bl, tam_table_t *table)
{
    if (bl->n == bl->cap) {
        size_t cap = bl->cap ? bl->cap * 2 : 8;
        bl->v = realloc(bl->v, cap * sizeof(*bl->v));
        bl->cap = cap;
    }
    tam_block_t *b = &bl->v[bl->n++];
    b->kind = B_TABLE;
    b->level = 0;
    b->ordered = 0;
    b->number = 0;
    b->runs = NULL;
    b->nruns = 0;
    b->table = table;
}

static void anchorlist_add(anchorlist_t *al, char *id, size_t block)
{
    if (id == NULL || id[0] == '\0') {
        free(id);
        return;
    }
    if (al->n == al->cap) {
        size_t cap = al->cap ? al->cap * 2 : 8;
        al->v = realloc(al->v, cap * sizeof(*al->v));
        al->cap = cap;
    }
    al->v[al->n].id = id;
    al->v[al->n].block = block;
    al->n++;
}

static void anchorlist_free(anchorlist_t *al)
{
    for (size_t i = 0; i < al->n; i++) {
        free(al->v[i].id);
    }
    free(al->v);
    memset(al, 0, sizeof(*al));
}

static void blocklist_free(blocklist_t *bl)
{
    for (size_t i = 0; i < bl->n; i++) {
        if (bl->v[i].table != NULL) {
            table_free(bl->v[i].table);
        }
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
    {"larr", 0x2190},  {"rarr", 0x2192},  {"times", 0xD7},   {"le", 0x2264},
    {"ge", 0x2265},    {"asymp", 0x2248},
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
    blocklist_t  blocks;
    anchorlist_t anchors;

    int        kind;
    int        level;
    int        ordered;
    int        number;
    int        has_block;
    spanlist_t runs;

    unsigned   style;
    char      *href;
    int        link_id;
    int        next_link_id;

    listctx_t *lists;
    size_t     nlists;
    size_t     cap_lists;

    int        in_pre;
    int        in_title;
    tam_str_t  title;
    char       skip[16];

    tam_table_t *curr_table;
    int          in_table;
    int          in_tr;
    int          in_cell;
    int          cell_is_header;

    char        *nav_next;
    char        *nav_prev;
    char        *nav_up;
    char        *nav_top;
    int           in_breadcrumb;
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

static void parser_finish_cell(parser_t *p)
{
    if (!p->in_cell || p->curr_table == NULL || p->curr_table->nrows == 0) {
        return;
    }
    tam_table_row_t *row = &p->curr_table->rows[p->curr_table->nrows - 1];
    if (row->ncells == row->cap_cells) {
        size_t cap = row->cap_cells ? row->cap_cells * 2 : 4;
        row->cells = realloc(row->cells, cap * sizeof(tam_table_cell_t));
        row->cap_cells = cap;
    }
    tam_table_cell_t *c = &row->cells[row->ncells++];
    c->runs = p->runs.v;
    c->nruns = p->runs.n;
    c->is_header = p->cell_is_header;
    if (c->is_header) {
        row->is_header_row = 1;
        for (size_t k = 0; k < c->nruns; k++) {
            c->runs[k].style |= TAM_STYLE_BOLD;
        }
    }
    if (row->ncells > p->curr_table->ncols) {
        p->curr_table->ncols = row->ncells;
    }
    spanlist_init(&p->runs);
    p->in_cell = 0;
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
    if (p->in_breadcrumb) return;
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
        spanlist_append(&p->runs, dec, dl, p->style | TAM_STYLE_CODE, p->href,
                        p->link_id);
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
    if (!has_nonspace && !p->has_block && !p->in_cell) {
        free(dec);
        return;
    }
    if (!p->has_block && !p->in_cell) {
        parser_begin_block(p, B_PARA, 0, 0, 0);
    }

    size_t i = 0;
    size_t start = 0;
    while (i < dl) {
        if (isspace((unsigned char)dec[i])) {
            if (i > start) {
                spanlist_append(&p->runs, dec + start, i - start, p->style,
                                p->href, p->link_id);
            }
            spanlist_append(&p->runs, " ", 1, p->style, p->href, p->link_id);
            while (i + 1 < dl && isspace((unsigned char)dec[i + 1])) {
                i++;
            }
            start = i + 1;
        }
        i++;
    }
    if (dl > start) {
        spanlist_append(&p->runs, dec + start, dl - start, p->style, p->href,
                        p->link_id);
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

    if (strcmp(name, "nav") == 0) {
        if (closing) {
            p->in_breadcrumb = 0;
        } else {
            char *label = extract_attr(s, end, "aria-label");
            if (label != NULL && strcasecmp(label, "Breadcrumb") == 0)
                p->in_breadcrumb = 1;
            free(label);
        }
        return;
    }
    if (p->in_breadcrumb) return;

    if (!closing) {
        /* The open block, or the next one, takes index blocks.n when it ends. */
        anchorlist_add(&p->anchors, extract_attr(s, end, "id"), p->blocks.n);
        if (strcmp(name, "a") == 0)
            anchorlist_add(&p->anchors, extract_attr(s, end, "name"), p->blocks.n);
    }

    if (strcmp(name, "a") == 0) {
        if (closing) {
            p->style &= ~TAM_STYLE_LINK;
            free(p->href);
            p->href = NULL;
            p->link_id = 0;
        } else {
            char *h = extract_attr(s, end, "href");
            free(p->href);
            p->href = h;
            if (h != NULL) {
                p->style |= TAM_STYLE_LINK;
                p->link_id = ++p->next_link_id;
            } else {
                p->style &= ~TAM_STYLE_LINK;
                p->link_id = 0;
            }
        }
        return;
    }
    if (strcmp(name, "link") == 0) {
        if (!closing) {
            char *rel = extract_attr(s, end, "rel");
            char *href = extract_attr(s, end, "href");
            if (rel != NULL && href != NULL) {
                if (strcasecmp(rel, "next") == 0) {
                    free(p->nav_next);
                    p->nav_next = strdup(href);
                } else if (strcasecmp(rel, "prev") == 0) {
                    free(p->nav_prev);
                    p->nav_prev = strdup(href);
                } else if (strcasecmp(rel, "up") == 0) {
                    free(p->nav_up);
                    p->nav_up = strdup(href);
                } else if (strcasecmp(rel, "top") == 0) {
                    free(p->nav_top);
                    p->nav_top = strdup(href);
                }
            }
            free(rel);
            free(href);
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
    if (strcmp(name, "em") == 0 || strcmp(name, "i") == 0 || strcmp(name, "dfn") == 0) {
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
            spanlist_append(&p->runs, "\n", 1, p->style, p->href, p->link_id);
        }
        return;
    }
    if (strcmp(name, "hr") == 0) {
        parser_begin_block(p, B_HR, 0, 0, 0);
        parser_end_block(p);
        return;
    }
    if (strcmp(name, "section") == 0 || strcmp(name, "article") == 0 ||
        strcmp(name, "div") == 0) {
        parser_end_block(p);
        return;
    }
    if (strcmp(name, "p") == 0) {
        if (closing) {
            parser_end_block(p);
        } else {
            char *cls = extract_attr(s, end, "class");
            int meta = cls != NULL && strcmp(cls, "level-line") == 0;
            free(cls);
            parser_begin_block(p, meta ? B_META : B_PARA, 0, 0, 0);
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
    if (strcmp(name, "aside") == 0) {
        parser_end_block(p);
        if (!closing) {
            char *classes = extract_attr(s, end, "class");
            static const struct { const char *key, *label; } labels[] = {
                {"note", "Note:"}, {"tip", "Tip:"}, {"caution", "Caution:"},
                {"tamlinux", "On Tamlinux:"}, {"why", "Why:"},
                {"peek", "Peek under the hood:"}
            };
            char *save = NULL;
            for (char *c = classes ? strtok_r(classes, " \t\r\n", &save) : NULL;
                 c; c = strtok_r(NULL, " \t\r\n", &save)) {
                for (size_t i = 0; i < sizeof(labels)/sizeof(labels[0]); i++) {
                    if (strcmp(c, labels[i].key)) continue;
                    parser_begin_block(p, B_PARA, 0, 0, 0);
                    spanlist_append(&p->runs, labels[i].label, strlen(labels[i].label),
                                    TAM_STYLE_BOLD, NULL, 0);
                    parser_end_block(p);
                    goto aside_done;
                }
            }
        aside_done:
            free(classes);
        }
        return;
    }
    if (strcmp(name, "pre") == 0) {
        if (closing) {
            parser_end_block(p);
            p->in_pre = 0;
        } else {
            char *risk = extract_attr(s, end, "data-risk");
            static const struct { const char *key, *label; } risks[] = {
                {"read-only", "[read-only]"}, {"scratch", "[scratch directory]"},
                {"user", "[changes your files]"}, {"system", "[changes the system]"},
                {"boot", "[affects boot]"}
            };
            for (size_t i = 0; risk && i < sizeof(risks)/sizeof(risks[0]); i++) {
                if (strcmp(risk, risks[i].key)) continue;
                parser_begin_block(p, B_PARA, 0, 0, 0);
                spanlist_append(&p->runs, risks[i].label, strlen(risks[i].label),
                                TAM_STYLE_BOLD, NULL, 0);
                parser_end_block(p);
            }
            free(risk);
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
    if (strcmp(name, "dl") == 0) {
        parser_end_block(p);
        return;
    }
    if (strcmp(name, "dt") == 0) {
        if (closing) {
            parser_end_block(p);
        } else {
            parser_begin_block(p, B_DT, 0, 0, 0);
        }
        return;
    }
    if (strcmp(name, "dd") == 0) {
        if (closing) {
            parser_end_block(p);
        } else {
            parser_begin_block(p, B_DD, 0, 0, 0);
        }
        return;
    }
    if (strcmp(name, "table") == 0) {
        if (closing) {
            parser_finish_cell(p);
            p->in_cell = 0;
            p->in_tr = 0;
            p->in_table = 0;
            if (p->curr_table != NULL) {
                if (p->curr_table->nrows > 0) {
                    blocklist_add_table(&p->blocks, p->curr_table);
                } else {
                    table_free(p->curr_table);
                }
                p->curr_table = NULL;
            }
        } else {
            parser_end_block(p);
            p->curr_table = calloc(1, sizeof(tam_table_t));
            p->in_table = 1;
            p->in_tr = 0;
            p->in_cell = 0;
        }
        return;
    }
    if (strcmp(name, "thead") == 0 || strcmp(name, "tbody") == 0 ||
        strcmp(name, "tfoot") == 0) {
        return;
    }
    if (strcmp(name, "tr") == 0) {
        if (!p->in_table || p->curr_table == NULL) {
            return;
        }
        if (closing) {
            parser_finish_cell(p);
            p->in_tr = 0;
        } else {
            parser_finish_cell(p);
            if (p->curr_table->nrows == p->curr_table->cap_rows) {
                size_t cap = p->curr_table->cap_rows ? p->curr_table->cap_rows * 2 : 4;
                p->curr_table->rows = realloc(p->curr_table->rows, cap * sizeof(tam_table_row_t));
                p->curr_table->cap_rows = cap;
            }
            tam_table_row_t *row = &p->curr_table->rows[p->curr_table->nrows++];
            memset(row, 0, sizeof(*row));
            p->in_tr = 1;
        }
        return;
    }
    if (strcmp(name, "th") == 0 || strcmp(name, "td") == 0) {
        if (!p->in_table || p->curr_table == NULL || !p->in_tr) {
            return;
        }
        if (closing) {
            parser_finish_cell(p);
        } else {
            parser_finish_cell(p);
            p->in_cell = 1;
            p->cell_is_header = (strcmp(name, "th") == 0);
            spanlist_init(&p->runs);
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
            const char *g = NULL;
            int in_quote = 0;
            char qchar = 0;
            for (size_t k = i + 1; k < n; k++) {
                if (in_quote) {
                    if (src[k] == qchar) {
                        in_quote = 0;
                    }
                } else if (src[k] == '"' || src[k] == '\'') {
                    in_quote = 1;
                    qchar = src[k];
                } else if (src[k] == '>') {
                    g = src + k;
                    break;
                }
            }
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

static const char *find_ci_bounded(const char *start, size_t len, const char *needle)
{
    size_t nlen = strlen(needle);
    if (nlen > len) return NULL;
    for (size_t i = 0; i + nlen <= len; i++) {
        if (strncasecmp(start + i, needle, nlen) == 0) return start + i;
    }
    return NULL;
}

static char *plain_fragment(const char *start, size_t len)
{
    char *raw = malloc(len + 1);
    if (raw == NULL) return NULL;
    size_t out = 0;
    int in_tag = 0;
    for (size_t i = 0; i < len; i++) {
        if (start[i] == '<') in_tag = 1;
        else if (start[i] == '>') in_tag = 0;
        else if (!in_tag) raw[out++] = start[i];
    }
    size_t decoded_len = 0;
    char *decoded = decode_entities(raw, out, &decoded_len);
    free(raw);
    if (decoded == NULL) return NULL;
    char *begin = decoded;
    while (*begin && (isspace((unsigned char)*begin) || *begin == '/')) begin++;
    size_t n = strlen(begin);
    while (n > 0 && (isspace((unsigned char)begin[n - 1]) || begin[n - 1] == '/'))
        begin[--n] = '\0';
    if (begin != decoded) memmove(decoded, begin, n + 1);
    return decoded;
}

static tam_line_t extract_location(const char *src, size_t len)
{
    spanlist_t spans = {0};
    const char *nav = find_ci_bounded(src, len, "<nav");
    const char *nav_end = NULL;
    while (nav != NULL) {
        const char *tag_end = memchr(nav, '>', len - (size_t)(nav - src));
        if (tag_end == NULL) break;
        char *label = extract_attr(nav + 4, tag_end, "aria-label");
        if (label != NULL && strcasecmp(label, "Breadcrumb") == 0) {
            nav_end = find_ci_bounded(tag_end + 1,
                len - (size_t)(tag_end + 1 - src), "</nav>");
            free(label);
            break;
        }
        free(label);
        size_t used = (size_t)(tag_end + 1 - src);
        if (used >= len) break;
        nav = find_ci_bounded(src + used, len - used, "<nav");
    }
    if (nav_end == NULL) return (tam_line_t){0};

    const char *body = strchr(nav, '>') + 1;
    const char *cursor = body;
    size_t link_id = 0;
    int have_part = 0;
    while (cursor < nav_end) {
        const char *a = find_ci_bounded(cursor, (size_t)(nav_end - cursor), "<a");
        if (a == NULL) break;
        const char *a_end = memchr(a, '>', (size_t)(nav_end - a));
        if (a_end == NULL) break;
        const char *close = find_ci_bounded(a_end + 1, (size_t)(nav_end - a_end - 1), "</a>");
        if (close == NULL) break;
        char *href = extract_attr(a + 2, a_end, "href");
        char *text = plain_fragment(a_end + 1, (size_t)(close - a_end - 1));
        if (href != NULL && text != NULL && text[0] != '\0') {
            if (have_part) spanlist_append(&spans, " > ", 3, 0, NULL, 0);
            spanlist_append(&spans, text, strlen(text), TAM_STYLE_LINK, href, (int)++link_id);
            have_part = 1;
        }
        free(href);
        free(text);
        cursor = close + 4;
    }
    char *tail = plain_fragment(cursor, (size_t)(nav_end - cursor));
    if (tail != NULL && tail[0] != '\0') {
        if (have_part) spanlist_append(&spans, " > ", 3, 0, NULL, 0);
        spanlist_append(&spans, tail, strlen(tail), 0, NULL, 0);
    }
    free(tail);
    return (tam_line_t){.spans = spans.v, .nspans = spans.n};
}

static void location_append(tam_document_t *doc, const char *label,
                           const char *href)
{
    if (doc == NULL || label == NULL || label[0] == '\0') return;
    tam_line_t *line = &doc->location;
    size_t extra = line->nspans > 0 ? 2 : 1;
    tam_span_t *spans = realloc(line->spans,
                                (line->nspans + extra) * sizeof(*spans));
    if (spans == NULL) return;
    line->spans = spans;
    if (line->nspans > 0) {
        line->spans[line->nspans++] = (tam_span_t){.text = strdup(" > ")};
    }
    int link_id = href != NULL ? (int)line->nspans + 1 : 0;
    line->spans[line->nspans++] = (tam_span_t){
        .text = strdup(label), .style = href != NULL ? TAM_STYLE_LINK : 0,
        .href = href != NULL ? strdup(href) : NULL, .link_id = link_id
    };
}

void tam_document_ensure_location(tam_document_t *doc, const char *href)
{
    if (doc == NULL || doc->location.nspans > 0) return;
    if (href == NULL || strcmp(href, "index.html") == 0) {
        location_append(doc, "Guide", NULL);
        return;
    }
    if (strncmp(href, "man:", 4) == 0) {
        location_append(doc, "Guide", "index.html");
        location_append(doc, "Man pages", "man:");
        location_append(doc, doc->title ? doc->title : href + 4, NULL);
        return;
    }
    if (strncmp(href, "tldr:", 5) == 0) {
        location_append(doc, "Guide", "index.html");
        location_append(doc, "tldr pages", "tam:tldr");
        location_append(doc, doc->title ? doc->title : href + 5, NULL);
        return;
    }
    if (strncmp(href, "tam:tldr:", 9) == 0) {
        location_append(doc, "Guide", "index.html");
        location_append(doc, "tldr pages", "tam:tldr");
        location_append(doc, doc->title ? doc->title : href + 9, NULL);
        return;
    }
    if (strncmp(href, "info:", 5) == 0) {
        const char *spec = href + 5;
        location_append(doc, "Guide", "index.html");
        location_append(doc, "Info manuals", "info:dir");
        if (strcmp(spec, "dir") != 0) {
            const char *slash = strchr(spec, '/');
            size_t n = slash ? (size_t)(slash - spec) : strlen(spec);
            char manual[128];
            if (n >= sizeof(manual)) n = sizeof(manual) - 1;
            memcpy(manual, spec, n);
            manual[n] = '\0';
            location_append(doc, manual, NULL);
            if (slash != NULL)
                location_append(doc, doc->title ? doc->title : slash + 1, NULL);
        }
        return;
    }
    location_append(doc, "Guide", "index.html");
    location_append(doc, doc->title ? doc->title : href, NULL);
}

/* ------------------------------------------------------------------------- */
/* Layout                                                                     */
/* ------------------------------------------------------------------------- */

static void start_line(spanlist_t *line, const char *prefix, size_t lead,
                       int first)
{
    spanlist_init(line);
    if (first) {
        spanlist_append(line, prefix, lead, 0, NULL, 0);
        return;
    }
    char sp[160];
    size_t m = lead < sizeof(sp) ? lead : sizeof(sp) - 1;
    memset(sp, ' ', m);
    spanlist_append(line, sp, m, 0, NULL, 0);
}

static size_t dwidth(const char *s, size_t n)
{
    return tam_display_width(s, n);
}

/* A token made only of sentence punctuation stays with what precedes it, so
 * it is never left alone at the start of a line. */
static int all_punct(const char *s, size_t n)
{
    if (n == 0) {
        return 0;
    }
    for (size_t i = 0; i < n; i++) {
        if (strchr(",.;:!?%)]}", s[i]) == NULL) {
            return 0;
        }
    }
    return 1;
}

/* Width of the punctuation token that follows the word ending at (r, i),
 * including a separating space if whitespace precedes it, so a break reserves
 * room for it instead of stranding it past the margin or at line start. */
static size_t peek_following_punct(const tam_span_t *runs, size_t nruns,
                                   size_t r, size_t i)
{
    size_t rr = r;
    size_t k = i;
    size_t sp = 0;

    while (rr < nruns) {
        const char *t = runs[rr].text;
        size_t tl = (t != NULL) ? strlen(t) : 0;
        while (k < tl && isspace((unsigned char)t[k])) {
            if (t[k] == '\n') {
                return 0;
            }
            sp = 1;
            k++;
        }
        if (k >= tl) {
            rr++;
            k = 0;
            continue;
        }
        size_t s = k;
        while (k < tl && !isspace((unsigned char)t[k]) && t[k] != '\n') {
            k++;
        }
        if (all_punct(t + s, k - s)) {
            return sp + dwidth(t + s, k - s);
        }
        return 0;
    }
    return 0;
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
    int prev = 0;
    int had_space = 0;
    size_t run_base = 0;
    size_t line_offset = 0;

    start_line(&line, prefix, lead, 1);
    col = (int)lead;

    for (size_t r = 0; r < nruns; r++) {
        const char *t = runs[r].text;
        size_t tl = strlen(t);
        size_t i = 0;
        while (i < tl) {
            if (t[i] == '\n') {
                out->logical_offset = line_offset;
                linelist_push(out, &line);
                line_offset = run_base + i + 1;
                start_line(&line, prefix, lead, 0);
                col = (int)lead;
                prev = 0;
                had_space = 0;
                i++;
                continue;
            }
            if (isspace((unsigned char)t[i])) {
                had_space = 1;
                i++;
                continue;
            }
            size_t ws = i;
            while (i < tl && !isspace((unsigned char)t[i]) && t[i] != '\n') {
                i++;
            }
            size_t wb = i - ws;
            size_t wl = dwidth(t + ws, wb);
            int tight = !had_space && strchr(",.;:!?%)]}", t[ws]) != NULL;
            int attach = !had_space && (prev == '(' || prev == '[' || prev == '{');
            int sep = (col > (int)lead && !tight && !attach) ? 1 : 0;
            size_t extra = peek_following_punct(runs, nruns, r, i);
            if (col > (int)lead && !all_punct(t + ws, wb) &&
                col + sep + (int)(wl + extra) > width) {
                out->logical_offset = line_offset;
                linelist_push(out, &line);
                line_offset = run_base + ws;
                start_line(&line, prefix, lead, 0);
                col = (int)lead;
                sep = 0;
                prev = 0;
            }
            if (sep) {
                spanlist_append(&line, " ", 1, runs[r].style, runs[r].href,
                                runs[r].link_id);
                col++;
            }
            if (wl > (size_t)(width - (int)lead)) {
                size_t off = ws;
                while (off < i) {
                    size_t char_start = off;
                    size_t bytes = 0;
                    wchar_t wc;
                    int cells = 0;
                    (void)tam_display_next(t, i, &off, &bytes, &wc, &cells);
                    if (cells > 0 && col > (int)lead && col + cells > width) {
                        out->logical_offset = run_base + char_start;
                        linelist_push(out, &line);
                        start_line(&line, prefix, lead, 0);
                        col = (int)lead;
                    }
                    if (cells > width - col) {
                        spanlist_append(&line, "?", 1, runs[r].style,
                                        runs[r].href, runs[r].link_id);
                        col++;
                    } else {
                        spanlist_append(&line, wc == 0xfffd ? "?" : t + char_start,
                                        wc == 0xfffd ? 1 : bytes,
                                        runs[r].style, runs[r].href,
                                        runs[r].link_id);
                        col += cells;
                    }
                }
                prev = (unsigned char)t[ws + wb - 1];
                had_space = 0;
                continue;
            }
            spanlist_append(&line, t + ws, wb, runs[r].style, runs[r].href,
                            runs[r].link_id);
            col += (int)wl;
            prev = (unsigned char)t[ws + wb - 1];
            had_space = 0;
        }
        run_base += tl;
    }
    out->logical_offset = line_offset;
    linelist_push(out, &line);
}

static void layout_pre(linelist_t *out, const tam_span_t *runs, size_t nruns,
                       int width)
{
    int indent = width > 2 ? 2 : width - 1;
    if (indent < 0) indent = 0;
    spanlist_t line;
    spanlist_init(&line);
    for (int i = 0; i < indent; i++) spanlist_append(&line, " ", 1, 0, NULL, 0);
    int col = indent;
    size_t run_base = 0;
    size_t line_offset = 0;

    for (size_t r = 0; r < nruns; r++) {
        const char *t = runs[r].text;
        size_t len = strlen(t);
        size_t off = 0;
        while (off < len) {
            size_t char_start = off;
            size_t bytes = 0;
            wchar_t wc;
            int cells = 0;
            (void)tam_display_next(t, len, &off, &bytes, &wc, &cells);
            if (t[char_start] == '\n') {
                out->logical_offset = line_offset;
                linelist_push(out, &line);
                line_offset = run_base + off;
                spanlist_init(&line);
                for (int i = 0; i < indent; i++) spanlist_append(&line, " ", 1, 0, NULL, 0);
                col = indent;
                continue;
            }
            if (cells > 0 && col + cells > width && col > indent) {
                out->logical_offset = line_offset;
                linelist_push(out, &line);
                line_offset = run_base + char_start;
                spanlist_init(&line);
                for (int i = 0; i < indent; i++) spanlist_append(&line, " ", 1, 0, NULL, 0);
                col = indent;
            }
            if (cells > width - col) {
                spanlist_append(&line, "?", 1, runs[r].style, runs[r].href,
                                runs[r].link_id);
                col++;
            } else {
                spanlist_append(&line, wc == 0xfffd ? "?" : t + char_start,
                                wc == 0xfffd ? 1 : bytes, runs[r].style,
                                runs[r].href, runs[r].link_id);
                col += cells;
            }
        }
        run_base += len;
    }
    out->logical_offset = line_offset;
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
    } else if (b->kind == B_DT) {
        snprintf(buf, cap, "%s", "");
    } else if (b->kind == B_DD) {
        snprintf(buf, cap, "    ");
    } else {
        snprintf(buf, cap, "%s", "");
    }
}

static size_t cell_text_len(const tam_table_cell_t *cell)
{
    size_t len = 0;
    for (size_t i = 0; i < cell->nruns; i++) {
        if (cell->runs[i].text != NULL) {
            len += dwidth(cell->runs[i].text, strlen(cell->runs[i].text));
        }
    }
    return len;
}

/* Width of the longest unbreakable word in a cell; words may span runs. */
static size_t cell_longest_word(const tam_table_cell_t *cell)
{
    size_t best = 0;
    size_t cur = 0;
    for (size_t r = 0; r < cell->nruns; r++) {
        const char *t = cell->runs[r].text;
        if (t == NULL) {
            continue;
        }
        size_t tl = strlen(t);
        size_t i = 0;
        while (i < tl) {
            if (isspace((unsigned char)t[i])) {
                cur = 0;
                i++;
                continue;
            }
            size_t ws = i;
            while (i < tl && !isspace((unsigned char)t[i])) {
                i++;
            }
            cur += dwidth(t + ws, i - ws);
            if (cur > best) {
                best = cur;
            }
        }
    }
    return best;
}

static void cell_wrap(const tam_table_cell_t *cell, int max_w,
                      spanlist_t **out_lines, size_t *out_nlines)
{
    spanlist_t *lines = NULL;
    size_t nlines = 0;
    size_t cap = 0;

    spanlist_t cur;
    spanlist_init(&cur);
    int col = 0;
    int prev = 0;
    int had_space = 0;

    for (size_t r = 0; r < cell->nruns; r++) {
        const char *t = cell->runs[r].text;
        if (t == NULL) {
            continue;
        }
        size_t tl = strlen(t);
        size_t i = 0;
        while (i < tl) {
            if (t[i] == '\n') {
                if (nlines == cap) {
                    cap = cap ? cap * 2 : 4;
                    lines = realloc(lines, cap * sizeof(*lines));
                }
                lines[nlines++] = cur;
                spanlist_init(&cur);
                col = 0;
                prev = 0;
                had_space = 0;
                i++;
                continue;
            }
            if (isspace((unsigned char)t[i])) {
                had_space = 1;
                i++;
                continue;
            }
            size_t ws = i;
            while (i < tl && !isspace((unsigned char)t[i]) && t[i] != '\n') {
                i++;
            }
            size_t wb = i - ws;
            size_t wl = dwidth(t + ws, wb);
            int tight = !had_space && strchr(",.;:!?%)]}", t[ws]) != NULL;
            int attach = !had_space && (prev == '(' || prev == '[' || prev == '{');
            int sep = (col > 0 && !tight && !attach) ? 1 : 0;
            size_t extra = peek_following_punct(cell->runs, cell->nruns, r, i);
            if (col > 0 && !all_punct(t + ws, wb) &&
                col + sep + (int)(wl + extra) > max_w) {
                if (nlines == cap) {
                    cap = cap ? cap * 2 : 4;
                    lines = realloc(lines, cap * sizeof(*lines));
                }
                lines[nlines++] = cur;
                spanlist_init(&cur);
                col = 0;
                sep = 0;
                prev = 0;
            }
            if (sep) {
                spanlist_append(&cur, " ", 1, cell->runs[r].style, cell->runs[r].href,
                                cell->runs[r].link_id);
                col++;
            }
            if (wl > (size_t)max_w) {
                size_t off = ws;
                while (off < i) {
                    size_t char_start = off;
                    size_t bytes = 0;
                    wchar_t wc;
                    int cells = 0;
                    (void)tam_display_next(t, i, &off, &bytes, &wc, &cells);
                    if (cells > 0 && col > 0 && col + cells > max_w) {
                        if (nlines == cap) {
                            cap = cap ? cap * 2 : 4;
                            spanlist_t *grown = realloc(lines, cap * sizeof(*lines));
                            if (grown == NULL) { spanlist_free(&cur); goto done; }
                            lines = grown;
                        }
                        lines[nlines++] = cur;
                        spanlist_init(&cur);
                        col = 0;
                    }
                    if (cells > max_w - col) {
                        spanlist_append(&cur, "?", 1, cell->runs[r].style,
                                        cell->runs[r].href, cell->runs[r].link_id);
                        col++;
                    } else {
                        spanlist_append(&cur, wc == 0xfffd ? "?" : t + char_start,
                                        wc == 0xfffd ? 1 : bytes,
                                        cell->runs[r].style, cell->runs[r].href,
                                        cell->runs[r].link_id);
                        col += cells;
                    }
                }
                prev = (unsigned char)t[ws + wb - 1];
                had_space = 0;
                continue;
            }
            spanlist_append(&cur, t + ws, wb, cell->runs[r].style, cell->runs[r].href,
                            cell->runs[r].link_id);
            col += (int)wl;
            prev = (unsigned char)t[ws + wb - 1];
            had_space = 0;
        }
    }
    if (nlines == cap) {
        cap = cap ? cap * 2 : 4;
        lines = realloc(lines, cap * sizeof(*lines));
    }
    lines[nlines++] = cur;

done:
    *out_lines = lines;
    *out_nlines = nlines;
}

static void layout_table_stacked(linelist_t *out, const tam_table_t *t, int width)
{
    size_t header = t->nrows > 0 && t->rows[0].is_header_row ? 0 : (size_t)-1;
    int first_item = 1;
    for (size_t r = 0; r < t->nrows; r++) {
        if (r == header) continue;
        const tam_table_row_t *row = &t->rows[r];
        if (!first_item) linelist_blank(out);
        first_item = 0;
        for (size_t c = 0; c < row->ncells; c++) {
            char label[160];
            size_t used = 0;
            if (header != (size_t)-1 && c < t->rows[header].ncells) {
                const tam_table_cell_t *h = &t->rows[header].cells[c];
                for (size_t k = 0; k < h->nruns && used + 1 < sizeof(label); k++) {
                    const char *text = h->runs[k].text != NULL ? h->runs[k].text : "";
                    size_t n = strlen(text);
                    if (n > sizeof(label) - used - 1) n = sizeof(label) - used - 1;
                    memcpy(label + used, text, n);
                    used += n;
                }
            } else {
                int n = snprintf(label, sizeof(label), "Column %zu", c + 1);
                used = n > 0 ? (size_t)n : 0;
                if (used >= sizeof(label)) used = sizeof(label) - 1;
            }
            label[used] = '\0';
            spanlist_t runs;
            spanlist_init(&runs);
            if (used > 0) spanlist_append(&runs, label, used, TAM_STYLE_BOLD, NULL, 0);
            spanlist_append(&runs, ": ", 2, 0, NULL, 0);
            const tam_table_cell_t *cell = &row->cells[c];
            for (size_t k = 0; k < cell->nruns; k++) {
                const tam_span_t *sp = &cell->runs[k];
                if (sp->text != NULL)
                    spanlist_append(&runs, sp->text, strlen(sp->text), sp->style,
                                    sp->href, sp->link_id);
            }
            layout_flow(out, runs.v, runs.n, width, "");
            spanlist_free(&runs);
        }
    }
}

static void layout_table(linelist_t *out, const tam_table_t *t, int width)
{
    if (t == NULL || t->nrows == 0 || t->ncols == 0) {
        return;
    }
    size_t ncols = t->ncols;
    int *col_w = calloc(ncols, sizeof(int));
    if (col_w == NULL) {
        return;
    }
    int *floor_w = calloc(ncols, sizeof(int));
    if (floor_w == NULL) {
        free(col_w);
        return;
    }
    for (size_t c = 0; c < ncols; c++) {
        col_w[c] = 3;
        floor_w[c] = 3;
    }
    for (size_t r = 0; r < t->nrows; r++) {
        const tam_table_row_t *row = &t->rows[r];
        for (size_t c = 0; c < row->ncells && c < ncols; c++) {
            size_t l = cell_text_len(&row->cells[c]);
            if ((int)l > col_w[c]) {
                col_w[c] = (int)l;
            }
            size_t w = cell_longest_word(&row->cells[c]);
            if ((int)w > floor_w[c]) {
                floor_w[c] = (int)w;
            }
        }
    }

    /* A column too wide for the terminal wraps its cells. It may shrink to
     * its longest word, but not below TABLE_MIN_COL unless its widest cell
     * is narrower; a column of one-word cells never wraps. */
    enum { TABLE_MIN_COL = 12 };
    int total_w = 0;
    int floor_total = 0;
    for (size_t c = 0; c < ncols; c++) {
        if (floor_w[c] < TABLE_MIN_COL) {
            floor_w[c] = col_w[c] < TABLE_MIN_COL ? col_w[c] : TABLE_MIN_COL;
        }
        total_w += col_w[c];
        floor_total += floor_w[c];
        if (c + 1 < ncols) {
            total_w += 2;
            floor_total += 2;
        }
    }
    if (floor_total > width) {
        free(floor_w);
        free(col_w);
        layout_table_stacked(out, t, width);
        return;
    }
    /* Narrow the widest shrinkable column one cell at a time until the
     * table fits, so short columns keep their natural width. */
    while (total_w > width) {
        size_t widest = ncols;
        for (size_t c = 0; c < ncols; c++) {
            if (col_w[c] > floor_w[c] &&
                (widest == ncols || col_w[c] > col_w[widest])) {
                widest = c;
            }
        }
        if (widest == ncols) {
            break;
        }
        col_w[widest]--;
        total_w--;
    }
    free(floor_w);
    for (size_t r = 0; r < t->nrows; r++) {
        const tam_table_row_t *row = &t->rows[r];
        spanlist_t **cell_sub = calloc(ncols, sizeof(spanlist_t *));
        size_t *n_sub = calloc(ncols, sizeof(size_t));
        size_t max_h = 1;

        for (size_t c = 0; c < ncols; c++) {
            if (c < row->ncells) {
                cell_wrap(&row->cells[c], col_w[c], &cell_sub[c], &n_sub[c]);
                if (n_sub[c] > max_h) {
                    max_h = n_sub[c];
                }
            }
        }

        for (size_t h = 0; h < max_h; h++) {
            spanlist_t line;
            spanlist_init(&line);
            for (size_t c = 0; c < ncols; c++) {
                int printed_w = 0;
                if (cell_sub[c] != NULL && h < n_sub[c]) {
                    spanlist_t *sub = &cell_sub[c][h];
                    for (size_t k = 0; k < sub->n; k++) {
                        spanlist_append(&line, sub->v[k].text,
                                        strlen(sub->v[k].text), sub->v[k].style,
                                        sub->v[k].href, sub->v[k].link_id);
                        printed_w += (int)dwidth(sub->v[k].text,
                                                 strlen(sub->v[k].text));
                    }
                }
                int pad = col_w[c] - printed_w;
                if (pad > 0 && c + 1 < ncols) {
                    char sp[160];
                    size_t m = (size_t)pad < sizeof(sp) ? (size_t)pad : sizeof(sp) - 1;
                    memset(sp, ' ', m);
                    sp[m] = '\0';
                    spanlist_append(&line, sp, m, 0, NULL, 0);
                }
                if (c + 1 < ncols) {
                    spanlist_append(&line, "  ", 2, 0, NULL, 0);
                }
            }
            linelist_push(out, &line);
        }

        for (size_t c = 0; c < ncols; c++) {
            if (cell_sub[c] != NULL) {
                for (size_t h = 0; h < n_sub[c]; h++) {
                    spanlist_free(&cell_sub[c][h]);
                }
                free(cell_sub[c]);
            }
        }
        free(cell_sub);
        free(n_sub);

        if (row->is_header_row) {
            spanlist_t sepline;
            spanlist_init(&sepline);
            for (size_t c = 0; c < ncols; c++) {
                int dashes = col_w[c];
                char *dash_buf = malloc((size_t)dashes + 1);
                if (dash_buf != NULL) {
                    memset(dash_buf, '-', (size_t)dashes);
                    dash_buf[dashes] = '\0';
                    spanlist_append(&sepline, dash_buf, (size_t)dashes, 0, NULL, 0);
                    free(dash_buf);
                }
                if (c + 1 < ncols) {
                    spanlist_append(&sepline, "  ", 2, 0, NULL, 0);
                }
            }
            linelist_push(out, &sepline);
        }
    }

    free(col_w);
}

/* Pad a line on the left so it ends at the right margin. A line that wrapped
 * stays left-aligned, so it never breaks awkwardly at the right. */
static void align_right(tam_line_t *line, int width)
{
    size_t used = 0;
    for (size_t i = 0; i < line->nspans; i++) {
        used += dwidth(line->spans[i].text, strlen(line->spans[i].text));
    }
    if ((int)used >= width) {
        return;
    }
    size_t pad = (size_t)width - used;
    tam_span_t *spans = realloc(line->spans, (line->nspans + 1) * sizeof(*spans));
    if (spans == NULL) {
        return;
    }
    char *text = malloc(pad + 1);
    if (text == NULL) {
        line->spans = spans;
        return;
    }
    memset(text, ' ', pad);
    text[pad] = '\0';
    memmove(spans + 1, spans, line->nspans * sizeof(*spans));
    spans[0] = (tam_span_t){ .text = text };
    line->spans = spans;
    line->nspans++;
}

static void layout_document(const blocklist_t *blocks, int width,
                            linelist_t *out)
{
    if (width < 1) width = 1;

    for (size_t i = 0; i < blocks->n; i++) {
        out->logical_block = i;
        out->logical_offset = 0;
        const tam_block_t *b = &blocks->v[i];
        if (i > 0) {
            const tam_block_t *prev = &blocks->v[i - 1];
            int tight = (b->kind == B_LIST && prev->kind == B_LIST &&
                         b->level == prev->level) ||
                        (b->kind == B_DD && prev->kind == B_DT) ||
                        (b->kind == B_PRE && prev->kind == B_DT) ||
                        prev->kind == B_META;
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
            spanlist_append(&l, dash, (size_t)n, 0, NULL, 0);
            free(dash);
            linelist_push(out, &l);
            continue;
        }
        if (b->kind == B_PRE) {
            layout_pre(out, b->runs, b->nruns, width);
            continue;
        }
        if (b->kind == B_TABLE && b->table != NULL) {
            layout_table(out, b->table, width);
            continue;
        }
        char pfx[80];
        block_prefix(b, pfx, sizeof(pfx));
        size_t first = out->n;
        layout_flow(out, b->runs, b->nruns, width, pfx);
        if (b->kind == B_META && out->n == first + 1) {
            align_right(&out->v[first], width);
        }
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
    if (p.curr_table != NULL) {
        parser_finish_cell(&p);
        if (p.curr_table->nrows > 0) {
            blocklist_add_table(&p.blocks, p.curr_table);
        } else {
            table_free(p.curr_table);
        }
        p.curr_table = NULL;
    }

    for (size_t i = 0; i < p.blocks.n; i++) {
        if (p.blocks.v[i].kind == B_HEAD) {
            for (size_t j = 0; j < p.blocks.v[i].nruns; j++) {
                p.blocks.v[i].runs[j].style |= TAM_STYLE_HEADING;
            }
        } else if (p.blocks.v[i].kind == B_DT) {
            for (size_t j = 0; j < p.blocks.v[i].nruns; j++) {
                p.blocks.v[i].runs[j].style |= TAM_STYLE_BOLD;
            }
        } else if (p.blocks.v[i].kind == B_META) {
            for (size_t j = 0; j < p.blocks.v[i].nruns; j++) {
                p.blocks.v[i].runs[j].style |= TAM_STYLE_DIM;
            }
        }
    }

    for (size_t i = 0; i < p.anchors.n; i++) {
        if (p.anchors.v[i].block >= p.blocks.n)
            p.anchors.v[i].block = p.blocks.n > 0 ? p.blocks.n - 1 : 0;
    }

    layout_backing_t *backing = calloc(1, sizeof(*backing));
    if (backing == NULL) {
        tam_str_free(p.title);
        blocklist_free(&p.blocks);
        anchorlist_free(&p.anchors);
        free(p.lists);
        free(p.href);
        return NULL;
    }
    backing->blocks = p.blocks;
    memset(&p.blocks, 0, sizeof(p.blocks));
    backing->anchors = p.anchors;
    memset(&p.anchors, 0, sizeof(p.anchors));

    linelist_t out;
    memset(&out, 0, sizeof(out));
    layout_document(&backing->blocks, width, &out);
    if (out.n == 0) {
        linelist_push_blank(&out);
    }

    tam_document_t *doc = calloc(1, sizeof(*doc));
    if (doc == NULL) {
        for (size_t i = 0; i < out.n; i++) {
            for (size_t j = 0; j < out.v[i].nspans; j++) {
                free(out.v[i].spans[j].text);
                free(out.v[i].spans[j].href);
            }
            free(out.v[i].spans);
        }
        free(out.v);
        blocklist_free(&backing->blocks);
        anchorlist_free(&backing->anchors);
        free(backing);
        tam_str_free(p.title);
        free(p.lists);
        free(p.href);
        return NULL;
    }
    const char *title = p.title ? tam_str_trim(p.title, " \t\r\n") : NULL;
    doc->title = strdup((title != NULL && title[0] != '\0') ? title : "tam");
    doc->location = extract_location(src, len);
    doc->lines = out.v;
    doc->nlines = out.n;
    doc->nav_next = p.nav_next;
    doc->nav_prev = p.nav_prev;
    doc->nav_up = p.nav_up;
    doc->nav_top = p.nav_top;
    doc->layout = backing;

    tam_str_free(p.title);
    free(p.lists);
    free(p.href);
    return doc;
}

static void document_lines_free(tam_line_t *lines, size_t nlines)
{
    for (size_t i = 0; i < nlines; i++) {
        for (size_t j = 0; j < lines[i].nspans; j++) {
            free(lines[i].spans[j].text);
            free(lines[i].spans[j].href);
        }
        free(lines[i].spans);
    }
    free(lines);
}

int tam_document_reflow(tam_document_t *doc, int content_width)
{
    if (doc == NULL || doc->layout == NULL || content_width < 1) return -1;
    layout_backing_t *backing = doc->layout;
    linelist_t out;
    memset(&out, 0, sizeof(out));
    layout_document(&backing->blocks, content_width, &out);
    if (out.n == 0) linelist_push_blank(&out);
    if (out.v == NULL || out.n == 0) {
        free(out.v);
        return -1;
    }
    tam_line_t *old_lines = doc->lines;
    size_t old_nlines = doc->nlines;
    doc->lines = out.v;
    doc->nlines = out.n;
    document_lines_free(old_lines, old_nlines);
    return 0;
}

void tam_document_free(tam_document_t *doc)
{
    if (doc == NULL) {
        return;
    }
    document_lines_free(doc->lines, doc->nlines);
    for (size_t i = 0; i < doc->location.nspans; i++) {
        free(doc->location.spans[i].text);
        free(doc->location.spans[i].href);
    }
    free(doc->location.spans);
    free(doc->folds);
    free(doc->title);
    free(doc->nav_next);
    free(doc->nav_prev);
    free(doc->nav_up);
    free(doc->nav_top);
    if (doc->layout != NULL) {
        layout_backing_t *backing = doc->layout;
        blocklist_free(&backing->blocks);
        anchorlist_free(&backing->anchors);
        free(backing);
    }
    free(doc);
}

int tam_document_anchor_line(const tam_document_t *doc, const char *id)
{
    if (doc == NULL || doc->layout == NULL || id == NULL || id[0] == '\0')
        return -1;
    const anchorlist_t *al = &((const layout_backing_t *)doc->layout)->anchors;
    const anchor_t *hit = NULL;
    for (size_t i = 0; i < al->n && hit == NULL; i++) {
        if (strcmp(al->v[i].id, id) == 0) hit = &al->v[i];
    }
    for (size_t i = 0; i < al->n && hit == NULL; i++) {
        if (strcasecmp(al->v[i].id, id) == 0) hit = &al->v[i];
    }
    if (hit == NULL) return -1;
    /* A block's leading blank separator shares its logical_block. */
    for (size_t i = 0; i < doc->nlines; i++) {
        if (doc->lines[i].logical_block == hit->block && doc->lines[i].nspans > 0)
            return (int)i;
    }
    return -1;
}

char *tam_document_text(const tam_document_t *doc)
{
    tam_str_t s = tam_str_new("");

    if (doc->location.nspans > 0) {
        for (size_t j = 0; j < doc->location.nspans; j++)
            s = tam_str_cat(s, doc->location.spans[j].text);
        s = tam_str_cat(s, "\n\n");
    }

    for (size_t i = 0; i < doc->nlines; i++) {
        for (size_t j = 0; j < doc->lines[i].nspans; j++) {
            s = tam_str_cat(s, doc->lines[i].spans[j].text);
        }
        s = tam_str_cat(s, "\n");
    }

    const char **links = NULL;
    size_t nlinks = 0;
    for (size_t j = 0; j < doc->location.nspans; j++) {
        const char *h = doc->location.spans[j].href;
        if (h == NULL) continue;
        int seen = 0;
        for (size_t k = 0; k < nlinks; k++)
            if (strcmp(links[k], h) == 0) { seen = 1; break; }
        if (!seen) {
            links = realloc(links, (nlinks + 1) * sizeof(*links));
            links[nlinks++] = h;
        }
    }
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
