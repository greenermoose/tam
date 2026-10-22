#ifndef TAM_CORPUS_H
#define TAM_CORPUS_H

/* Private to the command: corpus helpers shared by tam.c, corpus.c and
 * views.c. Public views are in views.h, identity in version.h. */

#include <stdbool.h>
#include <stddef.h>

#include "depth.h"
#include "index.h"

char *corpus_join(const char *root, const char *href);
char *find_knowledge_root(void);
void resolve_page(const char *root, const char *page, char *out,
                  size_t cap);
char *read_corpus_page(const char *root, const char *href,
                       size_t *out_len, tam_depth_info_t *info);
char *url_encode(const char *s);

#endif /* TAM_CORPUS_H */
