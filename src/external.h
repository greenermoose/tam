#ifndef TAM_EXTERNAL_H
#define TAM_EXTERNAL_H

#include <stddef.h>

/* External links. tam never fetches a web page itself: following one hands
 * the URL to the reader's own browser. With a graphical display that is
 * $BROWSER (a colon-separated list, %s standing for the URL) or xdg-open,
 * started detached; on a console it is $TAM_TEXT_BROWSER or the first of
 * w3m, lynx, elinks and links, run in this terminal while the pager waits.
 * No shell is involved: the URL is always one argument of its own. */

/* 1 for an http: or https: URL tam will hand off: at most 2048 bytes, no
 * spaces or control bytes. */
int tam_external_url_ok(const char *url);

/* 1 when WAYLAND_DISPLAY or DISPLAY names a graphical session. */
int tam_external_graphical(void);

/* The name of the program that would open a link now, into buf. Returns 0
 * when none is installed. */
int tam_external_opener(int graphical, char *buf, size_t cap);

/* What to tell the reader when no browser is installed. */
const char *tam_external_missing(int graphical);

/* Start the graphical browser detached from this terminal. Returns 0 once it
 * has started; otherwise -1, with the reason in msg. */
int tam_external_open_detached(const char *url, char *msg, size_t cap);

/* Run the text browser in the foreground on this terminal, reading from
 * tty_fd when standard input is not a terminal. The caller restores the
 * terminal before and after. Returns 0 when the browser ran, -1 otherwise;
 * msg says what happened either way. */
int tam_external_run_foreground(const char *url, int tty_fd,
                                char *msg, size_t cap);

#endif /* TAM_EXTERNAL_H */
