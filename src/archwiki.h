#ifndef TAM_ARCHWIKI_H
#define TAM_ARCHWIKI_H

#include "render.h"

/* Locate the installed or overridden Arch Wiki HTML directory (en/).
 * Honors $TAM_ARCHWIKI_DIR first, then /usr/share/doc/arch-wiki/html/en.
 * Caller must free. */
char *tam_find_archwiki_root(void);

/* Locate and render an Arch Wiki page to a tam_document_t.
 * `spec` may be a page slug/title (e.g. "Pacman", "Arch Linux", "Systemd#Units")
 * or prefixed with "archwiki:". Returns NULL if the page is not found. */
tam_document_t *tam_archwiki_load(const char *spec, int width);

/* Arch Wiki discovery and index URI grammar (query values are percent-encoded UTF-8):
 *   tam:archwiki
 *   tam:archwiki:open?query=PAGE
 *   tam:archwiki:keyword?query=TERMS[&offset=N]
 *   tam:archwiki:list[?letter=X][&offset=N] */
tam_document_t *tam_archwiki_uri_document(const char *uri, int width);

/* Execute the 'tam archwiki' subcommand with given argument vector. */
int tam_cmd_archwiki(int n, char **args);

#endif /* TAM_ARCHWIKI_H */
