#ifndef TAM_INFO_H
#define TAM_INFO_H

#include <tam.h>
#include "render.h"

/* Locate an info file for given manual name.
 * Searches $INFOPATH then /usr/share/info, /usr/local/share/info.
 * Checks .info.gz, .info, .gz, and unadorned name.
 * Returns 1 if found (path written to path_out), or 0 if not found. */
int tam_info_locate(const char *manual, char *path_out, size_t path_cap);

/* Render an info node into a tam_document_t.
 * spec can be:
 *   "" or NULL or "dir"   -> top-level info directory catalog ((dir)Top)
 *   "MANUAL"              -> Top node of MANUAL
 *   "MANUAL/NODE"         -> specific NODE of MANUAL
 *   "MANUAL(NODE)"        -> specific NODE of MANUAL
 *   "info:MANUAL"         -> Top node of MANUAL
 *   "info:MANUAL/NODE"    -> specific NODE of MANUAL
 * width is terminal layout width.
 * Returns NULL if the manual or node cannot be found or rendered. */
tam_document_t *tam_info_load(const char *spec, int width);

/* Read a whole file, decompressing it when it is gzip (zlib reads plain
 * files unchanged). NUL-terminated; NULL on failure. Caller frees. */
char *tam_read_gzip_file(const char *path, size_t *out_len);

/* Execute the 'tam info' CLI subcommand with given argument vector. */
int tam_cmd_info(int n, char **args);

#endif /* TAM_INFO_H */
