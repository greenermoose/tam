#define _POSIX_C_SOURCE 200809L

#include "sandbox.h"
#include "render.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *xstrdup(const char *s)
{
    return s != NULL ? strdup(s) : NULL;
}

typedef struct {
    char *s;
    size_t n;
    size_t cap;
} sbuf_t;

static void sb_append(sbuf_t *b, const char *data, size_t len)
{
    if (len == 0 || data == NULL) return;
    if (b->n + len + 1 > b->cap) {
        size_t ncap = b->cap ? b->cap * 2 : 128;
        while (ncap < b->n + len + 1) ncap *= 2;
        char *p = realloc(b->s, ncap);
        if (p == NULL) return;
        b->s = p;
        b->cap = ncap;
    }
    memcpy(b->s + b->n, data, len);
    b->n += len;
    b->s[b->n] = '\0';
}

static void sb_puts(sbuf_t *b, const char *s)
{
    if (s != NULL) sb_append(b, s, strlen(s));
}

static void sb_putc(sbuf_t *b, char c)
{
    sb_append(b, &c, 1);
}

tam_sandbox_lesson_t tam_sandbox_parse_lesson(const char *s)
{
    if (s == NULL) return TAM_LESSON_PLAIN;
    if (strcmp(s, "flags") == 0 || strcmp(s, "options") == 0 ||
        strcmp(s, "switches") == 0) return TAM_LESSON_FLAGS;
    if (strcmp(s, "paths") == 0 || strcmp(s, "files") == 0)
        return TAM_LESSON_PATHS;
    if (strcmp(s, "pipes") == 0 || strcmp(s, "pipelines") == 0)
        return TAM_LESSON_PIPES;
    if (strcmp(s, "variables") == 0 || strcmp(s, "quoting") == 0)
        return TAM_LESSON_VARIABLES;
    if (strcmp(s, "all") == 0) return TAM_LESSON_ALL;
    return TAM_LESSON_PLAIN;
}

tam_sb_vfs_node_t *tam_sb_vfs_new_dir(const char *name, const char *mode,
                                      const char *owner)
{
    tam_sb_vfs_node_t *n = calloc(1, sizeof(*n));
    if (n == NULL) return NULL;
    n->name = xstrdup(name ? name : "/");
    n->is_dir = 1;
    n->mode = xstrdup(mode ? mode : "drwxr-xr-x");
    n->owner = xstrdup(owner ? owner : "alex");
    n->size = 4096;
    return n;
}

static tam_sb_vfs_node_t *find_child(const tam_sb_vfs_node_t *dir,
                                     const char *name)
{
    if (dir == NULL || !dir->is_dir || name == NULL) return NULL;
    for (size_t i = 0; i < dir->nchildren; i++) {
        if (strcmp(dir->children[i]->name, name) == 0)
            return dir->children[i];
    }
    return NULL;
}

static void attach_child(tam_sb_vfs_node_t *parent, tam_sb_vfs_node_t *child)
{
    if (parent == NULL || child == NULL) return;
    tam_sb_vfs_node_t **arr = realloc(parent->children,
                                      (parent->nchildren + 1) * sizeof(*arr));
    if (arr == NULL) return;
    parent->children = arr;
    parent->children[parent->nchildren++] = child;
    child->parent = parent;
}

tam_sb_vfs_node_t *tam_sb_vfs_add_dir(tam_sb_vfs_node_t *parent,
                                      const char *name, const char *mode,
                                      const char *owner)
{
    if (parent == NULL || !parent->is_dir || name == NULL) return NULL;
    tam_sb_vfs_node_t *existing = find_child(parent, name);
    if (existing != NULL) return existing->is_dir ? existing : NULL;
    tam_sb_vfs_node_t *d = tam_sb_vfs_new_dir(name, mode, owner);
    if (d != NULL) attach_child(parent, d);
    return d;
}

tam_sb_vfs_node_t *tam_sb_vfs_add_file(tam_sb_vfs_node_t *parent,
                                       const char *name, const char *mode,
                                       const char *owner, size_t size,
                                       const char *content)
{
    if (parent == NULL || !parent->is_dir || name == NULL) return NULL;
    tam_sb_vfs_node_t *existing = find_child(parent, name);
    if (existing != NULL) {
        if (existing->is_dir) return NULL;
        free(existing->content);
        existing->content = xstrdup(content ? content : "");
        existing->size = size > 0 ? size : strlen(existing->content);
        return existing;
    }
    tam_sb_vfs_node_t *f = calloc(1, sizeof(*f));
    if (f == NULL) return NULL;
    f->name = xstrdup(name);
    f->is_dir = 0;
    f->mode = xstrdup(mode ? mode : "-rw-r--r--");
    f->owner = xstrdup(owner ? owner : "alex");
    f->content = xstrdup(content ? content : "");
    f->size = size > 0 ? size : strlen(f->content);
    attach_child(parent, f);
    return f;
}

tam_sb_vfs_node_t *tam_sb_vfs_ensure_path(tam_sb_vfs_node_t *root,
                                          const char *path)
{
    if (root == NULL) return NULL;
    if (path == NULL || path[0] == '\0' || strcmp(path, "/") == 0) return root;
    char *copy = xstrdup(path);
    if (copy == NULL) return root;
    tam_sb_vfs_node_t *cur = root;
    char *save = NULL;
    for (char *tok = strtok_r(copy, "/", &save); tok != NULL;
         tok = strtok_r(NULL, "/", &save)) {
        if (strcmp(tok, ".") == 0 || tok[0] == '\0') continue;
        if (strcmp(tok, "..") == 0) {
            if (cur->parent != NULL) cur = cur->parent;
            continue;
        }
        tam_sb_vfs_node_t *next = find_child(cur, tok);
        if (next == NULL) next = tam_sb_vfs_add_dir(cur, tok, NULL, NULL);
        if (next == NULL || !next->is_dir) {
            free(copy);
            return NULL;
        }
        cur = next;
    }
    free(copy);
    return cur;
}

tam_sb_vfs_node_t *tam_sb_vfs_resolve(tam_sb_vfs_node_t *root,
                                      tam_sb_vfs_node_t *cwd,
                                      const char *path)
{
    if (root == NULL) return NULL;
    if (path == NULL || path[0] == '\0') return cwd ? cwd : root;
    tam_sb_vfs_node_t *cur = NULL;
    const char *p = path;
    if (p[0] == '/') {
        cur = root;
        while (*p == '/') p++;
    } else if (strcmp(p, "~") == 0 || strncmp(p, "~/", 2) == 0) {
        cur = tam_sb_vfs_resolve(root, root, "/home/alex");
        if (cur == NULL) cur = root;
        p += (p[1] == '/') ? 2 : 1;
    } else {
        cur = cwd ? cwd : root;
    }
    if (*p == '\0') return cur;
    char *copy = xstrdup(p);
    if (copy == NULL) return NULL;
    char *save = NULL;
    for (char *tok = strtok_r(copy, "/", &save); tok != NULL;
         tok = strtok_r(NULL, "/", &save)) {
        if (strcmp(tok, ".") == 0) continue;
        if (strcmp(tok, "..") == 0) {
            if (cur->parent != NULL) cur = cur->parent;
            continue;
        }
        cur = find_child(cur, tok);
        if (cur == NULL) {
            free(copy);
            return NULL;
        }
    }
    free(copy);
    return cur;
}

tam_sb_vfs_node_t *tam_sb_vfs_clone(const tam_sb_vfs_node_t *node,
                                    tam_sb_vfs_node_t *parent)
{
    if (node == NULL) return NULL;
    tam_sb_vfs_node_t *c = calloc(1, sizeof(*c));
    if (c == NULL) return NULL;
    c->name = xstrdup(node->name);
    c->is_dir = node->is_dir;
    c->mode = xstrdup(node->mode);
    c->owner = xstrdup(node->owner);
    c->size = node->size;
    c->content = xstrdup(node->content);
    c->parent = parent;
    if (node->nchildren > 0) {
        c->children = calloc(node->nchildren, sizeof(*c->children));
        if (c->children != NULL) {
            for (size_t i = 0; i < node->nchildren; i++) {
                tam_sb_vfs_node_t *ch = tam_sb_vfs_clone(node->children[i], c);
                if (ch != NULL) c->children[c->nchildren++] = ch;
            }
        }
    }
    return c;
}

void tam_sb_vfs_free(tam_sb_vfs_node_t *node)
{
    if (node == NULL) return;
    for (size_t i = 0; i < node->nchildren; i++)
        tam_sb_vfs_free(node->children[i]);
    free(node->children);
    free(node->name);
    free(node->mode);
    free(node->owner);
    free(node->content);
    free(node);
}

void tam_sandbox_spec_free(tam_sandbox_spec_t *spec)
{
    if (spec == NULL) return;
    free(spec->title);
    free(spec->prompt);
    free(spec->cwd);
    free(spec->goal);
    free(spec->fallback);
    free(spec->init_text);
    tam_sb_vfs_free(spec->init_vfs);
    for (size_t i = 0; i < spec->nrules; i++) {
        free(spec->rules[i].match);
        free(spec->rules[i].out);
        free(spec->rules[i].err);
        free(spec->rules[i].note);
        free(spec->rules[i].cwd);
    }
    free(spec->rules);
    for (size_t i = 0; i < spec->nhints; i++)
        free(spec->hints[i].text);
    free(spec->hints);
    memset(spec, 0, sizeof(*spec));
}

static void vfs_pwd_str(const tam_sb_vfs_node_t *node, sbuf_t *out)
{
    if (node == NULL || node->parent == NULL) {
        sb_puts(out, "/");
        return;
    }
    if (node->parent->parent != NULL) {
        vfs_pwd_str(node->parent, out);
        sb_putc(out, '/');
    } else {
        sb_putc(out, '/');
    }
    sb_puts(out, node->name);
}

static void log_push(tam_sandbox_state_t *st, tam_sb_log_kind_t kind,
                     const char *prompt, const char *text)
{
    if (text == NULL) return;
    tam_sb_log_entry_t *arr = realloc(st->log,
                                      (st->nlog + 1) * sizeof(*arr));
    if (arr == NULL) return;
    st->log = arr;
    st->log[st->nlog].kind = kind;
    st->log[st->nlog].prompt = xstrdup(prompt);
    st->log[st->nlog].text = xstrdup(text);
    st->nlog++;
}

static void init_vfs_and_cwd(tam_sandbox_state_t *st)
{
    const tam_sandbox_spec_t *sp = st->spec;
    if (sp->init_vfs != NULL) {
        st->vfs_root = tam_sb_vfs_clone(sp->init_vfs, NULL);
    } else {
        st->vfs_root = tam_sb_vfs_new_dir("/", NULL, "root");
    }
    const char *init_cwd = (sp->cwd && sp->cwd[0]) ? sp->cwd : "/home/alex";
    st->cwd_node = tam_sb_vfs_ensure_path(st->vfs_root, init_cwd);
    if (st->cwd_node == NULL) st->cwd_node = st->vfs_root;
    sbuf_t b = {0};
    vfs_pwd_str(st->cwd_node, &b);
    free(st->cwd);
    st->cwd = b.s ? b.s : xstrdup("/");
    if (sp->init_text != NULL && sp->init_text[0] != '\0') {
        log_push(st, TAM_SB_LOG_OUT, NULL, sp->init_text);
    }
}

void tam_sandbox_state_init(tam_sandbox_state_t *st,
                            const tam_sandbox_spec_t *spec)
{
    memset(st, 0, sizeof(*st));
    st->spec = spec;
    st->hist_pos = -1;
    init_vfs_and_cwd(st);
}

void tam_sandbox_state_reset(tam_sandbox_state_t *st)
{
    if (st == NULL || st->spec == NULL) return;
    for (size_t i = 0; i < st->nlog; i++) {
        free(st->log[i].prompt);
        free(st->log[i].text);
    }
    free(st->log);
    st->log = NULL;
    st->nlog = 0;
    tam_sb_vfs_free(st->vfs_root);
    st->vfs_root = NULL;
    st->cwd_node = NULL;
    st->input[0] = '\0';
    st->input_len = 0;
    st->cursor = 0;
    st->commands_run = 0;
    st->goal_done = 0;
    st->last_exit = 0;
    st->hist_pos = -1;
    init_vfs_and_cwd(st);
}

void tam_sandbox_state_free(tam_sandbox_state_t *st)
{
    if (st == NULL) return;
    tam_sb_vfs_free(st->vfs_root);
    free(st->cwd);
    for (size_t i = 0; i < st->nhistory; i++) free(st->history[i]);
    free(st->history);
    for (size_t i = 0; i < st->nlog; i++) {
        free(st->log[i].prompt);
        free(st->log[i].text);
    }
    free(st->log);
    memset(st, 0, sizeof(*st));
}

static int glob_match(const char *pat, const char *str)
{
    if (pat == NULL || str == NULL) return 0;
    while (*pat != '\0') {
        if (*pat == '*') {
            while (*pat == '*') pat++;
            if (*pat == '\0') return 1;
            for (const char *s = str; *s != '\0'; s++) {
                if (glob_match(pat, s)) return 1;
            }
            return 0;
        } else if (*pat == '?') {
            if (*str == '\0') return 0;
            pat++;
            str++;
        } else {
            if (*pat != *str) return 0;
            pat++;
            str++;
        }
    }
    return *str == '\0';
}

static char *trim_copy(const char *s)
{
    if (s == NULL) return xstrdup("");
    while (isspace((unsigned char)*s)) s++;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) len--;
    char *r = malloc(len + 1);
    if (r == NULL) return NULL;
    memcpy(r, s, len);
    r[len] = '\0';
    return r;
}

static int split_argv(const char *cmd, char ***out_argv, size_t *out_argc)
{
    size_t cap = 8, argc = 0;
    char **argv = calloc(cap, sizeof(*argv));
    if (argv == NULL) return -1;
    const char *p = cmd;
    while (*p != '\0') {
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;
        sbuf_t tok = {0};
        while (*p != '\0' && !isspace((unsigned char)*p)) {
            if (*p == '\'') {
                p++;
                while (*p != '\0' && *p != '\'') sb_putc(&tok, *p++);
                if (*p == '\'') p++;
            } else if (*p == '"') {
                p++;
                while (*p != '\0' && *p != '"') {
                    if (*p == '\\' && p[1] != '\0') p++;
                    sb_putc(&tok, *p++);
                }
                if (*p == '"') p++;
            } else if (*p == '\\' && p[1] != '\0') {
                p++;
                sb_putc(&tok, *p++);
            } else {
                sb_putc(&tok, *p++);
            }
        }
        if (argc + 1 >= cap) {
            cap *= 2;
            char **narr = realloc(argv, cap * sizeof(*narr));
            if (narr == NULL) {
                free(tok.s);
                for (size_t i = 0; i < argc; i++) free(argv[i]);
                free(argv);
                return -1;
            }
            argv = narr;
        }
        argv[argc++] = tok.s ? tok.s : xstrdup("");
    }
    argv[argc] = NULL;
    *out_argv = argv;
    *out_argc = argc;
    return 0;
}

static void free_argv(char **argv, size_t argc)
{
    for (size_t i = 0; i < argc; i++) free(argv[i]);
    free(argv);
}

static void split_parent_base(const char *path, char **out_parent,
                              char **out_base)
{
    const char *slash = strrchr(path, '/');
    if (slash == NULL) {
        *out_parent = xstrdup(".");
        *out_base = xstrdup(path);
    } else if (slash == path) {
        *out_parent = xstrdup("/");
        *out_base = xstrdup(slash + 1);
    } else {
        size_t plen = (size_t)(slash - path);
        *out_parent = malloc(plen + 1);
        if (*out_parent) {
            memcpy(*out_parent, path, plen);
            (*out_parent)[plen] = '\0';
        }
        *out_base = xstrdup(slash + 1);
    }
}

static int cmp_node_name(const void *a, const void *b)
{
    const tam_sb_vfs_node_t *na = *(const tam_sb_vfs_node_t *const *)a;
    const tam_sb_vfs_node_t *nb = *(const tam_sb_vfs_node_t *const *)b;
    return strcmp(na->name, nb->name);
}

static int cmp_str_ptr(const void *a, const void *b)
{
    const char *sa = *(const char *const *)a;
    const char *sb = *(const char *const *)b;
    return strcmp(sa, sb);
}

static void run_tree_rec(const tam_sb_vfs_node_t *node, const char *prefix,
                         sbuf_t *out)
{
    if (node == NULL || !node->is_dir) return;
    for (size_t i = 0; i < node->nchildren; i++) {
        const tam_sb_vfs_node_t *ch = node->children[i];
        int last = (i + 1 == node->nchildren);
        sb_puts(out, prefix);
        sb_puts(out, last ? "`-- " : "|-- ");
        sb_puts(out, ch->name);
        if (ch->is_dir) sb_putc(out, '/');
        sb_putc(out, '\n');
        if (ch->is_dir) {
            sbuf_t np = {0};
            sb_puts(&np, prefix);
            sb_puts(&np, last ? "    " : "|   ");
            run_tree_rec(ch, np.s ? np.s : "", out);
            free(np.s);
        }
    }
}

static void run_find_rec(const tam_sb_vfs_node_t *node, const char *path,
                         const char *name_pat, sbuf_t *out)
{
    if (node == NULL) return;
    if (name_pat == NULL || glob_match(name_pat, node->name)) {
        sb_puts(out, path);
        sb_putc(out, '\n');
    }
    if (!node->is_dir) return;
    for (size_t i = 0; i < node->nchildren; i++) {
        sbuf_t np = {0};
        sb_puts(&np, path);
        if (strcmp(path, "/") != 0) sb_putc(&np, '/');
        sb_puts(&np, node->children[i]->name);
        run_find_rec(node->children[i], np.s ? np.s : "", name_pat, out);
        free(np.s);
    }
}

static int grep_line_matches(const char *line, const char *pat, int icase)
{
    if (pat == NULL || pat[0] == '\0') return 1;
    size_t plen = strlen(pat);
    size_t llen = strlen(line);
    int anch_start = (pat[0] == '^');
    int anch_end = (plen > 0 && pat[plen - 1] == '$' &&
                    !(plen == 1 && anch_start));
    const char *core = pat + (anch_start ? 1 : 0);
    size_t clen = plen - (anch_start ? 1 : 0) - (anch_end ? 1 : 0);
    if (anch_start && anch_end) {
        if (llen != clen) return 0;
        for (size_t i = 0; i < clen; i++) {
            char a = icase ? (char)tolower((unsigned char)line[i]) : line[i];
            char b = icase ? (char)tolower((unsigned char)core[i]) : core[i];
            if (a != b) return 0;
        }
        return 1;
    }
    if (anch_start) {
        if (llen < clen) return 0;
        for (size_t i = 0; i < clen; i++) {
            char a = icase ? (char)tolower((unsigned char)line[i]) : line[i];
            char b = icase ? (char)tolower((unsigned char)core[i]) : core[i];
            if (a != b) return 0;
        }
        return 1;
    }
    if (anch_end) {
        if (llen < clen) return 0;
        size_t off = llen - clen;
        for (size_t i = 0; i < clen; i++) {
            char a = icase ? (char)tolower((unsigned char)line[off + i]) : line[off + i];
            char b = icase ? (char)tolower((unsigned char)core[i]) : core[i];
            if (a != b) return 0;
        }
        return 1;
    }
    if (clen == 0) return 1;
    for (size_t off = 0; off + clen <= llen; off++) {
        size_t i = 0;
        for (; i < clen; i++) {
            char a = icase ? (char)tolower((unsigned char)line[off + i]) : line[off + i];
            char b = icase ? (char)tolower((unsigned char)core[i]) : core[i];
            if (a != b) break;
        }
        if (i == clen) return 1;
    }
    return 0;
}

static int exec_single_cmd(tam_sandbox_state_t *st, const char *cmd_str,
                           const char *stdin_data, sbuf_t *out, sbuf_t *err,
                           int *handled)
{
    char **argv = NULL;
    size_t argc = 0;
    *handled = 1;
    if (split_argv(cmd_str, &argv, &argc) != 0 || argc == 0) {
        free_argv(argv, argc);
        return 0;
    }
    const char *cmd = argv[0];
    int rc = 0;

    if (strcmp(cmd, "pwd") == 0) {
        sb_puts(out, st->cwd);
        sb_putc(out, '\n');
    } else if (strcmp(cmd, "whoami") == 0) {
        sb_puts(out, "alex\n");
    } else if (strcmp(cmd, "uname") == 0) {
        if (argc > 1 && strcmp(argv[1], "-a") == 0)
            sb_puts(out, "Linux tamlinux 6.12.0-arch1-1 #1 SMP PREEMPT_DYNAMIC x86_64 GNU/Linux\n");
        else
            sb_puts(out, "Linux\n");
    } else if (strcmp(cmd, "clear") == 0) {
        for (size_t i = 0; i < st->nlog; i++) {
            free(st->log[i].prompt);
            free(st->log[i].text);
        }
        free(st->log);
        st->log = NULL;
        st->nlog = 0;
    } else if (strcmp(cmd, "reset") == 0) {
        tam_sandbox_state_reset(st);
        sb_puts(out, "Sandbox reset to initial state.\n");
    } else if (strcmp(cmd, "help") == 0) {
        sb_puts(out, "Simulated shell commands: ls, cd, pwd, cat, echo, printf, grep, wc, sort, uniq, head, tail, find, tree, mkdir, touch, rm, clear, reset, help.\n");
        if (st->spec->nrules > 0) {
            sb_puts(out, "Lesson commands:");
            for (size_t i = 0; i < st->spec->nrules; i++) {
                sb_puts(out, i == 0 ? " " : ", ");
                sb_puts(out, st->spec->rules[i].match);
            }
            sb_putc(out, '\n');
        }
    } else if (strcmp(cmd, "cd") == 0) {
        const char *target = argc > 1 ? argv[1] : "/home/alex";
        tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, target);
        if (n == NULL) {
            sb_puts(err, "cd: ");
            sb_puts(err, target);
            sb_puts(err, ": No such file or directory\n");
            rc = 1;
        } else if (!n->is_dir) {
            sb_puts(err, "cd: ");
            sb_puts(err, target);
            sb_puts(err, ": Not a directory\n");
            rc = 1;
        } else {
            st->cwd_node = n;
            sbuf_t b = {0};
            vfs_pwd_str(n, &b);
            free(st->cwd);
            st->cwd = b.s ? b.s : xstrdup("/");
        }
    } else if (strcmp(cmd, "ls") == 0) {
        int show_all = 0, long_fmt = 0, one_per_line = 0;
        const char *target = NULL;
        for (size_t i = 1; i < argc; i++) {
            if (argv[i][0] == '-' && argv[i][1] != '\0') {
                for (const char *f = argv[i] + 1; *f != '\0'; f++) {
                    if (*f == 'a') show_all = 1;
                    else if (*f == 'l') long_fmt = 1;
                    else if (*f == '1') one_per_line = 1;
                }
            } else {
                target = argv[i];
            }
        }
        tam_sb_vfs_node_t *n = target
            ? tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, target)
            : st->cwd_node;
        if (n == NULL) {
            sb_puts(err, "ls: cannot access '");
            sb_puts(err, target);
            sb_puts(err, "': No such file or directory\n");
            rc = 2;
        } else if (!n->is_dir) {
            if (long_fmt) {
                char line[128];
                snprintf(line, sizeof(line), "%s 1 %-6s %-6s %5zu %s\n",
                         n->mode, n->owner, n->owner, n->size, n->name);
                sb_puts(out, line);
            } else {
                sb_puts(out, n->name);
                sb_putc(out, '\n');
            }
        } else {
            size_t count = n->nchildren;
            tam_sb_vfs_node_t **sorted = NULL;
            if (count > 0) {
                sorted = malloc(count * sizeof(*sorted));
                if (sorted != NULL) {
                    memcpy(sorted, n->children, count * sizeof(*sorted));
                    qsort(sorted, count, sizeof(*sorted), cmp_node_name);
                }
            }
            if (long_fmt && show_all) {
                sb_puts(out, "drwxr-xr-x 2 alex   alex    4096 .\n");
                sb_puts(out, "drwxr-xr-x 2 root   root    4096 ..\n");
            } else if (show_all) {
                sb_puts(out, one_per_line ? ".\n..\n" : ".  ..  ");
            }
            size_t shown = 0;
            for (size_t i = 0; sorted && i < count; i++) {
                tam_sb_vfs_node_t *ch = sorted[i];
                if (!show_all && ch->name[0] == '.') continue;
                if (long_fmt) {
                    char line[160];
                    snprintf(line, sizeof(line), "%s 1 %-6s %-6s %5zu %s%s\n",
                             ch->mode, ch->owner, ch->owner, ch->size,
                             ch->name, ch->is_dir ? "/" : "");
                    sb_puts(out, line);
                } else {
                    if (shown > 0 && !one_per_line) sb_puts(out, "  ");
                    sb_puts(out, ch->name);
                    if (ch->is_dir) sb_putc(out, '/');
                    if (one_per_line) sb_putc(out, '\n');
                }
                shown++;
            }
            if (!long_fmt && !one_per_line && (shown > 0 || show_all))
                sb_putc(out, '\n');
            free(sorted);
        }
    } else if (strcmp(cmd, "echo") == 0) {
        int no_nl = 0;
        size_t start = 1;
        if (argc > 1 && strcmp(argv[1], "-n") == 0) {
            no_nl = 1;
            start = 2;
        }
        for (size_t i = start; i < argc; i++) {
            if (i > start) sb_putc(out, ' ');
            sb_puts(out, argv[i]);
        }
        if (!no_nl) sb_putc(out, '\n');
    } else if (strcmp(cmd, "printf") == 0) {
        if (argc >= 2) {
            const char *fmt = argv[1];
            if (strcmp(fmt, "%s\\n") == 0 || strcmp(fmt, "%s\n") == 0) {
                for (size_t i = 2; i < argc; i++) {
                    sb_puts(out, argv[i]);
                    sb_putc(out, '\n');
                }
            } else {
                size_t ai = 2;
                for (const char *p = fmt; *p != '\0'; p++) {
                    if (*p == '\\' && p[1] == 'n') {
                        sb_putc(out, '\n');
                        p++;
                    } else if (*p == '\\' && p[1] == 't') {
                        sb_putc(out, '\t');
                        p++;
                    } else if (*p == '%' && p[1] == 's') {
                        if (ai < argc) sb_puts(out, argv[ai++]);
                        p++;
                    } else {
                        sb_putc(out, *p);
                    }
                }
            }
        }
    } else if (strcmp(cmd, "cat") == 0) {
        if (argc == 1) {
            if (stdin_data != NULL) sb_puts(out, stdin_data);
        } else {
            for (size_t i = 1; i < argc; i++) {
                tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, argv[i]);
                if (n == NULL) {
                    sb_puts(err, "cat: ");
                    sb_puts(err, argv[i]);
                    sb_puts(err, ": No such file or directory\n");
                    rc = 1;
                } else if (n->is_dir) {
                    sb_puts(err, "cat: ");
                    sb_puts(err, argv[i]);
                    sb_puts(err, ": Is a directory\n");
                    rc = 1;
                } else {
                    sb_puts(out, n->content ? n->content : "");
                    if (n->content && n->content[0] != '\0' &&
                        n->content[strlen(n->content) - 1] != '\n')
                        sb_putc(out, '\n');
                }
            }
        }
    } else if (strcmp(cmd, "grep") == 0) {
        int icase = 0, invert = 0;
        const char *pat = NULL;
        const char *fname = NULL;
        for (size_t i = 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                for (const char *f = argv[i] + 1; *f; f++) {
                    if (*f == 'i') icase = 1;
                    else if (*f == 'v') invert = 1;
                }
            } else if (pat == NULL) {
                pat = argv[i];
            } else {
                fname = argv[i];
            }
        }
        const char *src = stdin_data;
        if (fname != NULL) {
            tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, fname);
            if (n == NULL || n->is_dir) {
                sb_puts(err, "grep: ");
                sb_puts(err, fname);
                sb_puts(err, ": No such file\n");
                rc = 2;
                src = NULL;
            } else {
                src = n->content;
            }
        }
        if (src != NULL && pat != NULL) {
            int matched_any = 0;
            char *copy = xstrdup(src);
            char *p = copy;
            while (p != NULL && *p != '\0') {
                char *nl = strchr(p, '\n');
                if (nl != NULL) *nl = '\0';
                int m = grep_line_matches(p, pat, icase);
                if (invert) m = !m;
                if (m) {
                    sb_puts(out, p);
                    sb_putc(out, '\n');
                    matched_any = 1;
                }
                p = nl ? nl + 1 : NULL;
            }
            free(copy);
            if (!matched_any) rc = 1;
        }
    } else if (strcmp(cmd, "wc") == 0) {
        int count_l = 0, count_w = 0, count_c = 0;
        const char *fname = NULL;
        for (size_t i = 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                for (const char *f = argv[i] + 1; *f; f++) {
                    if (*f == 'l') count_l = 1;
                    else if (*f == 'w') count_w = 1;
                    else if (*f == 'c') count_c = 1;
                }
            } else {
                fname = argv[i];
            }
        }
        if (!count_l && !count_w && !count_c) count_l = count_w = count_c = 1;
        const char *src = stdin_data ? stdin_data : "";
        if (fname != NULL) {
            tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, fname);
            if (n == NULL || n->is_dir) {
                sb_puts(err, "wc: ");
                sb_puts(err, fname);
                sb_puts(err, ": No such file\n");
                rc = 1;
                src = NULL;
            } else {
                src = n->content ? n->content : "";
            }
        }
        if (src != NULL) {
            size_t lines = 0, words = 0, bytes = strlen(src);
            int in_w = 0;
            for (size_t i = 0; i < bytes; i++) {
                if (src[i] == '\n') lines++;
                if (isspace((unsigned char)src[i])) in_w = 0;
                else if (!in_w) { in_w = 1; words++; }
            }
            char buf[96];
            if (count_l && !count_w && !count_c)
                snprintf(buf, sizeof(buf), "%zu\n", lines);
            else if (!count_l && count_w && !count_c)
                snprintf(buf, sizeof(buf), "%zu\n", words);
            else if (!count_l && !count_w && count_c)
                snprintf(buf, sizeof(buf), "%zu\n", bytes);
            else
                snprintf(buf, sizeof(buf), "%zu %zu %zu\n", lines, words, bytes);
            sb_puts(out, buf);
        }
    } else if (strcmp(cmd, "head") == 0 || strcmp(cmd, "tail") == 0) {
        int is_tail = (strcmp(cmd, "tail") == 0);
        size_t nlines = 10;
        const char *fname = NULL;
        for (size_t i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
                nlines = (size_t)atoi(argv[++i]);
            } else if (argv[i][0] == '-' && isdigit((unsigned char)argv[i][1])) {
                nlines = (size_t)atoi(argv[i] + 1);
            } else {
                fname = argv[i];
            }
        }
        const char *src = stdin_data;
        if (fname != NULL) {
            tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, fname);
            if (n == NULL || n->is_dir) {
                sb_puts(err, cmd);
                sb_puts(err, ": cannot open file\n");
                rc = 1;
                src = NULL;
            } else {
                src = n->content;
            }
        }
        if (src != NULL) {
            size_t total = 0;
            for (const char *p = src; *p; p++) if (*p == '\n') total++;
            size_t skip = (is_tail && total > nlines) ? total - nlines : 0;
            size_t printed = 0, idx = 0;
            char *copy = xstrdup(src);
            char *p = copy;
            while (p != NULL && *p != '\0') {
                char *nl = strchr(p, '\n');
                if (nl != NULL) *nl = '\0';
                if (!is_tail) {
                    if (printed < nlines) {
                        sb_puts(out, p);
                        sb_putc(out, '\n');
                        printed++;
                    }
                } else {
                    if (idx >= skip) {
                        sb_puts(out, p);
                        sb_putc(out, '\n');
                    }
                }
                idx++;
                p = nl ? nl + 1 : NULL;
            }
            free(copy);
        }
    } else if (strcmp(cmd, "sort") == 0 || strcmp(cmd, "uniq") == 0) {
        int is_uniq = (strcmp(cmd, "uniq") == 0);
        const char *src = stdin_data;
        if (argc > 1) {
            tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, argv[1]);
            if (n != NULL && !n->is_dir) src = n->content;
        }
        if (src != NULL) {
            size_t cap = 16, nlines = 0;
            char **lines = calloc(cap, sizeof(*lines));
            char *copy = xstrdup(src);
            char *p = copy;
            while (p != NULL && *p != '\0') {
                char *nl = strchr(p, '\n');
                if (nl != NULL) *nl = '\0';
                if (nlines >= cap) {
                    cap *= 2;
                    lines = realloc(lines, cap * sizeof(*lines));
                }
                lines[nlines++] = xstrdup(p);
                p = nl ? nl + 1 : NULL;
            }
            free(copy);
            if (!is_uniq && nlines > 1)
                qsort(lines, nlines, sizeof(*lines), cmp_str_ptr);
            for (size_t i = 0; i < nlines; i++) {
                if (is_uniq && i > 0 && strcmp(lines[i], lines[i - 1]) == 0)
                    continue;
                sb_puts(out, lines[i]);
                sb_putc(out, '\n');
            }
            for (size_t i = 0; i < nlines; i++) free(lines[i]);
            free(lines);
        }
    } else if (strcmp(cmd, "mkdir") == 0) {
        for (size_t i = 1; i < argc; i++) {
            if (argv[i][0] == '-') continue;
            char *parent_path = NULL, *base = NULL;
            split_parent_base(argv[i], &parent_path, &base);
            tam_sb_vfs_node_t *pnode = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, parent_path);
            if (pnode == NULL || !pnode->is_dir || base[0] == '\0') {
                sb_puts(err, "mkdir: cannot create directory '");
                sb_puts(err, argv[i]);
                sb_puts(err, "'\n");
                rc = 1;
            } else {
                tam_sb_vfs_add_dir(pnode, base, NULL, NULL);
            }
            free(parent_path);
            free(base);
        }
    } else if (strcmp(cmd, "touch") == 0) {
        for (size_t i = 1; i < argc; i++) {
            if (argv[i][0] == '-') continue;
            char *parent_path = NULL, *base = NULL;
            split_parent_base(argv[i], &parent_path, &base);
            tam_sb_vfs_node_t *pnode = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, parent_path);
            if (pnode == NULL || !pnode->is_dir || base[0] == '\0') {
                sb_puts(err, "touch: cannot touch '");
                sb_puts(err, argv[i]);
                sb_puts(err, "'\n");
                rc = 1;
            } else if (find_child(pnode, base) == NULL) {
                tam_sb_vfs_add_file(pnode, base, NULL, NULL, 0, "");
            }
            free(parent_path);
            free(base);
        }
    } else if (strcmp(cmd, "rm") == 0) {
        int rec = 0;
        for (size_t i = 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                if (strchr(argv[i], 'r') || strchr(argv[i], 'R')) rec = 1;
                continue;
            }
            tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, argv[i]);
            if (n == NULL || n == st->vfs_root) {
                sb_puts(err, "rm: cannot remove '");
                sb_puts(err, argv[i]);
                sb_puts(err, "'\n");
                rc = 1;
            } else if (n->is_dir && !rec) {
                sb_puts(err, "rm: cannot remove '");
                sb_puts(err, argv[i]);
                sb_puts(err, "': Is a directory\n");
                rc = 1;
            } else if (n->parent != NULL) {
                tam_sb_vfs_node_t *p = n->parent;
                for (size_t k = 0; k < p->nchildren; k++) {
                    if (p->children[k] == n) {
                        memmove(&p->children[k], &p->children[k + 1],
                                (p->nchildren - k - 1) * sizeof(*p->children));
                        p->nchildren--;
                        break;
                    }
                }
                tam_sb_vfs_free(n);
            }
        }
    } else if (strcmp(cmd, "tree") == 0) {
        const char *target = argc > 1 ? argv[1] : ".";
        tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, target);
        if (n == NULL) {
            sb_puts(err, "tree: not found\n");
            rc = 1;
        } else {
            sb_puts(out, target);
            sb_putc(out, '\n');
            run_tree_rec(n, "", out);
        }
    } else if (strcmp(cmd, "find") == 0) {
        const char *target = ".";
        const char *name_pat = NULL;
        for (size_t i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-name") == 0 && i + 1 < argc) {
                name_pat = argv[++i];
            } else if (argv[i][0] != '-') {
                target = argv[i];
            }
        }
        tam_sb_vfs_node_t *n = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, target);
        if (n == NULL) {
            sb_puts(err, "find: not found\n");
            rc = 1;
        } else {
            run_find_rec(n, target, name_pat, out);
        }
    } else {
        *handled = 0;
        rc = 127;
    }

    free_argv(argv, argc);
    return rc;
}

static size_t split_pipeline(const char *line, char ***out_stages,
                             char **out_redir_file, int *out_append)
{
    *out_redir_file = NULL;
    *out_append = 0;
    size_t cap = 4, n = 0;
    char **stages = calloc(cap, sizeof(*stages));
    if (stages == NULL) return 0;

    sbuf_t cur = {0};
    int sq = 0, dq = 0;
    for (const char *p = line; *p != '\0'; p++) {
        if (*p == '\'' && !dq) { sq = !sq; sb_putc(&cur, *p); continue; }
        if (*p == '"' && !sq) { dq = !dq; sb_putc(&cur, *p); continue; }
        if (*p == '\\' && !sq && p[1] != '\0') {
            sb_putc(&cur, *p++);
            sb_putc(&cur, *p);
            continue;
        }
        if (!sq && !dq && *p == '|') {
            if (n >= cap) {
                cap *= 2;
                stages = realloc(stages, cap * sizeof(*stages));
            }
            stages[n++] = trim_copy(cur.s ? cur.s : "");
            cur.n = 0;
            if (cur.s) cur.s[0] = '\0';
            continue;
        }
        if (!sq && !dq && *p == '>') {
            *out_append = (p[1] == '>');
            if (*out_append) p++;
            p++;
            while (isspace((unsigned char)*p)) p++;
            *out_redir_file = trim_copy(p);
            break;
        }
        sb_putc(&cur, *p);
    }
    if (n >= cap) {
        cap *= 2;
        stages = realloc(stages, cap * sizeof(*stages));
    }
    stages[n++] = trim_copy(cur.s ? cur.s : "");
    free(cur.s);
    *out_stages = stages;
    return n;
}

void tam_sandbox_exec(tam_sandbox_state_t *st, const char *line)
{
    if (st == NULL || st->spec == NULL || line == NULL) return;
    char *trimmed = trim_copy(line);
    if (trimmed == NULL || trimmed[0] == '\0') {
        free(trimmed);
        return;
    }

    const char *prompt = (st->spec->prompt && st->spec->prompt[0])
        ? st->spec->prompt : "$ ";
    log_push(st, TAM_SB_LOG_CMD, prompt, trimmed);

    if (st->nhistory == 0 || strcmp(st->history[st->nhistory - 1], trimmed) != 0) {
        char **nh = realloc(st->history, (st->nhistory + 1) * sizeof(*nh));
        if (nh != NULL) {
            st->history = nh;
            st->history[st->nhistory++] = xstrdup(trimmed);
        }
    }
    st->hist_pos = -1;
    st->commands_run++;

    /* 1. Check teacher-scripted <tam-cmd> rules first. */
    for (size_t i = 0; i < st->spec->nrules; i++) {
        const tam_sb_cmd_rule_t *r = &st->spec->rules[i];
        if (glob_match(r->match, trimmed)) {
            if (r->out && r->out[0]) log_push(st, TAM_SB_LOG_OUT, NULL, r->out);
            if (r->err && r->err[0]) log_push(st, TAM_SB_LOG_ERR, NULL, r->err);
            if (r->cwd && r->cwd[0]) {
                tam_sb_vfs_node_t *nc = tam_sb_vfs_ensure_path(st->vfs_root, r->cwd);
                if (nc != NULL) {
                    st->cwd_node = nc;
                    free(st->cwd);
                    st->cwd = xstrdup(r->cwd);
                }
            }
            if (r->is_goal && !st->goal_done) {
                st->goal_done = 1;
                log_push(st, TAM_SB_LOG_NOTE, NULL, "[Done] Goal completed!");
            }
            if (r->note && r->note[0]) log_push(st, TAM_SB_LOG_NOTE, NULL, r->note);
            st->last_exit = r->exit_code;
            free(trimmed);
            return;
        }
    }

    /* 2. Run built-in simulated pipeline across VFS. */
    char **stages = NULL;
    char *redir_file = NULL;
    int redir_append = 0;
    size_t nstages = split_pipeline(trimmed, &stages, &redir_file, &redir_append);
    char *pipe_in = NULL;
    sbuf_t all_err = {0};
    int all_handled = 1;
    int last_rc = 0;

    for (size_t i = 0; i < nstages; i++) {
        sbuf_t out = {0}, err = {0};
        int handled = 0;
        last_rc = exec_single_cmd(st, stages[i], pipe_in, &out, &err, &handled);
        if (err.s && err.s[0]) sb_puts(&all_err, err.s);
        free(err.s);
        free(pipe_in);
        pipe_in = out.s;
        if (!handled) {
            all_handled = 0;
            break;
        }
    }
    for (size_t i = 0; i < nstages; i++) free(stages[i]);
    free(stages);

    if (!all_handled) {
        free(pipe_in);
        free(all_err.s);
        free(redir_file);
        if (st->spec->fallback && st->spec->fallback[0]) {
            log_push(st, TAM_SB_LOG_NOTE, NULL, st->spec->fallback);
        } else {
            log_push(st, TAM_SB_LOG_ERR, NULL,
                     "Simulated shell: command not available here (type 'help').");
        }
        st->last_exit = 127;
    } else {
        if (all_err.s && all_err.s[0])
            log_push(st, TAM_SB_LOG_ERR, NULL, all_err.s);
        free(all_err.s);

        if (redir_file != NULL && redir_file[0] != '\0') {
            char *pp = NULL, *base = NULL;
            split_parent_base(redir_file, &pp, &base);
            tam_sb_vfs_node_t *pn = tam_sb_vfs_resolve(st->vfs_root, st->cwd_node, pp);
            if (pn != NULL && pn->is_dir && base[0] != '\0') {
                tam_sb_vfs_node_t *existing = find_child(pn, base);
                if (redir_append && existing != NULL && !existing->is_dir) {
                    sbuf_t nb = {0};
                    sb_puts(&nb, existing->content);
                    sb_puts(&nb, pipe_in ? pipe_in : "");
                    tam_sb_vfs_add_file(pn, base, NULL, NULL, 0, nb.s ? nb.s : "");
                    free(nb.s);
                } else {
                    tam_sb_vfs_add_file(pn, base, NULL, NULL, 0, pipe_in ? pipe_in : "");
                }
            } else {
                log_push(st, TAM_SB_LOG_ERR, NULL, "Cannot write redirected file.");
            }
            free(pp);
            free(base);
        } else if (pipe_in != NULL && pipe_in[0] != '\0') {
            log_push(st, TAM_SB_LOG_OUT, NULL, pipe_in);
        }
        free(pipe_in);
        free(redir_file);
        st->last_exit = last_rc;
    }

    /* 3. Progressive hints if goal not yet met. */
    if (!st->goal_done) {
        for (size_t i = 0; i < st->spec->nhints; i++) {
            if (st->spec->hints[i].after == st->commands_run &&
                st->spec->hints[i].text != NULL) {
                log_push(st, TAM_SB_LOG_NOTE, NULL, st->spec->hints[i].text);
            }
        }
    }
    free(trimmed);
}

size_t tam_sandbox_tokenize(const char *line, tam_sandbox_lesson_t lesson,
                            tam_sb_tok_t *out, size_t max_toks)
{
    if (line == NULL || max_toks == 0) return 0;
    size_t n = 0;
    size_t i = 0;
    size_t len = strlen(line);
    int expect_cmd = 1;

    while (i < len && n < max_toks) {
        if (isspace((unsigned char)line[i])) {
            size_t st = i;
            while (i < len && isspace((unsigned char)line[i])) i++;
            out[n++] = (tam_sb_tok_t){ st, i - st, TAM_STYLE_SB_USER };
            continue;
        }
        if (line[i] == '|' || line[i] == '>' || line[i] == '<' || line[i] == ';') {
            size_t st = i++;
            if ((line[st] == '>' && line[i] == '>') ||
                (line[st] == '|' && line[i] == '|')) i++;
            unsigned s = (lesson == TAM_LESSON_PIPES || lesson == TAM_LESSON_ALL)
                ? TAM_STYLE_SB_OP : TAM_STYLE_SB_USER;
            out[n++] = (tam_sb_tok_t){ st, i - st, s };
            expect_cmd = (line[st] == '|' || line[st] == ';');
            continue;
        }
        if (line[i] == '\'' || line[i] == '"') {
            char q = line[i];
            size_t st = i++;
            while (i < len && line[i] != q) {
                if (line[i] == '\\' && q == '"' && i + 1 < len) i++;
                i++;
            }
            if (i < len && line[i] == q) i++;
            unsigned s = (lesson == TAM_LESSON_VARIABLES || lesson == TAM_LESSON_ALL)
                ? TAM_STYLE_SB_VAR : TAM_STYLE_SB_USER;
            out[n++] = (tam_sb_tok_t){ st, i - st, s };
            expect_cmd = 0;
            continue;
        }
        size_t st = i;
        while (i < len && !isspace((unsigned char)line[i]) &&
               line[i] != '|' && line[i] != '>' && line[i] != '<' &&
               line[i] != ';') {
            i++;
        }
        size_t tlen = i - st;
        unsigned style = TAM_STYLE_SB_USER;
        if (expect_cmd) {
            if (lesson != TAM_LESSON_PLAIN) style = TAM_STYLE_SB_CMD;
            expect_cmd = 0;
        } else if (line[st] == '-' && tlen > 1 &&
                   (lesson == TAM_LESSON_FLAGS || lesson == TAM_LESSON_ALL)) {
            style = TAM_STYLE_SB_FLAG;
        } else if (line[st] == '$' &&
                   (lesson == TAM_LESSON_VARIABLES || lesson == TAM_LESSON_ALL)) {
            style = TAM_STYLE_SB_VAR;
        } else if ((strchr(line + st, '/') != NULL &&
                    (size_t)(strchr(line + st, '/') - line) < i) &&
                   (lesson == TAM_LESSON_PATHS || lesson == TAM_LESSON_ALL)) {
            style = TAM_STYLE_SB_PATH;
        }
        out[n++] = (tam_sb_tok_t){ st, tlen, style };
    }
    return n;
}

void tam_sandbox_insert_bytes(tam_sandbox_state_t *st, const char *bytes,
                              size_t nbytes)
{
    if (st == NULL || bytes == NULL || nbytes == 0) return;
    if (st->input_len + nbytes >= sizeof(st->input)) return;
    memmove(st->input + st->cursor + nbytes, st->input + st->cursor,
            st->input_len - st->cursor + 1);
    memcpy(st->input + st->cursor, bytes, nbytes);
    st->input_len += nbytes;
    st->cursor += nbytes;
}

void tam_sandbox_backspace(tam_sandbox_state_t *st)
{
    if (st == NULL || st->cursor == 0) return;
    size_t prev = st->cursor - 1;
    while (prev > 0 && (((unsigned char)st->input[prev] & 0xC0) == 0x80))
        prev--;
    size_t del = st->cursor - prev;
    memmove(st->input + prev, st->input + st->cursor,
            st->input_len - st->cursor + 1);
    st->input_len -= del;
    st->cursor = prev;
}

void tam_sandbox_history_move(tam_sandbox_state_t *st, int dir)
{
    if (st == NULL || st->nhistory == 0) return;
    if (dir < 0) {
        if (st->hist_pos < 0) st->hist_pos = (int)st->nhistory - 1;
        else if (st->hist_pos > 0) st->hist_pos--;
    } else {
        if (st->hist_pos < 0) return;
        if (st->hist_pos + 1 < (int)st->nhistory) st->hist_pos++;
        else {
            st->hist_pos = -1;
            st->input[0] = '\0';
            st->input_len = 0;
            st->cursor = 0;
            return;
        }
    }
    if (st->hist_pos >= 0 && (size_t)st->hist_pos < st->nhistory) {
        snprintf(st->input, sizeof(st->input), "%s", st->history[st->hist_pos]);
        st->input_len = strlen(st->input);
        st->cursor = st->input_len;
    }
}
