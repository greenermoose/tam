#define _POSIX_C_SOURCE 200809L
#include "upgrade.h"
#include "render.h"
#include "version.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef TAM_SOURCE_REVISION
#define TAM_SOURCE_REVISION "unknown"
#endif
#ifndef TAM_SOURCE_DIRTY
#define TAM_SOURCE_DIRTY 1
#endif

static int run_helper(const char *verb, int argc, char **argv)
{
    (void)setenv("TAM_UPGRADE_RUNNING_COMMAND", TAM_COMMAND_VERSION, 1);
    (void)setenv("TAM_UPGRADE_RUNNING_REVISION", TAM_SOURCE_REVISION, 1);
    (void)setenv("TAM_UPGRADE_RUNNING_DIRTY", TAM_SOURCE_DIRTY ? "true" : "false", 1);
    char *root = tam_find_knowledge_root();
    char *corpus = tam_corpus_version(root);
    if (corpus != NULL) (void)setenv("TAM_UPGRADE_RUNNING_CORPUS", corpus, 1);
    if (root != NULL) (void)setenv("TAM_UPGRADE_CORPUS_ORIGIN", root, 1);
    free(corpus);
    free(root);
    char self[PATH_MAX + 1];
    ssize_t n = readlink("/proc/self/exe", self, PATH_MAX);
    if (n < 0 || n > PATH_MAX) {
        fprintf(stderr, "tam: cannot locate the installed upgrade helper: %s\n",
                strerror(errno));
        return 1;
    }
    self[n] = '\0';
    char *slash = strrchr(self, '/');
    if (slash == NULL) return 1;
    slash[1] = '\0';

    char helper[PATH_MAX + 1];
    int written = snprintf(helper, sizeof(helper), "%stam-upgrade", self);
    if (written < 0 || (size_t)written >= sizeof(helper) || access(helper, X_OK) != 0) {
        /* Source-tree builds keep the executable in build/ and the helper in bin/. */
        written = snprintf(helper, sizeof(helper), "%s../bin/tam-upgrade", self);
        if (written < 0 || (size_t)written >= sizeof(helper) || access(helper, X_OK) != 0) {
            fprintf(stderr, "tam: upgrade helper is missing beside this command\n");
            return 1;
        }
    }

    char **args = calloc((size_t)argc + 3, sizeof(*args));
    if (args == NULL) {
        fprintf(stderr, "tam: out of memory\n");
        return 1;
    }
    args[0] = helper;
    args[1] = (char *)verb;
    for (int i = 0; i < argc; i++) args[i + 2] = argv[i];
    args[argc + 2] = NULL;
    execv(helper, args);
    fprintf(stderr, "tam: cannot run upgrade helper: %s\n", strerror(errno));
    free(args);
    return 1;
}

int tam_cmd_upgrade(int argc, char **argv)
{
    return run_helper("upgrade", argc, argv);
}

int tam_cmd_switch(int argc, char **argv)
{
    return run_helper("switch", argc, argv);
}
