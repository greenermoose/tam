#ifndef TAM_ADDRESS_H
#define TAM_ADDRESS_H

#include <stddef.h>
#include "render.h"

/* Addresses: everything tam can open, the form `=` shows and `tam show`
 * takes.
 *   PATH, page:PATH          a guide page: PATH[?level=L][&fold=N][#id]
 *   man:NAME[(SEC)]          a manual page ("man:" alone: the Man pages view)
 *   info:MANUAL[/NODE]       an Info node
 *   tldr:[PLATFORM/]NAME     a tldr page
 *   tam:VIEW...              a generated view: topics, keywords, explain,
 *                            search, man, tldr
 *   tam:help, tam:prompt:*, tam:fold:*   pager actions, never loaded
 *   anything with "://", or another scheme   external, never fetched
 */
typedef enum {
    TAM_ADDR_NONE,      /* NULL or empty */
    TAM_ADDR_FRAGMENT,  /* #id within the current page */
    TAM_ADDR_STDIN,     /* "-" or "<stdin>" */
    TAM_ADDR_GUIDE,
    TAM_ADDR_MAN,
    TAM_ADDR_INFO,
    TAM_ADDR_TLDR,
    TAM_ADDR_VIEW,
    TAM_ADDR_ACTION,
    TAM_ADDR_EXTERNAL
} tam_addr_kind_t;

tam_addr_kind_t tam_addr_kind(const char *href);

/* Whether the reader's level lens applies: guide pages and the tam:explain,
 * tam:topics and tam:keywords views. */
int tam_addr_takes_lens(const char *href);

/* The value of `key` in the address's query ("?a=1&b=2"), percent-decoded
 * into buf. Returns 1 when the key is present. */
int tam_addr_param(const char *href, const char *key, char *buf, size_t cap);

/* What every loader needs: the guide root (may be NULL) and, for `tam show
 * -`, the page read from standard input. */
typedef struct {
    const char *root;
    const char *stdin_html;
    size_t      stdin_len;
} tam_loader_ctx_t;

/* The one page loader (a tam_page_loader_t taking a tam_loader_ctx_t):
 * routes an address to its source and renders it at `width`. NULL when it
 * cannot be opened, and always for actions and external links. */
tam_document_t *tam_address_load(void *ctx, const char *href, int width);

#endif /* TAM_ADDRESS_H */
