#ifndef TAM_DEPTH_H
#define TAM_DEPTH_H

#include <stddef.h>

/* Level ladder (milestone 7; the reader's lens since TCK21).
 *
 * A guide page declares the levels it offers with
 *   <meta name="level" content="beginner, intermediate, expert">
 * and, when it offers more than one, wraps each level's material in
 *   <section data-level="beginner"> ... </section>
 * Material outside level sections (title, summary, "where to next") is shared
 * by every level. Selecting a level removes the other levels' sections; the
 * page is never rewritten on disk.
 *
 * A level can be selected through an href suffix, "page.html?level=NAME".
 * Following such a link sets the reader's lens. */

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

/* Display label ("All", "Beginner", "Intermediate", "Expert") and key
 * number (0-3) of a lens; 0 is All. */
const char *tam_level_label(unsigned bit);
int tam_level_number(unsigned bit);

/* The reader's level lens (TCK21): 0 (All) or one level bit, the same in
 * every page of a session and remembered between sessions in the state file
 * tamlinux/tam/level. Resolved on first use from tam_lens_override(), then
 * TAM_LEVEL, then the state file, else 0. A missing or invalid value means
 * 0, silently. */
unsigned tam_lens_get(void);

/* A lens for this process only (--level or an address with level=); never
 * written to the state file. */
void tam_lens_override(unsigned level);

/* The reader changed the lens. Remember it, and save it unless TAM_LEVEL is
 * set, so the environment never overwrites the saved choice. */
void tam_lens_set(unsigned level);

/* Remove a "level=" query parameter from `href` in place. Returns 1 and
 * stores its level (0 for all) when a valid one was present, else 0. */
int tam_href_take_level(char *href, unsigned *level);

/* Write `href` with "level=NAME" added as a query parameter (before any
 * "#fragment"); a lens of 0 copies `href` unchanged. Returns 0 when it does
 * not fit. */
int tam_href_put_level(const char *href, unsigned level, char *out,
                       size_t cap);

/* Split an href of the form "path[?level=NAME]". Returns a newly allocated
 * path without the query and stores the selected level bit (0 when there is
 * no query or when ?level=all or ?level=0) in *level. Returns NULL when the
 * query is present but is not a valid level selection. */
char *tam_depth_split_href(const char *href, unsigned *level);

/* What tam_depth_apply_rev() found: the levels the page declares (0 for a
 * page without a level declaration, such as a hub) and how many level
 * sections the lens hid. */
typedef struct {
    unsigned levels;
    int      hidden;
} tam_depth_info_t;

/* Prepare a page's HTML for display at `level` (0 = every level).
 *
 * A page that declares levels gains a level line after the first </h1>:
 * laddered pages a level bar linking each offered level by `path`,
 * single-level pages "Written for: ...". When a level is selected, the other
 * levels' sections are replaced by one-line markers. A page that does not
 * offer the selected level is shown whole, with a banner naming the levels
 * it was written for and, from `related` (an HTML fragment of links; NULL
 * or empty links the level path), where to read at the selected level.
 *
 * When `rev > 0` and `updated` is non-empty, appends
 * `<p>Rev <rev> Last updated <updated></p>`. Returns a newly allocated,
 * NUL-terminated buffer whose length is stored in *out_len, or NULL on
 * allocation failure. */
char *tam_depth_apply_rev(const char *src, size_t len, const char *path,
                          unsigned level, int rev, const char *updated,
                          const char *related, size_t *out_len,
                          tam_depth_info_t *info);

/* Levels a page declares in <meta name="level">, or 0. */
unsigned tam_depth_declared(const char *src, size_t len);

#endif /* TAM_DEPTH_H */
