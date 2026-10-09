#include <tam.h>

#include "install.h"
#include "version.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

#define TAM_EXIT_USAGE 2
#define TAM_EXIT_FAILURE 1

typedef struct {
    char name[128];
    char pretty_name[128];
    char id[64];
    char id_like[64];
    char version_id[64];
    char build_id[64];
} os_release_t;

static void parse_os_release(os_release_t *info)
{
    memset(info, 0, sizeof(*info));
    size_t len = 0;
    char *data = tam_fs_read_file("/etc/os-release", &len);
    if (data == NULL) {
        data = tam_fs_read_file("/usr/lib/os-release", &len);
    }
    if (data == NULL) {
        snprintf(info->name, sizeof(info->name), "Linux");
        snprintf(info->id, sizeof(info->id), "unknown");
        return;
    }

    char *line = data;
    while (*line != '\0') {
        char *eol = strchr(line, '\n');
        if (eol != NULL) {
            *eol = '\0';
        }

        while (*line == ' ' || *line == '\t') {
            line++;
        }
        if (*line != '#' && *line != '\0') {
            char *eq = strchr(line, '=');
            if (eq != NULL) {
                *eq = '\0';
                char *key = line;
                char *val = eq + 1;

                char *k_end = eq - 1;
                while (k_end >= key && (*k_end == ' ' || *k_end == '\t')) {
                    *k_end-- = '\0';
                }

                while (*val == ' ' || *val == '\t') {
                    val++;
                }
                if (*val == '"' || *val == '\'') {
                    char quote = *val;
                    val++;
                    char *close_q = strchr(val, quote);
                    if (close_q != NULL) {
                        *close_q = '\0';
                    }
                } else {
                    char *v_end = val + strlen(val) - 1;
                    while (v_end >= val && (*v_end == ' ' || *v_end == '\t' || *v_end == '\r')) {
                        *v_end-- = '\0';
                    }
                }

                if (strcmp(key, "NAME") == 0) {
                    snprintf(info->name, sizeof(info->name), "%s", val);
                } else if (strcmp(key, "PRETTY_NAME") == 0) {
                    snprintf(info->pretty_name, sizeof(info->pretty_name), "%s", val);
                } else if (strcmp(key, "ID") == 0) {
                    snprintf(info->id, sizeof(info->id), "%s", val);
                } else if (strcmp(key, "ID_LIKE") == 0) {
                    snprintf(info->id_like, sizeof(info->id_like), "%s", val);
                } else if (strcmp(key, "VERSION_ID") == 0) {
                    snprintf(info->version_id, sizeof(info->version_id), "%s", val);
                } else if (strcmp(key, "BUILD_ID") == 0) {
                    snprintf(info->build_id, sizeof(info->build_id), "%s", val);
                }
            }
        }

        if (eol == NULL) {
            break;
        }
        line = eol + 1;
    }

    free(data);

    if (info->name[0] == '\0') {
        if (info->pretty_name[0] != '\0') {
            snprintf(info->name, sizeof(info->name), "%s", info->pretty_name);
        } else {
            snprintf(info->name, sizeof(info->name), "Linux");
        }
    }
    if (info->id[0] == '\0') {
        snprintf(info->id, sizeof(info->id), "unknown");
    }
}

static bool is_command_available(const char *cmd)
{
    if (cmd == NULL || cmd[0] == '\0') {
        return false;
    }
    if (strchr(cmd, '/') != NULL) {
        return access(cmd, X_OK) == 0;
    }
    const char *path_env = getenv("PATH");
    if (path_env == NULL || path_env[0] == '\0') {
        path_env = "/usr/local/bin:/usr/bin:/bin";
    }
    char *copy = strdup(path_env);
    if (copy == NULL) {
        return false;
    }
    bool found = false;
    char *token = copy;
    while (*token != '\0') {
        char *colon = strchr(token, ':');
        if (colon != NULL) {
            *colon = '\0';
        }
        if (token[0] != '\0') {
            char full[1024];
            snprintf(full, sizeof(full), "%s/%s", token, cmd);
            if (access(full, X_OK) == 0) {
                found = true;
                break;
            }
        }
        if (colon == NULL) {
            break;
        }
        token = colon + 1;
    }
    free(copy);
    return found;
}

static void get_package_tools(char *out, size_t maxlen)
{
    static const char *tools[] = {
        "pacman",
        "nix",
        "home-manager",
        "mise",
        "xbps-install",
        "xbps-query",
        "apt",
        "dpkg",
        NULL
    };
    out[0] = '\0';
    size_t cur_len = 0;
    for (int i = 0; tools[i] != NULL; i++) {
        if (is_command_available(tools[i])) {
            if (cur_len > 0) {
                cur_len += snprintf(out + cur_len, maxlen > cur_len ? maxlen - cur_len : 0, ", ");
            }
            cur_len += snprintf(out + cur_len, maxlen > cur_len ? maxlen - cur_len : 0, "%s", tools[i]);
        }
    }
    if (out[0] == '\0') {
        snprintf(out, maxlen, "none detected");
    }
}

static void get_privilege_tools(char *out, size_t maxlen)
{
    static const char *tools[] = {
        "pkexec",
        "sudo",
        "doas",
        NULL
    };
    out[0] = '\0';
    size_t cur_len = 0;
    for (int i = 0; tools[i] != NULL; i++) {
        if (is_command_available(tools[i])) {
            if (cur_len > 0) {
                cur_len += snprintf(out + cur_len, maxlen > cur_len ? maxlen - cur_len : 0, ", ");
            }
            cur_len += snprintf(out + cur_len, maxlen > cur_len ? maxlen - cur_len : 0, "%s", tools[i]);
        }
    }
    if (out[0] == '\0') {
        snprintf(out, maxlen, "none");
    }
}

static void get_root_filesystem(char *out, size_t maxlen)
{
    snprintf(out, maxlen, "unknown");
    FILE *f = fopen("/proc/mounts", "r");
    if (f == NULL) {
        return;
    }
    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        char dev[256], mnt[256], fstype[64];
        if (sscanf(line, "%255s %255s %63s", dev, mnt, fstype) == 3) {
            if (strcmp(mnt, "/") == 0) {
                snprintf(out, maxlen, "%s", fstype);
                break;
            }
        }
    }
    fclose(f);
}

static void get_graphics_device(char *out, size_t maxlen)
{
    snprintf(out, maxlen, "none detected");
    char path[64];
    for (int i = 0; i < 8; i++) {
        snprintf(path, sizeof(path), "/dev/dri/card%d", i);
        if (access(path, F_OK) == 0) {
            snprintf(out, maxlen, "%s (available)", path);
            return;
        }
    }
}

static const char *get_init_system(void)
{
    if (access("/run/systemd/system", F_OK) == 0) {
        return "systemd";
    }
    if (access("/run/runit", F_OK) == 0) {
        return "runit";
    }
    size_t len = 0;
    char *comm = tam_fs_read_file("/proc/1/comm", &len);
    if (comm != NULL) {
        char *nl = strchr(comm, '\n');
        if (nl != NULL) {
            *nl = '\0';
        }
        if (strcmp(comm, "systemd") == 0) {
            free(comm);
            return "systemd";
        }
        if (strcmp(comm, "runit") == 0) {
            free(comm);
            return "runit";
        }
        free(comm);
    }
    return "unknown";
}

static void get_profiles(const os_release_t *info, char *supported, size_t s_len, char *target_base, size_t t_len)
{
    if (strcasecmp(info->id, "omarchy") == 0 ||
        strcasecmp(info->id, "arch") == 0 ||
        strcasestr(info->id_like, "arch") != NULL) {
        snprintf(supported, s_len, "arch-hyprland, arch");
        snprintf(target_base, t_len, "arch");
    } else if (strcasecmp(info->id, "void") == 0) {
        snprintf(supported, s_len, "void");
        snprintf(target_base, t_len, "void");
    } else if (strcasecmp(info->id, "antix") == 0 ||
               strcasecmp(info->id, "debian") == 0 ||
               strcasestr(info->id_like, "debian") != NULL) {
        snprintf(supported, s_len, "antix");
        snprintf(target_base, t_len, "antix");
    } else {
        snprintf(supported, s_len, "none (unsupported host distribution)");
        snprintf(target_base, t_len, "unknown");
    }
}

static int do_inspect(void)
{
    os_release_t os;
    parse_os_release(&os);

    struct utsname uts;
    memset(&uts, 0, sizeof(uts));
    uname(&uts);

    char *cultivar = cultivar_version();

    char os_display[256];
    if (os.id_like[0] != '\0' && strcasecmp(os.id, os.id_like) != 0) {
        snprintf(os_display, sizeof(os_display), "%s (%s-like)", os.name, os.id_like);
    } else {
        snprintf(os_display, sizeof(os_display), "%s", os.name);
    }

    const char *os_ver = "rolling";
    if (os.version_id[0] != '\0') {
        os_ver = os.version_id;
    } else if (os.build_id[0] != '\0') {
        os_ver = os.build_id;
    }

    char pkg_tools[256];
    get_package_tools(pkg_tools, sizeof(pkg_tools));

    char priv_tools[128];
    get_privilege_tools(priv_tools, sizeof(priv_tools));

    char root_fs[64];
    get_root_filesystem(root_fs, sizeof(root_fs));

    char gpu[128];
    get_graphics_device(gpu, sizeof(gpu));

    char supported[128];
    char target_base[64];
    get_profiles(&os, supported, sizeof(supported), target_base, sizeof(target_base));

    printf("Host Information:\n");
    printf("  Operating system:  %s\n", os_display);
    printf("  OS version:        %s\n", os_ver);
    printf("  Kernel release:    %s\n", uts.release);
    printf("  Architecture:      %s\n", uts.machine);
    if (cultivar != NULL) {
        printf("  Cultivar version:  %s\n", cultivar);
        free(cultivar);
    } else {
        printf("  Cultivar version:  unavailable\n");
    }
    printf("\n");

    printf("Installation Capabilities:\n");
    printf("  Init system:       %s\n", get_init_system());
    printf("  Package tools:     %s\n", pkg_tools);
    printf("  Privilege tool:    %s\n", priv_tools);
    printf("  Root filesystem:   %s\n", root_fs);
    printf("  Graphics device:   %s\n", gpu);
    printf("\n");

    printf("Installation Profiles:\n");
    printf("  Supported:         %s\n", supported);
    printf("  Target base:       %s\n", target_base);

    return 0;
}

static void print_install_help(void)
{
    fputs(
        "tam install - Tamlinux installation framework\n\n"
        "Usage:\n"
        "  tam install                       show this overview and available subcommands\n"
        "  tam install inspect               inspect host facts and recognized capabilities\n"
        "  tam install inspect --pipe        print inspection report as plain text\n"
        "  tam install --help                show this help and exit (same as: tam install help)\n\n"
        "Upcoming commands:\n"
        "  tam install plan --profile PROF   print planned changes without modifying the system\n"
        "  tam install apply --profile PROF  apply reviewed installation plan\n"
        "  tam install verify                verify installed components and desktop integration\n",
        stdout);
}

int cmd_install(int n, char **args)
{
    if (n == 0) {
        print_install_help();
        return 0;
    }

    const char *sub = args[0];
    if (strcmp(sub, "-h") == 0 || strcmp(sub, "--help") == 0 || strcmp(sub, "help") == 0) {
        if (n > 1) {
            fprintf(stderr, TAM_COMMAND_NAME " install: too many arguments\n"
                            "Run '" TAM_COMMAND_NAME " install --help' for available options.\n");
            return TAM_EXIT_USAGE;
        }
        print_install_help();
        return 0;
    }

    if (strcmp(sub, "inspect") == 0) {
        for (int i = 1; i < n; i++) {
            if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
                continue;
            } else if (args[i][0] == '-') {
                fprintf(stderr, TAM_COMMAND_NAME " install inspect: unrecognized argument '%s'\n"
                                "Run '" TAM_COMMAND_NAME " install --help' for available options.\n", args[i]);
                return TAM_EXIT_USAGE;
            } else {
                fprintf(stderr, TAM_COMMAND_NAME " install inspect: too many arguments\n"
                                "Run '" TAM_COMMAND_NAME " install --help' for available options.\n");
                return TAM_EXIT_USAGE;
            }
        }
        return do_inspect();
    }

    if (strcmp(sub, "plan") == 0) {
        fprintf(stderr, TAM_COMMAND_NAME ": 'plan' is not implemented yet\n"
                        "Installation planning will be implemented in a later milestone of the installation framework.\n");
        return TAM_EXIT_FAILURE;
    }

    if (strcmp(sub, "apply") == 0) {
        fprintf(stderr, TAM_COMMAND_NAME ": 'apply' is not implemented yet\n"
                        "Installation execution will be implemented in a later milestone of the installation framework.\n");
        return TAM_EXIT_FAILURE;
    }

    if (strcmp(sub, "verify") == 0) {
        fprintf(stderr, TAM_COMMAND_NAME ": 'verify' is not implemented yet\n"
                        "Installation verification will be implemented in a later milestone of the installation framework.\n");
        return TAM_EXIT_FAILURE;
    }

    fprintf(stderr, TAM_COMMAND_NAME " install: unrecognized argument '%s'\n"
                    "Run '" TAM_COMMAND_NAME " install --help' for available options.\n", sub);
    return TAM_EXIT_USAGE;
}
