#ifndef TAM_INDEX_H
#define TAM_INDEX_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    char *topic;       /* primary topic name, e.g. "pipelines" */
    char *path;        /* relative path in corpus, e.g. "shell/pipelines.html" */
    char *title;       /* title, e.g. "Shell pipelines and redirection" */
    char *summary;     /* summary, e.g. "Shell pipelines, redirection, and core filters" */
    char *level;       /* declared levels, e.g. "beginner,intermediate,expert" */
    unsigned levels;   /* TAM_LEVEL_* mask parsed from level (see depth.h) */
    char *track;       /* track, e.g. "use", "maintain", "improve" */
    char **keywords;   /* array of keyword/alias strings */
    size_t num_keywords;
    char **prereqs;    /* topics to read first, e.g. "shell" */
    size_t num_prereqs;
} tam_index_entry_t;

typedef struct {
    tam_index_entry_t *entries;
    size_t count;
} tam_index_t;

typedef struct {
    const tam_index_entry_t *entry;
    int score;
    const char *match_type; /* "alias", "score", "substring", "fuzzy" */
} tam_search_result_t;

/* Load index from knowledge root (index.tsv). Returns NULL on error. */
tam_index_t *tam_index_load(const char *root);

/* Free index */
void tam_index_free(tam_index_t *idx);

/* Exact alias lookup (case-insensitive, trims outer quotes). Returns entry or NULL. */
const tam_index_entry_t *tam_index_lookup_alias(const tam_index_t *idx, const char *word);

/* Search the index. Populates results (up to max_results) sorted by score descending.
 * Returns total number of matches found. */
size_t tam_index_search(const tam_index_t *idx, const char *query,
                        tam_search_result_t *results, size_t max_results);

/* Compute closest fuzzy match across all aliases/topics with Levenshtein distance <= max_dist.
 * Returns distance (0..max_dist), or -1 if no candidate within max_dist. */
int tam_index_suggest_fuzzy(const tam_index_t *idx, const char *query,
                            const tam_index_entry_t **out_entry, const char **out_keyword,
                            int max_dist);

/* Print clear miss banner to stderr and return 1 (failure code). */
int tam_index_print_miss(FILE *out, const tam_index_t *idx, const char *query);

/* Print topic catalog. A non-zero level mask lists only topics offering that depth. */
void tam_index_print_catalog(FILE *out, const tam_index_t *idx, unsigned level);

/* Print topic catalog with output modifiers.
 * If terse is true: emits one topic slug per line.
 * If verbose is true: emits exhaustive diagnostics, titles, paths, levels, and keyword arrays.
 * Otherwise: emits concise topic names and summaries (Goldilocks bounded format).
 * A non-zero level mask filters to topics offering that depth. */
void tam_index_print_catalog_ex(FILE *out, const tam_index_t *idx, unsigned level, bool terse, bool verbose);

#endif /* TAM_INDEX_H */
