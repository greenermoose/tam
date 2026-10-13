#include <tam.h>

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

static char *safe_join(const char *dir, const char *file)
{
    if (dir == NULL || file == NULL) return NULL;
    size_t dlen = strlen(dir);
    size_t flen = strlen(file);
    char *out = malloc(dlen + flen + 2);
    if (out == NULL) return NULL;
    memcpy(out, dir, dlen);
    size_t pos = dlen;
    if (pos > 0 && out[pos - 1] != '/') {
        out[pos++] = '/';
    }
    memcpy(out + pos, file, flen + 1);
    return out;
}

tam_index_t *tam_index_load(const char *root)
{
    if (root == NULL) {
        return NULL;
    }
    char *path = safe_join(root, "index.tsv");
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
            is_header = 0;
            line = next_line;
            continue;
        }

        /* Parse TSV columns: topic, path, title, summary, keywords, level, track */
        char *cols[7] = {0};
        char *cur = trimmed;
        for (int c = 0; c < 7 && cur != NULL; c++) {
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
            entry->level = cols[5] ? strdup(cols[5]) : strdup("beginner");
            entry->track = cols[6] ? strdup(cols[6]) : strdup("use");

            /* Parse keywords */
            entry->keywords = NULL;
            entry->num_keywords = 0;
            if (cols[4] != NULL && cols[4][0] != '\0') {
                size_t kw_cap = 8;
                entry->keywords = malloc(kw_cap * sizeof(char *));
                char *kw_str = cols[4];
                while (kw_str != NULL && *kw_str != '\0') {
                    char *comma = strchr(kw_str, ',');
                    if (comma != NULL) {
                        *comma = '\0';
                        comma++;
                    }
                    char *k = trim_string(kw_str);
                    if (*k != '\0') {
                        if (entry->num_keywords >= kw_cap) {
                            kw_cap *= 2;
                            char **rek = realloc(entry->keywords, kw_cap * sizeof(char *));
                            if (rek == NULL) break;
                            entry->keywords = rek;
                        }
                        entry->keywords[entry->num_keywords++] = strdup(k);
                    }
                    kw_str = comma;
                }
            }

            idx->count++;
        }

        line = next_line;
    }

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
        for (size_t k = 0; k < e->num_keywords; k++) {
            free(e->keywords[k]);
        }
        free(e->keywords);
    }
    free(idx->entries);
    free(idx);
}

const tam_index_entry_t *tam_index_lookup_alias(const tam_index_t *idx, const char *word)
{
    if (idx == NULL || word == NULL) return NULL;
    char buf[256];
    size_t len = strlen(word);
    if (len >= sizeof(buf)) return NULL;
    memcpy(buf, word, len + 1);

    char *t = trim_string(buf);
    t = strip_quotes(t);
    if (*t == '\0') return NULL;

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (strcasecmp(t, e->topic) == 0) {
            return e;
        }
        for (size_t k = 0; k < e->num_keywords; k++) {
            if (strcasecmp(t, e->keywords[k]) == 0) {
                return e;
            }
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

    int best_dist = max_dist + 1;
    const tam_index_entry_t *best_entry = NULL;
    const char *best_kw = NULL;

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        int d = levenshtein(t, e->topic);
        if (d < best_dist) {
            best_dist = d;
            best_entry = e;
            best_kw = e->topic;
        }
        for (size_t k = 0; k < e->num_keywords; k++) {
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

size_t tam_index_search(const tam_index_t *idx, const char *query,
                        tam_search_result_t *results, size_t max_results)
{
    if (idx == NULL || query == NULL || results == NULL || max_results == 0) {
        return 0;
    }

    char buf[512];
    size_t qlen = strlen(query);
    if (qlen >= sizeof(buf)) {
        qlen = sizeof(buf) - 1;
    }
    memcpy(buf, query, qlen);
    buf[qlen] = '\0';

    char *trimmed = trim_string(buf);
    trimmed = strip_quotes(trimmed);
    if (*trimmed == '\0') {
        return 0;
    }

    /* 1. Exact alias match */
    const tam_index_entry_t *alias_match = tam_index_lookup_alias(idx, trimmed);
    size_t found = 0;

    if (alias_match != NULL) {
        results[found].entry = alias_match;
        results[found].score = 1000;
        results[found].match_type = "alias";
        found++;
    }

    /* 2. Tokenize and score */
    query_token_t tokens[16];
    size_t ntokens = 0;

    char *tok_str = trimmed;
    while (*tok_str != '\0' && ntokens < 16) {
        while (*tok_str != '\0' && (isspace((unsigned char)*tok_str) ||
                                    *tok_str == ',' || *tok_str == ';' ||
                                    *tok_str == '"' || *tok_str == '\'')) {
            tok_str++;
        }
        if (*tok_str == '\0') break;

        char *start = tok_str;
        while (*tok_str != '\0' && !isspace((unsigned char)*tok_str) &&
               *tok_str != ',' && *tok_str != ';') {
            tok_str++;
        }
        size_t tlen = (size_t)(tok_str - start);
        if (tlen > 0 && tlen < sizeof(tokens[0].raw)) {
            memcpy(tokens[ntokens].raw, start, tlen);
            tokens[ntokens].raw[tlen] = '\0';
            stem_word(tokens[ntokens].raw, tokens[ntokens].stem, sizeof(tokens[ntokens].stem));
            ntokens++;
        }
    }

    /* Filter out stop words unless all words are stop words */
    query_token_t valid_tokens[16];
    size_t nvalid = 0;
    for (size_t i = 0; i < ntokens; i++) {
        if (!is_stop_word(tokens[i].raw)) {
            valid_tokens[nvalid++] = tokens[i];
        }
    }
    if (nvalid == 0) {
        for (size_t i = 0; i < ntokens; i++) {
            valid_tokens[nvalid++] = tokens[i];
        }
    }

    /* Score each entry in the index */
    typedef struct {
        const tam_index_entry_t *entry;
        int score;
        const char *match_type;
    } scored_entry_t;

    scored_entry_t scored[64];
    size_t nscored = 0;

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        if (alias_match == e) {
            continue; /* Already added as alias match */
        }

        int score = 0;
        size_t matched_terms = 0;

        for (size_t t = 0; t < nvalid; t++) {
            const char *raw = valid_tokens[t].raw;
            const char *stem = valid_tokens[t].stem;
            bool term_hit = false;

            /* Topic match */
            if (strcasecmp(raw, e->topic) == 0) {
                score += 30;
                term_hit = true;
            } else {
                char e_stem[64];
                stem_word(e->topic, e_stem, sizeof(e_stem));
                if (strcasecmp(stem, e_stem) == 0) {
                    score += 20;
                    term_hit = true;
                }
            }

            /* Keyword match */
            for (size_t k = 0; k < e->num_keywords; k++) {
                const char *kw = e->keywords[k];
                if (strcasecmp(raw, kw) == 0) {
                    score += 25;
                    term_hit = true;
                } else if (contains_word_ci(kw, raw)) {
                    score += 20;
                    term_hit = true;
                } else {
                    char k_stem[64];
                    stem_word(kw, k_stem, sizeof(k_stem));
                    if (strcasecmp(stem, k_stem) == 0) {
                        score += 18;
                        term_hit = true;
                    }
                }
            }

            /* Title match */
            if (contains_word_ci(e->title, raw)) {
                score += 15;
                term_hit = true;
            } else if (contains_word_ci(e->title, stem)) {
                score += 10;
                term_hit = true;
            }

            /* Summary match */
            if (contains_word_ci(e->summary, raw)) {
                score += 8;
                term_hit = true;
            } else if (contains_word_ci(e->summary, stem)) {
                score += 6;
                term_hit = true;
            }

            if (term_hit) {
                matched_terms++;
            }
        }

        /* For multi-term queries, require that at least half the terms match */
        if (nvalid > 1 && matched_terms < (nvalid + 1) / 2) {
            score = 0;
        }

        if (matched_terms == nvalid && nvalid > 1) {
            score += 35; /* All query terms matched */
        }

        if (score > 0 && nscored < 64) {
            scored[nscored].entry = e;
            scored[nscored].score = score;
            scored[nscored].match_type = "terms";
            nscored++;
        }
    }

    /* 3. Substring/prefix fallback if no terms matched and single token */
    if (found == 0 && nscored == 0 && nvalid == 1) {
        const char *raw = valid_tokens[0].raw;
        size_t rlen = strlen(raw);
        if (rlen >= 3) {
            for (size_t i = 0; i < idx->count; i++) {
                const tam_index_entry_t *e = &idx->entries[i];
                if (strncasecmp(raw, e->topic, rlen) == 0 ||
                    strncasecmp(e->topic, raw, strlen(e->topic)) == 0) {
                    scored[nscored].entry = e;
                    scored[nscored].score = 10;
                    scored[nscored].match_type = "substring";
                    nscored++;
                    continue;
                }
                for (size_t k = 0; k < e->num_keywords; k++) {
                    const char *kw = e->keywords[k];
                    if (strncasecmp(raw, kw, rlen) == 0 ||
                        strncasecmp(kw, raw, strlen(kw)) == 0) {
                        scored[nscored].entry = e;
                        scored[nscored].score = 10;
                        scored[nscored].match_type = "substring";
                        nscored++;
                        break;
                    }
                }
            }
        }
    }

    /* Sort scored entries by score descending */
    for (size_t i = 0; i < nscored; i++) {
        for (size_t j = i + 1; j < nscored; j++) {
            if (scored[j].score > scored[i].score) {
                scored_entry_t tmp = scored[i];
                scored[i] = scored[j];
                scored[j] = tmp;
            }
        }
    }

    for (size_t i = 0; i < nscored && found < max_results; i++) {
        results[found].entry = scored[i].entry;
        results[found].score = scored[i].score;
        results[found].match_type = scored[i].match_type;
        found++;
    }

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

void tam_index_print_catalog(FILE *out, const tam_index_t *idx)
{
    fprintf(out,
        "tam explain - explain a curated topic or concept\n\n"
        "Usage:\n"
        "  tam explain TOPIC           explain a topic in the terminal\n"
        "  tam explain TOPIC --pipe    print topic explanation as plain text\n"
        "  tam explain -               read topic keyword from standard input\n\n"
        "Available topics:\n");

    for (size_t i = 0; i < idx->count; i++) {
        const tam_index_entry_t *e = &idx->entries[i];
        fprintf(out, "  %-12s %s\n", e->topic, e->summary);
        if (e->num_keywords > 0) {
            fprintf(out, "               Keywords: ");
            size_t printed = 0;
            for (size_t k = 0; k < e->num_keywords; k++) {
                if (strcasecmp(e->keywords[k], e->topic) == 0) {
                    continue; /* Don't repeat topic name */
                }
                fprintf(out, "%s%s", printed > 0 ? ", " : "", e->keywords[k]);
                printed++;
            }
            if (printed == 0) {
                fprintf(out, "%s", e->topic);
            }
            fprintf(out, "\n");
        }
    }
    fprintf(out,
        "\nRun 'tam explain <topic>' to view an explanation.\n"
        "Run 'tam show' to open the offline guide.\n");
}
