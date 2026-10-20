#ifndef TAM_TLDR_H
#define TAM_TLDR_H

#include <tam.h>
#include "render.h"

/* Locate the installed or checkout tldr directory (containing pages/ and index.tsv).
 * Honors $TAM_TLDR_DIR first, then <knowledge_root>/tldr. Caller must free. */
char *tam_find_tldr_root(const char *knowledge_root);

/* Locate and render a tldr page to a tam_document_t.
 * spec can be "tar", "git commit", "git-commit", "linux/cat", "common/cat",
 * or prefixed with "tldr:". Returns NULL if the page is not found. */
tam_document_t *tam_tldr_load(const char *spec, int width);

/* tldr discovery and index URI grammar (query values are percent-encoded UTF-8):
 *   tam:tldr
 *   tam:tldr:open?query=COMMAND
 *   tam:tldr:keyword?query=TERMS[&offset=N]
 *   tam:tldr:list[?letter=X][&platform=P][&offset=N] */
tam_document_t *tam_tldr_uri_document(const char *uri, int width);

/* Execute the 'tam tldr' subcommand with given argument vector. */
int tam_cmd_tldr(int n, char **args);

#endif /* TAM_TLDR_H */
