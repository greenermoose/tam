#ifndef TAM_RENDER_H
#define TAM_RENDER_H

#include <stddef.h>

/* Inline style bits. */
enum {
    TAM_STYLE_BOLD    = 1u << 0,
    TAM_STYLE_ITALIC  = 1u << 1,
    TAM_STYLE_CODE    = 1u << 2,
    TAM_STYLE_LINK    = 1u << 3,
    TAM_STYLE_HEADING = 1u << 4
};

/* A contiguous run of inline text with one style and optional link target. */
typedef struct {
    char    *text;
    unsigned style;
    char    *href; /* NULL unless TAM_STYLE_LINK */
    int      link_id; /* Unique identifier per <a> tag, or 0 if unlinked */
} tam_span_t;

/* A single visual line: an ordered list of styled spans. A line with no spans
 * is a blank separator line. */
typedef struct {
    tam_span_t *spans;
    size_t      nspans;
} tam_line_t;

/* A laid-out page ready for either front end. */
typedef struct {
    char       *title;
    tam_line_t *lines;
    size_t      nlines;
} tam_document_t;

/* Parse an HTML subset (len bytes) and lay it out to `width` columns. */
tam_document_t *tam_render_html(const char *src, size_t len, int width);
void tam_document_free(tam_document_t *doc);

/* Plain, escape-sequence-free text rendering of a document. Caller frees. */
char *tam_document_text(const tam_document_t *doc);

/* Callback used by the pager to load another page by relative href. */
typedef tam_document_t *(*tam_page_loader_t)(void *ctx, const char *href);

#endif /* TAM_RENDER_H */
