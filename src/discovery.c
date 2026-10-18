#include <tam.h>

#include "discovery.h"
#include "index.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *escape_html(const char *text)
{
    size_t n = strlen(text);
    char *out = malloc(n * 6 + 1);
    if (out == NULL) return NULL;
    size_t used = 0;
    for (size_t i = 0; i < n; i++) {
        const char *rep = NULL;
        switch (text[i]) {
        case '&': rep = "&amp;"; break;
        case '<': rep = "&lt;"; break;
        case '>': rep = "&gt;"; break;
        case '"': rep = "&quot;"; break;
        case '\'': rep = "&#39;"; break;
        default: out[used++] = text[i]; break;
        }
        if (rep != NULL) {
            size_t m = strlen(rep);
            memcpy(out + used, rep, m);
            used += m;
        }
    }
    out[used] = '\0';
    return out;
}

tam_document_t *tam_guide_search_document(const char *root,
                                          const char *query,
                                          int width)
{
    tam_str_t html = tam_str_new("<!DOCTYPE html><html><head><title>Guide Search</title></head><body>\n"
        "<nav aria-label=\"Breadcrumb\"><p><a href=\"index.html\">Guide</a> / Search</p></nav>\n"
        "<h1>Guide Search</h1>\n");
    if (query == NULL || query[0] == '\0') {
        html = tam_str_cat(html,
            "<p>Search the installed Tamlinux Guide. This searches guide topics and pages; "
            "manual lookup is available from <a href=\"tam:man\">Man pages</a>.</p>\n"
            "<p><a href=\"tam:prompt:guide-search\">Start a Guide search</a></p>\n");
    } else if (root == NULL) {
        html = tam_str_cat(html, "<p>The offline Guide is not installed, so Guide Search is unavailable.</p>\n");
    } else {
        tam_index_t *idx = tam_index_load(root);
        if (idx == NULL) {
            html = tam_str_cat(html, "<p>The Guide index could not be loaded.</p>\n");
        } else {
            tam_search_result_t results[32];
            size_t found = tam_index_search(idx, query, results, 32);
            char *escaped = escape_html(query);
            html = tam_str_cat_printf(html, "<p>Results for <strong>%s</strong>:</p>\n",
                                      escaped ? escaped : "");
            free(escaped);
            if (found == 0 || results[0].score <= 0) {
                html = tam_str_cat(html, "<p>No Guide pages matched this search.</p>\n");
            } else {
                html = tam_str_cat(html, "<table><tr><th>Guide page</th><th>Summary</th><th>Track</th></tr>\n");
                for (size_t i = 0; i < found; i++) {
                    const tam_index_entry_t *e = results[i].entry;
                    char *topic = escape_html(e->topic);
                    char *title = escape_html(e->title);
                    char *summary = escape_html(e->summary);
                    char *track = escape_html(e->track);
                    html = tam_str_cat_printf(html,
                        "<tr><td><a href=\"%s\">%s</a><br>%s</td><td>%s</td><td>%s</td></tr>\n",
                        e->path, topic ? topic : "", title ? title : "",
                        summary ? summary : "", track ? track : "");
                    free(topic); free(title); free(summary); free(track);
                }
                html = tam_str_cat(html, "</table>\n");
            }
            html = tam_str_cat(html,
                "<p><a href=\"tam:prompt:guide-search\">Search again</a> · "
                "<a href=\"tam:man\">Find a manual page</a></p>\n");
            tam_index_free(idx);
        }
    }
    html = tam_str_cat(html, "</body></html>\n");
    tam_document_t *doc = tam_render_html(html, strlen(html), width);
    tam_str_free(html);
    return doc;
}

tam_document_t *tam_guide_search_uri_document(const char *root,
                                              const char *uri,
                                              int width)
{
    const char *p = uri != NULL ? strchr(uri, '?') : NULL;
    if (p == NULL) return tam_guide_search_document(root, NULL, width);
    p++;
    while (*p != '\0') {
        const char *end = strchr(p, '&');
        size_t n = end ? (size_t)(end - p) : strlen(p);
        if (n >= 6 && strncmp(p, "query=", 6) == 0) {
            char *raw = strndup(p + 6, n - 6);
            if (raw == NULL) return NULL;
            size_t read = 0, write = 0, len = strlen(raw);
            while (read < len) {
                if (raw[read] == '%' && read + 2 < len) {
                    char hex[3] = {raw[read + 1], raw[read + 2], 0};
                    char *hex_end = NULL;
                    unsigned long value = strtoul(hex, &hex_end, 16);
                    if (hex_end == hex + 2) {
                        raw[write++] = (char)value;
                        read += 3;
                        continue;
                    }
                }
                raw[write++] = raw[read] == '+' ? ' ' : raw[read];
                read++;
            }
            raw[write] = '\0';
            tam_document_t *doc = tam_guide_search_document(root, raw, width);
            free(raw);
            return doc;
        }
        if (end == NULL) break;
        p = end + 1;
    }
    return tam_guide_search_document(root, NULL, width);
}
