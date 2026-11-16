#ifndef TAM_DOCUMENTATION_H
#define TAM_DOCUMENTATION_H

#include <stdbool.h>
#include <stddef.h>

/* Host documentation paths, independent of the guide installation. */
char *tam_documentation_root(const char *source);
bool tam_documentation_available(const char *source);
/* Evaluate corpus data-doc-source/data-doc-missing attributes in memory. */
char *tam_documentation_markup(const char *html, size_t length);

#endif
