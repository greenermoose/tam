#include <tam.h>

#include "address.h"

#include "depth.h"
#include "discovery.h"
#include "info.h"
#include "html.h"
#include "man.h"
#include "pager.h"
#include "render.h"
#include "tldr.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

#define TAM_COMMAND_NAME "tam"
#define TAM_EXIT_USAGE 2
#define TAM_EXIT_FAILURE 1

/* ------------------------------------------------------------------------- */
/* File locating                                                             */
/* ------------------------------------------------------------------------- */

static int file_readable(const char *path)
{
    return access(path, R_OK) == 0;
}

static int try_info_path(const char *dir, const char *base, const char *ext,
                         char *out, size_t cap)
{
    int n = snprintf(out, cap, "%s/%s%s", dir, base, ext);
    if (n > 0 && (size_t)n < cap && file_readable(out)) {
        return 1;
    }
    return 0;
}

static int locate_in_dir(const char *dir, const char *manual, char *path_out,
                         size_t path_cap)
{
    static const char *exts[] = {".info.gz", ".info", ".gz", ""};
    size_t nexts = sizeof(exts) / sizeof(exts[0]);

    /* 1. Exact match */
    for (size_t i = 0; i < nexts; i++) {
        if (try_info_path(dir, manual, exts[i], path_out, path_cap)) {
            return 1;
        }
    }

    /* 2. Case-insensitive scan in directory */
    DIR *d = opendir(dir);
    if (d == NULL) {
        return 0;
    }

    struct dirent *ent;
    size_t mlen = strlen(manual);
    int found = 0;

    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') {
            continue;
        }
        if (strncasecmp(ent->d_name, manual, mlen) == 0) {
            const char *rest = ent->d_name + mlen;
            for (size_t i = 0; i < nexts; i++) {
                if (strcasecmp(rest, exts[i]) == 0) {
                    snprintf(path_out, path_cap, "%s/%s", dir, ent->d_name);
                    found = 1;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
    closedir(d);
    return found;
}

int tam_info_locate(const char *manual, char *path_out, size_t path_cap)
{
    if (manual == NULL || manual[0] == '\0' || strcmp(manual, "dir") == 0) {
        manual = "dir";
    }

    /* 1. Check $INFOPATH if set */
    const char *infopath = getenv("INFOPATH");
    if (infopath != NULL && infopath[0] != '\0') {
        char *copy = strdup(infopath);
        if (copy != NULL) {
            char *tok = strtok(copy, ":");
            while (tok != NULL) {
                if (tok[0] != '\0' && locate_in_dir(tok, manual, path_out, path_cap)) {
                    free(copy);
                    return 1;
                }
                tok = strtok(NULL, ":");
            }
            free(copy);
        }
    }

    /* 2. Standard directories */
    static const char *std_dirs[] = {
        "/usr/share/info",
        "/usr/local/share/info",
    };
    size_t ndirs = sizeof(std_dirs) / sizeof(std_dirs[0]);

    for (size_t i = 0; i < ndirs; i++) {
        if (locate_in_dir(std_dirs[i], manual, path_out, path_cap)) {
            return 1;
        }
    }

    /* 3. User local directory */
    const char *xdg = getenv("XDG_DATA_HOME");
    const char *home = getenv("HOME");
    char udir[1024];
    if (xdg != NULL && xdg[0] != '\0') {
        snprintf(udir, sizeof(udir), "%s/info", xdg);
        if (locate_in_dir(udir, manual, path_out, path_cap)) {
            return 1;
        }
    } else if (home != NULL && home[0] != '\0') {
        snprintf(udir, sizeof(udir), "%s/.local/share/info", home);
        if (locate_in_dir(udir, manual, path_out, path_cap)) {
            return 1;
        }
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
/* Decompression and file reading                                            */
/* ------------------------------------------------------------------------- */

char *tam_read_gzip_file(const char *path, size_t *out_len)
{
    gzFile gz = gzopen(path, "rb");
    if (gz == NULL) {
        return NULL;
    }

    size_t cap = 65536;
    size_t len = 0;
    char *buf = malloc(cap);
    if (buf == NULL) {
        gzclose(gz);
        return NULL;
    }

    int n = 0;
    while ((n = gzread(gz, buf + len, (unsigned int)(cap - len - 1))) > 0) {
        len += (size_t)n;
        if (cap - len < 32768) {
            cap *= 2;
            char *next = realloc(buf, cap);
            if (next == NULL) {
                free(buf);
                gzclose(gz);
                return NULL;
            }
            buf = next;
        }
    }
    gzclose(gz);
    buf[len] = '\0';
    if (out_len != NULL) {
        *out_len = len;
    }
    return buf;
}

/* ------------------------------------------------------------------------- */
/* Tag Table and Indirect Table Parsing                                      */
/* ------------------------------------------------------------------------- */

typedef struct {
    char subfile[128];
    long offset;
} indirect_entry_t;

typedef struct {
    char name[128];
    long offset;
} tag_entry_t;

static void parse_tables(const char *buf, size_t len, int *is_indirect,
                         indirect_entry_t **ind_out, size_t *nind_out,
                         tag_entry_t **tags_out, size_t *ntags_out)
{
    *is_indirect = 0;
    *ind_out = NULL;
    *nind_out = 0;
    *tags_out = NULL;
    *ntags_out = 0;

    /* Check for Indirect table */
    const char *ind_hdr = strstr(buf, "Indirect:\n");
    if (ind_hdr == NULL) {
        ind_hdr = strstr(buf, "Indirect:\r\n");
    }
    if (ind_hdr != NULL) {
        const char *p = strchr(ind_hdr, '\n');
        if (p != NULL) {
            p++;
            size_t cap = 16;
            indirect_entry_t *ind = malloc(cap * sizeof(*ind));
            size_t nind = 0;

            while (p < buf + len && *p != '\x1f' && strncmp(p, "Tag Table:", 10) != 0) {
                while (p < buf + len && (*p == ' ' || *p == '\t')) p++;
                if (p >= buf + len || *p == '\n' || *p == '\r') {
                    if (p < buf + len && *p == '\r') p++;
                    if (p < buf + len && *p == '\n') p++;
                    continue;
                }
                const char *colon = strchr(p, ':');
                if (colon == NULL || colon >= buf + len) {
                    break;
                }
                size_t flen = (size_t)(colon - p);
                if (flen >= sizeof(ind[0].subfile)) {
                    flen = sizeof(ind[0].subfile) - 1;
                }
                char subfile[128];
                memcpy(subfile, p, flen);
                subfile[flen] = '\0';

                const char *val = colon + 1;
                while (val < buf + len && (*val == ' ' || *val == '\t')) val++;
                long off = atol(val);

                if (nind >= cap) {
                    cap *= 2;
                    ind = realloc(ind, cap * sizeof(*ind));
                }
                memcpy(ind[nind].subfile, subfile, sizeof(ind[nind].subfile));
                ind[nind].offset = off;
                nind++;

                const char *nl = strchr(val, '\n');
                if (nl == NULL) {
                    break;
                }
                p = nl + 1;
            }
            if (nind > 0) {
                *is_indirect = 1;
                *ind_out = ind;
                *nind_out = nind;
            } else {
                free(ind);
            }
        }
    }

    /* Check for Tag Table */
    const char *tag_hdr = strstr(buf, "Tag Table:\n");
    if (tag_hdr == NULL) {
        tag_hdr = strstr(buf, "Tag Table:\r\n");
    }
    if (tag_hdr != NULL) {
        const char *p = strchr(tag_hdr, '\n');
        if (p != NULL) {
            p++;
            size_t cap = 64;
            tag_entry_t *tags = malloc(cap * sizeof(*tags));
            size_t ntags = 0;

            while (p < buf + len && *p != '\x1f' && strncmp(p, "End Tag Table", 13) != 0) {
                while (p < buf + len && (*p == ' ' || *p == '\t')) p++;
                if (strncmp(p, "(Indirect)", 10) == 0) {
                    *is_indirect = 1;
                    const char *nl = strchr(p, '\n');
                    if (nl == NULL) break;
                    p = nl + 1;
                    continue;
                }
                if (strncmp(p, "Node: ", 6) == 0) {
                    const char *nstart = p + 6;
                    const char *del = nstart;
                    while (del < buf + len && *del != '\x7f' && *del != '\t' && *del != '\n' && *del != '\r') {
                        del++;
                    }
                    size_t nlen = (size_t)(del - nstart);
                    if (nlen >= sizeof(tags[0].name)) {
                        nlen = sizeof(tags[0].name) - 1;
                    }
                    char nname[128];
                    memcpy(nname, nstart, nlen);
                    nname[nlen] = '\0';

                    while (del < buf + len && (*del == '\x7f' || *del == ' ' || *del == '\t')) del++;
                    long off = atol(del);

                    if (ntags >= cap) {
                        cap *= 2;
                        tags = realloc(tags, cap * sizeof(*tags));
                    }
                    memcpy(tags[ntags].name, nname, sizeof(tags[ntags].name));
                    tags[ntags].offset = off;
                    ntags++;
                }
                const char *nl = strchr(p, '\n');
                if (nl == NULL) {
                    break;
                }
                p = nl + 1;
            }
            if (ntags > 0) {
                *tags_out = tags;
                *ntags_out = ntags;
            } else {
                free(tags);
            }
        }
    }
}

/* ------------------------------------------------------------------------- */
/* Node scanning and slicing                                                 */
/* ------------------------------------------------------------------------- */

static const char *scan_node_in_buffer(const char *buf, size_t len,
                                      const char *target_node, size_t *out_node_len)
{
    const char *p = buf;
    const char *end = buf + len;

    while (p < end) {
        const char *sep = memchr(p, '\x1f', (size_t)(end - p));
        if (sep == NULL) {
            break;
        }
        const char *line = sep + 1;
        while (line < end && (*line == '\n' || *line == '\r')) {
            line++;
        }
        if (line >= end) {
            break;
        }

        const char *eol = memchr(line, '\n', (size_t)(end - line));
        if (eol == NULL) {
            eol = end;
        }

        /* Check for Node: in this header line */
        const char *np = strstr(line, "Node:");
        if (np != NULL && np < eol) {
            np += 5;
            while (np < eol && (*np == ' ' || *np == '\t')) np++;
            const char *nend = np;
            while (nend < eol && *nend != ',' && *nend != '\r' && *nend != '\t') {
                nend++;
            }
            size_t nlen = (size_t)(nend - np);
            while (nlen > 0 && (np[nlen - 1] == ' ' || np[nlen - 1] == '\t')) {
                nlen--;
            }
            char cand[128];
            if (nlen >= sizeof(cand)) {
                nlen = sizeof(cand) - 1;
            }
            memcpy(cand, np, nlen);
            cand[nlen] = '\0';

            int match = 0;
            if (target_node == NULL || target_node[0] == '\0' || strcasecmp(target_node, "Top") == 0) {
                if (strcasecmp(cand, "Top") == 0) {
                    match = 1;
                }
            } else if (strcasecmp(cand, target_node) == 0) {
                match = 1;
            }

            if (match) {
                const char *node_start = line;
                const char *next_sep = memchr(node_start, '\x1f', (size_t)(end - node_start));
                if (next_sep == NULL) {
                    next_sep = end;
                }
                *out_node_len = (size_t)(next_sep - node_start);
                return node_start;
            }
        }

        p = eol + 1;
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */
/* HTML Entity Escaping and Link Creation                                    */
/* ------------------------------------------------------------------------- */


static char *clean_name(const char *in)
{
    while (*in == ' ' || *in == '\t') in++;
    char *out = strdup(in);
    if (out == NULL) return NULL;
    size_t len = strlen(out);
    while (len > 0 && (out[len - 1] == ' ' || out[len - 1] == '\t' ||
                       out[len - 1] == '\r' || out[len - 1] == '\n' ||
                       out[len - 1] == '.' || out[len - 1] == ',')) {
        out[--len] = '\0';
    }
    return out;
}

static char *make_info_href(const char *cur_manual, const char *raw_target)
{
    char *target = clean_name(raw_target);
    if (target == NULL || target[0] == '\0') {
        free(target);
        return NULL;
    }

    char *href = NULL;
    if (strcasecmp(target, "(dir)") == 0 || strcasecmp(target, "(dir)Top") == 0 ||
        strcasecmp(target, "dir") == 0) {
        href = strdup("info:dir");
    } else if (target[0] == '(') {
        const char *rparen = strchr(target, ')');
        if (rparen != NULL) {
            size_t mlen = (size_t)(rparen - target - 1);
            char man[128];
            if (mlen >= sizeof(man)) mlen = sizeof(man) - 1;
            memcpy(man, target + 1, mlen);
            man[mlen] = '\0';

            const char *sub = rparen + 1;
            while (*sub == ' ' || *sub == '\t') sub++;
            if (*sub == '\0' || strcmp(sub, ".") == 0 || strcasecmp(sub, "Top") == 0) {
                href = malloc(strlen(man) + 6);
                if (href != NULL) sprintf(href, "info:%s", man);
            } else {
                href = malloc(strlen(man) + strlen(sub) + 7);
                if (href != NULL) sprintf(href, "info:%s/%s", man, sub);
            }
        }
    } else if (cur_manual != NULL && cur_manual[0] != '\0') {
        href = malloc(strlen(cur_manual) + strlen(target) + 7);
        if (href != NULL) sprintf(href, "info:%s/%s", cur_manual, target);
    } else {
        href = malloc(strlen(target) + 6);
        if (href != NULL) sprintf(href, "info:%s", target);
    }

    free(target);
    return href;
}

/* ------------------------------------------------------------------------- */
/* Directory Parser (/usr/share/info/dir)                                    */
/* ------------------------------------------------------------------------- */

static char *parse_dir_to_html(const char *buf, size_t len)
{
    tam_str_t html = tam_page_begin("GNU Info Directory", NULL, NULL, 0);
    html = tam_str_cat(html,     "<h1>GNU Info Directory</h1>\n"
                                 "<p>The GNU Info directory catalog lists reference manuals "
                                 "available on this system.<br>\n"
                                 "Use Info for long-form manuals and related nodes; use <a href=\"tam:man\">Man pages</a> "
                                 "for concise command, system-call and file-format references.<br>\n"
                                 "Select an installed manual with <kbd>Tab</kbd> and press <kbd>Enter</kbd> to open it. "
                                 "Press <kbd>`</kbd> for the Guide, or <kbd>q</kbd> to exit.</p>\n");

    const char *menu = strstr(buf, "* Menu:\n");
    if (menu == NULL) {
        menu = strstr(buf, "* Menu:\r\n");
    }
    if (menu == NULL) {
        html = tam_str_cat(html, "<p>No menu found in Info directory catalog.</p>\n");
        return tam_page_end(html);
    }

    const char *p = strchr(menu, '\n');
    if (p == NULL) {
        return tam_page_end(html);
    }
    p++;

    int in_ul = 0;
    const char *end = buf + len;

    while (p < end) {
        const char *eol = memchr(p, '\n', (size_t)(end - p));
        if (eol == NULL) {
            eol = end;
        }

        /* Check for blank line */
        const char *t = p;
        while (t < eol && (*t == ' ' || *t == '\t' || *t == '\r')) t++;
        if (t == eol) {
            p = eol + 1;
            continue;
        }

        /* Check for section header */
        if (*p != '*' && *p != ' ' && *p != '\t') {
            size_t hlen = (size_t)(eol - p);
            while (hlen > 0 && (p[hlen - 1] == '\r' || p[hlen - 1] == ' ' || p[hlen - 1] == '\t')) {
                hlen--;
            }
            if (hlen > 0) {
                if (in_ul) {
                    html = tam_str_cat(html, "</ul>\n");
                    in_ul = 0;
                }
                html = tam_str_cat(html, "<h2>");
                html = tam_html_cat_len(html, p, hlen);
                html = tam_str_cat(html, "</h2>\n<ul class=\"info-menu\">\n");
                in_ul = 1;
            }
            p = eol + 1;
            continue;
        }

        /* Check for menu item: * Label: (manual)node. Description */
        if (*p == '*' && (p + 1 < eol) && p[1] == ' ') {
            const char *mstart = p + 2;
            while (mstart < eol && (*mstart == ' ' || *mstart == '\t')) mstart++;
            const char *colon = memchr(mstart, ':', (size_t)(eol - mstart));
            if (colon != NULL) {
                size_t llen = (size_t)(colon - mstart);
                char label[128];
                if (llen >= sizeof(label)) llen = sizeof(label) - 1;
                memcpy(label, mstart, llen);
                label[llen] = '\0';

                const char *tgt_start = colon + 1;
                while (tgt_start < eol && (*tgt_start == ' ' || *tgt_start == '\t')) tgt_start++;

                /* Find end of target, usually '.' */
                const char *tgt_end = tgt_start;
                while (tgt_end < eol && *tgt_end != '.') tgt_end++;
                if (tgt_end < eol && *tgt_end == '.') tgt_end++;

                size_t tlen = (size_t)(tgt_end - tgt_start);
                char raw_tgt[128];
                if (tlen >= sizeof(raw_tgt)) tlen = sizeof(raw_tgt) - 1;
                memcpy(raw_tgt, tgt_start, tlen);
                raw_tgt[tlen] = '\0';

                char *href = make_info_href(NULL, raw_tgt);

                const char *desc_start = tgt_end;
                while (desc_start < eol && (*desc_start == ' ' || *desc_start == '\t')) desc_start++;
                size_t dlen = (size_t)(eol - desc_start);
                while (dlen > 0 && (desc_start[dlen - 1] == '\r' || desc_start[dlen - 1] == ' ')) dlen--;

                if (!in_ul) {
                    html = tam_str_cat(html, "<ul class=\"info-menu\">\n");
                    in_ul = 1;
                }

                html = tam_str_cat(html, "  <li>");
                if (href != NULL) {
                    html = tam_str_cat_printf(html, "<a href=\"%s\">", href);
                    html = tam_html_cat_len(html, label, strlen(label));
                    html = tam_str_cat(html, "</a>");
                    free(href);
                } else {
                    html = tam_html_cat_len(html, label, strlen(label));
                }

                if (dlen > 0) {
                    html = tam_str_cat(html, ": ");
                    html = tam_html_cat_len(html, desc_start, dlen);
                }
                html = tam_str_cat(html, "</li>\n");
            }
        }

        p = eol + 1;
    }

    if (in_ul) {
        html = tam_str_cat(html, "</ul>\n");
    }
    return tam_page_end(html);
}

static void sanitize_link_string(char *s)
{
    size_t r = 0, w = 0;
    int in_space = 0;
    while (s[r] != '\0') {
        if (s[r] == '\n' || s[r] == '\r' || s[r] == '\t' || s[r] == ' ') {
            if (!in_space && w > 0) {
                s[w++] = ' ';
                in_space = 1;
            }
        } else {
            s[w++] = s[r];
            in_space = 0;
        }
        r++;
    }
    while (w > 0 && s[w - 1] == ' ') {
        w--;
    }
    s[w] = '\0';
}

static tam_str_t linkify_info_text(tam_str_t html, const char *cur_manual,
                                   const char *text, size_t len)
{
    const char *p = text;
    const char *end = text + len;

    while (p < end) {
        /* Check for *Note or *note */
        if ((end - p >= 6) && strncasecmp(p, "*note", 5) == 0 && (p[5] == ' ' || p[5] == '\t' || p[5] == '\n')) {
            const char *nstart = p + 6;
            while (nstart < end && (*nstart == ' ' || *nstart == '\t' || *nstart == '\n' || *nstart == '\r')) {
                nstart++;
            }
            /* Look for :: or : */
            const char *col = nstart;
            while (col < end && (col - nstart) < 120 && *col != ':') {
                if (*col == '\n') {
                    const char *next = col + 1;
                    if (next < end && (*next == '\n' || (*next == '\r' && next + 1 < end && next[1] == '\n'))) {
                        break;
                    }
                }
                col++;
            }
            if (col < end && *col == ':') {
                if (col + 1 < end && col[1] == ':') {
                    /* Format: *Note Target:: */
                    size_t tlen = (size_t)(col - nstart);
                    char target[128];
                    if (tlen >= sizeof(target)) tlen = sizeof(target) - 1;
                    memcpy(target, nstart, tlen);
                    target[tlen] = '\0';
                    sanitize_link_string(target);

                    char *href = make_info_href(cur_manual, target);
                    if (href != NULL) {
                        html = tam_str_cat_printf(html, "<a href=\"%s\">", href);
                        html = tam_html_cat_len(html, target, strlen(target));
                        html = tam_str_cat(html, "</a>");
                        free(href);
                        p = col + 2;
                        continue;
                    }
                } else {
                    /* Format: *Note Label: Target. */
                    size_t llen = (size_t)(col - nstart);
                    char label[128];
                    if (llen >= sizeof(label)) llen = sizeof(label) - 1;
                    memcpy(label, nstart, llen);
                    label[llen] = '\0';
                    sanitize_link_string(label);

                    const char *tstart = col + 1;
                    while (tstart < end && (*tstart == ' ' || *tstart == '\t' || *tstart == '\n' || *tstart == '\r')) tstart++;
                    const char *tend = tstart;
                    while (tend < end && (tend - tstart) < 120 && *tend != '.' && *tend != ',' && *tend != '\t') {
                        if (*tend == '\n') {
                            const char *next = tend + 1;
                            if (next < end && (*next == '\n' || (*next == '\r' && next + 1 < end && next[1] == '\n'))) {
                                break;
                            }
                        }
                        tend++;
                    }
                    size_t trlen = (size_t)(tend - tstart);
                    char target[128];
                    if (trlen >= sizeof(target)) trlen = sizeof(target) - 1;
                    memcpy(target, tstart, trlen);
                    target[trlen] = '\0';
                    sanitize_link_string(target);

                    char *href = make_info_href(cur_manual, target);
                    if (href != NULL) {
                        html = tam_str_cat_printf(html, "<a href=\"%s\">", href);
                        html = tam_html_cat_len(html, label, strlen(label));
                        html = tam_str_cat(html, "</a>");
                        free(href);
                        if (tend < end && (*tend == '.' || *tend == ',')) {
                            tend++;
                        }
                        p = tend;
                        continue;
                    }
                }
            }
        }

        /* Check for man page reference: NAME(SEC) */
        if (isalnum((unsigned char)*p)) {
            const char *wstart = p;
            while (p < end && (isalnum((unsigned char)*p) || *p == '_' || *p == '-' || *p == '.')) {
                p++;
            }
            if (p < end && *p == '(' && p + 2 < end && isdigit((unsigned char)p[1]) && p[2] == ')') {
                size_t nlen = (size_t)(p - wstart);
                char mname[64];
                if (nlen >= sizeof(mname)) nlen = sizeof(mname) - 1;
                memcpy(mname, wstart, nlen);
                mname[nlen] = '\0';
                char sec = p[1];

                html = tam_str_cat_printf(html, "<a href=\"man:%s(%c)\">%s(%c)</a>",
                                          mname, sec, mname, sec);
                p += 3;
                continue;
            }
            html = tam_html_cat_len(html, wstart, (size_t)(p - wstart));
            continue;
        }

        html = tam_html_cat_len(html, p, 1);
        p++;
    }

    return html;
}

static char *parse_node_to_html(const char *manual_name, const char *node_slice,
                                size_t slice_len)
{
    const char *end = node_slice + slice_len;
    const char *eol = memchr(node_slice, '\n', slice_len);
    if (eol == NULL) {
        eol = end;
    }

    /* Extract header fields */
    char file_field[128] = "";
    char node_field[128] = "";
    char next_field[128] = "";
    char prev_field[128] = "";
    char up_field[128] = "";

    const char *f = strstr(node_slice, "File:");
    if (f != NULL && f < eol) {
        f += 5;
        while (f < eol && (*f == ' ' || *f == '\t')) f++;
        const char *c = memchr(f, ',', (size_t)(eol - f));
        size_t l = c ? (size_t)(c - f) : (size_t)(eol - f);
        if (l >= sizeof(file_field)) l = sizeof(file_field) - 1;
        memcpy(file_field, f, l);
        file_field[l] = '\0';
    }

    const char *n = strstr(node_slice, "Node:");
    if (n != NULL && n < eol) {
        n += 5;
        while (n < eol && (*n == ' ' || *n == '\t')) n++;
        const char *c = memchr(n, ',', (size_t)(eol - n));
        size_t l = c ? (size_t)(c - n) : (size_t)(eol - n);
        if (l >= sizeof(node_field)) l = sizeof(node_field) - 1;
        memcpy(node_field, n, l);
        node_field[l] = '\0';
    }

    const char *nx = strstr(node_slice, "Next:");
    if (nx != NULL && nx < eol) {
        nx += 5;
        while (nx < eol && (*nx == ' ' || *nx == '\t')) nx++;
        const char *c = memchr(nx, ',', (size_t)(eol - nx));
        size_t l = c ? (size_t)(c - nx) : (size_t)(eol - nx);
        if (l >= sizeof(next_field)) l = sizeof(next_field) - 1;
        memcpy(next_field, nx, l);
        next_field[l] = '\0';
    }

    const char *pr = strstr(node_slice, "Prev:");
    if (pr != NULL && pr < eol) {
        pr += 5;
        while (pr < eol && (*pr == ' ' || *pr == '\t')) pr++;
        const char *c = memchr(pr, ',', (size_t)(eol - pr));
        size_t l = c ? (size_t)(c - pr) : (size_t)(eol - pr);
        if (l >= sizeof(prev_field)) l = sizeof(prev_field) - 1;
        memcpy(prev_field, pr, l);
        prev_field[l] = '\0';
    }

    const char *up = strstr(node_slice, "Up:");
    if (up != NULL && up < eol) {
        up += 3;
        while (up < eol && (*up == ' ' || *up == '\t')) up++;
        const char *c = memchr(up, ',', (size_t)(eol - up));
        size_t l = c ? (size_t)(c - up) : (size_t)(eol - up);
        if (l >= sizeof(up_field)) l = sizeof(up_field) - 1;
        memcpy(up_field, up, l);
        up_field[l] = '\0';
    }

    char *next_href = next_field[0] ? make_info_href(manual_name, next_field) : NULL;
    char *prev_href = prev_field[0] ? make_info_href(manual_name, prev_field) : NULL;
    char *up_href = up_field[0] ? make_info_href(manual_name, up_field) : NULL;
    char *top_href = make_info_href(manual_name, "Top");

    const char *node_name = node_field[0] ? node_field : "Top";
    tam_str_t head = tam_str_new("");
    static const char *const rels[] = { "top", "next", "prev", "up" };
    const char *rel_hrefs[] = { top_href, next_href, prev_href, up_href };
    for (size_t i = 0; i < 4; i++) {
        if (rel_hrefs[i] != NULL)
            head = tam_str_cat_printf(head, "<link rel=\"%s\" href=\"%s\">\n",
                                      rels[i], rel_hrefs[i]);
    }
    free(top_href);
    tam_str_t title = tam_str_cat(tam_str_new(manual_name), ": ");
    title = tam_str_cat(title, node_name);
    tam_str_t html = tam_page_begin(title, head, NULL, 0);
    tam_str_free(title);
    tam_str_free(head);

    /* Render top navigation bar */
    html = tam_str_cat(html, "<nav class=\"info-nav\">\n  <span><b>File:</b> ");
    html = tam_html_cat(html, manual_name);
    html = tam_str_cat(html, "</span> &nbsp;|&nbsp; <span><b>Node:</b> ");
    html = tam_html_cat(html, node_name);
    html = tam_str_cat(html, "</span>");
    static const char *const nav_labels[] = { "Up", "Next", "Prev" };
    const char *nav_hrefs[] = { up_href, next_href, prev_href };
    const char *nav_names[] = { up_field, next_field, prev_field };
    for (size_t i = 0; i < 3; i++) {
        if (nav_hrefs[i] == NULL) continue;
        html = tam_str_cat_printf(html, " &nbsp;|&nbsp; <span><b>%s:</b> <a href=\"%s\">",
                                  nav_labels[i], nav_hrefs[i]);
        html = tam_html_cat(html, nav_names[i]);
        html = tam_str_cat(html, "</a></span>");
    }
    html = tam_str_cat(html, "\n</nav>\n<hr>\n");

    free(next_href);
    free(prev_href);
    free(up_href);

    /* Body parsing */
    const char *p = (eol < end) ? eol + 1 : end;
    int in_menu = 0;
    int in_pre = 0;

    while (p < end) {
        const char *cur_eol = memchr(p, '\n', (size_t)(end - p));
        if (cur_eol == NULL) cur_eol = end;

        /* Check for * Menu: */
        if (!in_menu && strncmp(p, "* Menu:", 7) == 0) {
            in_menu = 1;
            if (in_pre) {
                html = tam_str_cat(html, "</code></pre>\n");
                in_pre = 0;
            }
            html = tam_str_cat(html, "<h3>Menu</h3>\n<ul class=\"info-menu\">\n");
            p = cur_eol + 1;
            continue;
        }

        if (in_menu) {
            /* Check if menu ended */
            if (*p == '*' && (p + 1 < cur_eol) && p[1] == ' ') {
                /* Menu item */
                const char *mstart = p + 2;
                while (mstart < cur_eol && (*mstart == ' ' || *mstart == '\t')) mstart++;
                const char *col = memchr(mstart, ':', (size_t)(cur_eol - mstart));
                if (col != NULL) {
                    size_t llen = (size_t)(col - mstart);
                    char label[128];
                    if (llen >= sizeof(label)) llen = sizeof(label) - 1;
                    memcpy(label, mstart, llen);
                    label[llen] = '\0';

                    char *href = NULL;
                    const char *desc = "";
                    if (col + 1 < cur_eol && col[1] == ':') {
                        /* * Label:: Description */
                        href = make_info_href(manual_name, label);
                        desc = col + 2;
                    } else {
                        /* * Label: Target. Description */
                        const char *tstart = col + 1;
                        while (tstart < cur_eol && (*tstart == ' ' || *tstart == '\t')) tstart++;
                        const char *tend = tstart;
                        while (tend < cur_eol && *tend != '.') tend++;
                        if (tend < cur_eol && *tend == '.') tend++;
                        size_t tlen = (size_t)(tend - tstart);
                        char target[128];
                        if (tlen >= sizeof(target)) tlen = sizeof(target) - 1;
                        memcpy(target, tstart, tlen);
                        target[tlen] = '\0';

                        href = make_info_href(manual_name, target);
                        desc = tend;
                    }
                    while (*desc == ' ' || *desc == '\t') desc++;
                    size_t dlen = (size_t)(cur_eol - desc);
                    while (dlen > 0 && (desc[dlen - 1] == '\r' || desc[dlen - 1] == ' ')) dlen--;

                    html = tam_str_cat(html, "  <li>");
                    if (href != NULL) {
                        html = tam_str_cat_printf(html, "<a href=\"%s\">", href);
                        html = tam_html_cat_len(html, label, strlen(label));
                        html = tam_str_cat(html, "</a>");
                        free(href);
                    } else {
                        html = tam_html_cat_len(html, label, strlen(label));
                    }
                    if (dlen > 0) {
                        html = tam_str_cat(html, ": ");
                        html = linkify_info_text(html, manual_name, desc, dlen);
                    }
                    html = tam_str_cat(html, "</li>\n");
                }
            } else {
                /* Non-menu line */
                const char *chk = p;
                while (chk < cur_eol && (*chk == ' ' || *chk == '\t' || *chk == '\r')) chk++;
                if (chk != cur_eol && *p != ' ' && *p != '\t') {
                    html = tam_str_cat(html, "</ul>\n");
                    in_menu = 0;
                    continue;
                }
            }
            p = cur_eol + 1;
            continue;
        }

        /* Check for blank line */
        const char *chk = p;
        while (chk < cur_eol && (*chk == ' ' || *chk == '\t' || *chk == '\r')) chk++;
        if (chk == cur_eol) {
            if (in_pre) {
                html = tam_str_cat(html, "</code></pre>\n");
                in_pre = 0;
            }
            p = cur_eol + 1;
            continue;
        }

        /* Check for underlined heading: line followed by underline line */
        const char *next_line = cur_eol + 1;
        if (next_line < end) {
            const char *next_eol = memchr(next_line, '\n', (size_t)(end - next_line));
            if (next_eol == NULL) next_eol = end;
            size_t ulen = (size_t)(next_eol - next_line);
            while (ulen > 0 && (next_line[ulen - 1] == '\r' || next_line[ulen - 1] == ' ')) ulen--;

            if (ulen >= 3) {
                char uc = next_line[0];
                if (uc == '*' || uc == '=' || uc == '-' || uc == '~') {
                    int all_same = 1;
                    for (size_t i = 1; i < ulen; i++) {
                        if (next_line[i] != uc) {
                            all_same = 0;
                            break;
                        }
                    }
                    if (all_same) {
                        size_t hlen = (size_t)(cur_eol - p);
                        while (hlen > 0 && (p[hlen - 1] == '\r' || p[hlen - 1] == ' ')) hlen--;
                        if (in_pre) {
                            html = tam_str_cat(html, "</code></pre>\n");
                            in_pre = 0;
                        }
                        const char *htag = (uc == '*' || uc == '=') ? "h1" : ((uc == '-') ? "h2" : "h3");
                        html = tam_str_cat_printf(html, "<%s>", htag);
                        html = tam_html_cat_len(html, p, hlen);
                        html = tam_str_cat_printf(html, "</%s>\n", htag);
                        p = next_eol + 1;
                        continue;
                    }
                }
            }
        }

        /* Check if indented block (code / synopsis / table) */
        int indent = 0;
        while (p + indent < cur_eol && p[indent] == ' ') indent++;
        if (indent >= 4) {
            if (!in_pre) {
                html = tam_str_cat(html, "<pre><code>");
                in_pre = 1;
            }
            size_t llen = (size_t)(cur_eol - p);
            html = linkify_info_text(html, manual_name, p, llen);
            html = tam_str_cat(html, "\n");
            p = cur_eol + 1;
            continue;
        }

        /* Normal paragraph line */
        if (in_pre) {
            html = tam_str_cat(html, "</code></pre>\n");
            in_pre = 0;
        }

        /* Collect full paragraph until blank line */
        const char *para_start = p;
        const char *para_end = cur_eol;
        while (para_end < end) {
            const char *nline = para_end + 1;
            if (nline >= end) break;
            const char *neol = memchr(nline, '\n', (size_t)(end - nline));
            if (neol == NULL) neol = end;
            const char *cchk = nline;
            while (cchk < neol && (*cchk == ' ' || *cchk == '\t' || *cchk == '\r')) cchk++;
            if (cchk == neol) break; /* Blank line */
            if (strncmp(nline, "* Menu:", 7) == 0) break;
            if (nline + 4 < neol && nline[0] == ' ' && nline[1] == ' ' && nline[2] == ' ' && nline[3] == ' ') break;
            para_end = neol;
        }

        html = tam_str_cat(html, "<p>");
        html = linkify_info_text(html, manual_name, para_start, (size_t)(para_end - para_start));
        html = tam_str_cat(html, "</p>\n");

        p = para_end + 1;
    }

    if (in_menu) {
        html = tam_str_cat(html, "</ul>\n");
    }
    if (in_pre) {
        html = tam_str_cat(html, "</code></pre>\n");
    }

    return tam_page_end(html);
}

/* ------------------------------------------------------------------------- */
/* Public API: tam_info_load                                                 */
/* ------------------------------------------------------------------------- */

tam_document_t *tam_info_load(const char *spec, int width)
{
    char manual_buf[128] = "dir";
    char node_buf[128] = "Top";

    if (spec != NULL && spec[0] != '\0') {
        const char *s = spec;
        if (strncmp(s, "info:", 5) == 0) {
            s += 5;
        }
        if (s[0] == '\0' || strcmp(s, "dir") == 0) {
            strcpy(manual_buf, "dir");
            strcpy(node_buf, "Top");
        } else {
            const char *slash = strchr(s, '/');
            const char *lparen = strchr(s, '(');
            if (slash != NULL) {
                size_t mlen = (size_t)(slash - s);
                if (mlen >= sizeof(manual_buf)) mlen = sizeof(manual_buf) - 1;
                memcpy(manual_buf, s, mlen);
                manual_buf[mlen] = '\0';
                snprintf(node_buf, sizeof(node_buf), "%s", slash + 1);
            } else if (lparen != NULL) {
                size_t mlen = (size_t)(lparen - s);
                if (mlen >= sizeof(manual_buf)) mlen = sizeof(manual_buf) - 1;
                memcpy(manual_buf, s, mlen);
                manual_buf[mlen] = '\0';
                const char *rparen = strchr(lparen, ')');
                if (rparen != NULL) {
                    size_t nlen = (size_t)(rparen - lparen - 1);
                    if (nlen >= sizeof(node_buf)) nlen = sizeof(node_buf) - 1;
                    memcpy(node_buf, lparen + 1, nlen);
                    node_buf[nlen] = '\0';
                }
            } else {
                snprintf(manual_buf, sizeof(manual_buf), "%s", s);
                strcpy(node_buf, "Top");
            }
        }
    }

    char file_path[1024];
    if (!tam_info_locate(manual_buf, file_path, sizeof(file_path))) {
        return NULL;
    }

    size_t file_len = 0;
    char *buf = tam_read_gzip_file(file_path, &file_len);
    if (buf == NULL) {
        return NULL;
    }

    /* Check if this is the directory file */
    if (strcmp(manual_buf, "dir") == 0) {
        char *html = parse_dir_to_html(buf, file_len);
        free(buf);
        if (html == NULL) return NULL;
        tam_document_t *doc = tam_render_html(html, strlen(html), width);
        tam_str_free(html);
        return doc;
    }

    /* Check for tag table and indirect table */
    int is_indirect = 0;
    indirect_entry_t *ind_table = NULL;
    size_t nind = 0;
    tag_entry_t *tag_table = NULL;
    size_t ntags = 0;

    parse_tables(buf, file_len, &is_indirect, &ind_table, &nind, &tag_table, &ntags);

    char *active_buf = buf;
    size_t active_len = file_len;

    if (is_indirect && ind_table != NULL && tag_table != NULL) {
        long target_off = -1;
        for (size_t i = 0; i < ntags; i++) {
            if (strcasecmp(tag_table[i].name, node_buf) == 0) {
                target_off = tag_table[i].offset;
                break;
            }
        }
        if (target_off >= 0) {
            /* Find matching subfile */
            const char *subfile = ind_table[0].subfile;
            for (size_t i = 0; i < nind; i++) {
                if (target_off >= ind_table[i].offset) {
                    subfile = ind_table[i].subfile;
                } else {
                    break;
                }
            }
            /* Locate directory of file_path */
            char dir[1024];
            snprintf(dir, sizeof(dir), "%s", file_path);
            char *slash = strrchr(dir, '/');
            if (slash != NULL) *slash = '\0';
            else strcpy(dir, ".");

            char subpath[1024];
            if (locate_in_dir(dir, subfile, subpath, sizeof(subpath))) {
                size_t sub_len = 0;
                char *sub_buf = tam_read_gzip_file(subpath, &sub_len);
                if (sub_buf != NULL) {
                    free(buf);
                    active_buf = sub_buf;
                    active_len = sub_len;
                }
            }
        }
    }

    free(ind_table);
    free(tag_table);

    size_t node_len = 0;
    const char *node_slice = scan_node_in_buffer(active_buf, active_len, node_buf, &node_len);
    if (node_slice == NULL) {
        free(active_buf);
        return NULL;
    }

    char *html = parse_node_to_html(manual_buf, node_slice, node_len);
    free(active_buf);
    if (html == NULL) {
        return NULL;
    }

    tam_document_t *doc = tam_render_html(html, strlen(html), width);
    tam_str_free(html);
    return doc;
}

/* ------------------------------------------------------------------------- */
/* CLI Command Execution: tam info                                            */
/* ------------------------------------------------------------------------- */

static void print_info_help(void)
{
    fputs(
        "tam info - browse GNU Info manuals with hyperlinked cross-references\n\n"
        "Usage:\n"
        "  tam info                  open top-level Info directory catalog\n"
        "  tam info MANUAL           open the Top node of a specific manual\n"
        "  tam info MANUAL NODE      open a specific node within a manual\n"
        "  tam info [ARGS] --pipe    output formatted plain text\n"
        "  tam info --help           show this help and exit\n\n"
        "Examples:\n"
        "  tam info                  browse the system Info directory\n"
        "  tam info coreutils        open the GNU Coreutils manual\n"
        "  tam info coreutils \"head invocation\"\n"
        "                            jump to head invocation documentation\n"
        "  tam info bash \"Shell Syntax\"\n"
        "                            jump to Bash shell syntax node\n"
        "  tam info libc             open the GNU C Library Reference Manual\n"
        "  tam info coreutils --pipe pipe manual text without pager\n\n"
        "Navigation keys in pager:\n"
        "  d/k, s/l                  move the attention line down / up\n"
        "  a/j f/;, ] / [ (or Tab / Shift-Tab)  select hyperlinks on the line / anywhere\n"
        "  g, Enter / Right Arrow    follow selected link\n"
        "  n / p                     navigate to next / previous node\n"
        "  u                         navigate to parent node (or back in history)\n"
        "  t                         jump to Top node of current manual\n"
        "  i                         jump to root Info directory catalog\n"
        "  `                         return to Tamlinux Guide index\n"
        "  /                         search text within the current page\n"
        "  q / Esc                   quit the pager\n",
        stdout);
}

int tam_cmd_info(int n, char **args)
{
    int pipe = 0;
    const char *manual = NULL;
    const char *node = NULL;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            pipe = 1;
        } else if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0 ||
                   strcmp(args[i], "help") == 0) {
            print_info_help();
            return 0;
        } else if (args[i][0] == '-') {
            fprintf(stderr, TAM_COMMAND_NAME ": unrecognized argument '%s'\n", args[i]);
            fprintf(stderr, "Run '%s info --help' for available options.\n", TAM_COMMAND_NAME);
            return TAM_EXIT_USAGE;
        } else if (manual == NULL) {
            manual = args[i];
        } else if (node == NULL) {
            node = args[i];
        } else {
            fprintf(stderr, TAM_COMMAND_NAME ": too many arguments\n");
            fprintf(stderr, "Run '%s info --help' for available options.\n", TAM_COMMAND_NAME);
            return TAM_EXIT_USAGE;
        }
    }

    char spec[256];
    if (manual == NULL || manual[0] == '\0') {
        strcpy(spec, "dir");
    } else if (node != NULL && node[0] != '\0') {
        snprintf(spec, sizeof(spec), "%s/%s", manual, node);
    } else {
        snprintf(spec, sizeof(spec), "%s", manual);
    }

    int interactive = !pipe && isatty(STDOUT_FILENO);
    int width = interactive ? tam_terminal_width() : 80;

    tam_document_t *doc = tam_info_load(spec, width);
    if (doc == NULL) {
        if (manual != NULL && node != NULL) {
            fprintf(stderr, TAM_COMMAND_NAME ": no node '%s' found in info manual '%s'\n",
                    node, manual);
        } else if (manual != NULL) {
            fprintf(stderr, TAM_COMMAND_NAME ": no info manual found for '%s'\n", manual);
        } else {
            fprintf(stderr, TAM_COMMAND_NAME ": could not open info directory catalog\n");
        }
        return TAM_EXIT_FAILURE;
    }

    char initial_href[300];
    snprintf(initial_href, sizeof(initial_href), "info:%s", spec);
    tam_document_ensure_location(doc, initial_href);

    int rc = 0;
    if (interactive) {
        char *root = tam_find_knowledge_root();
        tam_loader_ctx_t ctx = {.root = root};
        rc = tam_pager_run(doc, initial_href, tam_address_load, &ctx);
        free(root);
    } else {
        char *text = tam_document_text(doc);
        if (text != NULL) {
            fputs(text, stdout);
            free(text);
        }
        tam_document_free(doc);
    }

    return rc;
}
