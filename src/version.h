#ifndef TAM_VERSION_H
#define TAM_VERSION_H

#define TAM_COMMAND_NAME            "tam"
#define TAM_COMMAND_VERSION         "0.24.0"
#define TAM_DEFAULT_CORPUS_VERSION  "0.24.0"

char *cultivar_version(void);
char *tam_corpus_version(const char *root);
char *tam_corpus_build_id(const char *root);

#endif /* TAM_VERSION_H */
