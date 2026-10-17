#ifndef TAM_VERSION_H
#define TAM_VERSION_H

#define TAM_COMMAND_NAME            "tam"
#define TAM_COMMAND_VERSION         "0.29.3"
/* Reported only when a corpus has no readable version file. knowledge/version
 * is the corpus version's single authority, so a corpus release never edits
 * this file. */
#define TAM_DEFAULT_CORPUS_VERSION  "unversioned"

char *cultivar_version(void);
char *tam_corpus_version(const char *root);
char *tam_corpus_build_id(const char *root);

#endif /* TAM_VERSION_H */
