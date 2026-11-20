#include <tam.h>

#include "depth.h"
#include "index.h"
#include "man.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static const char *STOP_WORDS[] = {
    "a", "an", "the", "and", "or", "of", "to", "in", "on", "for",
    "how", "do", "does", "you", "what", "why", "i", "me", "my",
    "is", "are", "it", "its", "that", "this", "with", "at", "by",
    "from", "be", "can", "get", "run", "using", "use", NULL
};

static bool is_stop_word(const char *w)
{
    for (size_t i = 0; STOP_WORDS[i] != NULL; i++) {
        if (strcasecmp(w, STOP_WORDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

static char *trim_string(char *s)
{
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
        s++;
    }
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
                       s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[--len] = '\0';
    }
    return s;
}

static char *strip_quotes(char *s)
{
    size_t len = strlen(s);
    while (len >= 2 && ((s[0] == '\'' && s[len - 1] == '\'') ||
                        (s[0] == '"' && s[len - 1] == '"'))) {
        s[len - 1] = '\0';
        s++;
        len -= 2;
    }
    return s;
}

static void stem_word(const char *in, char *out, size_t out_cap)
{
    size_t len = strlen(in);
    if (len >= out_cap) {
        len = out_cap - 1;
    }
    for (size_t i = 0; i < len; i++) {
        out[i] = (char)tolower((unsigned char)in[i]);
    }
    out[len] = '\0';

    if (len > 4 && strcmp(out + len - 3, "ies") == 0) {
        out[len - 3] = '\0';
    } else if (len > 4 && strcmp(out + len - 3, "ing") == 0) {
        out[len - 3] = '\0';
    } else if (len > 3 && strcmp(out + len - 2, "ed") == 0) {
        out[len - 2] = '\0';
    } else if (len > 3 && strcmp(out + len - 2, "es") == 0) {
        out[len - 2] = '\0';
    } else if (len > 3 && out[len - 1] == 's') {
        out[len - 1] = '\0';
    }
}

static int levenshtein(const char *s1, const char *s2)
{
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);
    if (l1 == 0) return (int)l2;
    if (l2 == 0) return (int)l1;
    if (l2 >= 255) return 999;

    int d[256];
    for (size_t j = 0; j <= l2; j++) {
        d[j] = (int)j;
    }

    for (size_t i = 1; i <= l1; i++) {
        int prev = d[0];
        d[0] = (int)i;
        for (size_t j = 1; j <= l2; j++) {
            int temp = d[j];
            int cost = (tolower((unsigned char)s1[i - 1]) ==
                        tolower((unsigned char)s2[j - 1])) ? 0 : 1;
            int min = prev + cost;
            if (d[j] + 1 < min) min = d[j] + 1;
            if (d[j - 1] + 1 < min) min = d[j - 1] + 1;
            d[j] = min;
            prev = temp;
        }
    }
    return d[l2];
}

/* Split a comma-separated TSV field (modified in place) into an allocated
 * array of trimmed, non-empty strings. */
static char **split_list(char *field, size_t *count)
{
    *count = 0;
    if (field == NULL || field[0] == '\0') {
        return NULL;
    }
    size_t cap = 8;
    char **items = malloc(cap * sizeof(char *));
    if (items == NULL) {
        return NULL;
    }
    char *s = field;
    while (s != NULL && *s != '\0') {
        char *comma = strchr(s, ',');
        if (comma != NULL) {
            *comma = '\0';
            comma++;
        }
        char *k = trim_string(s);
        if (*k != '\0') {
            if (*count >= cap) {
                cap *= 2;
                char **re = realloc(items, cap * sizeof(char *));
                if (re == NULL) break;
                items = re;
            }
            items[(*count)++] = strdup(k);
        }
        s = comma;
    }
    return items;
}

static void load_parent_projection(tam_index_t *idx, const char *root)
{
    char *path = tam_path_join(root, "index.parents.tsv");
    if (path == NULL) return;
    char *content = tam_fs_read_file(path, NULL);
    free(path);
    if (content == NULL) return; /* Older corpora have no parent projection. */

    char *line = content;
    bool header = true;
    while (line != NULL && *line != '\0') {
        char *next = strchr(line, '\n');
        if (next != NULL) {
            *next = '\0';
            next++;
        }
        if (header) {
            header = false;
            line = next;
            continue;
        }
        char *tab = strchr(line, '\t');
        if (tab != NULL) {
            *tab++ = '\0';
            const tam_index_entry_t *found = tam_index_lookup_path(idx, line);
            if (found != NULL && *tab != '\0') {
                tam_index_entry_t *entry = (tam_index_entry_t *)found;
                entry->parent = strdup(tab);
            }
        }
        line = next;
    }
    free(content);
}

static void load_kind_projection(tam_index_t *idx, const char *root)
{
    char *path = tam_path_join(root, "index.kinds.tsv");
    if (path == NULL) return;
    char *content = tam_fs_read_file(path, NULL);
    free(path);
    if (content == NULL) return; /* Older corpora have no parent projection. */

    char *line = content;
    bool header = true;
    while (line != NULL && *line != '\0') {
        char *next = strchr(line, '\n');
        if (next != NULL) {
            *next = '\0';
            next++;
        }
        if (header) {
            header = false;
            line = next;
            continue;
        }
        char *tab = strchr(line, '\t');
        if (tab != NULL) {
            *tab++ = '\0';
            const tam_index_entry_t *found = tam_index_lookup_path(idx, line);
            if (found != NULL && *tab != '\0') {
                tam_index_entry_t *entry = (tam_index_entry_t *)found;
                free(entry->kind);
                entry->kind = strdup(tab);
            }
        }
        line = next;
    }
    free(content);
}

static tam_path_def_t *find_path_mut(tam_index_t *idx, const char *id)
{
    if (idx == NULL || id == NULL || id[0] == '\0') return NULL;
    for (size_t i = 0; i < idx->num_paths; i++) {
        if (strcasecmp(idx->paths[i].id, id) == 0)
            return &idx->paths[i];
    }
    return NULL;
}

static void load_paths_projection(tam_index_t *idx, const char *root)
{
    char *path = tam_path_join(root, "index.paths.tsv");
    if (path == NULL) return;
    char *content = tam_fs_read_file(path, NULL);
    free(path);
    if (content == NULL) return; /* Older corpora have no paths projection. */

    size_t pcap = 0;
    char *line = content;
    bool header = true;
    while (line != NULL && *line != '\0') {
        char *next = strchr(line, '\n');
        if (next != NULL) {
            *next = '\0';
            next++;
        }
        char *trimmed = trim_string(line);
        if (*trimmed == '\0') {
            line = next;
            continue;
        }
        if (header) {
            header = false;
            line = next;
            continue;
        }
        char *cols[6] = {0};
        char *cur = trimmed;
        for (int c = 0; c < 6 && cur != NULL; c++) {
            cols[c] = cur;
            char *tab = strchr(cur, '\t');
            if (tab != NULL) {
                *tab = '\0';
                cur = tab + 1;
            } else {
                cur = NULL;
            }
        }
        if (cols[0] != NULL && strcmp(cols[0], "P") == 0 &&
            cols[1] != NULL && cols[1][0] != '\0') {
            if (idx->num_paths >= pcap) {
                size_t next_cap = pcap ? pcap * 2 : 16;
                tam_path_def_t *re = realloc(idx->paths, next_cap * sizeof(*re));
                if (re == NULL) break;
                idx->paths = re;
                pcap = next_cap;
            }
            tam_path_def_t *p = &idx->paths[idx->num_paths++];
            memset(p, 0, sizeof(*p));
            p->id = strdup(cols[1]);
            p->title = cols[2] && cols[2][0] ? strdup(cols[2]) : strdup(cols[1]);
            p->intro_path = cols[3] && cols[3][0] ? strdup(cols[3]) : strdup("");
            p->summary = cols[4] ? strdup(cols[4]) : strdup("");
            p->skills = cols[5] ? split_list(cols[5], &p->num_skills) : NULL;
        } else if (cols[0] != NULL && strcmp(cols[0], "S") == 0 &&
                   cols[1] != NULL && cols[2] != NULL && cols[3] != NULL) {
            tam_path_def_t *p = find_path_mut(idx, cols[1]);
            if (p != NULL) {
                tam_path_step_t *re = realloc(
                    p->steps, (p->num_steps + 1) * sizeof(*p->steps));
                if (re != NULL) {
                    p->steps = re;
                    tam_path_step_t *st = &p->steps[p->num_steps++];
                    st->step = atoi(cols[2]);
                    st->path = strdup(cols[3]);
                    st->title = cols[4] && cols[4][0] ? strdup(cols[4]) : strdup(cols[3]);
                }
            }
        }
        line = next;
    }
    free(content);
}

tam_index_t *tam_index_load(const char *root)
{
    if (root == NULL) {
        return NULL;
    }
    char *path = tam_path_join(root, "index.tsv");
    if (path == NULL) {
        return NULL;
    }
    size_t len = 0;
    char *content = tam_fs_read_file(path, &len);
    free(path);
    if (content == NULL) {
        return NULL;
    }

    tam_index_t *idx = calloc(1, sizeof(tam_index_t));
    if (idx == NULL) {
        free(content);
        return NULL;
    }

    size_t cap = 16;
    idx->entries = calloc(cap, sizeof(tam_index_entry_t));
    if (idx->entries == NULL) {
        free(content);
        free(idx);
        return NULL;
    }

    char *line = content;
    int is_header = 1;
    int has_level_col = 0;

    while (line != NULL && *line != '\0') {
        char *next_line = strchr(line, '\n');
        if (next_line != NULL) {
            *next_line = '\0';
            next_line++;
        }

        char *trimmed = trim_string(line);
        if (*trimmed == '\0') {
            line = next_line;
            continue;
        }

        if (is_header) {
            has_level_col = (strstr(trimmed, "\tlevel\t") != NULL);
            is_header = 0;
            line = next_line;
            continue;
        }

        /* Support both 9-column (level retired) and legacy 10-column index.tsv. */
        char *cols[10] = {0};
        char *cur = trimmed;
        for (int c = 0; c < 10 && cur != NULL; c++) {
            cols[c] = cur;
            char *tab = strchr(cur, '\t');
            if (tab != NULL) {
                *tab = '\0';
                cur = tab + 1;
            } else {
                cur = NULL;
            }
        }

        if (cols[0] != NULL && cols[1] != NULL && cols[2] != NULL) {
            if (idx->count >= cap) {
                cap *= 2;
                tam_index_entry_t *re = realloc(idx->entries, cap * sizeof(tam_index_entry_t));
                if (re == NULL) break;
                idx->entries = re;
            }

            tam_index_entry_t *entry = &idx->entries[idx->count];
            entry->topic = strdup(cols[0]);
            entry->path = strdup(cols[1]);
            entry->title = strdup(cols[2]);
            entry->summary = cols[3] ? strdup(cols[3]) : strdup("");
            entry->keywords = split_list(cols[4], &entry->num_keywords);
            entry->parent = NULL;
            entry->kind = NULL;
            if (has_level_col) {
                entry->level = (cols[5] && cols[5][0]) ? strdup(cols[5]) : NULL;
                entry->levels = tam_level_parse_list(entry->level);
                entry->track = (cols[6] && cols[6][0]) ? strdup(cols[6]) : strdup("use");
                entry->prereqs = split_list(cols[7], &entry->num_prereqs);
                entry->rev = (cols[8] && cols[8][0]) ? atoi(cols[8]) : 0;
                entry->updated = (cols[9] && cols[9][0]) ? strdup(cols[9]) : strdup("");
            } else {
                entry->level = NULL;
                entry->levels = TAM_LEVEL_ALL;
                entry->track = (cols[5] && cols[5][0]) ? strdup(cols[5]) : strdup("use");
                entry->prereqs = split_list(cols[6], &entry->num_prereqs);
                entry->rev = (cols[7] && cols[7][0]) ? atoi(cols[7]) : 0;
                entry->updated = (cols[8] && cols[8][0]) ? strdup(cols[8]) : strdup("");
            }

            idx->count++;
        }

        line = next_line;
    }

    load_parent_projection(idx, root);
    load_kind_projection(idx, root);
    load_paths_projection(idx, root);
    free(content);
    return idx;
}

void tam_index_free(tam_index_t *idx)
{
    if (idx == NULL) return;
    for (size_t i = 0; i < idx->count; i++) {
        tam_index_entry_t *e = &idx->entries[i];
        free(e->topic);
        free(e->path);
        free(e->title);
        free(e->summary);
        free(e->level);
        free(e->track);
        free(e->parent);
        free(e->kind);
        free(e->updated);
        for (size_t k = 0; k < e->num_keywords; k++) {
            free(e->keywords[k]);
        }
        free(e->keywords);
        for (size_t k = 0; k < e->num_prereqs; k++) {
            free(e->prereqs[k]);
        }
        free(e->prereqs);
    }
    free(idx->entries);
    for (size_t i = 0; i < idx->num_paths; i++) {
        tam_path_def_t *p = &idx->paths[i];
        free(p->id);
        free(p->title);
        free(p->intro_path);
        free(p->summary);
        for (size_t k = 0; k < p->num_skills; k++) {
            free(p->skills[k]);
        }
        free(p->skills);
        for (size_t k = 0; k < p->num_steps; k++) {
            free(p->steps[k].path);
            free(p->steps[k].title);
        }
        free(p->steps);
    }
    free(idx->paths);
    free(idx);
}

const tam_path_def_t *tam_index_find_path(const tam_index_t *idx, const char *id)
{
    if (idx == NULL || id == NULL || id[0] == '\0') return NULL;
    for (size_t i = 0; i < idx->num_paths; i++) {
        if (strcasecmp(idx->paths[i].id, id) == 0)
            return &idx->paths[i];
    }
    return NULL;
}

size_t tam_index_paths_for_page(const tam_index_t *idx, const char *page_path,
                                const tam_path_def_t **out_paths,
                                size_t *out_step_idx, size_t max_entries)
{
    if (idx == NULL || page_path == NULL || page_path[0] == '\0' || max_entries == 0)
        return 0;
    size_t n = 0;
    for (size_t i = 0; i < idx->num_paths && n < max_entries; i++) {
        const tam_path_def_t *p = &idx->paths[i];
        for (size_t s = 0; s < p->num_steps; s++) {
            if (p->steps[s].path != NULL && strcasecmp(p->steps[s].path, page_path) == 0) {
                if (out_paths != NULL) out_paths[n] = p;
                if (out_step_idx != NULL) out_step_idx[n] = s;
                n++;
                break;
            }
        }
    }
    return n;
}

const tam_index_entry_t *tam_index_lookup_path(const tam_index_t *idx, const char *path)
{
    if (idx == NULL || path == NULL || path[0] == '\0') {
        return NULL;
    }
    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (e->path != NULL && strcasecmp(path, e->path) == 0) {
            return e;
        }
    }
    return NULL;
}

int tam_index_suggest_fuzzy(const tam_index_t *idx, const char *query,
                            const tam_index_entry_t **out_entry, const char **out_keyword,
                            int max_dist)
{
    if (idx == NULL || query == NULL) return -1;
    char buf[256];
    size_t len = strlen(query);
    if (len >= sizeof(buf)) return -1;
    memcpy(buf, query, len + 1);

    char *t = trim_string(buf);
    t = strip_quotes(t);
    if (*t == '\0') return -1;

    int query_len = (int)strlen(t);
    int best_dist = max_dist + 1;
    const tam_index_entry_t *best_entry = NULL;
    const char *best_kw = NULL;

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        int d = abs(query_len - (int)strlen(e->topic)) < best_dist
            ? levenshtein(t, e->topic) : best_dist;
        if (d < best_dist) {
            best_dist = d;
            best_entry = e;
            best_kw = e->topic;
        }
        for (size_t k = 0; k < e->num_keywords; k++) {
            if (abs(query_len - (int)strlen(e->keywords[k])) >= best_dist) continue;
            d = levenshtein(t, e->keywords[k]);
            if (d < best_dist) {
                best_dist = d;
                best_entry = e;
                best_kw = e->keywords[k];
            }
        }
    }

    if (best_dist <= max_dist && best_entry != NULL) {
        if (out_entry) *out_entry = best_entry;
        if (out_keyword) *out_keyword = best_kw;
        return best_dist;
    }
    return -1;
}

typedef struct {
    char raw[64];
    char stem[64];
} query_token_t;

static bool contains_word_ci(const char *haystack, const char *needle)
{
    if (haystack == NULL || needle == NULL || *needle == '\0') return false;
    size_t nlen = strlen(needle);
    const char *p = haystack;
    while (*p != '\0') {
        if (strncasecmp(p, needle, nlen) == 0) {
            char prev = (p == haystack) ? ' ' : *(p - 1);
            char next = *(p + nlen);
            if (!isalnum((unsigned char)prev) && !isalnum((unsigned char)next)) {
                return true;
            }
        }
        p++;
    }
    return false;
}

static bool is_glossary_index_entry(const tam_index_entry_t *e)
{
    if (e == NULL || e->path == NULL) return false;
    if (e->kind != NULL) return strcmp(e->kind, "glossary") == 0;
    if (strncmp(e->path, "glossary/", 9) == 0 && !strstr(e->path, "/index.html")) return true;
    if (strcmp(e->path, "glossary/index.html") == 0) return false;
    if (strncmp(e->path, "learn/glossary/", 15) == 0) return true;
    if (e->topic != NULL && strcmp(e->topic, "glossary") != 0 &&
        strncmp(e->topic, "glossary-", 9) == 0) return true;
    if (e->track != NULL && strcmp(e->track, "glossary") == 0 &&
        e->topic != NULL && strcmp(e->topic, "glossary") != 0) return true;
    return false;
}

/* Exact keywords describe a page's subject and related subjects alike. */
enum match_strength {
    MATCH_NONE, MATCH_KEYWORD, MATCH_GLOSSARY_NAME, MATCH_TITLE, MATCH_TOPIC
};

static char *normalize_query(char *query)
{
    return trim_string(strip_quotes(trim_string(query)));
}

static bool exact_span(const char *query, const char *start, size_t len)
{
    return strlen(query) == len && strncasecmp(query, start, len) == 0;
}

/* Glossary titles often pair the long name and acronym, e.g. NAME (ID)
 * or ID - NAME. Require the part to be an authored keyword as well. */
static bool glossary_title_name(const char *title, const char *query)
{
    const char *dash = strstr(title, " - ");
    if (dash && (exact_span(query, title, (size_t)(dash - title)) ||
                 strcasecmp(query, dash + 3) == 0)) return true;
    const char *paren = strstr(title, " (");
    size_t len = strlen(title);
    return paren && title[len - 1] == ')' &&
           (exact_span(query, title, (size_t)(paren - title)) ||
            exact_span(query, paren + 2, (size_t)(title + len - 1 - (paren + 2))));
}

static enum match_strength exact_match(const tam_index_entry_t *e, const char *query)
{
    if (strcasecmp(query, e->topic) == 0) return MATCH_TOPIC;
    if (strcasecmp(query, e->title) == 0) return MATCH_TITLE;

    bool glossary = is_glossary_index_entry(e);
    if (glossary) {
        if (strncasecmp(e->topic, "glossary-", 9) == 0 &&
            strcasecmp(query, e->topic + 9) == 0) return MATCH_GLOSSARY_NAME;

        const char *leaf = strrchr(e->path, '/');
        if (leaf) {
            leaf++;
            size_t len = strlen(leaf);
            if (len > 5 && strcmp(leaf + len - 5, ".html") == 0 &&
                strcmp(leaf, "index.html") != 0 &&
                exact_span(query, leaf, len - 5)) return MATCH_GLOSSARY_NAME;
        }
    }
    for (size_t k = 0; k < e->num_keywords; k++) {
        if (strcasecmp(query, e->keywords[k]) == 0) {
            return glossary && glossary_title_name(e->title, query)
                ? MATCH_GLOSSARY_NAME : MATCH_KEYWORD;
        }
    }
    return MATCH_NONE;
}

static const tam_index_entry_t *lookup_exact(const tam_index_t *idx, const char *query,
                                              bool glossary_only, bool *ambiguous)
{
    const tam_index_entry_t *best = NULL;
    enum match_strength strength = MATCH_NONE;
    *ambiguous = false;
    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (glossary_only && !is_glossary_index_entry(e)) continue;
        enum match_strength match = exact_match(e, query);
        if (match > strength) {
            strength = match;
            best = e;
            *ambiguous = false;
        } else if (match != MATCH_NONE && match == strength) {
            *ambiguous = true;
        }
    }
    return *ambiguous ? NULL : best;
}

const tam_index_entry_t *tam_index_lookup_exact(const tam_index_t *idx, const char *query,
                                                bool *ambiguous)
{
    *ambiguous = false;
    char buf[512];
    if (idx == NULL || query == NULL || strlen(query) >= sizeof(buf)) return NULL;
    strcpy(buf, query);
    char *q = normalize_query(buf);
    return *q ? lookup_exact(idx, q, false, ambiguous) : NULL;
}

const char *tam_index_purpose(const tam_index_entry_t *e)
{
    if (is_glossary_index_entry(e)) return "Definitions";
    const char *k = e->kind ? e->kind : "";
    if (strcmp(k, "path") == 0) return "Learning paths";
    if (strcmp(k, "how-to") == 0 || strcmp(k, "tutorial") == 0) return "Practical guidance";
    if (strcmp(k, "bibliography") == 0) return "Sources";
    if (strcmp(k, "hub") == 0) return "Subject guides";
    return "Explanations and reference";
}

static int score_glossary_candidate(const char *q, size_t qlen, const char *cand)
{
    if (cand == NULL || *cand == '\0') return -1;
    char clc[256];
    size_t clen = strlen(cand);
    if (clen >= sizeof(clc)) clen = sizeof(clc) - 1;
    for (size_t i = 0; i < clen; i++) {
        clc[i] = (char)tolower((unsigned char)cand[i]);
    }
    clc[clen] = '\0';

    /* Exact match */
    if (strcmp(q, clc) == 0) {
        return 0;
    }

    /* Prefix match */
    if (strncmp(clc, q, qlen) == 0) {
        return 100 + (int)(clen - qlen);
    }

    /* Word match */
    if (contains_word_ci(cand, q)) {
        return 200 + (int)(clen - qlen);
    }

    /* Substring match */
    if (strstr(clc, q) != NULL) {
        return 300 + (int)(clen - qlen);
    }

    /* Fuzzy match */
    int min_d = levenshtein(q, clc);
    if (strchr(clc, ' ') != NULL) {
        char copy[256];
        memcpy(copy, clc, clen + 1);
        char *w = strtok(copy, " ");
        while (w != NULL) {
            int dw = levenshtein(q, w);
            if (dw < min_d) min_d = dw;
            w = strtok(NULL, " ");
        }
    }

    /* Relative closeness: edits vs shorter string, hard-capped at 3.
     * Query-only caps let short aliases (bus) absorb nonsense (bogus). */
    size_t shorter = qlen < clen ? qlen : clen;
    int max_allowed = (int)(shorter / 3);
    if (max_allowed < 1) max_allowed = 1;
    if (max_allowed > 3) max_allowed = 3;
    if (min_d <= max_allowed) {
        return 1000 + min_d * 50 + (int)clen;
    }

    return -1;
}

const tam_index_entry_t *tam_index_closest_glossary_match(const tam_index_t *idx, const char *query,
                                                           bool *ambiguous)
{
    *ambiguous = false;
    if (idx == NULL || query == NULL) return NULL;
    char buf[256];
    size_t len = strlen(query);
    if (len >= sizeof(buf)) return NULL;
    memcpy(buf, query, len + 1);

    char *t = normalize_query(buf);
    if (*t == '\0') return NULL;
    const tam_index_entry_t *exact = lookup_exact(idx, t, true, ambiguous);
    if (exact != NULL || *ambiguous) return exact;

    char q[256];
    size_t qlen = strlen(t);
    if (qlen >= sizeof(q)) return NULL;
    for (size_t i = 0; i < qlen; i++) {
        q[i] = (char)tolower((unsigned char)t[i]);
    }
    q[qlen] = '\0';

    int best_score = -1;
    const tam_index_entry_t *best_entry = NULL;

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (!is_glossary_index_entry(e)) continue;

        int entry_best = -1;

        #define EVAL_CAND(cand_str) do { \
            const char *_cs = (cand_str); \
            if (_cs != NULL && *_cs != '\0') { \
                int s = score_glossary_candidate(q, qlen, _cs); \
                if (s >= 0 && (entry_best < 0 || s < entry_best)) { \
                    entry_best = s; \
                } \
            } \
        } while (0)

        /* 1. Title */
        EVAL_CAND(e->title);

        /* 2. Topic/slug (without glossary- prefix) */
        if (e->topic != NULL) {
            if (strncmp(e->topic, "glossary-", 9) == 0) {
                EVAL_CAND(e->topic + 9);
            } else {
                EVAL_CAND(e->topic);
            }
        }

        /* 3. Filename slug from path */
        if (e->path != NULL && strncmp(e->path, "learn/glossary/", 15) == 0) {
            const char *leaf = e->path + 15;
            char leaf_buf[64];
            size_t leaf_len = strlen(leaf);
            if (leaf_len > 5 && strcmp(leaf + leaf_len - 5, ".html") == 0) {
                size_t base_len = leaf_len - 5;
                if (base_len < sizeof(leaf_buf)) {
                    memcpy(leaf_buf, leaf, base_len);
                    leaf_buf[base_len] = '\0';
                    EVAL_CAND(leaf_buf);
                }
            }
        }

        /* 4. Keywords */
        for (size_t k = 0; k < e->num_keywords; k++) {
            const char *kw = e->keywords[k];
            if (kw != NULL) {
                if (strncmp(kw, "glossary-", 9) == 0) {
                    EVAL_CAND(kw + 9);
                } else if (strcmp(kw, "learning") != 0) {
                    EVAL_CAND(kw);
                }
            }
        }

        #undef EVAL_CAND

        if (entry_best >= 0) {
            if (best_score < 0 || entry_best < best_score) {
                best_score = entry_best;
                best_entry = e;
                *ambiguous = false;
            } else if (entry_best == best_score) {
                *ambiguous = true;
            }
        }
    }

    return *ambiguous ? NULL : best_entry;
}

static bool query_separator(char c)
{
    return isspace((unsigned char)c) || c == ',' || c == ';';
}

static size_t query_tokens(const char *query, query_token_t tokens[16])
{
    size_t count = 0;
    bool has_meaningful = false;
    while (*query && count < 16) {
        while (*query && query_separator(*query)) query++;
        const char *start = query;
        while (*query && !query_separator(*query)) query++;
        size_t len = (size_t)(query - start);
        if (len == 0 || len >= sizeof(tokens[0].raw)) continue;

        bool duplicate = false;
        for (size_t i = 0; i < count; i++) {
            if (exact_span(tokens[i].raw, start, len)) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        memcpy(tokens[count].raw, start, len);
        tokens[count].raw[len] = '\0';
        stem_word(tokens[count].raw, tokens[count].stem, sizeof(tokens[count].stem));
        has_meaningful |= !is_stop_word(tokens[count].raw);
        count++;
    }
    if (!has_meaningful) return count;

    size_t kept = 0;
    for (size_t i = 0; i < count; i++) {
        if (!is_stop_word(tokens[i].raw)) tokens[kept++] = tokens[i];
    }
    return kept;
}

static int score_term(const tam_index_entry_t *e, const query_token_t *token)
{
    const char *raw = token->raw;
    const char *stem = token->stem;
    char field_stem[64];
    stem_word(e->topic, field_stem, sizeof(field_stem));
    int score = strcasecmp(raw, e->topic) == 0 ? 30 :
                strcasecmp(stem, field_stem) == 0 ? 20 : 0;

    int keyword_score = 0;
    for (size_t k = 0; k < e->num_keywords; k++) {
        const char *kw = e->keywords[k];
        if (strcasecmp(raw, kw) == 0) {
            keyword_score = 25;
            break;
        }
        if (contains_word_ci(kw, raw)) {
            keyword_score = 20;
        } else if (keyword_score < 18) {
            stem_word(kw, field_stem, sizeof(field_stem));
            if (strcasecmp(stem, field_stem) == 0) keyword_score = 18;
        }
    }
    score += keyword_score;
    if (contains_word_ci(e->title, raw)) score += 15;
    else if (contains_word_ci(e->title, stem)) score += 10;
    if (contains_word_ci(e->summary, raw)) score += 8;
    else if (contains_word_ci(e->summary, stem)) score += 6;
    return score;
}

static bool matches_prefix(const tam_index_entry_t *e, const char *raw, size_t len)
{
    if (strncasecmp(e->topic, raw, len) == 0 ||
        strncasecmp(e->title, raw, len) == 0) return true;
    for (size_t k = 0; k < e->num_keywords; k++) {
        if (strncasecmp(e->keywords[k], raw, len) == 0) return true;
    }
    return false;
}

static int compare_results(const void *a, const void *b)
{
    const tam_search_result_t *left = a;
    const tam_search_result_t *right = b;
    if (left->score != right->score) return left->score > right->score ? -1 : 1;
    int by_title = strcasecmp(left->entry->title, right->entry->title);
    return by_title ? by_title : strcmp(left->entry->path, right->entry->path);
}

size_t tam_index_search(const tam_index_t *idx, const char *query,
                        tam_search_result_t *results, size_t max_results)
{
    if (idx == NULL || query == NULL || results == NULL || max_results == 0) return 0;

    char buf[512];
    snprintf(buf, sizeof(buf), "%s", query);
    char *q = normalize_query(buf);
    if (*q == '\0') return 0;

    query_token_t tokens[16];
    size_t n = query_tokens(q, tokens);
    tam_search_result_t *scored = calloc(idx->count ? idx->count : 1, sizeof(*scored));
    if (scored == NULL) return 0;
    size_t found = 0;
    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        enum match_strength strength = exact_match(e, q);
        if (strength != MATCH_NONE) {
            scored[found++] = (tam_search_result_t){e, 1000 * strength};
            continue;
        }

        int score = 0;
        size_t matched = 0;
        for (size_t t = 0; t < n; t++) {
            int term = score_term(e, &tokens[t]);
            if (term > 0) {
                score += term;
                matched++;
            }
        }

        if (matched == 0 || (n > 1 && (matched < 2 || matched < (n + 1) / 2))) continue;

        /* Rank by coverage, then field quality. Each term adds at most 78
         * points and there are at most 16 terms. Exact matches stay above
         * these scores, including queries with many terms. */
        int quality = score / (int)matched;
        if (contains_word_ci(e->title, q)) quality += 12;
        scored[found++] = (tam_search_result_t){e, 16 + 56 * (int)matched + quality / 2};
    }
    if (found == 0 && n == 1) {
        const char *raw = tokens[0].raw;
        size_t len = strlen(raw);
        if (len >= 3) {
            for (size_t i = 0; i < idx->count; i++) {
                const tam_index_entry_t *e = &idx->entries[i];
                if (matches_prefix(e, raw, len)) {
                    scored[found++] = (tam_search_result_t){e, 10};
                }
            }
        }
    }

    qsort(scored, found, sizeof(*scored), compare_results);
    if (found > max_results) found = max_results;
    memcpy(results, scored, found * sizeof(*results));
    free(scored);
    return found;
}

int tam_index_print_miss(FILE *out, const tam_index_t *idx, const char *query)
{
    fprintf(out, "tam: no topic found for '%s'\n\n", query);

    const tam_index_entry_t *sug_entry = NULL;
    const char *sug_kw = NULL;
    if (tam_index_suggest_fuzzy(idx, query, &sug_entry, &sug_kw, 1) >= 0 && sug_kw != NULL) {
        fprintf(out, "Did you mean '%s'?\n\n", sug_kw);
    }

    fprintf(out, "Top candidates:\n");
    size_t shown = 0;
    for (size_t i = 0; i < idx->count && shown < 3; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        fprintf(out, "  %-12s %s\n", e->topic, e->summary);
        fprintf(out, "               Run 'tam explain %s' to view.\n", e->topic);
        shown++;
    }
    fprintf(out, "\n");

    char *man_path = tam_man_resolve_path(query, NULL);
    if (man_path != NULL) {
        free(man_path);
        fprintf(out, "Run 'tam man %s' to view the system manual page.\n", query);
    }

    fprintf(out, "Run 'tam show' to explore the offline guide.\n");
    return 1;
}

void tam_index_print_catalog_ex(FILE *out, const tam_index_t *idx, unsigned level, bool terse, bool verbose)
{
    if (terse) {
        for (size_t i = 0; i < idx->count; i++) {
            const tam_index_entry_t *e = &idx->entries[i];
            fprintf(out, "%s\n", e->topic);
        }
        return;
    }

    if (verbose) {
        if (level != 0) {
            fprintf(out, "Topics A to Z (%s):\n\n", tam_level_name(level));
        } else {
            fprintf(out, "Topics A to Z:\n\n");
        }
        for (size_t i = 0; i < idx->count; i++) {
            const tam_index_entry_t *e = &idx->entries[i];
            fprintf(out, "  %-14s %s\n", e->topic, e->summary);
            fprintf(out, "    Title:       %s\n", e->title);
            fprintf(out, "    Path:        %s\n", e->path);
            if (e->rev > 0 && e->updated != NULL && e->updated[0] != '\0') {
                fprintf(out, "    Revision:    Rev %d Last updated %s\n", e->rev, e->updated);
            }
            if (e->track != NULL && e->track[0] != '\0') {
                fprintf(out, "    Track:       %s\n", e->track);
            }
            if (e->num_prereqs > 0) {
                fprintf(out, "    Prereqs:     ");
                for (size_t p = 0; p < e->num_prereqs; p++) {
                    fprintf(out, "%s%s", p > 0 ? ", " : "", e->prereqs[p]);
                }
                fprintf(out, "\n");
            }
            if (e->num_keywords > 0) {
                fprintf(out, "    Keywords:    ");
                for (size_t k = 0; k < e->num_keywords; k++) {
                    fprintf(out, "%s%s", k > 0 ? ", " : "", e->keywords[k]);
                }
                fprintf(out, "\n");
            }
            fprintf(out, "\n");
        }
        fprintf(out, "Run 'tam explain <topic>' to view an explanation.\n");
        fprintf(out, "Run 'tam show' to open the offline guide.\n");
        return;
    }

    if (level != 0) {
        fprintf(out, "Topics A to Z (%s):\n", tam_level_name(level));
    } else {
        fprintf(out, "Topics A to Z:\n");
    }

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        fprintf(out, "  %-12s %s\n", e->topic, e->summary);
    }
    fprintf(out,
        "\nRun 'tam explain <topic>' to view an explanation.\n"
        "Run 'tam show' to open the offline guide.\n");
}

void tam_index_print_catalog(FILE *out, const tam_index_t *idx, unsigned level)
{
    tam_index_print_catalog_ex(out, idx, level, false, false);
}

size_t tam_index_find_topic_pages(const tam_index_t *idx, const char *topic,
                                  unsigned level, const tam_index_entry_t **out_entries,
                                  size_t max_entries)
{
    (void)level;
    if (out_entries == NULL || max_entries == 0) return 0;

    tam_search_result_t *results = calloc(max_entries, sizeof(*results));
    if (results == NULL) return 0;
    size_t found = tam_index_search(idx, topic, results, max_entries);
    for (size_t i = 0; i < found; i++) out_entries[i] = results[i].entry;
    free(results);
    return found;
}

static int keyword_cmp(const void *a, const void *b)
{
    const tam_keyword_info_t *ka = a;
    const tam_keyword_info_t *kb = b;
    return strcasecmp(ka->keyword, kb->keyword);
}

tam_keyword_list_t *tam_index_get_keywords(const tam_index_t *idx, unsigned level)
{
    (void)level;
    if (idx == NULL) return NULL;

    tam_keyword_list_t *list = malloc(sizeof(tam_keyword_list_t));
    if (list == NULL) return NULL;
    size_t cap = 256;
    list->count = 0;
    list->items = malloc(cap * sizeof(tam_keyword_info_t));
    if (list->items == NULL) {
        free(list);
        return NULL;
    }

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];

        size_t total_cand = 1 + e->num_keywords;
        for (size_t c = 0; c < total_cand; c++) {
            const char *raw = (c == 0) ? e->topic : e->keywords[c - 1];
            if (raw == NULL) continue;

            char buf[256];
            size_t rlen = strlen(raw);
            if (rlen >= sizeof(buf)) continue;
            memcpy(buf, raw, rlen + 1);
            char *kw = trim_string(buf);
            kw = strip_quotes(kw);
            if (*kw == '\0') continue;

            bool found = false;
            for (size_t j = 0; j < list->count; j++) {
                if (strcasecmp(list->items[j].keyword, kw) == 0) {
                    list->items[j].count++;
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (list->count >= cap) {
                    cap *= 2;
                    tam_keyword_info_t *re = realloc(list->items, cap * sizeof(tam_keyword_info_t));
                    if (re == NULL) break;
                    list->items = re;
                }
                list->items[list->count].keyword = strdup(kw);
                list->items[list->count].count = 1;
                list->items[list->count].primary_summary = strdup(e->summary ? e->summary : "");
                list->count++;
            }
        }
    }

    if (list->count > 1) {
        qsort(list->items, list->count, sizeof(tam_keyword_info_t), keyword_cmp);
    }
    return list;
}

void tam_keyword_list_free(tam_keyword_list_t *list)
{
    if (list == NULL) return;
    for (size_t i = 0; i < list->count; i++) {
        free(list->items[i].keyword);
        free(list->items[i].primary_summary);
    }
    free(list->items);
    free(list);
}


