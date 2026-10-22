#include <tam.h>

#include "address.h"
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

tam_addr_kind_t tam_addr_kind(const char *href)
{
    if (href == NULL || href[0] == '\0') return TAM_ADDR_NONE;
    if (href[0] == '#') return TAM_ADDR_FRAGMENT;
    if (strcmp(href, "-") == 0 || href[0] == '<') return TAM_ADDR_STDIN;
    if (strstr(href, "://") != NULL) return TAM_ADDR_EXTERNAL;
    if (href[strcspn(href, ":/?#")] != ':' || has_prefix(href, "page:"))
        return TAM_ADDR_GUIDE;
    if (has_prefix(href, "man:")) return TAM_ADDR_MAN;
    if (has_prefix(href, "info:")) return TAM_ADDR_INFO;
    if (has_prefix(href, "tldr:")) return TAM_ADDR_TLDR;
    if (strcmp(href, "tam:help") == 0 || has_prefix(href, "tam:prompt:") ||
        has_prefix(href, "tam:fold:"))
        return TAM_ADDR_ACTION;
    if (has_prefix(href, "tam:")) return TAM_ADDR_VIEW;
    return TAM_ADDR_EXTERNAL;
}

int tam_addr_takes_lens(const char *href)
{
    tam_addr_kind_t kind = tam_addr_kind(href);
    return kind == TAM_ADDR_GUIDE ||
           (kind == TAM_ADDR_VIEW && (has_prefix(href, "tam:explain") ||
                                      has_prefix(href, "tam:topics") ||
                                      has_prefix(href, "tam:keywords")));
}

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
    (void)c;
    return tam_tldr_uri_document(href, w);
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

static tam_document_t *load_topics(const tam_loader_ctx_t *c, const char *href, int w)
{
    return c->root != NULL ? tam_topics_document(c->root, view_level(href), w) : NULL;
}

static const struct {
    const char *prefix;
    tam_document_t *(*load)(const tam_loader_ctx_t *, const char *, int);
} routes[] = {
    { "man:",         load_man },
    { "info:",        load_info },
    { "tldr:",        load_tldr },
    { "tam:tldr",     load_tldr_view },
    { "tam:man",      load_man_view },
    { "tam:search",   load_search },
    { "tam:explain",  load_explain },
    { "tam:keywords", load_keywords },
    { "tam:topics",   load_topics },
};

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
        return tam_corpus_page_document(ctx, has_prefix(href, "page:") ? href + 5 : href, width);
    default:
        break;
    }
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++) {
        if (has_prefix(href, routes[i].prefix))
            return routes[i].load(ctx, href, width);
    }
    return NULL;
}
