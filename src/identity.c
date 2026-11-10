/* Tamlinux and corpus identity: the cultivar name and version from the
 * invoking user's files (the pkexec caller under pkexec), and the corpus
 * version and build id. */

#include <tam.h>

#include "ask.h"
#include "depth.h"
#include "fold.h"
#include "address.h"
#include "html.h"
#include "discovery.h"
#include "index.h"
#include "info.h"
#include "upgrade.h"
#include "install.h"
#include "man.h"
#include "pager.h"
#include "render.h"
#include "tldr.h"
#include "views.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <locale.h>
#include <pwd.h>
#include <limits.h>

#include "version.h"

#ifndef TAM_DATADIR
#define TAM_DATADIR "/usr/local/share/tamlinux/knowledge"
#endif

/* Set by the Makefile from `git describe` when building from a checkout;
 * empty for packaged builds, which have no Git metadata. */
#ifndef TAM_BUILD
#define TAM_BUILD ""
#endif

#define TAM_EXIT_USAGE 2
#define TAM_EXIT_FAILURE 1

#include "corpus.h"

static char *trim_in_place(char *s);
static const struct passwd *cultivar_passwd(void);
static char *cultivar_file(const char *leaf);

typedef struct { char *name, *version; } bundle_identity_t;

static int bundle_identity_line(void *user, size_t line, int format, char **fields, size_t n)
{
    (void)line; (void)format;
    bundle_identity_t *b = user;
    if (n != 2) return 1;
    char **value = strcmp(fields[0], "name") == 0 ? &b->name :
                   strcmp(fields[0], "version") == 0 ? &b->version : NULL;
    if (!value) return 0;
    if (*value || !fields[1][0] || strlen(fields[1]) > 100) return 1;
    for (const char *p = fields[1]; *p; p++)
        if (!isalnum((unsigned char)*p) && *p != '.' && *p != '-' && *p != '_' && *p != '+') return 1;
    *value = strdup(fields[1]);
    return *value ? 0 : 1;
}

/* Only an executable installed in a bundle's bin/ uses that bundle's
 * identity. Merely placing a dormant bundle on the machine must not change
 * the identity reported by a development or old-delivery command. */
static char *bundle_identity(char **version)
{
    char exe[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);
    if (n <= 0 || n >= (ssize_t)sizeof exe - 1) return NULL;
    exe[n] = '\0';
    char *slash = strrchr(exe, '/');
    if (!slash) return NULL;
    *slash = '\0';
    slash = strrchr(exe, '/');
    if (!slash || strcmp(slash + 1, "bin") != 0) return NULL;
    *slash = '\0';
    char *path = tam_path_join(exe, "BUNDLE");
    size_t len = 0;
    char *text = path ? tam_fs_read_file(path, &len) : NULL;
    bundle_identity_t b = {0};
    tam_rec_error_t error;
    bool valid = text && tam_rec_parse(text, len, 2, 1, bundle_identity_line, &b, &error) == 0 &&
                 b.name && strcmp(b.name, "tamlinux") == 0 && b.version;
    free(text); free(b.name);
    if (!valid) { free(path); free(b.version); return NULL; }
    if (version) *version = b.version;
    else free(b.version);
    return path;
}

static char *trim_in_place(char *s)
{
    size_t len = strlen(s);
    while (len > 0) {
        char c = s[len - 1];
        if (c != '\n' && c != '\r' && c != ' ' && c != '\t') {
            break;
        }
        s[--len] = '\0';
    }
    return s;
}

/* Tamlinux is deployed per user (D181): its version and cultivar name sit in
 * that user's ~/.config/tamlinux. Under pkexec the user is the one who ran
 * pkexec (PKEXEC_UID), not root. */
static const struct passwd *cultivar_passwd(void)
{
    const char *pk = getenv("PKEXEC_UID");
    if (geteuid() == 0 && pk != NULL && pk[0] != '\0') {
        char *end = NULL;
        unsigned long uid = strtoul(pk, &end, 10);
        if (end != pk && *end == '\0') {
            const struct passwd *pw = getpwuid((uid_t)uid);
            if (pw != NULL) {
                return pw;
            }
        }
    }
    return getpwuid(geteuid());
}

bool cultivar_for_root(void)
{
    const char *pk = getenv("PKEXEC_UID");
    return geteuid() == 0 && (pk == NULL || pk[0] == '\0');
}

char *cultivar_user(void)
{
    const struct passwd *pw = cultivar_passwd();
    return strdup(pw != NULL && pw->pw_name != NULL ? pw->pw_name : "this user");
}

char *cultivar_path(const char *leaf)
{
    char *bundle = bundle_identity(NULL);
    if (bundle) {
        if (strcmp(leaf, "version") == 0) return bundle;
        if (strcmp(leaf, "cultivar") == 0) {
            *strrchr(bundle, '/') = '\0';
            char *path = tam_path_join(bundle, "share/tamlinux/cultivar");
            free(bundle);
            return path;
        }
        free(bundle);
    }
    const char *xdg = getenv("XDG_CONFIG_HOME");
    char *config = NULL;

    if (xdg != NULL && xdg[0] != '\0' && getenv("PKEXEC_UID") == NULL) {
        config = strdup(xdg);
    } else {
        const struct passwd *pw = cultivar_passwd();
        const char *home = getenv("HOME");
        if (getenv("PKEXEC_UID") != NULL && pw != NULL && pw->pw_dir != NULL) {
            home = pw->pw_dir;
        }
        if (home != NULL && home[0] != '\0') {
            config = tam_path_join(home, "/.config");
        }
    }
    if (config == NULL) {
        return NULL;
    }
    char *dir = tam_path_join(config, "/tamlinux/");
    free(config);
    if (dir == NULL) {
        return NULL;
    }
    char *path = tam_path_join(dir, leaf);
    free(dir);
    return path;
}

static char *cultivar_file(const char *leaf)
{
    char *path = cultivar_path(leaf);
    if (path == NULL) {
        return NULL;
    }

    char *buf = tam_fs_read_file(path, NULL);
    free(path);
    if (buf == NULL) {
        return NULL;
    }

    trim_in_place(buf);
    if (buf[0] == '\0') {
        free(buf);
        return NULL;
    }
    return buf;
}

/* The version file wins over $TAMLINUX_VERSION: activation rewrites the file,
 * while a long-running process keeps the value it started with. */
char *cultivar_version(void)
{
    char *version = NULL;
    char *bundle = bundle_identity(&version);
    if (bundle) { free(bundle); return version; }
    char *from_file = cultivar_file("version");
    if (from_file != NULL) {
        return from_file;
    }

    const char *env = getenv("TAMLINUX_VERSION");
    if (env != NULL && env[0] != '\0') {
        return strdup(env);
    }
    return NULL;
}

char *cultivar_name(void)
{
    return cultivar_file("cultivar");
}

/* "Incipiens 0.4.5", or the bare version when no name is installed. */
char *cultivar_label(void)
{
    char *version = cultivar_version();
    if (version == NULL) {
        return NULL;
    }
    char *name = cultivar_name();
    if (name == NULL) {
        return version;
    }
    size_t len = strlen(name) + 1 + strlen(version) + 1;
    char *label = malloc(len);
    if (label != NULL) snprintf(label, len, "%s %s", name, version);
    free(name);
    free(version);
    return label;
}

char *tam_corpus_version(const char *root)
{
    if (root == NULL || root[0] == '\0') {
        return NULL;
    }
    char *path = tam_path_join(root, "/version");
    if (path == NULL) {
        return NULL;
    }
    char *buf = tam_fs_read_file(path, NULL);
    free(path);
    if (buf != NULL) {
        trim_in_place(buf);
        if (buf[0] != '\0') {
            return buf;
        }
        free(buf);
    }
    return strdup(TAM_DEFAULT_CORPUS_VERSION);
}

char *tam_corpus_build_id(const char *root)
{
    if (root == NULL || root[0] == '\0') {
        return NULL;
    }
    char *path = tam_path_join(root, "/.build_id");
    if (path == NULL) {
        return NULL;
    }
    char *buf = tam_fs_read_file(path, NULL);
    free(path);
    if (buf != NULL) {
        trim_in_place(buf);
        if (buf[0] != '\0') {
            return buf;
        }
        free(buf);
    }
    return NULL;
}
