#define _GNU_SOURCE
#include "documentation.h"
#include "html_scan.h"
#include "tldr.h"
#include "archwiki.h"
#include <tam.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef TAM_DOCDIR
#define TAM_DOCDIR "/usr/share/doc"
#endif

char *tam_documentation_root(const char *source)
{
    const char *leaf = !strcmp(source, "info") ? "info" :
                       !strcmp(source, "tldr") ? "tldr" :
                       !strcmp(source, "archwiki") ? "arch-wiki/html/en" : NULL;
    if (!leaf) return NULL;
    const char *base = getenv("TAM_DOC_DIR");
    return tam_path_join(base && base[0] ? base : TAM_DOCDIR, leaf);
}

static bool readable_entry(const char *directory, const char *suffix)
{
    DIR *dir = directory ? opendir(directory) : NULL;
    if (!dir) return false;
    struct dirent *entry;
    bool found = false;
    while ((entry = readdir(dir))) {
        size_t n = strlen(entry->d_name), s = strlen(suffix);
        if (n <= s || strcmp(entry->d_name + n - s, suffix)) continue;
        struct stat st;
        if (!fstatat(dirfd(dir), entry->d_name, &st, 0) && S_ISREG(st.st_mode) && st.st_size > 0 &&
            !faccessat(dirfd(dir), entry->d_name, R_OK, 0)) { found = true; break; }
    }
    closedir(dir);
    return found;
}

static bool info_here(const char *path)
{
    const char *extensions[] = {".info", ".info.gz", ".info.bz2", ".info.xz", ".info.zst"};
    for (size_t i = 0; i < sizeof extensions / sizeof *extensions; i++)
        if (readable_entry(path, extensions[i])) return true;
    return false;
}

bool tam_documentation_available(const char *source)
{
    char *root = NULL;
    bool found = false;
    if (!strcmp(source, "info")) {
        const char *paths = getenv("INFOPATH");
        char *copy = paths ? strdup(paths) : NULL;
        for (char *save = NULL, *p = copy ? strtok_r(copy, ":", &save) : NULL; p; p = strtok_r(NULL, ":", &save))
            if (info_here(p)) { found = true; break; }
        free(copy);
        root = tam_documentation_root(source);
        found = found || info_here(root);
        if (!found) {
            const char *xdg = getenv("XDG_DATA_HOME"), *home = getenv("HOME");
            char *local = xdg && *xdg ? tam_path_join(xdg, "info") :
                          home && *home ? tam_path_join(home, ".local/share/info") : NULL;
            found = info_here(local);
            free(local);
        }
    } else if (!strcmp(source, "tldr")) {
        root = tam_find_tldr_root(NULL);
        char *common = root ? tam_path_join(root, "pages/common") : NULL;
        char *linux = root ? tam_path_join(root, "pages/linux") : NULL;
        found = readable_entry(common, ".md") || readable_entry(linux, ".md");
        free(common); free(linux);
    } else if (!strcmp(source, "archwiki")) {
        root = tam_find_archwiki_root();
        found = readable_entry(root, ".html");
    }
    free(root);
    return found;
}

char *tam_documentation_markup(const char *html, size_t length)
{
    /* One snapshot per page; no filesystem operations for ordinary markup. */
    if (!strstr(html, "data-doc-")) return strndup(html, length);
    const char *ids[] = {"info", "tldr", "archwiki"};
    bool available[3];
    for (size_t i = 0; i < 3; i++) available[i] = tam_documentation_available(ids[i]);
    tam_str_t out = tam_str_new("");
    bool unlinked = false;
    unsigned hidden_span = 0;
    for (size_t i = 0; i < length;) {
        tam_tag_t tag;
        if (html[i] != '<' || !tam_tag_read(html, length, i, &tag)) {
            if (!hidden_span) out = tam_str_cat_len(out, html + i, 1);
            i++;
            continue;
        }
        bool emit = !hidden_span;
        if (hidden_span) {
            if (tam_tag_is(&tag, "span", 0)) hidden_span++;
            if (tam_tag_is(&tag, "span", 1)) hidden_span--;
        } else {
            char source[32];
            const char *attribute = tam_tag_is(&tag, "a", 0) ? "data-doc-source" :
                                    tam_tag_is(&tag, "span", 0) ? "data-doc-missing" : NULL;
            if (attribute && tam_tag_attr(html, &tag, attribute, source, sizeof source)) {
                bool present = false;
                for (size_t j = 0; j < 3; j++) if (!strcmp(source, ids[j])) present = available[j];
                if (tam_tag_is(&tag, "a", 0) && !present) { unlinked = true; emit = false; }
                if (tam_tag_is(&tag, "span", 0) && present) { hidden_span = 1; emit = false; }
            } else if (tam_tag_is(&tag, "a", 1) && unlinked) { unlinked = false; emit = false; }
        }
        if (emit) out = tam_str_cat_len(out, html + i, tag.end - i);
        i = tag.end;
    }
    char *result = out ? strdup(out) : NULL;
    tam_str_free(out);
    return result;
}
