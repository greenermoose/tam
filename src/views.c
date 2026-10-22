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
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }
    tam_keyword_list_t *list = tam_index_get_keywords(idx, level);
    if (list == NULL) {
        tam_index_free(idx);
        return NULL;
    }

    static const tam_crumb_t crumbs[] = { { "index.html", "Guide" }, { NULL, "Keyword Index" } };
    tam_str_t html = tam_page_begin("Keyword Index", NULL, crumbs, 2);
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Keyword Index (%s)</h1>\n", tam_level_name(level));
    } else {
        html = tam_str_cat(html, "<h1>Keyword Index</h1>\n");
    }
    html = tam_str_cat(html,
        "<p>Every word <code>tam explain</code> recognizes and what it leads to.<br>\n"
        "Select a keyword with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to see its topic guide. "
        "Press <kbd>`</kbd> for the guide index, or <kbd>q</kbd> to quit.</p>\n"
        "<table>\n"
        "<tr><th>Keyword</th><th>Pages</th><th>Covers</th></tr>\n");

    for (size_t i = 0; i < list->count; i++) {
        char *enc_kw = url_encode(list->items[i].keyword);
        char *esc_kw = tam_html_escape(list->items[i].keyword);
        char *esc_sum = tam_html_escape(list->items[i].primary_summary);
        size_t cnt = list->items[i].count;

        if (level != 0) {
            html = tam_str_cat_printf(html,
                "<tr><td><a href=\"tam:explain?topic=%s&amp;level=%s\">%s</a></td><td>%zu page%s</td><td>%s</td></tr>\n",
                enc_kw ? enc_kw : "", tam_level_name(level), esc_kw, cnt, (cnt == 1 ? "" : "s"), esc_sum);
        } else {
            html = tam_str_cat_printf(html,
                "<tr><td><a href=\"tam:explain?topic=%s\">%s</a></td><td>%zu page%s</td><td>%s</td></tr>\n",
                enc_kw ? enc_kw : "", esc_kw, cnt, (cnt == 1 ? "" : "s"), esc_sum);
        }
        free(enc_kw);
        free(esc_kw);
        free(esc_sum);
    }
    html = tam_str_cat(html, "</table>\n");
    tam_keyword_list_free(list);
    tam_index_free(idx);
    return tam_page_render(html, width);
}

tam_document_t *tam_topics_document(const char *root, unsigned level, int width)
{
    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        return NULL;
    }

    static const tam_crumb_t crumbs[] = { { "index.html", "Guide" }, { NULL, "Topics" } };
    tam_str_t html = tam_page_begin("Topics A to Z", NULL, crumbs, 2);
    if (level != 0) {
        html = tam_str_cat_printf(html, "<h1>Topics A to Z (%s)</h1>\n",
                                  tam_level_name(level));
    } else {
        html = tam_str_cat(html, "<h1>Topics A to Z</h1>\n");
    }
    html = tam_str_cat(html,
        "<p>Every topic tam can explain, one line each.<br>\n"
        "Select a topic with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to read it. "
        "Press <kbd>`</kbd> for the guide index, or <kbd>q</kbd> to quit.</p>\n"
        "<table>\n"
        "<tr><th>Topic</th><th>Covers</th></tr>\n");

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (level != 0 && !(e->levels & level)) {
            continue;
        }
        if (level != 0) {
            html = tam_str_cat_printf(html, "<tr><td><a href=\"%s?level=%s\">",
                                      e->path, tam_level_name(level));
        } else {
            html = tam_str_cat_printf(html, "<tr><td><a href=\"%s\">", e->path);
        }
        html = tam_html_cat(html, e->topic);
        html = tam_str_cat(html, "</a></td><td>");
        html = tam_html_cat(html, e->summary);
        html = tam_str_cat(html, "</td></tr>\n");
    }
    html = tam_str_cat(html, "</table>\n");
    tam_index_free(idx);
    return tam_page_render(html, width);
}
