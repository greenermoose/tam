/* Generated guide views built from the corpus index: the topic guide, the
 * keyword index and the topics list. */

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

char *url_encode(const char *s)
{
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    char *out = malloc(len * 3 + 1);
    if (out == NULL) return NULL;
    char *o = out;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            *o++ = (char)c;
        } else {
            sprintf(o, "%%%02X", c);
            o += 3;
        }
    }
    *o = '\0';
    return out;
}

tam_document_t *tam_topic_guide_document(const char *root, const char *topic, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }
    const tam_index_entry_t *matches[64];
    size_t num_matches = tam_index_find_topic_pages(idx, topic, level, matches, 64);
    /* No page at the reader's level: list every level under a banner rather
     * than fail. */
    unsigned missing = 0;
    if (num_matches == 0 && level != 0) {
        num_matches = tam_index_find_topic_pages(idx, topic, 0, matches, 64);
        missing = level;
        level = 0;
    }
    if (num_matches == 0) {
        tam_index_free(idx);
        return NULL;
    }

    char *esc_topic = tam_html_escape(topic);
    static const tam_crumb_t crumbs[] = {
        { "index.html", "Guide" }, { "tam:topics", "Topics" }, { NULL, "Topic Guide" }
    };
    tam_str_t title = tam_str_cat(tam_str_new("Topic Guide: "), topic);
    tam_str_t html = tam_page_begin(title, NULL, crumbs, 3);
    tam_str_free(title);
    unsigned lens = missing != 0 ? missing : level;
    if (lens != 0) {
        char *line = tam_level_lines(missing != 0 ? TAM_LEVEL_ALL : lens, lens,
                                     missing != 0, NULL);
        if (line != NULL) {
            html = tam_str_cat(html, line);
            free(line);
        }
    }
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Topic Guide: %s (%s Level)</h1>\n", esc_topic, tam_level_name(level));
    } else {
        html = tam_str_cat_printf(html, "<h1>Topic Guide: %s</h1>\n", esc_topic);
    }
    html = tam_str_cat_printf(html,
        "<p>Topic guide to pages in the Tamlinux guide covering <strong>%s</strong>.<br>\n"
        "Select a page with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to view it.<br>\n"
        "Press <kbd>u</kbd> to go back, <kbd>`</kbd> to return to the guide index, or <kbd>q</kbd> to exit.</p>\n"
        "<dl>\n",
        esc_topic);
    free(esc_topic);

    for (size_t i = 0; i < num_matches; i++) {
        const tam_index_entry_t *e = matches[i];
        char *esc_title = tam_html_escape(e->title);
        char *esc_summary = tam_html_escape(e->summary);
        char track_cap[32] = {0};
        if (e->track != NULL && e->track[0] != '\0') {
            snprintf(track_cap, sizeof(track_cap), "%s", e->track);
            track_cap[0] = toupper((unsigned char)track_cap[0]);
        } else {
            snprintf(track_cap, sizeof(track_cap), "Use");
        }
        char levels_buf[128] = {0};
        if (e->levels != 0) {
            char lvl_names[64];
            tam_level_format(e->levels, ", ", lvl_names, sizeof(lvl_names));
            snprintf(levels_buf, sizeof(levels_buf), " | Levels: %s", lvl_names);
        }

        if (level != 0) {
            html = tam_str_cat_printf(html,
                "<dt><a href=\"%s?level=%s\">%s</a> [%s%s]</dt>\n<dd>%s</dd>\n",
                e->path, tam_level_name(level), esc_title, track_cap, levels_buf, esc_summary);
        } else {
            html = tam_str_cat_printf(html,
                "<dt><a href=\"%s\">%s</a> [%s%s]</dt>\n<dd>%s</dd>\n",
                e->path, esc_title, track_cap, levels_buf, esc_summary);
        }
        free(esc_title);
        free(esc_summary);
    }
    html = tam_str_cat(html, "</dl>\n");
    tam_index_free(idx);
    return tam_page_render(html, width);
}

tam_document_t *tam_topic_keywords_document(const char *root, unsigned level, int width)
{
    (void)level;
    return tam_guide_search_document(root, NULL, width);
}

static int entry_title_order(const void *a, const void *b)
{
    const tam_index_entry_t *ea = *(const tam_index_entry_t * const *)a;
    const tam_index_entry_t *eb = *(const tam_index_entry_t * const *)b;
    int cmp = strcasecmp(ea->title, eb->title);
    return cmp ? cmp : strcmp(ea->path, eb->path);
}

tam_document_t *tam_catalog_document(const char *root, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }

    static const tam_crumb_t crumbs[] = { { "index.html", "Guide" }, { NULL, "Catalog" } };
    tam_str_t html = tam_page_begin("All Pages A to Z", NULL, crumbs, 2);
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>All Pages A to Z (%s)</h1>\n",
                                  tam_level_name(level));
    } else {
        html = tam_str_cat(html, "<h1>All Pages A to Z</h1>\n");
    }
    html = tam_str_cat(html,
        "<table>\n"
        "<tr><th>Page</th><th>Subject Matter</th></tr>\n");

    const tam_index_entry_t **ordered = calloc(idx->count ? idx->count : 1, sizeof(*ordered));
    if (ordered == NULL) { tam_index_free(idx); tam_str_free(html); return NULL; }
    for (size_t i = 0; i < idx->count; i++) ordered[i] = &idx->entries[i];
    qsort(ordered, idx->count, sizeof(*ordered), entry_title_order);
    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = ordered[i];
        if (level != 0) {
            html = tam_str_cat_printf(html, "<tr><td><a href=\"%s?level=%s\">",
                                      e->path, tam_level_name(level));
        } else {
            html = tam_str_cat_printf(html, "<tr><td><a href=\"%s\">", e->path);
        }
        html = tam_html_cat(html, e->title);
        html = tam_str_cat(html, "</a></td><td>");
        html = tam_html_cat(html, e->summary);
        html = tam_str_cat(html, "</td></tr>\n");
    }
    html = tam_str_cat(html, "</table>\n");
    free(ordered);
    tam_index_free(idx);
    return tam_page_render(html, width);
}

tam_document_t *tam_topics_document(const char *root, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) return NULL;
    static const tam_crumb_t crumbs[] = { { "index.html", "Guide" }, { NULL, "Browse topics" } };
    tam_str_t html = tam_page_begin("Browse topics", NULL, crumbs, 2);
    html = tam_str_cat(html, "<h1>Topics Covered in Your Tamlinux User Guide</h1><dl>");
    const tam_index_entry_t **ordered = calloc(idx->count ? idx->count : 1, sizeof(*ordered));
    if (ordered == NULL) { tam_index_free(idx); tam_str_free(html); return NULL; }
    size_t shown = 0;
    static const char * const subjects[] = {"hardware", "systems", "terminal", "networking", "security", "programming", "desktop", "tamlinux"};
    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        int subject = 0;
        for (size_t j = 0; j < sizeof(subjects)/sizeof(subjects[0]); j++)
            if (e->track && strcmp(e->track, subjects[j]) == 0) subject = 1;
        if (!subject || !e->parent || strcmp(e->parent, "index.html") != 0) continue;
        ordered[shown++] = e;
    }
    qsort(ordered, shown, sizeof(*ordered), entry_title_order);
    for (size_t i = 0; i < shown; i++) {
        const tam_index_entry_t *e = ordered[i];
        html = tam_str_cat_printf(html, "<dt><a href=\"%s\">", e->path);
        html = tam_html_cat(html, e->title);
        html = tam_str_cat(html, "</a></dt><dd>");
        html = tam_html_cat(html, e->summary);
        html = tam_str_cat(html, "</dd>");
    }
    free(ordered);
    html = tam_str_cat(html, "</dl><p><a href=\"tam:catalog\">All pages A–Z</a> · <a href=\"tam:paths\">Learn step by step</a> · <a href=\"tam:search\">Search</a></p>");
    tam_index_free(idx);
    if (!shown) { tam_str_free(html); return tam_catalog_document(root, level, width); }
    return tam_page_render(html, width);
}

static tam_str_t append_step_href(tam_str_t html, const char *path,
                                  const char *path_id, unsigned level)
{
    char *enc_id = url_encode(path_id);
    html = tam_str_cat_printf(html, "%s?path=%s", path, enc_id ? enc_id : path_id);
    if (level != 0 && tam_level_name(level) != NULL) {
        html = tam_str_cat_printf(html, "&amp;level=%s", tam_level_name(level));
    }
    free(enc_id);
    return html;
}

tam_document_t *tam_paths_document(const char *root, const char *path_id,
                                   unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }

    if (path_id != NULL && path_id[0] != '\0') {
        const tam_path_def_t *p = tam_index_find_path(idx, path_id);
        if (p == NULL) {
            tam_index_free(idx);
            return NULL;
        }
        static const tam_crumb_t crumbs[] = {
            { "index.html", "Guide" }, { "tam:paths", "Learning Paths" }, { NULL, "Path" }
        };
        tam_str_t doc_title = tam_str_cat(tam_str_new("Learning Path: "), p->title);
        tam_str_t html = tam_page_begin(doc_title, NULL, crumbs, 3);
        tam_str_free(doc_title);

        html = tam_str_cat(html, "<h1>");
        html = tam_html_cat(html, p->title);
        html = tam_str_cat(html, "</h1>\n<p>");
        html = tam_html_cat(html, p->summary);
        html = tam_str_cat(html, "</p>\n");

        if (p->num_steps == 0) {
            html = tam_str_cat(html, "<p>No steps are assigned to this path yet.</p>\n");
        } else {
            html = tam_str_cat(html, "<p><a href=\"");
            html = append_step_href(html, p->steps[0].path, p->id, level);
            html = tam_str_cat(html, "\">Start this path</a>");
            if (p->intro_path[0] != '\0') {
                html = tam_str_cat_printf(html, " | <a href=\"%s\">Path overview and practice</a>",
                                          p->intro_path);
            }
            html = tam_str_cat(html, "</p>\n<h2>Steps</h2>\n<ol>\n");
            for (size_t i = 0; i < p->num_steps; i++) {
                const tam_path_step_t *st = &p->steps[i];
                const tam_index_entry_t *e = tam_index_lookup_path(idx, st->path);
                html = tam_str_cat(html, "<li><a href=\"");
                html = append_step_href(html, st->path, p->id, level);
                html = tam_str_cat(html, "\">");
                html = tam_html_cat(html, st->title);
                html = tam_str_cat(html, "</a>");
                if (e != NULL && e->summary != NULL && e->summary[0] != '\0') {
                    html = tam_str_cat(html, " &mdash; ");
                    html = tam_html_cat(html, e->summary);
                }
                html = tam_str_cat(html, "</li>\n");
            }
            html = tam_str_cat(html, "</ol>\n");
        }
        html = tam_str_cat(html, "<p><a href=\"tam:paths\">Browse all learning paths</a></p>\n");
        tam_index_free(idx);
        return tam_page_render(html, width);
    }

    static const tam_crumb_t crumbs[] = { { "index.html", "Guide" }, { NULL, "Learning Paths" } };
    tam_str_t html = tam_page_begin("Learning Paths", NULL, crumbs, 2);
    html = tam_str_cat(html,
        "<h1>Learn step by step</h1>\n"
        "<p>Choose what you want to understand or do. Read the overview for prerequisites, stages, and a final task.<br>\n"
        "Follow any path to keep <strong>Back</strong>, <strong>Next</strong>, and <kbd>Right Arrow</kbd> "
        "on that course even across shared pages.</p>\n");

    if (idx->num_paths == 0) {
        html = tam_str_cat(html, "<p>No learning paths are indexed.</p>\n");
    } else {
        const char *groups[] = {"hardware", "systems", "terminal", "networking", "security", "programming", "desktop", "tamlinux", "help", "other"};
        for (size_t g = 0; g < sizeof(groups)/sizeof(groups[0]); g++) {
        int opened = 0;
        for (size_t i = 0; i < idx->num_paths; i++) {
            const tam_path_def_t *p = &idx->paths[i];
            const tam_index_entry_t *first = p->num_steps ? tam_index_lookup_path(idx, p->steps[0].path) : NULL;
            const char *group = first && first->track ? first->track : "other";
            size_t match = 0;
            for (; match + 1 < sizeof(groups)/sizeof(groups[0]); match++)
                if (strcmp(group, groups[match]) == 0) break;
            if (match != g) continue;
            if (!opened) {
                html = tam_str_cat(html, "<h2>");
                html = tam_html_cat(html, strcmp(groups[g], "help") == 0 ? "Getting started" : groups[g]);
                html = tam_str_cat(html, "</h2><dl>"); opened = 1;
            }
            char *enc_id = url_encode(p->id);
            html = tam_str_cat_printf(html, "<dt><a href=\"tam:path?id=%s\">",
                                      enc_id ? enc_id : p->id);
            html = tam_html_cat(html, p->title);
            html = tam_str_cat_printf(html, "</a> (%zu step%s)",
                                      p->num_steps, p->num_steps == 1 ? "" : "s");
            if (p->num_steps > 0) {
                html = tam_str_cat(html, " &mdash; <a href=\"");
                html = append_step_href(html, p->steps[0].path, p->id, level);
                html = tam_str_cat(html, "\">Start</a>");
            }
            if (p->intro_path[0] != '\0') {
                html = tam_str_cat_printf(html, " | <a href=\"%s\">Overview</a>", p->intro_path);
            }
            html = tam_str_cat(html, "</dt>\n<dd>");
            html = tam_html_cat(html, p->summary);
            html = tam_str_cat(html, "</dd>\n");
            free(enc_id);
        }
        if (opened) html = tam_str_cat(html, "</dl>\n");
        }
    }
    tam_index_free(idx);
    return tam_page_render(html, width);
}
