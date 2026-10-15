#ifndef TAM_INSTALL_H
#define TAM_INSTALL_H

/* Returns process exit code (0 for success, TAM_EXIT_USAGE for usage error, TAM_EXIT_FAILURE on failure) */
int cmd_inspect(int n, char **args);
int cmd_check(int n, char **args);
int cmd_install(int n, char **args);

#endif /* TAM_INSTALL_H */
