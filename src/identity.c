/* Tamlinux bundle and legacy per-user identity, and corpus build identity. */

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

#ifndef TAM_CULTIVAR_ROOT
#define TAM_CULTIVAR_ROOT ""
#endif
#ifndef TAM_ACTIVE_CULTIVAR_ROOT
#define TAM_ACTIVE_CULTIVAR_ROOT "/usr/lib/tamlinux/current"
#endif

static const char *configured_cultivar_root(void)
{
    return TAM_CULTIVAR_ROOT[0] ? TAM_CULTIVAR_ROOT : "";
}

static bool valid_version_token(const char *s)
{
    if (!s || !s[0] || strlen(s) > 100) return false;
    for (const char *p = s; *p; p++) {
        if (!isalnum((unsigned char)*p) && *p != '.' && *p != '-' && *p != '_' && *p != '+')
            return false;
    }
    return true;
}

static char *adjacent_root(void)
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
    return strdup(exe);
}

/* Takes ownership of path and returns it only for a valid VERSION file. */
static char *read_version_identity(char *path, char **version)
{
    char *text = path ? tam_fs_read_file(path, NULL) : NULL;
    if (!text) { free(path); return NULL; }
    trim_in_place(text);
    if (!valid_version_token(text)) { free(text); free(path); return NULL; }
    if (version) *version = text;
    else free(text);
    return path;
}

static char *read_root_identity(const char *root, char **version)
{
    if (!root || !root[0]) return NULL;
    return read_version_identity(tam_path_join(root, "VERSION"), version);
}

static char *bundle_identity(char **version)
{
    const char *fixed = configured_cultivar_root();
    if (fixed[0])
        return read_root_identity(fixed, version);

    char *adj = adjacent_root();
    char *path = adj ? read_root_identity(adj, version) : NULL;
    free(adj);
    if (path != NULL || !TAM_ACTIVE_CULTIVAR_ROOT[0]) return path;

    return read_root_identity(TAM_ACTIVE_CULTIVAR_ROOT, version);
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

static char *user_cultivar_path(const char *leaf)
{
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

static char *bundle_cultivar_path(char *manifest)
{
    *strrchr(manifest, '/') = '\0';
    char *path = tam_path_join(manifest, "share/tamlinux/cultivar");
    free(manifest);
    return path;
}

char *cultivar_path(const char *leaf)
{
    const char *fixed = configured_cultivar_root();
    if (fixed[0]) {
        if (strcmp(leaf, "version") == 0)
            return tam_path_join(fixed, "VERSION");
        if (strcmp(leaf, "cultivar") == 0) return tam_path_join(fixed, "share/tamlinux/cultivar");
    }
    char *manifest = bundle_identity(NULL);
    if (manifest != NULL) {
        if (strcmp(leaf, "version") == 0) return manifest;
        if (strcmp(leaf, "cultivar") == 0) return bundle_cultivar_path(manifest);
        free(manifest);
    }
    return user_cultivar_path(leaf);
}

static char *read_cultivar_file(char *path)
{
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
static char *legacy_cultivar_version(void)
{
    char *from_file = read_cultivar_file(user_cultivar_path("version"));
    if (from_file != NULL) {
        return from_file;
    }

    const char *env = getenv("TAMLINUX_VERSION");
    if (env != NULL && env[0] != '\0') {
        return strdup(env);
    }
    return NULL;
}

char *cultivar_version(void)
{
    char *version = NULL;
    free(bundle_identity(&version));
    if (version != NULL || configured_cultivar_root()[0]) return version;
    return legacy_cultivar_version();
}

char *cultivar_name(void)
{
    return read_cultivar_file(cultivar_path("cultivar"));
}

/* "Incipiens 0.4.5", or the bare version when no name is installed. */
char *cultivar_label(void)
{
    char *version = NULL;
    char *manifest = bundle_identity(&version);
    char *name;
    if (manifest != NULL) {
        name = read_cultivar_file(bundle_cultivar_path(manifest));
    } else {
        if (configured_cultivar_root()[0]) return NULL;
        version = legacy_cultivar_version();
        name = read_cultivar_file(user_cultivar_path("cultivar"));
    }
    if (version == NULL) {
        free(name);
        return NULL;
    }
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
