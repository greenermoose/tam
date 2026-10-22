#ifndef TAM_RENDER_H
#define TAM_RENDER_H

#include <stddef.h>

/* Inline style bits. */
enum {
    TAM_STYLE_BOLD    = 1u << 0,
    TAM_STYLE_ITALIC  = 1u << 1,
    TAM_STYLE_CODE    = 1u << 2,
    TAM_STYLE_LINK    = 1u << 3,
    TAM_STYLE_HEADING = 1u << 4,
    TAM_STYLE_DIM     = 1u << 5  /* page metadata, such as the level line */
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
    size_t      logical_block;
    size_t      logical_offset;
} tam_line_t;

typedef struct { unsigned id; int quiz, open; } tam_fold_t;

/* A laid-out page ready for either front end. */
typedef struct {
    char       *title;
    tam_line_t  location; /* Fixed Guide/manual location row, when available. */
    tam_line_t *lines;
    size_t      nlines;
    size_t      nlead;  /* Leading level lines, before the title. */
    char       *nav_next;
    char       *nav_prev;
    char       *nav_up;
    char       *nav_top;
    unsigned    levels; /* Levels a guide page declares; 0 for other pages. */
    int         hidden; /* Level sections the reader's lens hid. */
    tam_fold_t *folds;
    size_t      nfolds;
    void       *layout; /* private parsed blocks, retained for width reflow */
} tam_document_t;

/* Parse an HTML subset (len bytes) and lay it out to `width` columns. */
tam_document_t *tam_render_html(const char *src, size_t len, int width);
void tam_document_free(tam_document_t *doc);
/* Atomically relayout a retained parsed document at the requested width. */
int tam_document_reflow(tam_document_t *doc, int content_width);
/* First line of the block holding an id= or <a name=> anchor, or -1. */
int tam_document_anchor_line(const tam_document_t *doc, const char *id);
/* Ensure a generated/manual document has a truthful location row. */
void tam_document_ensure_location(tam_document_t *doc, const char *href);

/* Plain, escape-sequence-free text rendering of a document. Caller frees. */
char *tam_document_text(const tam_document_t *doc);

/* Callback used by the pager to load another page by relative href. */
typedef tam_document_t *(*tam_page_loader_t)(void *ctx, const char *href,
                                             int content_width);

/* Find the knowledge directory root. Returns newly allocated string or NULL. */
char *tam_find_knowledge_root(void);


#endif /* TAM_RENDER_H */
