#ifndef TAM_SANDBOX_H
#define TAM_SANDBOX_H

#include <stddef.h>

typedef enum {
    TAM_LESSON_PLAIN = 0,
    TAM_LESSON_FLAGS,
    TAM_LESSON_PATHS,
    TAM_LESSON_PIPES,
    TAM_LESSON_VARIABLES,
    TAM_LESSON_ALL
} tam_sandbox_lesson_t;

typedef struct tam_sb_vfs_node {
    char *name;
    int is_dir;
    char *mode;
    char *owner;
    size_t size;
    char *content;
    struct tam_sb_vfs_node *parent;
    struct tam_sb_vfs_node **children;
    size_t nchildren;
} tam_sb_vfs_node_t;

typedef struct {
    char *match;
    char *out;
    char *err;
    char *note;
    char *cwd;
    int exit_code;
    int is_goal;
} tam_sb_cmd_rule_t;

typedef struct {
    int after;
    char *text;
} tam_sb_hint_t;

typedef struct {
    char *title;
    char *prompt;
    char *cwd;
    char *goal;
    char *fallback;
    char *init_text;
    tam_sandbox_lesson_t lesson;
    int rows;
    tam_sb_vfs_node_t *init_vfs;
    tam_sb_cmd_rule_t *rules;
    size_t nrules;
    tam_sb_hint_t *hints;
    size_t nhints;
} tam_sandbox_spec_t;

typedef enum {
    TAM_SB_LOG_CMD = 0,
    TAM_SB_LOG_OUT,
    TAM_SB_LOG_ERR,
    TAM_SB_LOG_NOTE
} tam_sb_log_kind_t;

typedef struct {
    tam_sb_log_kind_t kind;
    char *prompt;
    char *text;
} tam_sb_log_entry_t;

typedef struct {
    const tam_sandbox_spec_t *spec;
    tam_sb_vfs_node_t *vfs_root;
    tam_sb_vfs_node_t *cwd_node;
    char *cwd;
    char **history;
    size_t nhistory;
    int hist_pos;
    tam_sb_log_entry_t *log;
    size_t nlog;
    char input[512];
    size_t input_len;
    size_t cursor;
    int commands_run;
    int goal_done;
    int last_exit;
} tam_sandbox_state_t;

typedef struct {
    size_t start;
    size_t len;
    unsigned style;
} tam_sb_tok_t;

tam_sandbox_lesson_t tam_sandbox_parse_lesson(const char *s);
void tam_sandbox_spec_free(tam_sandbox_spec_t *spec);

tam_sb_vfs_node_t *tam_sb_vfs_new_dir(const char *name, const char *mode,
                                      const char *owner);
tam_sb_vfs_node_t *tam_sb_vfs_add_dir(tam_sb_vfs_node_t *parent,
                                      const char *name, const char *mode,
                                      const char *owner);
tam_sb_vfs_node_t *tam_sb_vfs_add_file(tam_sb_vfs_node_t *parent,
                                       const char *name, const char *mode,
                                       const char *owner, size_t size,
                                       const char *content);
tam_sb_vfs_node_t *tam_sb_vfs_ensure_path(tam_sb_vfs_node_t *root,
                                          const char *path);
tam_sb_vfs_node_t *tam_sb_vfs_resolve(tam_sb_vfs_node_t *root,
                                      tam_sb_vfs_node_t *cwd,
                                      const char *path);
tam_sb_vfs_node_t *tam_sb_vfs_clone(const tam_sb_vfs_node_t *node,
                                    tam_sb_vfs_node_t *parent);
void tam_sb_vfs_free(tam_sb_vfs_node_t *node);

void tam_sandbox_state_init(tam_sandbox_state_t *st,
                            const tam_sandbox_spec_t *spec);
void tam_sandbox_state_reset(tam_sandbox_state_t *st);
void tam_sandbox_state_free(tam_sandbox_state_t *st);

void tam_sandbox_exec(tam_sandbox_state_t *st, const char *line);
size_t tam_sandbox_tokenize(const char *line, tam_sandbox_lesson_t lesson,
                            tam_sb_tok_t *out, size_t max_toks);
void tam_sandbox_insert_bytes(tam_sandbox_state_t *st, const char *bytes,
                              size_t nbytes);
void tam_sandbox_backspace(tam_sandbox_state_t *st);
void tam_sandbox_history_move(tam_sandbox_state_t *st, int dir);

#endif /* TAM_SANDBOX_H */
