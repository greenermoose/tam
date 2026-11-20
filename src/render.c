#include <tam.h>
#include <tam_uni.h>
#include "display.h"
#include "external.h"
#include "render.h"
#include "html_scan.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

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
    B_META, /* <p class="level-line">: dimmed, right-aligned when it fits */
    B_SANDBOX
};

enum {
    ALIGN_LEFT = 0,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

typedef struct {
    tam_span_t *runs;
    size_t      nruns;
    int         is_header;
    size_t      colspan;
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
    int          level;   /* heading level, list depth, or B_PARA alignment */
    int          ordered;
    int          number;
    int          box;     /* non-zero when inside <tam-box> */
    int          tight_before;
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
    blocklist_t          blocks;
    anchorlist_t         anchors;
    tam_sandbox_spec_t  *sandboxes;
    size_t               nsandboxes;
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
    char *target = href != NULL ? strdup(href) : NULL;
    if (href != NULL && target == NULL) return -1;
    tam_span_t *s = &l->v[l->n++];
    s->text = malloc(len + 1);
    if (s->text == NULL) {
        free(target);
        l->n--;
        return -1;
    }
    memcpy(s->text, text, len);
    s->text[len] = '\0';
    s->style = style;
    s->href = target;
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

/* Work on decoded text, using byte offsets to keep the original styles. */
static int copy_url_runs(spanlist_t *out, const tam_span_t **run,
                         size_t *offset, size_t len, const char *url, int id)
{
    while (len > 0) {
        size_t available = strlen((*run)->text + *offset);
        size_t count = len < available ? len : available;
        unsigned style = (*run)->style | (url != NULL ? TAM_STYLE_LINK : 0);
        if (spanlist_append(out, (*run)->text + *offset, count, style, url, id) < 0)
            return -1;
        len -= count;
        if (count == available) {
            (*run)++;
            *offset = 0;
        } else {
            *offset += count;
        }
    }
    return 0;
}

static int url_delimiter(const char *s, size_t len)
{
    unsigned char c = (unsigned char)*s;
    if (c < 0x80) return isspace(c) || strchr("<>\"", c) != NULL;
    uint32_t cp;
    if (tam_utf8_decode(s, len, &cp) == 0) return 0;
    return cp == 0xa0 || (cp >= 0x2000 && cp <= 0x200a) ||
           (cp >= 0x2013 && cp <= 0x201f) || cp == 0x202f || cp == 0x3000;
}

static char *manual_url(const char *text, size_t len, size_t start, size_t scheme,
                        size_t *end, size_t *resume)
{
    const char *closer = NULL;
    if (start > 0) {
        switch (text[start - 1]) {
        case '<': closer = ">"; break;
        case '"': closer = "\""; break;
        case '\'': closer = "'"; break;
        }
    }
    if (start >= 3) {
        if (memcmp(text + start - 3, "“", 3) == 0) closer = "”";
        if (memcmp(text + start - 3, "‘", 3) == 0) closer = "’";
    }
    size_t closer_len = closer != NULL ? strlen(closer) : 0;
    size_t stop = start + scheme;
    int parens = 0, brackets = 0;
    while (stop < len && !url_delimiter(text + stop, len - stop) &&
           !(closer != NULL && strncmp(text + stop, closer, closer_len) == 0)) {
        switch (text[stop]) {
        case '(': parens++; break;
        case ')': parens--; break;
        case '[': brackets++; break;
        case ']': brackets--; break;
        }
        stop++;
    }
    *resume = stop;
    if (closer == NULL || strncmp(text + stop, closer, closer_len) != 0) {
        while (stop > start + scheme) {
            char last = text[stop - 1];
            if (last == ')' && parens < 0) parens++;
            else if (last == ']' && brackets < 0) brackets++;
            else if (strchr(".,;:!?'", last) == NULL) break;
            stop--;
        }
    }
    *end = stop;
    int in_authority = 1;
    if (brackets != 0) return NULL;
    for (size_t i = start + scheme; i < stop; i++) {
        unsigned char c = (unsigned char)text[i];
        if (strchr("/?#", c) != NULL) in_authority = 0;
        /* Square brackets delimit IP literals in the authority. In a path
         * they can be part of a manual's glob example, as in curl(1). */
        if (!in_authority && (c == '[' || c == ']')) return NULL;
        if (c == '%' && (i + 2 >= stop ||
            !isxdigit((unsigned char)text[i + 1]) ||
            !isxdigit((unsigned char)text[i + 2]))) return NULL;
    }
    char *url = strndup(text + start, stop - start);
    if (tam_external_url_ok(url)) return url;
    free(url);
    return NULL;
}

/* Explicit anchors divide the text into independent scan ranges. */
static void link_manual_urls(tam_span_t **runs, size_t *nruns, int *next_id)
{
    spanlist_t result = {0};
    int id = *next_id;
    for (size_t i = 0; i < *nruns;) {
        tam_span_t *span = *runs + i;
        if (span->href != NULL) {
            if (spanlist_append(&result, span->text, strlen(span->text),
                                span->style, span->href, span->link_id) < 0) goto fail;
            i++;
            continue;
        }

        tam_str_t text = tam_str_new("");
        if (text == NULL) goto fail;
        while (i < *nruns && (*runs)[i].href == NULL) {
            tam_str_t next = tam_str_cat(text, (*runs)[i].text);
            if (next == NULL) {
                tam_str_free(text);
                goto fail;
            }
            text = next;
            i++;
        }
        size_t len = tam_str_len(text);
        const tam_span_t *run = span;
        size_t offset = 0, copied = 0;
        for (size_t start = 0; start < len;) {
            size_t scheme = strncasecmp(text + start, "https://", 8) == 0 ? 8 :
                            strncasecmp(text + start, "http://", 7) == 0 ? 7 : 0;
            if (!scheme || (start > 0 && (isalnum((unsigned char)text[start - 1]) ||
                                          strchr("+.-/", text[start - 1]) != NULL))) {
                start++;
                continue;
            }
            size_t end, resume;
            char *url = manual_url(text, len, start, scheme, &end, &resume);
            if (url != NULL) {
                int failed = copy_url_runs(&result, &run, &offset, start - copied, NULL, 0) < 0 ||
                             copy_url_runs(&result, &run, &offset, end - start, url, ++id) < 0;
                free(url);
                if (failed) {
                    tam_str_free(text);
                    goto fail;
                }
                copied = end;
            }
            start = resume;
        }
        tam_str_free(text);
        if (copy_url_runs(&result, &run, &offset, len - copied, NULL, 0) < 0) goto fail;
    }
    if (id != *next_id) {
        spanlist_t old = {.v = *runs, .n = *nruns};
        spanlist_free(&old);
        *runs = result.v;
        *nruns = result.n;
        *next_id = id;
        return;
    }
fail:
    spanlist_free(&result);
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
    ll->v[ll->n].sandbox_id = 0;
    ll->v[ll->n].sandbox_role = 0;
    ll->v[ll->n].sandbox_cursor_col = -1;
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
    ll->v[ll->n].sandbox_id = 0;
    ll->v[ll->n].sandbox_role = 0;
    ll->v[ll->n].sandbox_cursor_col = -1;
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
                          int number, int box, int tight_before,
                          tam_span_t *runs, size_t nruns)
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
    b->box = box;
    b->tight_before = tight_before;
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

static void blocklist_add_table(blocklist_t *bl, int box, int tight_before,
                                tam_table_t *table)
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
    b->box = box;
    b->tight_before = tight_before;
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

/* Attribute `attr` of the attribute list s..end, entities decoded; NULL
 * when absent. */
static char *extract_attr(const char *s, const char *end, const char *attr)
{
    size_t v, len;
    if (s >= end || !tam_html_attr(s, 0, (size_t)(end - s), attr, &v, &len)) {
        return NULL;
    }
    return decode_entities(s + v, len, NULL);
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
    size_t       cell_colspan;

    char        *nav_next;
    char        *nav_prev;
    char        *nav_up;
    char        *nav_top;
    int           in_breadcrumb;
    int           box_depth;
    int           box_seq;
    int           tight_next;
    int           block_tight;
    tam_sandbox_spec_t *sandboxes;
    size_t              nsandboxes;
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
        int box = p->box_depth > 0 ? p->box_seq : 0;
        blocklist_add(&p->blocks, p->kind, p->level, p->ordered, p->number,
                      box, p->block_tight, p->runs.v, p->runs.n);
        spanlist_init(&p->runs);
    } else {
        if (p->block_tight) {
            p->tight_next = 1;
        }
        spanlist_free(&p->runs);
    }
    p->block_tight = 0;
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
    c->colspan = p->cell_colspan ? p->cell_colspan : 1;
    if (c->is_header) {
        row->is_header_row = 1;
        for (size_t k = 0; k < c->nruns; k++) {
            c->runs[k].style |= TAM_STYLE_BOLD;
        }
    }
    size_t row_cols = 0;
    for (size_t i = 0; i < row->ncells; i++) {
        row_cols += row->cells[i].colspan ? row->cells[i].colspan : 1;
    }
    if (row_cols > p->curr_table->ncols) {
        p->curr_table->ncols = row_cols;
    }
    spanlist_init(&p->runs);
    p->in_cell = 0;
    p->cell_colspan = 0;
}

static void parser_begin_block(parser_t *p, int kind, int level, int ordered,
                               int number)
{
    parser_end_block(p);
    p->kind = kind;
    p->level = level;
    p->ordered = ordered;
    p->number = number;
    p->block_tight = p->tight_next;
    p->tight_next = 0;
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

/* Handle one complete tag t of src: s..end are its attributes. */
static void parser_handle_tag(parser_t *p, const char *src, const tam_tag_t *t)
{
    const char *name = t->name;
    int closing = t->closing;
    const char *s = src + t->attrs;
    const char *end = src + t->end - 1;
    if (name[0] == '\0') {
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
    if (strcmp(name, "meta") == 0) {
        if (!closing) {
            char *m_name = extract_attr(s, end, "name");
            char *m_content = extract_attr(s, end, "content");
            if (m_name != NULL && m_content != NULL) {
                if (strcasecmp(m_name, "next") == 0) {
                    free(p->nav_next);
                    p->nav_next = strdup(m_content);
                } else if (strcasecmp(m_name, "prev") == 0) {
                    free(p->nav_prev);
                    p->nav_prev = strdup(m_content);
                } else if (strcasecmp(m_name, "up") == 0) {
                    free(p->nav_up);
                    p->nav_up = strdup(m_content);
                } else if (strcasecmp(m_name, "top") == 0) {
                    free(p->nav_top);
                    p->nav_top = strdup(m_content);
                }
            }
            free(m_name);
            free(m_content);
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
    if (strcmp(name, "span") == 0) {
        if (closing) {
            p->style &= ~TAM_STYLE_LVL_MASK;
        } else {
            char *cls = extract_attr(s, end, "class");
            if (cls != NULL) {
                if (strcmp(cls, "lvl-0") == 0) {
                    p->style |= TAM_STYLE_LVL_0;
                } else if (strcmp(cls, "lvl-1") == 0) {
                    p->style |= TAM_STYLE_LVL_1;
                } else if (strcmp(cls, "lvl-2") == 0) {
                    p->style |= TAM_STYLE_LVL_2;
                } else if (strcmp(cls, "lvl-3") == 0) {
                    p->style |= TAM_STYLE_LVL_3;
                }
                free(cls);
            }
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
    if (strcmp(name, "tam-box") == 0) {
        parser_end_block(p);
        p->tight_next = 0;
        if (closing) {
            if (p->box_depth > 0) {
                p->box_depth--;
            }
        } else {
            char *cls = extract_attr(s, end, "class");
            if (cls != NULL && strcmp(cls, "tight") == 0) {
                p->tight_next = 1;
            }
            free(cls);
            if (p->box_depth == 0) {
                p->box_seq++;
            }
            p->box_depth++;
        }
        return;
    }
    if (strcmp(name, "p") == 0) {
        if (closing) {
            parser_end_block(p);
        } else {
            char *cls = extract_attr(s, end, "class");
            int meta = cls != NULL && strcmp(cls, "level-line") == 0;
            free(cls);
            int align = ALIGN_LEFT;
            char *al = extract_attr(s, end, "align");
            if (al != NULL) {
                if (strcasecmp(al, "center") == 0) {
                    align = ALIGN_CENTER;
                } else if (strcasecmp(al, "right") == 0) {
                    align = ALIGN_RIGHT;
                } else if (strcasecmp(al, "left") == 0) {
                    align = ALIGN_LEFT;
                }
                free(al);
            }
            parser_begin_block(p, meta ? B_META : B_PARA, align, 0, 0);
        }
        return;
    }
    if (name[0] == 'h' && name[1] >= '1' && name[1] <= '6' && name[2] == '\0') {
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
                    int box = p->box_depth > 0 ? p->box_seq : 0;
                    blocklist_add_table(&p->blocks, box, p->tight_next,
                                        p->curr_table);
                    p->tight_next = 0;
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
            p->cell_colspan = 1;
            char *cs = extract_attr(s, end, "colspan");
            if (cs != NULL) {
                char *endp = NULL;
                unsigned long val = strtoul(cs, &endp, 10);
                if (endp != cs && val >= 1 && val <= 64) {
                    p->cell_colspan = (size_t)val;
                }
                free(cs);
            }
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

static size_t parse_tam_sandbox(parser_t *p, const char *src, size_t n,
                                const tam_tag_t *open_tag)
{
    parser_end_block(p);

    tam_sandbox_spec_t spec;
    memset(&spec, 0, sizeof(spec));
    const char *as = src + open_tag->attrs;
    const char *ae = src + open_tag->end;
    spec.title = extract_attr(as, ae, "title");
    spec.prompt = extract_attr(as, ae, "prompt");
    spec.cwd = extract_attr(as, ae, "cwd");
    spec.goal = extract_attr(as, ae, "goal");
    char *lesson_str = extract_attr(as, ae, "lesson");
    spec.lesson = tam_sandbox_parse_lesson(lesson_str);
    free(lesson_str);
    char *rows_str = extract_attr(as, ae, "rows");
    spec.rows = rows_str ? atoi(rows_str) : 4;
    free(rows_str);
    if (spec.rows < 2) spec.rows = 2;
    if (spec.rows > 16) spec.rows = 16;

    spec.init_vfs = tam_sb_vfs_new_dir("/", NULL, "root");
    const char *init_cwd = (spec.cwd && spec.cwd[0]) ? spec.cwd : "/home/alex";
    tam_sb_vfs_node_t *default_dir = tam_sb_vfs_ensure_path(spec.init_vfs, init_cwd);
    if (default_dir == NULL) default_dir = spec.init_vfs;

    tam_sb_vfs_node_t *dir_stack[16];
    int dir_depth = 0;
    dir_stack[0] = default_dir;

    int in_fs = 0;
    int in_cmd = 0;
    tam_sb_cmd_rule_t cur_rule;
    memset(&cur_rule, 0, sizeof(cur_rule));

    const char *cap_tag = NULL;
    tam_str_t cap_buf = tam_str_new("");
    char *file_name = NULL, *file_mode = NULL, *file_owner = NULL;
    size_t file_size = 0;
    int hint_after = 1;

    size_t i = open_tag->end;
    while (i < n) {
        if (src[i] == '<') {
            tam_tag_t t;
            int complete = tam_tag_read(src, n, i, &t);
            if (t.comment) {
                i = t.end;
                continue;
            }
            if (!complete) {
                if (cap_tag != NULL) cap_buf = tam_str_cat_len(cap_buf, src + i, 1);
                i++;
                continue;
            }
            if (t.closing && strcmp(t.name, "tam-sandbox") == 0) {
                i = t.end;
                break;
            }
            if (strcmp(t.name, "tam-fs") == 0) {
                in_fs = !t.closing;
            } else if (in_fs && strcmp(t.name, "dir") == 0) {
                if (!t.closing) {
                    char *dname = extract_attr(src + t.attrs, src + t.end, "name");
                    char *dmode = extract_attr(src + t.attrs, src + t.end, "mode");
                    char *downer = extract_attr(src + t.attrs, src + t.end, "owner");
                    tam_sb_vfs_node_t *cur = dir_stack[dir_depth];
                    tam_sb_vfs_node_t *nd = NULL;
                    if (dname != NULL && dname[0] == '/') {
                        nd = tam_sb_vfs_ensure_path(spec.init_vfs, dname);
                    } else if (dname != NULL && dname[0] != '\0') {
                        nd = tam_sb_vfs_add_dir(cur, dname, dmode, downer);
                    }
                    int self_closing = (t.end >= 2 && src[t.end - 2] == '/');
                    if (!self_closing && dir_depth < 15) {
                        dir_depth++;
                        dir_stack[dir_depth] = nd ? nd : cur;
                    }
                    free(dname);
                    free(dmode);
                    free(downer);
                } else if (dir_depth > 0) {
                    dir_depth--;
                }
            } else if (in_fs && strcmp(t.name, "file") == 0) {
                if (!t.closing) {
                    free(file_name); free(file_mode); free(file_owner);
                    file_name = extract_attr(src + t.attrs, src + t.end, "name");
                    file_mode = extract_attr(src + t.attrs, src + t.end, "mode");
                    file_owner = extract_attr(src + t.attrs, src + t.end, "owner");
                    char *sz = extract_attr(src + t.attrs, src + t.end, "size");
                    file_size = sz ? (size_t)strtoul(sz, NULL, 10) : 0;
                    free(sz);
                    int self_closing = (t.end >= 2 && src[t.end - 2] == '/');
                    if (self_closing) {
                        if (file_name != NULL && file_name[0] != '\0') {
                            tam_sb_vfs_add_file(dir_stack[dir_depth], file_name,
                                                file_mode, file_owner, file_size, "");
                        }
                        free(file_name); free(file_mode); free(file_owner);
                        file_name = file_mode = file_owner = NULL;
                    } else {
                        cap_tag = "file";
                        tam_str_free(cap_buf);
                        cap_buf = tam_str_new("");
                    }
                } else {
                    if (file_name != NULL && file_name[0] != '\0') {
                        char *dec = decode_entities(cap_buf, strlen(cap_buf), NULL);
                        tam_sb_vfs_add_file(dir_stack[dir_depth], file_name,
                                            file_mode, file_owner, file_size,
                                            dec ? dec : "");
                        free(dec);
                    }
                    free(file_name); free(file_mode); free(file_owner);
                    file_name = file_mode = file_owner = NULL;
                    cap_tag = NULL;
                }
            } else if (strcmp(t.name, "tam-cmd") == 0) {
                if (!t.closing) {
                    memset(&cur_rule, 0, sizeof(cur_rule));
                    cur_rule.match = extract_attr(src + t.attrs, src + t.end, "match");
                    cur_rule.cwd = extract_attr(src + t.attrs, src + t.end, "cwd");
                    cur_rule.note = extract_attr(src + t.attrs, src + t.end, "hint");
                    char *ex = extract_attr(src + t.attrs, src + t.end, "exit");
                    cur_rule.exit_code = ex ? atoi(ex) : 0;
                    free(ex);
                    char *gl = extract_attr(src + t.attrs, src + t.end, "goal");
                    cur_rule.is_goal = (gl && (strcmp(gl, "true") == 0 ||
                                               strcmp(gl, "1") == 0));
                    free(gl);
                    int self_closing = (t.end >= 2 && src[t.end - 2] == '/');
                    if (self_closing) {
                        if (cur_rule.match != NULL) {
                            spec.rules = realloc(spec.rules,
                                (spec.nrules + 1) * sizeof(*spec.rules));
                            spec.rules[spec.nrules++] = cur_rule;
                        } else {
                            free(cur_rule.cwd); free(cur_rule.note);
                        }
                        memset(&cur_rule, 0, sizeof(cur_rule));
                    } else {
                        in_cmd = 1;
                        cap_tag = "tam-cmd";
                        tam_str_free(cap_buf);
                        cap_buf = tam_str_new("");
                    }
                } else {
                    if (in_cmd) {
                        if (cur_rule.out == NULL && strlen(cap_buf) > 0) {
                            char *dec = decode_entities(cap_buf, strlen(cap_buf), NULL);
                            if (dec != NULL) {
                                char *trimmed = dec;
                                while (*trimmed == '\n' || *trimmed == '\r') trimmed++;
                                cur_rule.out = strdup(trimmed);
                                free(dec);
                            }
                        }
                        if (cur_rule.match != NULL) {
                            spec.rules = realloc(spec.rules,
                                (spec.nrules + 1) * sizeof(*spec.rules));
                            spec.rules[spec.nrules++] = cur_rule;
                        } else {
                            free(cur_rule.out); free(cur_rule.err);
                            free(cur_rule.note); free(cur_rule.cwd);
                        }
                        memset(&cur_rule, 0, sizeof(cur_rule));
                        in_cmd = 0;
                        cap_tag = NULL;
                    }
                }
            } else if (in_cmd && (strcmp(t.name, "out") == 0 ||
                                  strcmp(t.name, "err") == 0 ||
                                  strcmp(t.name, "note") == 0)) {
                if (!t.closing) {
                    cap_tag = strcmp(t.name, "out") == 0 ? "out"
                            : strcmp(t.name, "err") == 0 ? "err" : "note";
                    tam_str_free(cap_buf);
                    cap_buf = tam_str_new("");
                } else {
                    char *dec = decode_entities(cap_buf, strlen(cap_buf), NULL);
                    if (strcmp(t.name, "out") == 0) {
                        free(cur_rule.out);
                        cur_rule.out = dec;
                    } else if (strcmp(t.name, "err") == 0) {
                        free(cur_rule.err);
                        cur_rule.err = dec;
                    } else {
                        free(cur_rule.note);
                        cur_rule.note = dec;
                    }
                    cap_tag = "tam-cmd";
                    tam_str_free(cap_buf);
                    cap_buf = tam_str_new("");
                }
            } else if (strcmp(t.name, "tam-hint") == 0) {
                if (!t.closing) {
                    char *af = extract_attr(src + t.attrs, src + t.end, "after");
                    hint_after = af ? atoi(af) : 1;
                    free(af);
                    if (hint_after < 1) hint_after = 1;
                    cap_tag = "tam-hint";
                    tam_str_free(cap_buf);
                    cap_buf = tam_str_new("");
                } else {
                    char *dec = decode_entities(cap_buf, strlen(cap_buf), NULL);
                    if (dec != NULL && dec[0] != '\0') {
                        spec.hints = realloc(spec.hints,
                            (spec.nhints + 1) * sizeof(*spec.hints));
                        spec.hints[spec.nhints].after = hint_after;
                        spec.hints[spec.nhints].text = dec;
                        spec.nhints++;
                    } else {
                        free(dec);
                    }
                    cap_tag = NULL;
                }
            } else if (strcmp(t.name, "tam-fallback") == 0) {
                if (!t.closing) {
                    cap_tag = "tam-fallback";
                    tam_str_free(cap_buf);
                    cap_buf = tam_str_new("");
                } else {
                    free(spec.fallback);
                    spec.fallback = decode_entities(cap_buf, strlen(cap_buf), NULL);
                    cap_tag = NULL;
                }
            } else if (strcmp(t.name, "tam-init") == 0) {
                if (!t.closing) {
                    cap_tag = "tam-init";
                    tam_str_free(cap_buf);
                    cap_buf = tam_str_new("");
                } else {
                    free(spec.init_text);
                    spec.init_text = decode_entities(cap_buf, strlen(cap_buf), NULL);
                    cap_tag = NULL;
                }
            }
            i = t.end;
            continue;
        }
        const char *lt = memchr(src + i, '<', n - i);
        size_t j = lt ? (size_t)(lt - src) : n;
        if (cap_tag != NULL) {
            cap_buf = tam_str_cat_len(cap_buf, src + i, j - i);
        }
        i = j;
    }

    free(file_name);
    free(file_mode);
    free(file_owner);
    free(cur_rule.match);
    free(cur_rule.out);
    free(cur_rule.err);
    free(cur_rule.note);
    free(cur_rule.cwd);
    tam_str_free(cap_buf);

    tam_sandbox_spec_t *ns = realloc(p->sandboxes,
        (p->nsandboxes + 1) * sizeof(*ns));
    if (ns != NULL) {
        p->sandboxes = ns;
        p->sandboxes[p->nsandboxes++] = spec;
        int box = p->box_depth > 0 ? p->box_seq : 0;
        blocklist_add(&p->blocks, B_SANDBOX, 0, 0, (int)p->nsandboxes,
                      box, p->tight_next, NULL, 0);
        p->tight_next = 0;
    } else {
        tam_sandbox_spec_free(&spec);
    }
    return i;
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
            tam_tag_t t;
            int complete = tam_tag_read(src, n, i, &t);
            if (t.comment) {
                i = t.end;
            } else if (!complete) {
                /* A '<' that opens no tag is text. */
                parser_add_text(p, src + i, 1);
                i++;
            } else if (!t.closing && strcmp(t.name, "tam-sandbox") == 0) {
                i = parse_tam_sandbox(p, src, n, &t);
            } else {
                parser_handle_tag(p, src, &t);
                i = t.end;
            }
            continue;
        }

        const char *lt = memchr(src + i, '<', n - i);
        size_t j = lt ? (size_t)(lt - src) : n;
        parser_add_text(p, src + i, j - i);
        i = j;
    }
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

/* The next complete element tag at or after src[*i] (comments skipped); *i
 * moves past it. Returns 0 when there is none before len. */
static int next_tag(const char *src, size_t len, size_t *i, tam_tag_t *t)
{
    while (*i < len) {
        const char *lt = memchr(src + *i, '<', len - *i);
        if (lt == NULL) return 0;
        size_t at = (size_t)(lt - src);
        if (tam_tag_read(src, len, at, t) && !t->comment) {
            *i = t->end;
            return 1;
        }
        *i = t->complete ? t->end : at + 1;
    }
    return 0;
}

static tam_line_t extract_location(const char *src, size_t len)
{
    spanlist_t spans = {0};
    tam_tag_t t;
    size_t i = 0, body = 0, nav_end = 0;
    int found = 0;
    while (!found && next_tag(src, len, &i, &t)) {
        char label[32];
        found = tam_tag_is(&t, "nav", 0) &&
                tam_tag_attr(src, &t, "aria-label", label, sizeof(label)) &&
                strcasecmp(label, "Breadcrumb") == 0;
        body = t.end;
    }
    found = 0;
    while (!found && next_tag(src, len, &i, &t)) {
        found = tam_tag_is(&t, "nav", 1);
        nav_end = t.start;
    }
    if (!found) return (tam_line_t){0};

    size_t cursor = body;
    size_t link_id = 0;
    int have_part = 0;
    i = body;
    while (next_tag(src, nav_end, &i, &t)) {
        if (!tam_tag_is(&t, "a", 0)) continue;
        tam_tag_t close;
        size_t j = t.end;
        int closed = 0;
        while (!closed && next_tag(src, nav_end, &j, &close)) {
            closed = tam_tag_is(&close, "a", 1);
        }
        if (!closed) break;
        char *href = extract_attr(src + t.attrs, src + t.end - 1, "href");
        char *text = plain_fragment(src + t.end, close.start - t.end);
        if (href != NULL && text != NULL && text[0] != '\0') {
            if (have_part) spanlist_append(&spans, " > ", 3, 0, NULL, 0);
            spanlist_append(&spans, text, strlen(text), TAM_STYLE_LINK, href, (int)++link_id);
            have_part = 1;
        }
        free(href);
        free(text);
        cursor = close.end;
        i = close.end;
    }
    char *tail = plain_fragment(src + cursor, nav_end - cursor);
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
    if (strncmp(href, "archwiki:", 9) == 0) {
        location_append(doc, "Guide", "index.html");
        location_append(doc, "Arch Wiki", "tam:archwiki");
        location_append(doc, doc->title ? doc->title : href + 9, NULL);
        return;
    }
    if (strncmp(href, "tam:archwiki:", 13) == 0) {
        location_append(doc, "Guide", "index.html");
        location_append(doc, "Arch Wiki", "tam:archwiki");
        location_append(doc, doc->title ? doc->title : href + 13, NULL);
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

/* Display width of any non-whitespace tokens immediately following (r, i)
 * with no intervening whitespace, plus any trailing punctuation token that
 * peek_following_punct reserves after that unspaced cluster. */
static size_t peek_unspaced_and_punct(const tam_span_t *runs, size_t nruns,
                                      size_t r, size_t i)
{
    size_t rr = r;
    size_t k = i;
    size_t w = 0;

    while (rr < nruns) {
        const char *t = runs[rr].text;
        size_t tl = (t != NULL) ? strlen(t) : 0;
        if (k >= tl) {
            rr++;
            k = 0;
            continue;
        }
        if (isspace((unsigned char)t[k])) {
            break;
        }
        size_t s = k;
        while (k < tl && !isspace((unsigned char)t[k]) && t[k] != '\n') {
            k++;
        }
        w += dwidth(t + s, k - s);
    }
    return w + peek_following_punct(runs, nruns, rr, k);
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
            int sep = (col > (int)lead && had_space) ? 1 : 0;
            size_t extra = had_space
                ? peek_unspaced_and_punct(runs, nruns, r, i)
                : 0;
            int should_break = had_space
                ? (col > (int)lead && !all_punct(t + ws, wb) &&
                   col + sep + (int)(wl + extra) > width)
                : (col > (int)lead && col + (int)wl > width);
            if (should_break) {
                out->logical_offset = line_offset;
                linelist_push(out, &line);
                line_offset = run_base + ws;
                start_line(&line, prefix, lead, 0);
                col = (int)lead;
                sep = 0;
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
                had_space = 0;
                continue;
            }
            spanlist_append(&line, t + ws, wb, runs[r].style, runs[r].href,
                            runs[r].link_id);
            col += (int)wl;
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
            int sep = (col > 0 && had_space) ? 1 : 0;
            size_t extra = had_space
                ? peek_unspaced_and_punct(cell->runs, cell->nruns, r, i)
                : 0;
            int should_break = had_space
                ? (col > 0 && !all_punct(t + ws, wb) &&
                   col + sep + (int)(wl + extra) > max_w)
                : (col > 0 && col + (int)wl > max_w);
            if (should_break) {
                if (nlines == cap) {
                    cap = cap ? cap * 2 : 4;
                    lines = realloc(lines, cap * sizeof(*lines));
                }
                lines[nlines++] = cur;
                spanlist_init(&cur);
                col = 0;
                sep = 0;
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
                had_space = 0;
                continue;
            }
            spanlist_append(&cur, t + ws, wb, cell->runs[r].style, cell->runs[r].href,
                            cell->runs[r].link_id);
            col += (int)wl;
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

static const tam_table_cell_t *row_cell_at_col(const tam_table_row_t *row,
                                               size_t target_col)
{
    size_t col = 0;
    for (size_t i = 0; i < row->ncells; i++) {
        size_t span = row->cells[i].colspan ? row->cells[i].colspan : 1;
        if (target_col >= col && target_col < col + span) {
            return &row->cells[i];
        }
        col += span;
    }
    return NULL;
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
        size_t col = 0;
        for (size_t i = 0; i < row->ncells; i++) {
            const tam_table_cell_t *cell = &row->cells[i];
            size_t span = cell->colspan ? cell->colspan : 1;
            char label[160];
            size_t used = 0;
            if (header != (size_t)-1) {
                const tam_table_cell_t *prev_h = NULL;
                for (size_t s = 0; s < span; s++) {
                    const tam_table_cell_t *h =
                        row_cell_at_col(&t->rows[header], col + s);
                    if (h == NULL || h == prev_h) continue;
                    prev_h = h;
                    if (used > 0 && used + 3 < sizeof(label)) {
                        memcpy(label + used, " / ", 3);
                        used += 3;
                    }
                    for (size_t k = 0; k < h->nruns && used + 1 < sizeof(label); k++) {
                        const char *text = h->runs[k].text != NULL ? h->runs[k].text : "";
                        size_t n = strlen(text);
                        if (n > sizeof(label) - used - 1) n = sizeof(label) - used - 1;
                        memcpy(label + used, text, n);
                        used += n;
                    }
                }
            }
            if (used == 0) {
                int n = span > 1
                    ? snprintf(label, sizeof(label), "Columns %zu-%zu",
                               col + 1, col + span)
                    : snprintf(label, sizeof(label), "Column %zu", col + 1);
                used = n > 0 ? (size_t)n : 0;
                if (used >= sizeof(label)) used = sizeof(label) - 1;
            }
            label[used] = '\0';
            spanlist_t runs;
            spanlist_init(&runs);
            if (used > 0) spanlist_append(&runs, label, used, TAM_STYLE_BOLD, NULL, 0);
            spanlist_append(&runs, ": ", 2, 0, NULL, 0);
            for (size_t k = 0; k < cell->nruns; k++) {
                const tam_span_t *sp = &cell->runs[k];
                if (sp->text != NULL)
                    spanlist_append(&runs, sp->text, strlen(sp->text), sp->style,
                                    sp->href, sp->link_id);
            }
            layout_flow(out, runs.v, runs.n, width, "");
            spanlist_free(&runs);
            col += span;
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
        size_t col = 0;
        for (size_t i = 0; i < row->ncells && col < ncols; i++) {
            size_t span = row->cells[i].colspan ? row->cells[i].colspan : 1;
            if (col + span > ncols) {
                span = ncols - col;
            }
            if (span == 1) {
                size_t l = cell_text_len(&row->cells[i]);
                if ((int)l > col_w[col]) {
                    col_w[col] = (int)l;
                }
                size_t w = cell_longest_word(&row->cells[i]);
                if ((int)w > floor_w[col]) {
                    floor_w[col] = (int)w;
                }
            }
            col += span;
        }
    }
    for (size_t r = 0; r < t->nrows; r++) {
        const tam_table_row_t *row = &t->rows[r];
        size_t col = 0;
        for (size_t i = 0; i < row->ncells && col < ncols; i++) {
            size_t span = row->cells[i].colspan ? row->cells[i].colspan : 1;
            if (col + span > ncols) {
                span = ncols - col;
            }
            if (span > 1) {
                int span_w = 0;
                for (size_t s = 0; s < span; s++) {
                    span_w += col_w[col + s];
                    if (s + 1 < span) {
                        span_w += 2;
                    }
                }
                int l = (int)cell_text_len(&row->cells[i]);
                if (l > span_w) {
                    col_w[col + span - 1] += l - span_w;
                }
            }
            col += span;
        }
    }

    /* A column too wide for the terminal wraps its cells. It may shrink to
     * its longest word, but not below TABLE_MIN_COL unless its widest cell
     * is narrower; a column of one-word cells never wraps. */
    enum { TABLE_MIN_COL = 12 };
    for (size_t c = 0; c < ncols; c++) {
        if (floor_w[c] < TABLE_MIN_COL) {
            floor_w[c] = col_w[c] < TABLE_MIN_COL ? col_w[c] : TABLE_MIN_COL;
        }
    }
    for (size_t r = 0; r < t->nrows; r++) {
        const tam_table_row_t *row = &t->rows[r];
        size_t col = 0;
        for (size_t i = 0; i < row->ncells && col < ncols; i++) {
            size_t span = row->cells[i].colspan ? row->cells[i].colspan : 1;
            if (col + span > ncols) {
                span = ncols - col;
            }
            if (span > 1) {
                int span_floor = 0;
                for (size_t s = 0; s < span; s++) {
                    span_floor += floor_w[col + s];
                    if (s + 1 < span) {
                        span_floor += 2;
                    }
                }
                int w = (int)cell_longest_word(&row->cells[i]);
                while (span_floor < w) {
                    size_t target = span;
                    for (size_t s = span; s > 0; s--) {
                        if (floor_w[col + s - 1] < col_w[col + s - 1]) {
                            target = s - 1;
                            break;
                        }
                    }
                    if (target == span) {
                        break;
                    }
                    floor_w[col + target]++;
                    span_floor++;
                }
            }
            col += span;
        }
    }
    int total_w = 0;
    int floor_total = 0;
    for (size_t c = 0; c < ncols; c++) {
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
        size_t ncells = row->ncells;
        spanlist_t **cell_sub = ncells ? calloc(ncells, sizeof(spanlist_t *)) : NULL;
        size_t *n_sub = ncells ? calloc(ncells, sizeof(size_t)) : NULL;
        size_t max_h = 1;

        size_t col = 0;
        for (size_t i = 0; i < ncells && col < ncols; i++) {
            size_t span = row->cells[i].colspan ? row->cells[i].colspan : 1;
            if (col + span > ncols) {
                span = ncols - col;
            }
            int cell_w = 0;
            for (size_t s = 0; s < span; s++) {
                cell_w += col_w[col + s];
                if (s + 1 < span) {
                    cell_w += 2;
                }
            }
            if (cell_sub != NULL && n_sub != NULL) {
                cell_wrap(&row->cells[i], cell_w, &cell_sub[i], &n_sub[i]);
                if (n_sub[i] > max_h) {
                    max_h = n_sub[i];
                }
            }
            col += span;
        }

        for (size_t h = 0; h < max_h; h++) {
            spanlist_t line;
            spanlist_init(&line);
            col = 0;
            for (size_t i = 0; i < ncells && col < ncols; i++) {
                size_t span = row->cells[i].colspan ? row->cells[i].colspan : 1;
                if (col + span > ncols) {
                    span = ncols - col;
                }
                int cell_w = 0;
                for (size_t s = 0; s < span; s++) {
                    cell_w += col_w[col + s];
                    if (s + 1 < span) {
                        cell_w += 2;
                    }
                }
                int printed_w = 0;
                if (cell_sub != NULL && cell_sub[i] != NULL && h < n_sub[i]) {
                    spanlist_t *sub = &cell_sub[i][h];
                    for (size_t k = 0; k < sub->n; k++) {
                        spanlist_append(&line, sub->v[k].text,
                                        strlen(sub->v[k].text), sub->v[k].style,
                                        sub->v[k].href, sub->v[k].link_id);
                        printed_w += (int)dwidth(sub->v[k].text,
                                                 strlen(sub->v[k].text));
                    }
                }
                int pad = cell_w - printed_w;
                if (pad > 0 && i + 1 < ncells && col + span < ncols) {
                    while (pad > 0) {
                        char sp[160];
                        size_t m = (size_t)pad < sizeof(sp) - 1 ? (size_t)pad : sizeof(sp) - 1;
                        memset(sp, ' ', m);
                        sp[m] = '\0';
                        spanlist_append(&line, sp, m, 0, NULL, 0);
                        pad -= (int)m;
                    }
                }
                if (i + 1 < ncells && col + span < ncols) {
                    spanlist_append(&line, "  ", 2, 0, NULL, 0);
                }
                col += span;
            }
            linelist_push(out, &line);
        }

        for (size_t i = 0; i < ncells; i++) {
            if (cell_sub != NULL && cell_sub[i] != NULL) {
                for (size_t h = 0; h < n_sub[i]; h++) {
                    spanlist_free(&cell_sub[i][h]);
                }
                free(cell_sub[i]);
            }
        }
        free(cell_sub);
        free(n_sub);

        if (row->is_header_row) {
            spanlist_t sepline;
            spanlist_init(&sepline);
            col = 0;
            for (size_t i = 0; i < ncells && col < ncols; i++) {
                size_t span = row->cells[i].colspan ? row->cells[i].colspan : 1;
                if (col + span > ncols) {
                    span = ncols - col;
                }
                int dashes = 0;
                for (size_t s = 0; s < span; s++) {
                    dashes += col_w[col + s];
                    if (s + 1 < span) {
                        dashes += 2;
                    }
                }
                char *dash_buf = malloc((size_t)dashes + 1);
                if (dash_buf != NULL) {
                    memset(dash_buf, '-', (size_t)dashes);
                    dash_buf[dashes] = '\0';
                    spanlist_append(&sepline, dash_buf, (size_t)dashes, 0, NULL, 0);
                    free(dash_buf);
                }
                if (col + span < ncols) {
                    spanlist_append(&sepline, "  ", 2, 0, NULL, 0);
                }
                col += span;
            }
            for (; col < ncols; col++) {
                int dashes = col_w[col];
                char *dash_buf = malloc((size_t)dashes + 1);
                if (dash_buf != NULL) {
                    memset(dash_buf, '-', (size_t)dashes);
                    dash_buf[dashes] = '\0';
                    spanlist_append(&sepline, dash_buf, (size_t)dashes, 0, NULL, 0);
                    free(dash_buf);
                }
                if (col + 1 < ncols) {
                    spanlist_append(&sepline, "  ", 2, 0, NULL, 0);
                }
            }
            linelist_push(out, &sepline);
        }
    }

    free(col_w);
}

/* Number of leading lines laid out from level-line blocks. The pager letters
 * them (b, c, ...) so the title is line 1. */
static size_t lead_lines(const blocklist_t *blocks, const linelist_t *out)
{
    size_t n = 0;
    while (n < out->n && out->v[n].logical_block < blocks->n &&
           blocks->v[out->v[n].logical_block].kind == B_META) {
        n++;
    }
    return n < out->n ? n : 0;
}

static size_t line_display_width(const tam_line_t *line)
{
    size_t used = 0;
    for (size_t i = 0; i < line->nspans; i++) {
        used += dwidth(line->spans[i].text, strlen(line->spans[i].text));
    }
    return used;
}

static void pad_line_left(tam_line_t *line, size_t pad)
{
    if (pad == 0 || line->nspans == 0) {
        return;
    }
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

/* Pad a line on the left so it ends at the right margin. A level-line that
 * wrapped stays left-aligned, so it never breaks awkwardly at the right. */
static void align_right(tam_line_t *line, int width)
{
    size_t used = line_display_width(line);
    if (used == 0 || (int)used >= width) {
        return;
    }
    pad_line_left(line, (size_t)width - used);
}

/* Pad a line on the left so it is centered within width. */
static void align_center(tam_line_t *line, int width)
{
    size_t used = line_display_width(line);
    if (used == 0 || (int)used >= width) {
        return;
    }
    pad_line_left(line, ((size_t)width - used) / 2);
}

static void layout_single_block(const blocklist_t *blocks, size_t i, int width,
                                linelist_t *out, int right_align_meta)
{
    out->logical_block = i;
    out->logical_offset = 0;
    const tam_block_t *b = &blocks->v[i];

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
        return;
    }
    if (b->kind == B_PRE) {
        layout_pre(out, b->runs, b->nruns, width);
        return;
    }
    if (b->kind == B_TABLE && b->table != NULL) {
        layout_table(out, b->table, width);
        return;
    }
    char pfx[80];
    block_prefix(b, pfx, sizeof(pfx));
    size_t first = out->n;
    layout_flow(out, b->runs, b->nruns, width, pfx);
    if (b->kind == B_META && right_align_meta && out->n == first + 1) {
        align_right(&out->v[first], width);
    } else if (b->kind == B_PARA && b->level == ALIGN_CENTER) {
        for (size_t ln = first; ln < out->n; ln++) {
            align_center(&out->v[ln], width);
        }
    } else if (b->kind == B_PARA && b->level == ALIGN_RIGHT) {
        for (size_t ln = first; ln < out->n; ln++) {
            align_right(&out->v[ln], width);
        }
    }
}

static void layout_box_push_border(linelist_t *out, size_t logical_block,
                                   const char *left, char fill, size_t count,
                                   const char *right)
{
    out->logical_block = logical_block;
    out->logical_offset = 0;
    spanlist_t l;
    spanlist_init(&l);
    size_t ll = strlen(left);
    size_t rl = strlen(right);
    char *buf = malloc(ll + count + rl + 1);
    if (buf != NULL) {
        memcpy(buf, left, ll);
        memset(buf + ll, fill, count);
        memcpy(buf + ll + count, right, rl);
        buf[ll + count + rl] = '\0';
        spanlist_append(&l, buf, ll + count + rl, 0, NULL, 0);
        free(buf);
    }
    linelist_push(out, &l);
}

static int blocks_are_tight(const tam_block_t *prev, const tam_block_t *b)
{
    return b->tight_before ||
           prev->kind == B_META ||
           (b->kind == B_LIST && prev->kind == B_LIST &&
            b->level == prev->level) ||
           (b->kind == B_DD && prev->kind == B_DT) ||
           (b->kind == B_PRE && prev->kind == B_DT);
}

static void layout_box(const blocklist_t *blocks, size_t start, size_t end,
                       int width, linelist_t *out)
{
    linelist_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    layout_single_block(blocks, start, width, &hdr, 1);

    size_t hdr_w = hdr.n > 0 ? line_display_width(&hdr.v[hdr.n - 1]) : 0;

    int inner_max = width > 5 ? width - 5 : 1;
    linelist_t body;
    memset(&body, 0, sizeof(body));
    for (size_t k = start + 1; k < end; k++) {
        if (k > start + 1 && !blocks_are_tight(&blocks->v[k - 1], &blocks->v[k])) {
            body.logical_block = k;
            body.logical_offset = 0;
            linelist_blank(&body);
        }
        layout_single_block(blocks, k, inner_max, &body, 0);
    }

    size_t max_body_w = 0;
    for (size_t k = 0; k < body.n; k++) {
        size_t w = line_display_width(&body.v[k]);
        if (w > max_body_w) {
            max_body_w = w;
        }
    }

    size_t box_inner_w = max_body_w;
    if (hdr_w >= 2 && box_inner_w < hdr_w - 2) {
        box_inner_w = hdr_w - 2;
    }

    for (size_t k = 0; k < body.n; k++) {
        size_t lb = body.v[k].logical_block;
        if (blocks->v[lb].kind == B_META && body.v[k].nspans > 0) {
            int single = !(k > 0 && body.v[k - 1].nspans > 0 &&
                           body.v[k - 1].logical_block == lb) &&
                         !(k + 1 < body.n && body.v[k + 1].nspans > 0 &&
                           body.v[k + 1].logical_block == lb);
            if (single) {
                align_right(&body.v[k], (int)box_inner_w);
            }
        }
    }

    for (size_t k = 0; k + 1 < hdr.n; k++) {
        out->logical_block = hdr.v[k].logical_block;
        out->logical_offset = hdr.v[k].logical_offset;
        linelist_grow(out);
        out->v[out->n++] = hdr.v[k];
    }
    if (hdr.n > 0) {
        tam_line_t top = hdr.v[hdr.n - 1];
        size_t total_w = 5 + box_inner_w;
        size_t dashes = total_w > hdr_w + 2 ? total_w - hdr_w - 2 : 1;
        spanlist_t sl = { .v = top.spans, .n = top.nspans, .cap = top.nspans };
        char *tail = malloc(1 + dashes + 2);
        if (tail != NULL) {
            tail[0] = ' ';
            memset(tail + 1, '-', dashes);
            tail[1 + dashes] = '+';
            tail[2 + dashes] = '\0';
            spanlist_append(&sl, tail, dashes + 2, 0, NULL, 0);
            free(tail);
        }
        out->logical_block = start;
        out->logical_offset = 0;
        linelist_push(out, &sl);
    }
    free(hdr.v);

    layout_box_push_border(out, start + 1 < end ? start + 1 : start,
                           " |", ' ', box_inner_w + 2, "|");

    for (size_t k = 0; k < body.n; k++) {
        tam_line_t *bl = &body.v[k];
        size_t lw = line_display_width(bl);
        size_t rpad = box_inner_w > lw ? box_inner_w - lw + 1 : 1;
        tam_span_t *spans = malloc((bl->nspans + 2) * sizeof(*spans));
        if (spans != NULL) {
            spans[0] = (tam_span_t){ .text = strdup(" | ") };
            for (size_t m = 0; m < bl->nspans; m++) {
                spans[1 + m] = bl->spans[m];
            }
            char *right = malloc(rpad + 2);
            if (right != NULL) {
                memset(right, ' ', rpad);
                right[rpad] = '|';
                right[rpad + 1] = '\0';
            }
            spans[1 + bl->nspans] = (tam_span_t){ .text = right };
            free(bl->spans);
            out->logical_block = bl->logical_block;
            out->logical_offset = bl->logical_offset;
            linelist_grow(out);
            out->v[out->n++] = (tam_line_t){
                .spans = spans,
                .nspans = bl->nspans + 2,
                .logical_block = bl->logical_block,
                .logical_offset = bl->logical_offset
            };
        }
    }
    free(body.v);

    layout_box_push_border(out, end - 1, " |", ' ', box_inner_w + 2, "|");
    layout_box_push_border(out, end - 1, " +", '-', box_inner_w + 2, "+");
}

enum { SANDBOX_MAX_INNER_WIDTH = 96 };

static void sb_begin_body_line(spanlist_t *line)
{
    spanlist_init(line);
    spanlist_append(line, " | ", 3, TAM_STYLE_SB_BORDER | TAM_STYLE_SB_BG, NULL, 0);
}

static void sb_end_body_line(linelist_t *out, spanlist_t *line, size_t logical_block,
                             int sb_id, int role, size_t remaining)
{
    char right[SANDBOX_MAX_INNER_WIDTH + 2];
    memset(right, ' ', remaining + 1);
    right[remaining + 1] = '|';
    spanlist_append(line, right, remaining + 2,
                    TAM_STYLE_SB_BORDER | TAM_STYLE_SB_BG, NULL, 0);
    out->logical_block = logical_block;
    out->logical_offset = 0;
    linelist_push(out, line);
    out->v[out->n - 1].sandbox_id = sb_id;
    out->v[out->n - 1].sandbox_role = role;
}

static void sb_push_bordered_spans(linelist_t *out, size_t logical_block,
                                    int sb_id, int role, size_t box_inner_w,
                                    const tam_span_t *inner, size_t ninner)
{
    spanlist_t line;
    sb_begin_body_line(&line);
    size_t remaining = box_inner_w;
    for (size_t i = 0; i < ninner; i++) {
        size_t cells;
        size_t len = tam_display_prefix(inner[i].text, strlen(inner[i].text),
                                        remaining, &cells);
        spanlist_append(&line, inner[i].text, len, inner[i].style | TAM_STYLE_SB_BG, NULL, 0);
        remaining -= cells;
        if (inner[i].text[len] != '\0') break;
    }
    sb_end_body_line(out, &line, logical_block, sb_id, role, remaining);
}

static void sb_push_border_line(linelist_t *out, size_t logical_block,
                                int sb_id, size_t box_inner_w, int header,
                                const char *title)
{
    unsigned style = TAM_STYLE_SB_BORDER | TAM_STYLE_SB_BG;
    spanlist_t sl;
    spanlist_init(&sl);
    const char *start = header ? "[-] " : " +-";
    size_t used = strlen(start);
    spanlist_append(&sl, start, used, style, NULL, 0);
    size_t width = box_inner_w + 5;
    if (title != NULL) {
        size_t cells;
        size_t len = tam_display_prefix(title, strlen(title), width - used - 2, &cells);
        spanlist_append(&sl, title, len, style | TAM_STYLE_BOLD, NULL, 0);
        spanlist_append(&sl, " ", 1, style, NULL, 0);
        used += cells + 1;
    }
    char tail[SANDBOX_MAX_INNER_WIDTH + 5];
    size_t dashes = width - used - 1;
    memset(tail, '-', dashes);
    tail[dashes] = '+';
    spanlist_append(&sl, tail, dashes + 1, style, NULL, 0);
    out->logical_block = logical_block;
    out->logical_offset = 0;
    linelist_push(out, &sl);
    out->v[out->n - 1].sandbox_id = sb_id;
    out->v[out->n - 1].sandbox_role = 1;
}

static const char *sandbox_title(const tam_sandbox_spec_t *spec)
{
    return spec->title && spec->title[0] ? spec->title : "Shell Sandbox";
}

static void layout_sandbox_toggle(const tam_sandbox_spec_t *spec, int sb_id,
                                  size_t logical_block, int width, linelist_t *out)
{
    static const char prefix[] = "[+] Sandbox: ";
    static const char hint[] = " (-> to try)";
    char href[40];
    snprintf(href, sizeof(href), "tam:sandbox:%d", sb_id);
    const char *title = sandbox_title(spec);
    spanlist_t sl;
    spanlist_init(&sl);
    spanlist_append(&sl, prefix, sizeof(prefix) - 1, TAM_STYLE_LINK, href, 0);
    size_t prefix_width = sizeof(prefix) - 1;
    size_t available = (size_t)width > prefix_width ? (size_t)width - prefix_width : 0;
    size_t cells;
    size_t len = tam_display_prefix(title, strlen(title), available, &cells);
    spanlist_append(&sl, title, len, TAM_STYLE_LINK, href, 0);
    if (available - cells >= sizeof(hint) - 1) {
        spanlist_append(&sl, hint, sizeof(hint) - 1, TAM_STYLE_LINK, href, 0);
    }
    out->logical_block = logical_block;
    out->logical_offset = 0;
    linelist_push(out, &sl);
    out->v[out->n - 1].sandbox_id = sb_id;
    out->v[out->n - 1].sandbox_role = 1;
}

static void sb_push_wrapped_text(linelist_t *out, size_t logical_block,
                                 int sb_id, int role, size_t box_inner_w,
                                 const char *text, unsigned style)
{
    if (text == NULL || text[0] == '\0') return;
    const char *p = text;
    while (*p != '\0') {
        const char *nl = strchr(p, '\n');
        size_t seg_len = nl ? (size_t)(nl - p) : strlen(p);
        if (seg_len == 0) {
            tam_span_t empty = { .text = "", .style = style };
            sb_push_bordered_spans(out, logical_block, sb_id, role,
                                     box_inner_w, &empty, 0);
        } else {
            size_t off = 0;
            while (off < seg_len) {
                size_t take = tam_display_prefix(p + off, seg_len - off,
                                                 box_inner_w, NULL);
                if (take == 0) break;
                char *chunk = malloc(take + 1);
                if (chunk != NULL) {
                    memcpy(chunk, p + off, take);
                    chunk[take] = '\0';
                    tam_span_t sp = { .text = chunk, .style = style };
                    sb_push_bordered_spans(out, logical_block, sb_id, role,
                                             box_inner_w, &sp, 1);
                    free(chunk);
                }
                off += take;
            }
        }
        if (nl == NULL) break;
        p = nl + 1;
        if (*p == '\0') break;
    }
}

static void sb_push_notice(linelist_t *out, size_t logical_block, int sb_id,
                           size_t box_inner_w, const char *text)
{
    size_t len = strlen(text);
    size_t cells = tam_display_width(text, len);
    size_t padding = box_inner_w > cells ? box_inner_w - cells : 0;
    char spaces[SANDBOX_MAX_INNER_WIDTH + 1];
    memset(spaces, ' ', padding);
    spaces[padding] = '\0';
    if (padding > 0) {
        tam_span_t spans[] = {
            { .text = spaces },
            { .text = (char *)text, .style = TAM_STYLE_SB_COMP | TAM_STYLE_DIM },
        };
        sb_push_bordered_spans(out, logical_block, sb_id, 2, box_inner_w, spans, 2);
    } else {
        sb_push_wrapped_text(out, logical_block, sb_id, 2, box_inner_w, text,
                             TAM_STYLE_SB_COMP | TAM_STYLE_DIM);
    }
}

static void layout_sandbox_input(const tam_sandbox_state_t *state, const char *prompt,
                                  int sb_id, size_t logical_block, size_t width,
                                  linelist_t *out)
{
    size_t prompt_cells;
    size_t prompt_len = tam_display_prefix(prompt, strlen(prompt), width - 1, &prompt_cells);
    if (prompt[prompt_len] != '\0') {
        prompt_len = tam_display_prefix(prompt, strlen(prompt), width / 2, &prompt_cells);
    }
    size_t available = width - prompt_cells;
    const char *input = state->input;
    size_t cursor = state->cursor;
    size_t cursor_cells = tam_display_width(input, cursor);
    size_t next = cursor;
    int char_cells = 1;
    tam_display_next(input, state->input_len, &next, NULL, NULL, &char_cells);
    size_t cursor_width = char_cells > 1 && (size_t)char_cells <= available
        ? (size_t)char_cells : 1;
    size_t start = 0;
    /* Scroll input horizontally to keep the cursor and its character visible. */
    while (cursor_cells + cursor_width > available && start < cursor) {
        int cells;
        tam_display_next(input, cursor, &start, NULL, NULL, &cells);
        cursor_cells -= (size_t)cells;
    }
    size_t input_cells;
    size_t end = start + tam_display_prefix(input + start, state->input_len - start,
                                             available, &input_cells);

    spanlist_t line;
    sb_begin_body_line(&line);
    spanlist_append(&line, prompt, prompt_len, TAM_STYLE_SB_COMP | TAM_STYLE_SB_BG, NULL, 0);
    tam_sb_tok_t tokens[64];
    size_t ntokens = tam_sandbox_tokenize(input, state->spec->lesson, tokens, 64);
    size_t unstyled = start;
    for (size_t i = 0; i < ntokens; i++) {
        size_t from = tokens[i].start > start ? tokens[i].start : start;
        size_t to = tokens[i].start + tokens[i].len;
        if (to > end) to = end;
        if (to <= from) continue;
        spanlist_append(&line, input + from, to - from,
                        tokens[i].style | TAM_STYLE_SB_BG, NULL, 0);
        unstyled = to;
    }
    spanlist_append(&line, input + unstyled, end - unstyled,
                    TAM_STYLE_SB_USER | TAM_STYLE_SB_BG, NULL, 0);
    sb_end_body_line(out, &line, logical_block, sb_id, 3, available - input_cells);
    out->v[out->n - 1].sandbox_cursor_col = (int)(3 + prompt_cells + cursor_cells);
}

static void layout_sandbox_state(const tam_sandbox_spec_t *spec,
                                 const tam_sandbox_state_t *state, int sb_id,
                                 size_t logical_block, int width, linelist_t *out)
{
    size_t box_inner_w = width > 5 ? (size_t)(width - 5) : 1;
    if (box_inner_w > SANDBOX_MAX_INNER_WIDTH) box_inner_w = SANDBOX_MAX_INNER_WIDTH;

    const char *prompt = tam_sandbox_prompt(spec);
    sb_push_border_line(out, logical_block, sb_id, box_inner_w, 1, sandbox_title(spec));

    if (state != NULL) {
        sb_push_notice(out, logical_block, sb_id, box_inner_w,
                       "Practice commands in this sandbox -- Esc to close");
        sb_push_notice(out, logical_block, sb_id, box_inner_w,
                       "Type a command, then press Return or Enter");
    } else {
        sb_push_notice(out, logical_block, sb_id, box_inner_w,
                       "Practice commands in this sandbox -- real terminals may differ slightly.");
    }

    if (spec->goal != NULL && spec->goal[0] != '\0') {
        sb_push_wrapped_text(out, logical_block, sb_id, 2, box_inner_w,
                             "Goals", TAM_STYLE_SB_HINT | TAM_STYLE_BOLD);
        char goal[320];
        snprintf(goal, sizeof(goal), "[%c] %s",
                 state != NULL && state->goal_done ? 'x' : ' ', spec->goal);
        sb_push_wrapped_text(out, logical_block, sb_id, 2, box_inner_w,
                             goal, TAM_STYLE_SB_HINT);
    }

    sb_push_bordered_spans(out, logical_block, sb_id, 2, box_inner_w, NULL, 0);

    if (state != NULL && state->nlog > 0) {
        size_t max_entries = 8;
        size_t start_e = state->nlog > max_entries ? state->nlog - max_entries : 0;
        for (size_t e = start_e; e < state->nlog; e++) {
            const tam_sb_log_entry_t *le = &state->log[e];
            if (le->kind == TAM_SB_LOG_CMD) {
                const char *pr = le->prompt ? le->prompt : "$ ";
                tam_sb_tok_t toks[64];
                size_t ntoks = tam_sandbox_tokenize(le->text, spec->lesson, toks, 64);
                tam_span_t spans[65];
                size_t ns = 0;
                spans[ns++] = (tam_span_t){ .text = (char *)pr, .style = TAM_STYLE_SB_COMP };
                char *bufs[64];
                for (size_t k = 0; k < ntoks && ns < 65; k++) {
                    bufs[k] = malloc(toks[k].len + 1);
                    if (bufs[k] != NULL) {
                        memcpy(bufs[k], le->text + toks[k].start, toks[k].len);
                        bufs[k][toks[k].len] = '\0';
                        spans[ns++] = (tam_span_t){ .text = bufs[k], .style = toks[k].style };
                    }
                }
                sb_push_bordered_spans(out, logical_block, sb_id, 2,
                                         box_inner_w, spans, ns);
                for (size_t k = 0; k < ntoks; k++) free(bufs[k]);
            } else {
                unsigned st = le->kind == TAM_SB_LOG_ERR ? TAM_STYLE_SB_ERR
                            : le->kind == TAM_SB_LOG_NOTE ? TAM_STYLE_SB_HINT
                            : TAM_STYLE_SB_COMP;
                sb_push_wrapped_text(out, logical_block, sb_id, 2, box_inner_w,
                                     le->text, st);
            }
        }
    } else if (spec->init_text != NULL && spec->init_text[0] != '\0') {
        sb_push_wrapped_text(out, logical_block, sb_id, 2, box_inner_w,
                             spec->init_text, TAM_STYLE_SB_COMP);
    }

    if (state != NULL)
        layout_sandbox_input(state, prompt, sb_id, logical_block, box_inner_w, out);
    sb_push_border_line(out, logical_block, sb_id, box_inner_w, 0, NULL);
}

static void layout_document(const blocklist_t *blocks,
                            const tam_sandbox_spec_t *sandboxes,
                            size_t nsandboxes, int width, linelist_t *out)
{
    if (width < 1) width = 1;

    for (size_t i = 0; i < blocks->n;) {
        const tam_block_t *b = &blocks->v[i];
        if (i > 0) {
            const tam_block_t *prev = &blocks->v[i - 1];
            int tight = (b->box == 0 && prev->box == 0 &&
                         blocks_are_tight(prev, b)) ||
                        (b->box > 0 && prev->box != b->box && b->tight_before);
            if (!tight) {
                out->logical_block = i;
                out->logical_offset = 0;
                linelist_blank(out);
            }
        }
        if (b->kind == B_SANDBOX && b->number >= 1 &&
            (size_t)b->number <= nsandboxes) {
            layout_sandbox_toggle(&sandboxes[b->number - 1], b->number, i, width, out);
            i++;
            continue;
        }
        if (b->box > 0) {
            size_t end = i + 1;
            while (end < blocks->n && blocks->v[end].box == b->box) {
                end++;
            }
            layout_box(blocks, i, end, width, out);
            i = end;
            continue;
        }
        layout_single_block(blocks, i, width, out, 1);
        i++;
    }
}

/* ------------------------------------------------------------------------- */
/* Public API                                                                 */
/* ------------------------------------------------------------------------- */

static tam_document_t *render_html(const char *src, size_t len, int width, int manual)
{
    parser_t p;
    memset(&p, 0, sizeof(p));
    p.title = tam_str_new("");

    parser_parse(&p, src, len);
    parser_end_block(&p);
    if (p.curr_table != NULL) {
        parser_finish_cell(&p);
        if (p.curr_table->nrows > 0) {
            int box = p.box_depth > 0 ? p.box_seq : 0;
            blocklist_add_table(&p.blocks, box, p.tight_next, p.curr_table);
            p.tight_next = 0;
        } else {
            table_free(p.curr_table);
        }
        p.curr_table = NULL;
    }

    for (size_t i = 0; i < p.blocks.n; i++) {
        if (manual) {
            tam_block_t *block = &p.blocks.v[i];
            link_manual_urls(&block->runs, &block->nruns, &p.next_link_id);
            if (block->table != NULL) {
                for (size_t r = 0; r < block->table->nrows; r++) {
                    tam_table_row_t *row = &block->table->rows[r];
                    for (size_t c = 0; c < row->ncells; c++) {
                        tam_table_cell_t *cell = &row->cells[c];
                        link_manual_urls(&cell->runs, &cell->nruns, &p.next_link_id);
                    }
                }
            }
        }
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
                if (!(p.blocks.v[i].runs[j].style & TAM_STYLE_LVL_MASK)) {
                    p.blocks.v[i].runs[j].style |= TAM_STYLE_DIM;
                }
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
    backing->sandboxes = p.sandboxes;
    backing->nsandboxes = p.nsandboxes;
    memset(&p.sandboxes, 0, sizeof(p.sandboxes));
    p.nsandboxes = 0;

    linelist_t out;
    memset(&out, 0, sizeof(out));
    layout_document(&backing->blocks, backing->sandboxes, backing->nsandboxes,
                    width, &out);
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
    doc->nlead = lead_lines(&backing->blocks, &out);
    doc->nav_next = p.nav_next;
    doc->nav_prev = p.nav_prev;
    doc->nav_up = p.nav_up;
    doc->nav_top = p.nav_top;
    doc->sandboxes = backing->sandboxes;
    doc->nsandboxes = backing->nsandboxes;
    doc->layout = backing;

    tam_str_free(p.title);
    free(p.lists);
    free(p.href);
    return doc;
}

tam_document_t *tam_render_html(const char *src, size_t len, int width)
{
    return render_html(src, len, width, 0);
}

tam_document_t *tam_render_man_html(const char *src, size_t len, int width)
{
    return render_html(src, len, width, 1);
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
    layout_document(&backing->blocks, backing->sandboxes, backing->nsandboxes,
                    content_width, &out);
    if (out.n == 0) linelist_push_blank(&out);
    if (out.v == NULL || out.n == 0) {
        free(out.v);
        return -1;
    }
    tam_line_t *old_lines = doc->lines;
    size_t old_nlines = doc->nlines;
    doc->lines = out.v;
    doc->nlines = out.n;
    doc->nlead = lead_lines(&backing->blocks, &out);
    document_lines_free(old_lines, old_nlines);
    return 0;
}

int tam_document_sync_sandbox(tam_document_t *doc, int content_width,
                              int sandbox_id, const tam_sandbox_state_t *state,
                              int expanded)
{
    if (doc == NULL || doc->sandboxes == NULL || sandbox_id < 1 ||
        (size_t)sandbox_id > doc->nsandboxes || content_width < 1)
        return -1;

    size_t first = doc->nlines, last = 0;
    size_t logical_block = 0;
    for (size_t i = 0; i < doc->nlines; i++) {
        if (doc->lines[i].sandbox_id == sandbox_id) {
            if (first == doc->nlines) {
                first = i;
                logical_block = doc->lines[i].logical_block;
            }
            last = i + 1;
        }
    }
    if (first >= last) return -1;

    linelist_t nl;
    memset(&nl, 0, sizeof(nl));
    const tam_sandbox_spec_t *spec = &doc->sandboxes[sandbox_id - 1];
    if (expanded) {
        layout_sandbox_state(spec, state, sandbox_id, logical_block, content_width, &nl);
    } else {
        layout_sandbox_toggle(spec, sandbox_id, logical_block, content_width, &nl);
    }
    if (nl.n == 0) {
        free(nl.v);
        return -1;
    }

    size_t old_count = last - first;
    size_t new_nlines = doc->nlines - old_count + nl.n;
    tam_line_t *merged = malloc(new_nlines * sizeof(*merged));
    if (merged == NULL) {
        document_lines_free(nl.v, nl.n);
        return -1;
    }
    if (first > 0) memcpy(merged, doc->lines, first * sizeof(*merged));
    memcpy(merged + first, nl.v, nl.n * sizeof(*merged));
    if (last < doc->nlines) {
        memcpy(merged + first + nl.n, doc->lines + last,
               (doc->nlines - last) * sizeof(*merged));
    }
    for (size_t i = first; i < last; i++) {
        for (size_t j = 0; j < doc->lines[i].nspans; j++) {
            free(doc->lines[i].spans[j].text);
            free(doc->lines[i].spans[j].href);
        }
        free(doc->lines[i].spans);
    }
    free(doc->lines);
    free(nl.v);
    doc->lines = merged;
    doc->nlines = new_nlines;
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
    free(doc->revision);
    free(doc->nav_next);
    free(doc->nav_prev);
    free(doc->nav_up);
    free(doc->nav_top);
    if (doc->layout != NULL) {
        layout_backing_t *backing = doc->layout;
        blocklist_free(&backing->blocks);
        anchorlist_free(&backing->anchors);
        for (size_t i = 0; i < backing->nsandboxes; i++)
            tam_sandbox_spec_free(&backing->sandboxes[i]);
        free(backing->sandboxes);
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

    /* The revision stamp is pager chrome, not a page line, but plain text has
     * no chrome: keep it here so the corpus build and guides can check it. */
    if (doc->revision != NULL && doc->revision[0] != '\0') {
        s = tam_str_cat(s, "\n");
        s = tam_str_cat(s, doc->revision);
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
