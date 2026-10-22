#ifndef TAM_VIEWS_H
#define TAM_VIEWS_H

#include "address.h"
#include "render.h"

/* Generated guide views and guide pages, built from the corpus index
 * (src/tam.c). `level` is a lens: 0 for all levels or one level bit. */
tam_document_t *tam_topics_document(const char *root, unsigned level, int width);
tam_document_t *tam_topic_keywords_document(const char *root, unsigned level, int width);
tam_document_t *tam_topic_guide_document(const char *root, const char *topic,
                                         unsigned level, int width);

/* A guide page (or the page read from standard input), with its levels and
 * folds applied. */
tam_document_t *tam_corpus_page_document(const tam_loader_ctx_t *ctx,
                                         const char *href, int width);

#endif /* TAM_VIEWS_H */
