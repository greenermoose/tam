#ifndef TAM_CMD_RENDER_H
#define TAM_CMD_RENDER_H

#include <stddef.h>
#include "sandbox.h"

/* Inline style bits. */
enum {
    TAM_STYLE_BOLD     = 1u << 0,
    TAM_STYLE_ITALIC   = 1u << 1,
    TAM_STYLE_CODE     = 1u << 2,
    TAM_STYLE_LINK     = 1u << 3,
    TAM_STYLE_HEADING  = 1u << 4,
    TAM_STYLE_DIM      = 1u << 5,  /* page metadata, such as the level line */
    TAM_STYLE_LVL_0    = 1u << 6,  /* level 0: all text */
    TAM_STYLE_LVL_1    = 1u << 7,  /* level 1: novice text */
    TAM_STYLE_LVL_2    = 1u << 8,  /* level 2: learner text */
    TAM_STYLE_LVL_3    = 1u << 9,  /* level 3: expert text */
    TAM_STYLE_LVL_MASK = (1u << 6) | (1u << 7) | (1u << 8) | (1u << 9),
    TAM_STYLE_SB_BG    = 1u << 10, /* sandbox background tint */
    TAM_STYLE_SB_COMP  = 1u << 11, /* computer-provided output / prompt */
    TAM_STYLE_SB_ERR   = 1u << 12, /* simulated stderr */
    TAM_STYLE_SB_USER  = 1u << 13, /* user-typed input */
    TAM_STYLE_SB_CMD   = 1u << 14, /* command token */
    TAM_STYLE_SB_FLAG  = 1u << 15, /* option / flag / switch token */
    TAM_STYLE_SB_PATH  = 1u << 16, /* path / file argument token */
    TAM_STYLE_SB_OP    = 1u << 17, /* pipeline / redirection operator */
    TAM_STYLE_SB_VAR   = 1u << 18, /* variable / quoted token */
    TAM_STYLE_SB_HINT  = 1u << 19, /* pedagogical hint / goal badge */
    TAM_STYLE_SB_BORDER = 1u << 20 /* sandbox border */
};

/* A contiguous run of inline text with one style and optional link target. */
typedef struct {
    char    *text;
    unsigned style;
    char    *href; /* NULL unless TAM_STYLE_LINK */
    int      link_id; /* Unique identifier per link, or 0 if unlinked */
} tam_span_t;

/* A single visual line: an ordered list of styled spans. A line with no spans
 * is a blank separator line. */
typedef struct {
    tam_span_t *spans;
    size_t      nspans;
    size_t      logical_block;
    size_t      logical_offset;
    int         sandbox_id;   /* 1-based sandbox index, or 0 */
    int         sandbox_role; /* 0=none, 1=border, 2=body, 3=input */
    int         sandbox_cursor_col; /* Display column on the sandbox input line */
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
    char       *revision; /* "Rev N Last updated ..." pager chrome, or NULL. */
    tam_fold_t *folds;
    size_t      nfolds;
    tam_sandbox_spec_t *sandboxes;
    size_t      nsandboxes;
    void       *layout; /* private parsed blocks, retained for width reflow */
} tam_document_t;

/* Parse an HTML subset (len bytes) and lay it out to `width` columns. */
tam_document_t *tam_doc_render_html(const char *src, size_t len, int width);
/* Render groff HTML, discovering bare web URLs in manual text. */
tam_document_t *tam_doc_render_man_html(const char *src, size_t len, int width);
void tam_document_free(tam_document_t *doc);
/* Atomically relayout a retained parsed document at the requested width. */
int tam_document_reflow(tam_document_t *doc, int content_width);
/* Re-render one sandbox. Expanded with a NULL state prints the exercise. */
int tam_document_sync_sandbox(tam_document_t *doc, int content_width,
                              int sandbox_id, const tam_sandbox_state_t *state,
                              int expanded);
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


#endif /* TAM_CMD_RENDER_H */
