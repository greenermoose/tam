#ifndef TAM_VERSION_H
#define TAM_VERSION_H

#define TAM_COMMAND_NAME            "tam"
#define TAM_COMMAND_VERSION         "0.39.1"
/* Reported only when a corpus has no readable version file. knowledge/version
 * is the corpus version's single authority, so a corpus release never edits
 * this file. */
#define TAM_DEFAULT_CORPUS_VERSION  "unversioned"

#include <stdbool.h>

char *cultivar_version(void);
char *cultivar_name(void);
char *cultivar_label(void);
char *cultivar_path(const char *leaf);
/* Path of `leaf` in tam's state directory, $XDG_STATE_HOME/tamlinux/tam
 * (fallback ~/.local/state/tamlinux/tam). With `create`, the directory is
 * made first. NULL when no home is known or it cannot be made. */
char *tam_state_path(const char *leaf, bool create);
char *cultivar_user(void);
bool cultivar_for_root(void);
char *tam_corpus_version(const char *root);
char *tam_corpus_build_id(const char *root);

#endif /* TAM_VERSION_H */
