#include <tam.h>

#include "address.h"
#include "discovery.h"
#include "html.h"
#include "index.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tam_document_t *tam_guide_search_document(const char *root,
                                          const char *query,
                                          int width)
{
    static const tam_crumb_t crumbs[] = { { "index.html", "Guide" }, { NULL, "Search" } };
    tam_index_t *idx = root ? tam_index_load(root) : NULL;
    tam_str_t html = tam_page_begin("Guide Search", NULL, crumbs, 2);
    html = tam_str_cat(html, "<h1>Guide Search</h1>\n");
    if (query == NULL || query[0] == '\0') {
        html = tam_str_cat(html,
            "<p>Search the installed Tamlinux Guide. This searches guide topics and pages; "
            "manual lookup is available from <a href=\"tam:man\">Man pages</a>.</p>\n"
            "<p><a href=\"tam:prompt:guide-search\">Search for a word, subject, or task</a></p>\n");
    } else if (root == NULL) {
        html = tam_str_cat(html, "<p>The offline Guide is not installed, so Guide Search is unavailable.</p>\n");
    } else {
        if (idx == NULL) {
            html = tam_str_cat(html, "<p>The Guide index could not be loaded.</p>\n");
        } else {
            tam_search_result_t *results = calloc(idx->count ? idx->count : 1, sizeof(*results));
            size_t found = results ? tam_index_search(idx, query, results, idx->count) : 0;
            html = tam_str_cat(html, "<p>Results for <strong>");
            html = tam_html_cat(html, query);
            html = tam_str_cat(html, "</strong>:</p>\n");
            if (found == 0 || results[0].score <= 0) {
                html = tam_str_cat(html, "<p>No Guide pages matched this search.</p>\n");
            } else {
                const char *purposes[6];
                size_t num_purposes = 0;
                for (size_t first = 0; first < found; first++) {
                    const char *purpose = tam_index_purpose(results[first].entry);
                    size_t g = 0;
                    while (g < num_purposes && strcmp(purposes[g], purpose) != 0) g++;
                    if (g < num_purposes) continue;
                    if (num_purposes == sizeof(purposes) / sizeof(purposes[0])) break;
                    purposes[num_purposes++] = purpose;

                    size_t shown = 0;
                    for (size_t i = first; i < found; i++) {
                        const tam_index_entry_t *e = results[i].entry;
                        if (strcmp(tam_index_purpose(e), purpose) != 0) continue;
                        if (shown == 0) {
                            html = tam_str_cat_printf(html, "<h2>%s</h2><dl>", purpose);
                        }
                        if (shown == 5) html = tam_str_cat(html, "</dl><details><summary>More matches</summary><dl>");
                        html = tam_str_cat_printf(html, "<dt><a href=\"%s\">", e->path);
                        html = tam_html_cat(html, e->title);
                        html = tam_str_cat(html, "</a></dt><dd>");
                        html = tam_html_cat(html, e->summary);
                        html = tam_str_cat(html, "</dd>");
                        shown++;
                    }
                    if (shown) html = tam_str_cat(html, "</dl>");
                    if (shown > 5) html = tam_str_cat(html, "</details>");
                }
            }
            free(results);
            html = tam_str_cat(html,
                "<p><a href=\"tam:prompt:guide-search\">Search again</a> · "
                "<a href=\"tam:man\">Find a manual page</a></p>\n");
        }
    }
    if (idx != NULL) {
        bool ambiguous;
        const tam_index_entry_t *glossary = tam_index_lookup_exact(idx, "glossary", &ambiguous);
        if (glossary) html = tam_str_cat_printf(html, "<p><a href=\"%s\">Glossary A–Z</a></p>", glossary->path);
        tam_index_free(idx);
    }
    html = tam_str_cat(html, "<p><a href=\"tam:hubs\">Browse hubs</a> · <a href=\"tam:catalog\">All pages A–Z</a></p>");
    return tam_page_render(html, width);
}

tam_document_t *tam_guide_search_uri_document(const char *root,
                                              const char *uri,
                                              int width)
{
    char query[4096];
    if (tam_addr_param(uri, "query", query, sizeof(query)))
        return tam_guide_search_document(root, query, width);
    return tam_guide_search_document(root, NULL, width);
}
