#include <tam.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "version.h"

#define TAM_EXIT_USAGE 2

static char *join_path(const char *dir, const char *rest)
{
    size_t dlen = strlen(dir);
    size_t rlen = strlen(rest);
    char *out = malloc(dlen + rlen + 1);
    if (out == NULL) {
        return NULL;
    }
    memcpy(out, dir, dlen);
    memcpy(out + dlen, rest, rlen + 1);
    return out;
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

static char *cultivar_version_from_file(void)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");
    char *path = NULL;

    if (xdg != NULL && xdg[0] != '\0') {
        path = join_path(xdg, "/tamlinux/version");
    } else if (home != NULL && home[0] != '\0') {
        path = join_path(home, "/.config/tamlinux/version");
    }

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

static char *cultivar_version(void)
{
    char *from_file = cultivar_version_from_file();
    if (from_file != NULL) {
        return from_file;
    }

    const char *env = getenv("TAMLINUX_VERSION");
    if (env != NULL && env[0] != '\0') {
        return strdup(env);
    }
    return NULL;
}

static void print_intro(void)
{
    tam_str_t s = tam_str_new("");
    if (s == NULL) {
        return;
    }
    s = tam_str_cat(s, TAM_COMMAND_NAME " - the Tamlinux command\n\n");
    s = tam_str_cat(s,
        "Learn about this system, keep it healthy, and find your way\n"
        "around Tamlinux, offline, from the terminal.\n\n");
    s = tam_str_cat_printf(s,
        "This is an early development build (tam %s).\n\n",
        TAM_COMMAND_VERSION);
    s = tam_str_cat(s,
        "Get started:\n"
        "  tam --help       list options and where to begin\n"
        "  tam --version    show version information\n\n");
    s = tam_str_cat(s,
        "More commands (browse, explain, install) appear as they are made.\n");
    fputs(s, stdout);
    tam_str_free(s);
}

static void print_help(void)
{
    fputs(
        "tam - the Tamlinux command\n\n"
        "Usage:\n"
        "  tam              print a short introduction and exit\n"
        "  tam --help       show this help and exit\n"
        "  tam --version    show version information and exit\n\n"
        "This early build implements orientation only. Commands such as\n"
        "browse, explain, welcome and install are not available yet.\n",
        stdout);
}

static void print_version(void)
{
    char *cultivar = cultivar_version();

    printf(TAM_COMMAND_NAME " command %s\n", TAM_COMMAND_VERSION);
    printf("libtam %s\n", tam_version());
    if (cultivar != NULL) {
        printf("Tamlinux cultivar %s\n", cultivar);
        free(cultivar);
    } else {
        printf("Tamlinux cultivar: unavailable (no installed cultivar metadata)\n");
    }
}

static int usage_error(const char *bad)
{
    fprintf(stderr, TAM_COMMAND_NAME ": unrecognized argument '%s'\n", bad);
    fprintf(stderr, "Run '%s --help' for available options.\n",
            TAM_COMMAND_NAME);
    return TAM_EXIT_USAGE;
}

static int too_many_arguments(void)
{
    fprintf(stderr, TAM_COMMAND_NAME ": too many arguments\n");
    fprintf(stderr, "Run '%s --help' for available options.\n",
            TAM_COMMAND_NAME);
    return TAM_EXIT_USAGE;
}

int main(int argc, char **argv)
{
    if (argc <= 1) {
        print_intro();
        return 0;
    }

    if (argc > 2) {
        return too_many_arguments();
    }

    const char *arg = argv[1];
    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
        print_help();
        return 0;
    }
    if (strcmp(arg, "-V") == 0 || strcmp(arg, "--version") == 0) {
        print_version();
        return 0;
    }
    return usage_error(arg);
}
