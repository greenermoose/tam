#define _POSIX_C_SOURCE 200809L
#include "upgrade.h"
#include "render.h"
#include "version.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

int tam_cmd_grow(int argc, char **argv)
{
    if (argc == 1 && (strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "-h") == 0)) {
        puts("Usage: tam grow [OPTIONS]\n"
             "Build and install a Tamlinux scion from local or configured sources.");
        return 0;
    }
    return run_helper("grow", argc, argv);
}

int tam_cmd_rollback(int argc, char **argv)
{
    if (argc == 1 && (strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "-h") == 0)) {
        puts("Usage: tam rollback [OPTIONS]\n"
             "Roll back the active Tamlinux cultivar to the previous installed version.");
        return 0;
    }
    return run_helper("rollback", argc, argv);
}

static int remove_tree_recursive(const char *path)
{
    struct stat st;
    if (lstat(path, &st) != 0) {
        return errno == ENOENT ? 0 : -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        return unlink(path);
    }
    DIR *d = opendir(path);
    if (!d) return -1;
    struct dirent *ent;
    int rc = 0;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        char child[PATH_MAX];
        int w = snprintf(child, sizeof(child), "%s/%s", path, ent->d_name);
        if (w < 0 || (size_t)w >= sizeof(child)) {
            errno = ENAMETOOLONG;
            rc = -1;
            break;
        }
        if (remove_tree_recursive(child) != 0) {
            rc = -1;
            break;
        }
    }
    closedir(d);
    if (rc != 0) return rc;
    return rmdir(path);
}

int tam_cmd_uninstall(int argc, char **argv)
{
    int remove_self = 0;
    const char *root = "/";

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            puts("Usage: tam uninstall [--self] [--root DIR]\n"
                 "Remove the installed Tamlinux scion, and with --self also remove\n"
                 "the tam-bud binary (/usr/bin/tam) and bud docs (/usr/share/doc/tamlinux).");
            return 0;
        } else if (strcmp(argv[i], "--self") == 0) {
            remove_self = 1;
        } else if (strcmp(argv[i], "--root") == 0 && i + 1 < argc) {
            root = argv[++i];
        } else {
            fprintf(stderr, "tam uninstall: unknown argument: %s\n", argv[i]);
            return 2;
        }
    }

    const char *prefix = (strcmp(root, "/") == 0) ? "" : root;
    char state_active[PATH_MAX];
    snprintf(state_active, sizeof(state_active), "%s/var/lib/tamlinux/active.json", prefix);

    struct stat st;
    if (lstat(state_active, &st) == 0) {
        /* A scion is installed; refuse self-only removal while scion is active. */
        fprintf(stderr,
                "tam uninstall: an active Tamlinux scion is recorded at %s.\n"
                "Remove the scion first with transaction.py uninstall --root %s.\n",
                state_active, root);
        return 1;
    }

    if (!remove_self) {
        puts("No active Tamlinux scion is installed. Pass --self to remove tam-bud as well.");
        return 0;
    }

    char tam_bin[PATH_MAX];
    char bud_docs[PATH_MAX];
    snprintf(tam_bin, sizeof(tam_bin), "%s/usr/bin/tam", prefix);
    snprintf(bud_docs, sizeof(bud_docs), "%s/usr/share/doc/tamlinux", prefix);

    if (remove_tree_recursive(bud_docs) != 0) {
        fprintf(stderr, "tam uninstall: failed to remove %s: %s\n", bud_docs, strerror(errno));
        return 1;
    }
    if (unlink(tam_bin) != 0 && errno != ENOENT) {
        fprintf(stderr, "tam uninstall: failed to remove %s: %s\n", tam_bin, strerror(errno));
        return 1;
    }
    return 0;
}

int tam_cmd_run(int argc, char **argv)
{
    if (argc < 1 || strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "-h") == 0) {
        puts("Usage: tam run COMMAND [ARGS...]\n"
             "Launch an installed Tamlinux cultivar command.");
        return argc < 1 ? 2 : 0;
    }
    const char *cmd = argv[0];
    if (!cmd[0] || strchr(cmd, '/') != NULL) {
        fprintf(stderr, "tam run: command name must not contain '/': %s\n", cmd);
        return 2;
    }

    const char *env_bin = getenv("TAM_CULTIVAR_BIN");
    const char *dirs[4];
    int ndirs = 0;
    if (env_bin && env_bin[0]) dirs[ndirs++] = env_bin;
    dirs[ndirs++] = "/usr/bin/tamlinux";
    dirs[ndirs++] = "/usr/lib/tamlinux/current/bin";

    char target[PATH_MAX];
    int found = 0;
    for (int i = 0; i < ndirs; i++) {
        int w = snprintf(target, sizeof(target), "%s/%s", dirs[i], cmd);
        if (w > 0 && (size_t)w < sizeof(target) && access(target, X_OK) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "tam run: '%s' is not installed in the active Tamlinux cultivar\n", cmd);
        return 127;
    }

    char **exec_argv = calloc((size_t)argc + 1, sizeof(*exec_argv));
    if (!exec_argv) {
        fprintf(stderr, "tam run: out of memory\n");
        return 1;
    }
    exec_argv[0] = target;
    for (int i = 1; i < argc; i++) exec_argv[i] = argv[i];
    exec_argv[argc] = NULL;
    execv(target, exec_argv);
    fprintf(stderr, "tam run: cannot execute %s: %s\n", target, strerror(errno));
    free(exec_argv);
    return 126;
}
