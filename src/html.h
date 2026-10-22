#ifndef TAM_HTML_H
#define TAM_HTML_H

#include <tam.h>
#include "render.h"

/* HTML for the pages tam generates (topics, search, man, Info and tldr
 * views). Every generated page is HTML read by the one renderer, so these
 * helpers keep escaping and framing in one place. */

/* Escape &, <, >, " and ' for HTML text or a quoted attribute. NULL reads as
 * "". Returns a newly allocated string, or NULL on allocation failure. */
char *tam_html_escape(const char *text);

/* Append escaped text (all of it, or `len` bytes) to s. */
tam_str_t tam_html_cat(tam_str_t s, const char *text);
tam_str_t tam_html_cat_len(tam_str_t s, const char *text, size_t len);

/* One breadcrumb: a link, or plain text when href is NULL (the last one). */
typedef struct {
    const char *href;
    const char *label;
} tam_crumb_t;

/* Start a page: the document head with the escaped title and `head_extra`
 * (raw HTML such as <link> tags, or NULL), then the body and, when ncrumbs
 * is not zero, the breadcrumb row. Labels are escaped; hrefs are written as
 * given. */
tam_str_t tam_page_begin(const char *title, const char *head_extra,
                         const tam_crumb_t *crumbs, size_t ncrumbs);

/* Close the body and the document. */
tam_str_t tam_page_end(tam_str_t html);

/* Close the page, render it at `width` and free the HTML. */
tam_document_t *tam_page_render(tam_str_t html, int width);

#endif /* TAM_HTML_H */
