#ifndef TAM_DISCOVERY_H
#define TAM_DISCOVERY_H

#include "render.h"

/* Render the in-reader Guide Search entry or linked corpus-ranked results.
 * URI grammar is tam:search or tam:search?query=<percent-encoded UTF-8>. */
tam_document_t *tam_guide_search_document(const char *root,
                                          const char *query,
                                          int width);
tam_document_t *tam_guide_search_uri_document(const char *root,
                                              const char *uri,
                                              int width);

#endif /* TAM_DISCOVERY_H */
