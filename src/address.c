#include <tam.h>

#include "address.h"
#include "archwiki.h"
#include "depth.h"
#include "discovery.h"
#include "info.h"
#include "man.h"
#include "tldr.h"
#include "views.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int has_prefix(const char *s, const char *prefix)
{
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* tam_addr_kind and tam_addr_takes_lens defined after route table. */
static int hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    c = (char)tolower((unsigned char)c);
    return c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
}

/* Percent-decode src[0..end) into buf ('+' reads as a space). */
static void decode_into(const char *src, const char *end, char *buf, size_t cap)
{
    size_t used = 0;
    for (const char *p = src; p < end && used + 1 < cap; p++) {
        int hi, lo;
        if (*p == '%' && end - p > 2 && (hi = hex_value(p[1])) >= 0 &&
            (lo = hex_value(p[2])) >= 0) {
            buf[used++] = (char)(hi * 16 + lo);
            p += 2;
        } else {
            buf[used++] = *p == '+' ? ' ' : *p;
        }
    }
    buf[used] = '\0';
}

int tam_addr_param(const char *href, const char *key, char *buf, size_t cap)
{
    if (href == NULL || key == NULL || cap == 0) return 0;
    const char *q = strchr(href, '?');
    size_t klen = strlen(key);
    while (q != NULL) {
        q++;
        const char *end = q + strcspn(q, "&#");
        if ((size_t)(end - q) >= klen && strncmp(q, key, klen) == 0 && q[klen] == '=') {
            decode_into(q + klen + 1, end, buf, cap);
            return 1;
        }
        q = *end == '&' ? end : NULL;
    }
    return 0;
}

int tam_addr_take_param(char *href, const char *key, char *buf, size_t cap)
{
    if (href == NULL || key == NULL) return 0;
    size_t klen = strlen(key);
    char *q = strchr(href, '?');
    if (q == NULL) return 0;
    char *p = q + 1;
    while (*p != '\0' && *p != '#') {
        char *end = p + strcspn(p, "&#");
        if ((size_t)(end - p) > klen && strncmp(p, key, klen) == 0 && p[klen] == '=') {
            if (buf != NULL && cap > 0) {
                size_t n = (size_t)(end - (p + klen + 1));
                if (n >= cap) n = cap - 1;
                memcpy(buf, p + klen + 1, n);
                buf[n] = '\0';
            }
            if (*end == '&') {
                memmove(p, end + 1, strlen(end + 1) + 1);
            } else {
                char *cut = (p == q + 1) ? q : (p - 1);
                memmove(cut, end, strlen(end) + 1);
            }
            return 1;
        }
        if (*end != '&') break;
        p = end + 1;
    }
    return 0;
}

/* The lens a view address names with level=, or 0 for all levels. */
static unsigned view_level(const char *href)
{
    char value[32];
    unsigned level = 0;
    if (tam_addr_param(href, "level", value, sizeof(value)))
        (void)tam_level_parse_arg(value, &level);
    return level;
}

static tam_document_t *load_man(const tam_loader_ctx_t *c, const char *href, int w)
{
    (void)c;
    return href[4] == '\0' ? tam_man_uri_document("tam:man", w) : tam_man_load(href + 4, w);
}

static tam_document_t *load_info(const tam_loader_ctx_t *c, const char *href, int w)
{
    (void)c;
    return tam_info_load(href + 5, w);
}

static tam_document_t *load_tldr(const tam_loader_ctx_t *c, const char *href, int w)
{
    (void)c;
    return tam_tldr_load(href + 5, w);
}

static tam_document_t *load_tldr_view(const tam_loader_ctx_t *c, const char *href, int w)
{
    return tam_tldr_uri_document(href, w, c->plain_output);
}

static tam_document_t *load_archwiki(const tam_loader_ctx_t *c, const char *href, int w)
{
    (void)c;
    return href[9] == '\0' ? tam_archwiki_uri_document("tam:archwiki", w) : tam_archwiki_load(href + 9, w);
}

static tam_document_t *load_archwiki_view(const tam_loader_ctx_t *c, const char *href, int w)
{
    (void)c;
    return tam_archwiki_uri_document(href, w);
}

static tam_document_t *load_man_view(const tam_loader_ctx_t *c, const char *href, int w)
{
    (void)c;
    return tam_man_uri_document(href, w);
}

static tam_document_t *load_search(const tam_loader_ctx_t *c, const char *href, int w)
{
    return tam_guide_search_uri_document(c->root, href, w);
}

/* tam:explain?topic=T[&level=L], tam:explain:T, or the keyword index. */
static tam_document_t *load_explain(const tam_loader_ctx_t *c, const char *href, int w)
{
    char topic[256] = {0};
    if (!tam_addr_param(href, "topic", topic, sizeof(topic)) && href[11] == ':')
        decode_into(href + 12, href + 12 + strcspn(href + 12, "?#"), topic, sizeof(topic));
    if (c->root == NULL) return NULL;
    if (topic[0] != '\0')
        return tam_topic_guide_document(c->root, topic, view_level(href), w);
    return tam_topic_keywords_document(c->root, view_level(href), w);
}

static tam_document_t *load_keywords(const tam_loader_ctx_t *c, const char *href, int w)
{
    return c->root != NULL ? tam_topic_keywords_document(c->root, view_level(href), w) : NULL;
}

static tam_document_t *load_catalog(const tam_loader_ctx_t *c, const char *href, int w)
{
    return tam_catalog_document(c->root, view_level(href), w);
}

static tam_document_t *load_topics(const tam_loader_ctx_t *c, const char *href, int w)
{
    return c->root != NULL ? tam_topics_document(c->root, view_level(href), w) : NULL;
}

static tam_document_t *load_hubs(const tam_loader_ctx_t *c, const char *href, int w)
{
    return c->root != NULL ? tam_hubs_document(c->root, view_level(href), w) : NULL;
}

static tam_document_t *load_paths(const tam_loader_ctx_t *c, const char *href, int w)
{
    if (c->root == NULL) return NULL;
    char id[256] = {0};
    if (!tam_addr_param(href, "id", id, sizeof(id)) && strncmp(href, "tam:path:", 9) == 0)
        decode_into(href + 9, href + 9 + strcspn(href + 9, "?#"), id, sizeof(id));
    return tam_paths_document(c->root, id[0] ? id : NULL, view_level(href), w);
}

typedef tam_document_t *(*tam_view_loader_t)(const tam_loader_ctx_t *,
                                             const char *, int);

typedef struct {
    const char       *prefix;
    tam_addr_kind_t   kind;
    int               takes_lens;
    tam_view_loader_t load;
} tam_route_t;

/* One descriptor table for address classification, lens eligibility, and
 * loading. Order matters where prefixes overlap: tam:paths before tam:path. */
static const tam_route_t routes[] = {
    /* Schemes */
    { "man:",         TAM_ADDR_MAN,      0, load_man },
    { "info:",        TAM_ADDR_INFO,     0, load_info },
    { "tldr:",        TAM_ADDR_TLDR,     0, load_tldr },
    { "archwiki:",    TAM_ADDR_ARCHWIKI, 0, load_archwiki },
    /* Actions - never loaded through the address loader */
    { "tam:help",     TAM_ADDR_ACTION,   0, NULL },
    { "tam:start",    TAM_ADDR_ACTION,   0, NULL },
    { "tam:prompt:",  TAM_ADDR_ACTION,   0, NULL },
    { "tam:fold:",    TAM_ADDR_ACTION,   0, NULL },
    { "tam:sandbox:", TAM_ADDR_ACTION,   0, NULL },
    /* Generated views */
    { "tam:tldr",     TAM_ADDR_VIEW,     0, load_tldr_view },
    { "tam:archwiki", TAM_ADDR_VIEW,     0, load_archwiki_view },
    { "tam:man",      TAM_ADDR_VIEW,     0, load_man_view },
    { "tam:search",   TAM_ADDR_VIEW,     0, load_search },
    { "tam:explain",  TAM_ADDR_VIEW,     1, load_explain },
    { "tam:keywords", TAM_ADDR_VIEW,     1, load_keywords },
    { "tam:topics",   TAM_ADDR_VIEW,     1, load_topics },
    { "tam:hubs",     TAM_ADDR_VIEW,     1, load_hubs },
    { "tam:catalog",  TAM_ADDR_VIEW,     1, load_catalog },
    { "tam:paths",    TAM_ADDR_VIEW,     1, load_paths },
    { "tam:path",     TAM_ADDR_VIEW,     1, load_paths },
};

static const tam_route_t *find_route(const char *href)
{
    if (href == NULL) return NULL;
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        if (has_prefix(href, routes[i].prefix))
            return &routes[i];
    }
    return NULL;
}

tam_addr_kind_t tam_addr_kind(const char *href)
{
    if (href == NULL || href[0] == '\0') return TAM_ADDR_NONE;
    if (href[0] == '#') return TAM_ADDR_FRAGMENT;
    if (strcmp(href, "-") == 0 || href[0] == '<') return TAM_ADDR_STDIN;

    if (href[strcspn(href, ":/?#")] != ':' || has_prefix(href, "page:"))
        return TAM_ADDR_GUIDE;

    const tam_route_t *r = find_route(href);
    if (r != NULL) return r->kind;
    if (has_prefix(href, "tam:")) return TAM_ADDR_VIEW;
    return TAM_ADDR_EXTERNAL;
}

int tam_addr_takes_lens(const char *href)
{
    tam_addr_kind_t kind = tam_addr_kind(href);
    if (kind == TAM_ADDR_GUIDE) return 1;
    if (kind != TAM_ADDR_VIEW) return 0;
    const tam_route_t *r = find_route(href);
    return r != NULL && r->takes_lens;
}

int tam_addr_fold_id(const char *href, unsigned *id)
{
    if (id != NULL) *id = 0;
    if (href == NULL || strncmp(href, "tam:fold:", 9) != 0) return 0;
    char *end = NULL;
    unsigned long v = strtoul(href + 9, &end, 10);
    if (end == href + 9 || *end != '\0' || v == 0 || v > (unsigned)-1)
        return 0;
    if (id != NULL) *id = (unsigned)v;
    return 1;
}

int tam_addr_sandbox_id(const char *href, unsigned *id)
{
    if (id != NULL) *id = 0;
    if (href == NULL || strncmp(href, "tam:sandbox:", 12) != 0) return 0;
    char *end = NULL;
    unsigned long v = strtoul(href + 12, &end, 10);
    if (end == href + 12 || *end != '\0' || v == 0 || v > (unsigned)-1)
        return 0;
    if (id != NULL) *id = (unsigned)v;
    return 1;
}

const char *tam_addr_cli_alias(const char *page)
{
    if (page == NULL) return NULL;
    if (strcmp(page, "topics") == 0) return "tam:topics";
    if (strcmp(page, "hubs") == 0) return "tam:hubs";
    if (strcmp(page, "paths") == 0) return "tam:paths";
    return NULL;
}

const char *tam_addr_strip_page_prefix(const char *href)
{
    return (href != NULL && has_prefix(href, "page:")) ? href + 5 : href;
}

tam_document_t *tam_address_load(void *vctx, const char *href, int width)
{
    const tam_loader_ctx_t *ctx = vctx;
    switch (tam_addr_kind(href)) {
    case TAM_ADDR_NONE:
    case TAM_ADDR_FRAGMENT:
    case TAM_ADDR_ACTION:
    case TAM_ADDR_EXTERNAL:
        return NULL;
    case TAM_ADDR_GUIDE:
    case TAM_ADDR_STDIN:
        return tam_corpus_page_document(ctx, tam_addr_strip_page_prefix(href), width);
    default:
        break;
    }
    const tam_route_t *r = find_route(href);
    if (r != NULL && r->load != NULL)
        return r->load(ctx, href, width);
    return NULL;
}
