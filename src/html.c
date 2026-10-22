#include <tam.h>

#include "html.h"

#include <stdlib.h>
#include <string.h>

static const char *html_entity(char c)
{
    switch (c) {
    case '&':  return "&amp;";
    case '<':  return "&lt;";
    case '>':  return "&gt;";
    case '"':  return "&quot;";
    case '\'': return "&#39;";
    default:   return NULL;
    }
}

tam_str_t tam_html_cat_len(tam_str_t s, const char *text, size_t len)
{
    size_t run = 0;
    for (size_t i = 0; s != NULL && text != NULL && i < len; i++) {
        const char *entity = html_entity(text[i]);
        if (entity == NULL) {
            run++;
            continue;
        }
        s = tam_str_cat_len(s, text + i - run, run);
        run = 0;
        if (s != NULL) s = tam_str_cat(s, entity);
    }
    if (s != NULL && run > 0) s = tam_str_cat_len(s, text + len - run, run);
    return s;
}

tam_str_t tam_html_cat(tam_str_t s, const char *text)
{
    return text != NULL ? tam_html_cat_len(s, text, strlen(text)) : s;
}

char *tam_html_escape(const char *text)
{
    tam_str_t s = tam_html_cat(tam_str_new(""), text);
    if (s == NULL) return NULL;
    char *out = strdup(s);
    tam_str_free(s);
    return out;
}

tam_str_t tam_page_begin(const char *title, const char *head_extra,
                         const tam_crumb_t *crumbs, size_t ncrumbs)
{
    tam_str_t s = tam_str_new("<!DOCTYPE html>\n<html>\n<head>\n<title>");
    s = tam_html_cat(s, title);
    s = tam_str_cat(s, "</title>\n");
    if (head_extra != NULL) s = tam_str_cat(s, head_extra);
    s = tam_str_cat(s, "</head>\n<body>\n");
    if (ncrumbs == 0) return s;
    s = tam_str_cat(s, "<nav aria-label=\"Breadcrumb\"><p>");
    for (size_t i = 0; i < ncrumbs; i++) {
        if (i > 0) s = tam_str_cat(s, " / ");
        if (crumbs[i].href != NULL) {
            s = tam_str_cat_printf(s, "<a href=\"%s\">", crumbs[i].href);
            s = tam_html_cat(s, crumbs[i].label);
            s = tam_str_cat(s, "</a>");
        } else {
            s = tam_html_cat(s, crumbs[i].label);
        }
    }
    return tam_str_cat(s, "</p></nav>\n");
}

tam_str_t tam_page_end(tam_str_t html)
{
    return tam_str_cat(html, "</body>\n</html>\n");
}

tam_document_t *tam_page_render(tam_str_t html, int width)
{
    html = tam_page_end(html);
    if (html == NULL) return NULL;
    tam_document_t *doc = tam_render_html(html, tam_str_len(html), width);
    tam_str_free(html);
    return doc;
}
