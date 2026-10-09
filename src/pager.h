#ifndef TAM_PAGER_H
#define TAM_PAGER_H

#include "render.h"

/* Terminal width helper shared with the corpus loader: TIOCGWINSZ, then
 * COLUMNS, then 80. */
int tam_terminal_width(void);

/* Run an interactive pager. Takes ownership of `initial`; `initial_href` is
 * the root-relative path used as the initial history entry. `load` resolves
 * further root-relative hrefs. Returns 0 on normal exit. */
int tam_pager_run(tam_document_t *initial, const char *initial_href,
                  tam_page_loader_t load, void *ctx);

#endif /* TAM_PAGER_H */
