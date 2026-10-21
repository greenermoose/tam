#ifndef TAM_FOLD_H
#define TAM_FOLD_H

#include "render.h"

/* Annotate folds before level filtering, so numbers survive lens changes. */
char *tam_fold_number(const char *src, size_t len);
/* Transform numbered HTML, retaining visible fold metadata on the document.
 * Pipe output moves answers to the nearest enclosing section's end. */
tam_document_t *tam_fold_render(const char *src, size_t len, int width,
                                const char *open, int pipe);
/* Remove the pager's private fold query from an address. Caller frees both. */
char *tam_fold_split_href(const char *href, char **open);

#endif
