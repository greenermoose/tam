#ifndef TAM_PAGER_INT_H
#define TAM_PAGER_INT_H

/* Private to the pager. pager.c runs the loop; pager_draw.c paints the
 * screen and status row; pager_input.c reads keys; pager_links.c selects
 * links and moves the attention line; pager_nav.c follows links and keeps
 * history, folds and the lens; pager_search.c searches and prompts. */

#include <tam.h>

#include <signal.h>
#include <stddef.h>

#include "pager_screen.h"
#include "render.h"
#include "terminal.h"

extern volatile sig_atomic_t g_stop;
extern volatile sig_atomic_t g_resized;
extern tam_terminal_t *g_term;
extern tam_buf_t *g_capture;
extern tam_pager_screen_t g_screen;
extern int g_attention;
extern unsigned char g_pending[128];
extern size_t g_pending_len;
extern int g_last_width;
extern int g_last_height;

typedef struct {
    int               link_id;
    int               header;
    const char       *href;
    size_t            line_start;
    size_t            line_end;
    int               col_start;
    int               col_end;
    const tam_span_t *span;
} linkref_t;

typedef struct {
    size_t line_idx;
    size_t byte_start;
    size_t byte_end;
} search_hit_t;

typedef struct {
    char *href;
    char *selected_href;
    int selected_link_id;
    int selected_header;
    int   top;
    int   cursor_line;
    size_t top_block;
    size_t top_offset;
    size_t cursor_block;
    size_t cursor_offset;
    int width;
} nav_state_t;

typedef struct {
    tam_document_t *doc;
    char           *cur_href;
    linkref_t      *links;
    size_t          nlinks;
    nav_state_t    *history;
    size_t          nhistory;
    nav_state_t    *forward;
    size_t          nforward;
    int             top;
    int             selected;
    int             cursor_line;
    int             sweep_dir;
    int             width;
    int             layout_width;
    int             height;
    int             input_fd;
    char            msg[160];
    tam_page_loader_t load;
    void              *ctx;
    char            search_query[128];
    search_hit_t   *hits;
    size_t          nhits;
    int             cur_hit;
    int             sync_probe_sent;
    int             sync_queried;
    int             sync_output;
    int             help_open;
    int             column_active;
    int             show_address;
    unsigned        lens; /* The reader's level lens (depth.h). */
    char            version[64]; /* "v<command> (<corpus>)", right of the status row */
    unsigned char  *fold_open;
    size_t          nfold_open;
} pager_t;

int terminal_height(void);
void out(const char *s);
int content_rows(const pager_t *st);
int content_width(const pager_t *st);
void pager_build_links(pager_t *st);
const linkref_t *selected_link(const pager_t *st);
int link_is_visible(const pager_t *st, int link_idx);
int find_first_link_on_line(const pager_t *st, size_t line_idx);
void go_page_top(pager_t *st);
void set_cursor_line(pager_t *st, int new_line, int dir);
void pager_clear_search(pager_t *st);
void pager_perform_search(pager_t *st, const char *query);
void pager_next_hit(pager_t *st, int dir);
void pager_search_prompt(pager_t *st);
int pager_prompt_action(pager_t *st, const char *href, char *out,
                        size_t out_cap);
void pager_reflow(pager_t *st, int width);
int breadcrumb_focused(const pager_t *st);
int draw(pager_t *st);
int lens_href(const char *href);
tam_document_t *load_at_lens(pager_t *st, const char *href);
void follow(pager_t *st, const char *href);
void set_fold_here(pager_t *st, int open);
int hide_fold_here(pager_t *st);
void reveal_next(pager_t *st);
void toggle_all_folds(pager_t *st);
void set_lens(pager_t *st, unsigned char key);
void go_back(pager_t *st);
void go_forward(pager_t *st);
void select_next_link(pager_t *st, int dir, int include_header);
void move_line_up(pager_t *st);
void move_line_down(pager_t *st);
void move_column(pager_t *st, int dir);
void follow_selected(pager_t *st);
int read_byte(int fd, int timeout_ms, unsigned char *c);
void handle_key(pager_t *st, unsigned char c);

#endif /* TAM_PAGER_INT_H */
