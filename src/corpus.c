/* The offline guide: finding the corpus, resolving page names, and reading
 * a page at a level with its folds and the related pages for its level line. */

#include <tam.h>

#include "ask.h"
#include "depth.h"
#include "fold.h"
#include "address.h"
#include "html.h"
#include "discovery.h"
#include "index.h"
#include "info.h"
#include "upgrade.h"
#include "install.h"
#include "man.h"
#include "pager.h"
#include "render.h"
#include "tldr.h"
#include "views.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <locale.h>
#include <pwd.h>

#include "version.h"

#ifndef TAM_DATADIR
#define TAM_DATADIR "/usr/local/share/tamlinux/knowledge"
#endif

/* Set by the Makefile from `git describe` when building from a checkout;
 * empty for packaged builds, which have no Git metadata. */
#ifndef TAM_BUILD
#define TAM_BUILD ""
#endif

#define TAM_EXIT_USAGE 2
#define TAM_EXIT_FAILURE 1

#include "corpus.h"

static int knowledge_ok(const char *dir);
static int knowledge_file_exists(const char *root, const char *rel);
static void resolve_page_path(const char *root, const char *page, char *out,
                       size_t cap);
static int shared_prereqs(const tam_index_entry_t *a, const tam_index_entry_t *b);
static int parent_relation(const tam_index_entry_t *page,
                    const tam_index_entry_t *e);
static char *related_at_level(const tam_index_t *idx,
                       const tam_index_entry_t *page, unsigned level);

/* A corpus-relative page path under root; never a URL or a path that
 * leaves the corpus. */
char *corpus_join(const char *root, const char *href)
{
    if (href == NULL || strstr(href, "://") != NULL) {
        return NULL;
    }
    return tam_path_join_under(root, href);
}

static int knowledge_ok(const char *dir)
{
    if (dir == NULL || dir[0] == '\0') {
        return 0;
    }
    char *p = tam_path_join(dir, "/index.html");
    int ok = p != NULL && tam_fs_file_exists(p);
    free(p);
    return ok;
}

char *tam_find_knowledge_root(void)
{
    const char *env = getenv("TAM_KNOWLEDGE_DIR");
    if (env != NULL && knowledge_ok(env)) {
        return strdup(env);
    }

    /* The corpus installed with this binary comes before any shared data
     * directory, so a development install under one prefix never renders
     * the pages of a packaged install under another. */
    const char *compiled = getenv("TAM_DATADIR");
    if (compiled == NULL || compiled[0] == '\0') {
        compiled = TAM_DATADIR;
    }
    if (knowledge_ok(compiled)) {
        return strdup(compiled);
    }

    const char *xdg = getenv("XDG_DATA_HOME");
    if (xdg != NULL && xdg[0] != '\0') {
        char *p = tam_path_join(xdg, "/tamlinux/knowledge");
        if (knowledge_ok(p)) {
            return p;
        }
        free(p);
    }

    const char *home = getenv("HOME");
    if (home != NULL && home[0] != '\0') {
        char *p = tam_path_join(home, "/.local/share/tamlinux/knowledge");
        if (knowledge_ok(p)) {
            return p;
        }
        free(p);
    }

    const char *dirs = getenv("XDG_DATA_DIRS");
    if (dirs != NULL && dirs[0] != '\0') {
        const char *s = dirs;
        while (*s != '\0') {
            const char *colon = strchr(s, ':');
            size_t len = colon ? (size_t)(colon - s) : strlen(s);
            if (len > 0) {
                char *base = malloc(len + 1);
                memcpy(base, s, len);
                base[len] = '\0';
                char *p = tam_path_join(base, "/tamlinux/knowledge");
                free(base);
                if (knowledge_ok(p)) {
                    return p;
                }
                free(p);
            }
            if (colon == NULL) {
                break;
            }
            s = colon + 1;
        }
    }

    return NULL;
}

char *find_knowledge_root(void)
{
    return tam_find_knowledge_root();
}

static int knowledge_file_exists(const char *root, const char *rel)
{
    char *p = corpus_join(root, rel);
    int ok = p != NULL && tam_fs_file_exists(p);
    free(p);
    return ok;
}

static void resolve_page_path(const char *root, const char *page, char *out,
                              size_t cap)
{
    static const char suffix[] = ".html";
    if (strstr(page, ".html") != NULL) {
        snprintf(out, cap, "%s", page);
        return;
    }
    char cand[600];
    int n = snprintf(cand, sizeof(cand), "%s%s", page, suffix);
    if (n > 0 && (size_t)n < sizeof(cand) && knowledge_file_exists(root, cand)) {
        snprintf(out, cap, "%s", cand);
        return;
    }
    n = snprintf(cand, sizeof(cand), "%s/index.html", page);
    if (n > 0 && (size_t)n < sizeof(cand) && knowledge_file_exists(root, cand)) {
        snprintf(out, cap, "%s", cand);
        return;
    }
    tam_index_t *idx = tam_index_load(root);
    if (idx != NULL) {
        const tam_index_entry_t *entry = tam_index_lookup_alias(idx, page);
        if (entry != NULL && entry->path != NULL) {
            snprintf(out, cap, "%s", entry->path);
            tam_index_free(idx);
            return;
        }
        tam_index_free(idx);
    }
    snprintf(out, cap, "%s%s", page, suffix);
}

/* Resolve a page name, keeping any "?level=NAME" level selection. */
void resolve_page(const char *root, const char *page, char *out,
                  size_t cap)
{
    const char *q = strchr(page, '?');
    if (q == NULL) {
        resolve_page_path(root, page, out, cap);
        return;
    }
    char base[600];
    size_t n = (size_t)(q - page);
    if (n >= sizeof(base)) {
        n = sizeof(base) - 1;
    }
    memcpy(base, page, n);
    base[n] = '\0';
    char path[600];
    resolve_page_path(root, base, path, sizeof(path));
    snprintf(out, cap, "%s%s", path, q);
}

static int shared_prereqs(const tam_index_entry_t *a, const tam_index_entry_t *b)
{
    int n = 0;
    for (size_t i = 0; i < a->num_prereqs; i++) {
        for (size_t j = 0; j < b->num_prereqs; j++) {
            if (strcmp(a->prereqs[i], b->prereqs[j]) == 0) {
                n++;
            }
        }
    }
    return n;
}

/* How `e` relates to `page` through the index's parent links: 0 for a
 * child, 1 for a sibling, 2 for the parent, -1 for any other page. */
static int parent_relation(const tam_index_entry_t *page,
                           const tam_index_entry_t *e)
{
    if (e->parent != NULL && strcmp(e->parent, page->path) == 0) {
        return 0;
    }
    if (page->parent != NULL && e->parent != NULL &&
        strcmp(e->parent, page->parent) == 0) {
        return 1;
    }
    if (page->parent != NULL && strcmp(page->parent, e->path) == 0) {
        return 2;
    }
    return -1;
}

/* Links to at most three relatives of `page` that offer `level`: its
 * children, then its siblings, then its parent (by the index's parent
 * links), each ranked by shared prerequisites, then by title. The level path
 * page is left out, since the level line already links it. NULL when there
 * are none, so the level line links only the level path. */
static char *related_at_level(const tam_index_t *idx,
                              const tam_index_entry_t *page, unsigned level)
{
    enum { MAX_RELATED = 3 };
    char level_path[32];
    snprintf(level_path, sizeof(level_path), "%s.html", tam_level_name(level));
    const tam_index_entry_t *best[MAX_RELATED];
    int best_rel[MAX_RELATED];
    int best_score[MAX_RELATED];
    size_t nbest = 0;
    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (e == page || !(e->levels & level) ||
            strcmp(e->path, level_path) == 0) {
            continue;
        }
        int rel = parent_relation(page, e);
        if (rel < 0) {
            continue;
        }
        int score = shared_prereqs(page, e);
        size_t at = nbest;
        while (at > 0 && (rel < best_rel[at - 1] ||
                          (rel == best_rel[at - 1] &&
                           (score > best_score[at - 1] ||
                            (score == best_score[at - 1] &&
                             strcasecmp(e->title, best[at - 1]->title) < 0))))) {
            at--;
        }
        if (at >= MAX_RELATED) {
            continue;
        }
        size_t last = nbest < MAX_RELATED ? nbest : MAX_RELATED - 1;
        for (size_t k = last; k > at; k--) {
            best[k] = best[k - 1];
            best_rel[k] = best_rel[k - 1];
            best_score[k] = best_score[k - 1];
        }
        best[at] = e;
        best_rel[at] = rel;
        best_score[at] = score;
        if (nbest < MAX_RELATED) {
            nbest++;
        }
    }
    if (nbest == 0) {
        return NULL;
    }
    tam_str_t html = tam_str_new("");
    for (size_t k = 0; k < nbest; k++) {
        char *title = tam_html_escape(best[k]->title);
        /* Titles may hold commas, so links are separated by bars. */
        html = tam_str_cat_printf(html, "%s<a href=\"%s\">%s</a>",
                                  k > 0 ? " | " : "", best[k]->path, title);
        free(title);
    }
    char *out = strdup(html);
    tam_str_free(html);
    return out;
}

/* Read a corpus page and prepare it at its selected level. `href` may carry
 * "?level=NAME". A page without that level is shown whole under a banner.
 * Returns NULL when the page is missing or unreadable. */
char *read_corpus_page(const char *root, const char *href,
                       size_t *out_len, tam_depth_info_t *info)
{
    unsigned level = 0;
    char *rel = tam_depth_split_href(href, &level);
    if (rel == NULL) {
        return NULL;
    }
    char *path = corpus_join(root, rel);
    if (path == NULL) {
        free(rel);
        return NULL;
    }
    size_t len = 0;
    char *src = tam_fs_read_file(path, &len);
    free(path);
    if (src == NULL) {
        free(rel);
        return NULL;
    }
    int rev = 0;
    char updated[64] = {0};
    char *related = NULL;
    tam_index_t *idx = tam_index_load(root);
    if (idx != NULL) {
        const tam_index_entry_t *e = tam_index_lookup_path(idx, rel);
        if (e != NULL && e->rev > 0 && e->updated != NULL && e->updated[0] != '\0') {
            rev = e->rev;
            snprintf(updated, sizeof(updated), "%s", e->updated);
        }
        if (e != NULL && level != 0 && !(tam_depth_declared(src, len) & level)) {
            related = related_at_level(idx, e, level);
        }
        tam_index_free(idx);
    }
    char *numbered = tam_fold_number(src, len);
    if (numbered == NULL) { free(related); free(src); free(rel); return NULL; }
    char *html = tam_depth_apply_rev(numbered, strlen(numbered), rel, level, rev,
                                     updated[0] ? updated : NULL, related,
                                     out_len, info);
    free(numbered);
    free(related);
    free(src);
    free(rel);
    return html;
}

tam_document_t *tam_corpus_page_document(const tam_loader_ctx_t *ctx,
                                         const char *href, int content_width)
{
    if (ctx == NULL || href == NULL || href[0] == '\0') {
        return NULL;
    }
    if (ctx->root == NULL && ctx->stdin_html == NULL) {
        return NULL;
    }
    char *open = NULL;
    char *bare = tam_fold_split_href(href, &open);
    if (bare == NULL) { free(open); return NULL; }
    if (ctx->root == NULL && strcmp(bare, "<stdin>") != 0) {
        free(bare); free(open); return NULL;
    }
    size_t len = 0;
    tam_depth_info_t info = {0};
    char *src = strcmp(bare, "<stdin>") == 0 && ctx->stdin_html != NULL
        ? tam_fold_number(ctx->stdin_html, ctx->stdin_len)
        : read_corpus_page(ctx->root, bare, &len, &info);
    free(bare);
    if (src == NULL) {
        free(open);
        return NULL;
    }
    tam_document_t *doc = tam_fold_render(src, strlen(src), content_width, open, 0);
    free(open);
    free(src);
    if (doc != NULL) {
        doc->levels = info.levels;
        doc->hidden = info.hidden;
    }
    return doc;
}
