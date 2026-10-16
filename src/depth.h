#ifndef TAM_DEPTH_H
#define TAM_DEPTH_H

#include <stddef.h>

/* Content-depth ladder (milestone 7).
 *
 * A guide page declares the depths it offers with
 *   <meta name="level" content="beginner, intermediate, expert">
 * and, when it offers more than one, wraps each depth's material in
 *   <section data-level="beginner"> ... </section>
 * Material outside level sections (title, summary, "where to next") is shared
 * by every depth. Selecting a depth removes the other levels' sections; the
 * page is never rewritten on disk.
 *
 * A depth can be selected through an href suffix, "page.html?level=NAME", so
 * pager history and in-page links keep the reader's chosen depth. */

enum {
    TAM_LEVEL_BEGINNER     = 1u << 0,
    TAM_LEVEL_INTERMEDIATE = 1u << 1,
    TAM_LEVEL_EXPERT       = 1u << 2
};

#define TAM_LEVEL_ALL \
    (TAM_LEVEL_BEGINNER | TAM_LEVEL_INTERMEDIATE | TAM_LEVEL_EXPERT)

/* One level name ("beginner"|"1", "intermediate"|"2", "expert"|"3";
 * case-insensitive) to its bit, or 0 when the name is not a single level. */
unsigned tam_level_parse(const char *name);

/* Parse a CLI or query level argument ("0"|"all" -> *level = 0, returns 1;
 * "1"|"beginner", "2"|"intermediate", "3"|"expert" -> *level = bit, returns 1;
 * anything else -> returns 0). */
int tam_level_parse_arg(const char *arg, unsigned *level);

/* A comma- or space-separated list of level names to a mask. Unknown names
 * are ignored. */
unsigned tam_level_parse_list(const char *list);

/* Name of a single level bit, or NULL. */
const char *tam_level_name(unsigned bit);

/* Number of levels in a mask. */
int tam_level_count(unsigned mask);

/* Write the levels of a mask, ladder order, joined by `sep`. Returns `out`. */
char *tam_level_format(unsigned mask, const char *sep, char *out, size_t cap);

/* Split an href of the form "path[?level=NAME]". Returns a newly allocated
 * path without the query and stores the selected level bit (0 when there is
 * no query or when ?level=all or ?level=0) in *level. Returns NULL when the
 * query is present but is not a valid level selection. */
char *tam_depth_split_href(const char *href, unsigned *level);

/* Prepare a page's HTML for display at `level` (0 = every depth).
 *
 * Laddered pages (more than one declared level) gain a short depth line
 * after the first </h1> linking each depth by `path`; when a level is
 * selected, the other levels' sections are removed.
 *
 * Stores the page's declared levels in *page_levels (pages without a level
 * declaration count as beginner). Returns NULL when `level` is not offered
 * by the page or on allocation failure; otherwise a newly allocated,
 * NUL-terminated buffer whose length is stored in *out_len. */
char *tam_depth_apply(const char *src, size_t len, const char *path,
                      unsigned level, size_t *out_len, unsigned *page_levels);

/* Prepare a page's HTML for display at `level`, optionally injecting
 * `<p>Rev <rev> Last updated <updated></p>` after the first </h1> when
 * `rev > 0` and `updated` is non-empty. */
char *tam_depth_apply_rev(const char *src, size_t len, const char *path,
                          unsigned level, int rev, const char *updated,
                          size_t *out_len, unsigned *page_levels);

#endif /* TAM_DEPTH_H */
