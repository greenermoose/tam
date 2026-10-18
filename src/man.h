#ifndef TAM_MAN_H
#define TAM_MAN_H

#include <tam.h>
#include "render.h"

/* Linkify groff HTML so manual cross-references like <b>awk</b>(1) become
 * <a href="man:awk(1)"><b>awk</b>(1)</a>. Caller must free returned string. */
char *tam_man_linkify_html(const char *src);

/* Locate and render a manual page to a tam_document_t.
 * spec can be "name", "name(sec)", "sec name", or "man:name(sec)".
 * Returns NULL if the manual page cannot be found or rendered. */
tam_document_t *tam_man_load(const char *spec, int width);

/* Manual discovery URI grammar (query values are percent-encoded UTF-8):
 *   tam:man
 *   tam:man:open?query=PAGE[%20SECTION] (or PAGE(SECTION))
 *   tam:man:keyword?query=TERMS
 *   tam:man:section?section=SECTION[&offset=N]
 * Lookup text is capped at 120 bytes; sections are 1-8 alphanumeric bytes.
 * This loader never executes a shell command from a query value. */
tam_document_t *tam_man_uri_document(const char *uri, int width);

/* Resolve manual page source path using man -w. Caller must free returned path. */
char *tam_man_resolve_path(const char *name, const char *sec);

/* Execute the 'tam man' command with given argument vector. */
int tam_cmd_man(int n, char **args);

#endif /* TAM_MAN_H */
