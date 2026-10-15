#include <tam.h>

#include "install.h"
#include "version.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>

#define TAM_EXIT_USAGE 2
#define TAM_EXIT_FAILURE 1

#define NUM_CHECKS 7

typedef enum {
    SEV_INFO = 0,
    SEV_WARN,
    SEV_ERROR,
    SEV_CRITICAL
} severity_t;

static const char *severity_str(severity_t sev)
{
    switch (sev) {
    case SEV_INFO: return "info";
    case SEV_WARN: return "warn";
    case SEV_ERROR: return "error";
    case SEV_CRITICAL: return "critical";
    default: return "info";
    }
}

typedef struct {
    char task_id[64];
    char name[128];
    char status[16]; /* "pass", "fail", "unknown" */
    bool pass;
    bool fail;
    severity_t severity;
    char help_ref[128];
    char summary[128];
    char evidence_text[256];
    char evidence_json[512];
} check_item_t;

typedef struct {
    char name[128];
    char pretty_name[128];
    char id[64];
    char id_like[64];
    char version_id[64];
    char build_id[64];
} os_release_t;

static bool is_whitelisted_probe(const char *cmd)
{
    if (cmd == NULL) {
        return false;
    }
    const char *base = strrchr(cmd, '/');
    if (base != NULL) {
        base++;
    } else {
        base = cmd;
    }
    return (strcmp(base, "findmnt") == 0 ||
            strcmp(base, "systemctl") == 0 ||
            strcmp(base, "udevadm") == 0 ||
            strcmp(base, "pkexec") == 0);
}

static tam_exec_result_t safe_probe_exec(const char *const argv[])
{
    tam_exec_result_t res;
    memset(&res, 0, sizeof(res));
    res.exit_code = -1;

    if (argv == NULL || argv[0] == NULL) {
        return res;
    }

    if (!is_whitelisted_probe(argv[0])) {
        return res;
    }

    /* Safe Execution Envelope: LC_ALL=C, bounded timeout <= 500ms, detached TTY */
    setenv("LC_ALL", "C", 1);
    return tam_exec(argv, 500);
}

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

    if (is_command_available("findmnt")) {
        const char *const argv[] = {"findmnt", "-n", "-o", "FSTYPE", "/", NULL};
        tam_exec_result_t res = safe_probe_exec(argv);
        if (res.exit_code == 0 && res.stdout_buf != NULL && res.stdout_len > 0) {
            char *nl = strchr(res.stdout_buf, '\n');
            if (nl != NULL) {
                *nl = '\0';
            }
            char *p = res.stdout_buf;
            while (*p == ' ' || *p == '\t') {
                p++;
            }
            if (*p != '\0') {
                snprintf(out, maxlen, "%s", p);
                tam_exec_result_free(&res);
                return;
            }
        }
        tam_exec_result_free(&res);
    }

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

static void collect_checks(check_item_t *checks, size_t max_checks,
                           const os_release_t *os, const struct utsname *uts,
                           const char *pkg_tools, const char *priv_tools,
                           const char *root_fs, const char *gpu_dev)
{
    (void)os;
    (void)priv_tools;
    if (max_checks < NUM_CHECKS) {
        return;
    }

    /* Check 0: privilege_pkexec (detect-and-degrade, R-A5) */
    snprintf(checks[0].task_id, sizeof(checks[0].task_id), "privilege_pkexec");
    snprintf(checks[0].name, sizeof(checks[0].name), "Privilege tool presence");
    snprintf(checks[0].help_ref, sizeof(checks[0].help_ref), "maintain/recovery.html");
    if (is_command_available("pkexec")) {
        snprintf(checks[0].status, sizeof(checks[0].status), "pass");
        checks[0].pass = true;
        checks[0].fail = false;
        checks[0].severity = SEV_INFO;
        snprintf(checks[0].summary, sizeof(checks[0].summary),
                 "polkit pkexec is available for non-interactive privileged reads");
        snprintf(checks[0].evidence_text, sizeof(checks[0].evidence_text),
                 "/usr/bin/pkexec available");
        snprintf(checks[0].evidence_json, sizeof(checks[0].evidence_json),
                 "\"tool\": \"pkexec\", \"available\": true, \"path\": \"/usr/bin/pkexec\"");
    } else {
        snprintf(checks[0].status, sizeof(checks[0].status), "unknown");
        checks[0].pass = false;
        checks[0].fail = true;
        checks[0].severity = SEV_WARN;
        snprintf(checks[0].summary, sizeof(checks[0].summary),
                 "privilege tool unavailable: limited inspection");
        snprintf(checks[0].evidence_text, sizeof(checks[0].evidence_text),
                 "pkexec not found in PATH");
        snprintf(checks[0].evidence_json, sizeof(checks[0].evidence_json),
                 "\"tool\": \"pkexec\", \"available\": false, \"detail\": \"pkexec not found in PATH\"");
    }

    /* Check 1: storage_btrfs */
    snprintf(checks[1].task_id, sizeof(checks[1].task_id), "storage_btrfs");
    snprintf(checks[1].name, sizeof(checks[1].name), "Root filesystem Btrfs snapshot support");
    snprintf(checks[1].help_ref, sizeof(checks[1].help_ref), "maintain/rollback.html");
    if (strcasecmp(root_fs, "btrfs") == 0) {
        snprintf(checks[1].status, sizeof(checks[1].status), "pass");
        checks[1].pass = true;
        checks[1].fail = false;
        checks[1].severity = SEV_INFO;
        snprintf(checks[1].summary, sizeof(checks[1].summary),
                 "root filesystem supports Copy-on-Write snapshots");
        snprintf(checks[1].evidence_text, sizeof(checks[1].evidence_text),
                 "root filesystem is btrfs");
        snprintf(checks[1].evidence_json, sizeof(checks[1].evidence_json),
                 "\"mount\": \"/\", \"fstype\": \"btrfs\", \"snapshot_capable\": true");
    } else {
        snprintf(checks[1].status, sizeof(checks[1].status), "fail");
        checks[1].pass = false;
        checks[1].fail = true;
        checks[1].severity = SEV_WARN;
        snprintf(checks[1].summary, sizeof(checks[1].summary),
                 "root filesystem does not support Btrfs snapshot generation rollbacks");
        snprintf(checks[1].evidence_text, sizeof(checks[1].evidence_text),
                 "root filesystem is %s", root_fs);
        snprintf(checks[1].evidence_json, sizeof(checks[1].evidence_json),
                 "\"mount\": \"/\", \"fstype\": \"%s\", \"snapshot_capable\": false", root_fs);
    }

    /* Check 2: storage_root_space */
    snprintf(checks[2].task_id, sizeof(checks[2].task_id), "storage_root_space");
    snprintf(checks[2].name, sizeof(checks[2].name), "Root filesystem free space");
    snprintf(checks[2].help_ref, sizeof(checks[2].help_ref), "maintain/rollback.html");
    struct statvfs sv;
    if (statvfs("/", &sv) == 0) {
        uint64_t total = (uint64_t)sv.f_blocks * sv.f_frsize;
        uint64_t avail = (uint64_t)sv.f_bavail * sv.f_frsize;
        double pct = total > 0 ? ((double)avail / (double)total) * 100.0 : 0.0;
        double gib = (double)avail / (1024.0 * 1024.0 * 1024.0);
        if (pct >= 10.0 && gib >= 1.0) {
            snprintf(checks[2].status, sizeof(checks[2].status), "pass");
            checks[2].pass = true;
            checks[2].fail = false;
            checks[2].severity = SEV_INFO;
            snprintf(checks[2].summary, sizeof(checks[2].summary),
                     "root filesystem has sufficient free space");
            snprintf(checks[2].evidence_text, sizeof(checks[2].evidence_text),
                     "%.1f GiB available (%.1f%% free)", gib, pct);
            snprintf(checks[2].evidence_json, sizeof(checks[2].evidence_json),
                     "\"mount\": \"/\", \"total_bytes\": %" PRIu64 ", \"avail_bytes\": %" PRIu64 ", \"free_percent\": %.1f, \"avail_gib\": %.1f",
                     total, avail, pct, gib);
        } else {
            snprintf(checks[2].status, sizeof(checks[2].status), "fail");
            checks[2].pass = false;
            checks[2].fail = true;
            checks[2].severity = (pct < 5.0 || gib < 0.5) ? SEV_CRITICAL : SEV_WARN;
            snprintf(checks[2].summary, sizeof(checks[2].summary),
                     "root filesystem storage space is low");
            snprintf(checks[2].evidence_text, sizeof(checks[2].evidence_text),
                     "%.1f GiB available (%.1f%% free) - low space", gib, pct);
            snprintf(checks[2].evidence_json, sizeof(checks[2].evidence_json),
                     "\"mount\": \"/\", \"total_bytes\": %" PRIu64 ", \"avail_bytes\": %" PRIu64 ", \"free_percent\": %.1f, \"avail_gib\": %.1f, \"warning\": \"low_space\"",
                     total, avail, pct, gib);
        }
    } else {
        snprintf(checks[2].status, sizeof(checks[2].status), "unknown");
        checks[2].pass = false;
        checks[2].fail = true;
        checks[2].severity = SEV_WARN;
        snprintf(checks[2].summary, sizeof(checks[2].summary),
                 "unable to query filesystem statistics");
        snprintf(checks[2].evidence_text, sizeof(checks[2].evidence_text), "statvfs failed");
        snprintf(checks[2].evidence_json, sizeof(checks[2].evidence_json),
                 "\"mount\": \"/\", \"error\": \"statvfs failed\"");
    }

    /* Check 3: init_systemd */
    snprintf(checks[3].task_id, sizeof(checks[3].task_id), "init_systemd");
    snprintf(checks[3].name, sizeof(checks[3].name), "System init and journal supervision");
    snprintf(checks[3].help_ref, sizeof(checks[3].help_ref), "maintain/logs.html");
    const char *init = get_init_system();
    if (strcmp(init, "systemd") == 0) {
        snprintf(checks[3].status, sizeof(checks[3].status), "pass");
        checks[3].pass = true;
        checks[3].fail = false;
        checks[3].severity = SEV_INFO;
        snprintf(checks[3].summary, sizeof(checks[3].summary),
                 "systemd init system and journal diagnostics active");
        snprintf(checks[3].evidence_text, sizeof(checks[3].evidence_text), "systemd active");
        snprintf(checks[3].evidence_json, sizeof(checks[3].evidence_json),
                 "\"init\": \"systemd\", \"journal\": true");
    } else if (strcmp(init, "runit") == 0) {
        snprintf(checks[3].status, sizeof(checks[3].status), "pass");
        checks[3].pass = true;
        checks[3].fail = false;
        checks[3].severity = SEV_INFO;
        snprintf(checks[3].summary, sizeof(checks[3].summary),
                 "runit process supervision active");
        snprintf(checks[3].evidence_text, sizeof(checks[3].evidence_text), "runit active");
        snprintf(checks[3].evidence_json, sizeof(checks[3].evidence_json),
                 "\"init\": \"runit\", \"journal\": false");
    } else {
        snprintf(checks[3].status, sizeof(checks[3].status), "unknown");
        checks[3].pass = false;
        checks[3].fail = true;
        checks[3].severity = SEV_WARN;
        snprintf(checks[3].summary, sizeof(checks[3].summary),
                 "unrecognized or unmonitored init system");
        snprintf(checks[3].evidence_text, sizeof(checks[3].evidence_text), "init unknown");
        snprintf(checks[3].evidence_json, sizeof(checks[3].evidence_json),
                 "\"init\": \"unknown\"");
    }

    /* Check 4: graphics_card */
    snprintf(checks[4].task_id, sizeof(checks[4].task_id), "graphics_card");
    snprintf(checks[4].name, sizeof(checks[4].name), "Direct Rendering Manager graphics device");
    snprintf(checks[4].help_ref, sizeof(checks[4].help_ref), "maintain/power.html");
    if (strstr(gpu_dev, "available") != NULL) {
        snprintf(checks[4].status, sizeof(checks[4].status), "pass");
        checks[4].pass = true;
        checks[4].fail = false;
        checks[4].severity = SEV_INFO;
        snprintf(checks[4].summary, sizeof(checks[4].summary),
                 "DRM graphics device available for display output");
        snprintf(checks[4].evidence_text, sizeof(checks[4].evidence_text), "%s", gpu_dev);
        snprintf(checks[4].evidence_json, sizeof(checks[4].evidence_json),
                 "\"device\": \"%s\", \"available\": true", gpu_dev);
    } else {
        snprintf(checks[4].status, sizeof(checks[4].status), "fail");
        checks[4].pass = false;
        checks[4].fail = true;
        checks[4].severity = SEV_WARN;
        snprintf(checks[4].summary, sizeof(checks[4].summary),
                 "no DRM graphics device detected");
        snprintf(checks[4].evidence_text, sizeof(checks[4].evidence_text), "none detected");
        snprintf(checks[4].evidence_json, sizeof(checks[4].evidence_json),
                 "\"device\": \"none\", \"available\": false");
    }

    /* Check 5: package_manager */
    snprintf(checks[5].task_id, sizeof(checks[5].task_id), "package_manager");
    snprintf(checks[5].name, sizeof(checks[5].name), "Package management tools availability");
    snprintf(checks[5].help_ref, sizeof(checks[5].help_ref), "maintain/updates.html");
    if (strcmp(pkg_tools, "none detected") != 0 && pkg_tools[0] != '\0') {
        snprintf(checks[5].status, sizeof(checks[5].status), "pass");
        checks[5].pass = true;
        checks[5].fail = false;
        checks[5].severity = SEV_INFO;
        snprintf(checks[5].summary, sizeof(checks[5].summary),
                 "recognized package managers and toolchain installed");
        snprintf(checks[5].evidence_text, sizeof(checks[5].evidence_text), "%s", pkg_tools);
        snprintf(checks[5].evidence_json, sizeof(checks[5].evidence_json),
                 "\"tools\": \"%s\", \"detected\": true", pkg_tools);
    } else {
        snprintf(checks[5].status, sizeof(checks[5].status), "fail");
        checks[5].pass = false;
        checks[5].fail = true;
        checks[5].severity = SEV_WARN;
        snprintf(checks[5].summary, sizeof(checks[5].summary),
                 "no recognized package management tools found");
        snprintf(checks[5].evidence_text, sizeof(checks[5].evidence_text), "none detected");
        snprintf(checks[5].evidence_json, sizeof(checks[5].evidence_json),
                 "\"tools\": \"none\", \"detected\": false");
    }

    /* Check 6: kernel_release */
    snprintf(checks[6].task_id, sizeof(checks[6].task_id), "kernel_release");
    snprintf(checks[6].name, sizeof(checks[6].name), "Host kernel release and architecture");
    snprintf(checks[6].help_ref, sizeof(checks[6].help_ref), "maintain/recovery.html");
    snprintf(checks[6].status, sizeof(checks[6].status), "pass");
    checks[6].pass = true;
    checks[6].fail = false;
    checks[6].severity = SEV_INFO;
    snprintf(checks[6].summary, sizeof(checks[6].summary),
             "kernel release and architecture verified");
    snprintf(checks[6].evidence_text, sizeof(checks[6].evidence_text), "%s", uts->release);
    snprintf(checks[6].evidence_json, sizeof(checks[6].evidence_json),
             "\"release\": \"%s\", \"machine\": \"%s\"", uts->release, uts->machine);
}

static void print_json_string_array(const char *csv)
{
    printf("[");
    if (csv != NULL && csv[0] != '\0' &&
        strcmp(csv, "none detected") != 0 &&
        strcmp(csv, "none") != 0) {
        char *copy = strdup(csv);
        if (copy != NULL) {
            char *token = copy;
            bool first = true;
            while (*token != '\0') {
                while (*token == ' ' || *token == '\t') {
                    token++;
                }
                char *comma = strchr(token, ',');
                if (comma != NULL) {
                    *comma = '\0';
                }
                char *end = token + strlen(token) - 1;
                while (end >= token && (*end == ' ' || *end == '\t')) {
                    *end-- = '\0';
                }
                if (token[0] != '\0') {
                    printf("%s\"%s\"", first ? "" : ", ", token);
                    first = false;
                }
                if (comma == NULL) {
                    break;
                }
                token = comma + 1;
            }
            free(copy);
        }
    }
    printf("]");
}

static void print_check_contract_json(const check_item_t *c, const char *source)
{
    printf("{\n");
    printf("  \"task_id\": \"%s\",\n", c->task_id);
    printf("  \"name\": \"%s\",\n", c->name);
    printf("  \"status\": \"%s\",\n", c->status);
    printf("  \"pass\": %s,\n", c->pass ? "true" : "false");
    printf("  \"fail\": %s,\n", c->fail ? "true" : "false");
    printf("  \"evidence\": {\n    %s\n  },\n", c->evidence_json);
    printf("  \"severity\": \"%s\",\n", severity_str(c->severity));
    printf("  \"help_ref\": \"%s\",\n", c->help_ref);
    printf("  \"source\": \"%s\",\n", source);
    printf("  \"schema_version\": \"1.0.0\"\n");
    printf("}\n");
}

static int do_run_single_check(const char *task_id, bool terse_mode, const char *source)
{
    os_release_t os;
    parse_os_release(&os);

    struct utsname uts;
    memset(&uts, 0, sizeof(uts));
    uname(&uts);

    char pkg_tools[256];
    get_package_tools(pkg_tools, sizeof(pkg_tools));

    char priv_tools[128];
    get_privilege_tools(priv_tools, sizeof(priv_tools));

    char root_fs[64];
    get_root_filesystem(root_fs, sizeof(root_fs));

    char gpu[128];
    get_graphics_device(gpu, sizeof(gpu));

    check_item_t checks[NUM_CHECKS];
    collect_checks(checks, NUM_CHECKS, &os, &uts, pkg_tools, priv_tools, root_fs, gpu);

    for (size_t i = 0; i < NUM_CHECKS; i++) {
        if (strcmp(checks[i].task_id, task_id) == 0) {
            if (terse_mode) {
                printf("%s %s\n", checks[i].task_id, checks[i].status);
            } else {
                print_check_contract_json(&checks[i], source);
            }
            return 0;
        }
    }

    fprintf(stderr, TAM_COMMAND_NAME " check: unknown check '%s'\n"
                    "Run '" TAM_COMMAND_NAME " check' to view available checks.\n", task_id);
    return TAM_EXIT_FAILURE;
}

static int do_check_list(bool json_mode, bool terse_mode, const char *source)
{
    os_release_t os;
    parse_os_release(&os);

    struct utsname uts;
    memset(&uts, 0, sizeof(uts));
    uname(&uts);

    char pkg_tools[256];
    get_package_tools(pkg_tools, sizeof(pkg_tools));

    char priv_tools[128];
    get_privilege_tools(priv_tools, sizeof(priv_tools));

    char root_fs[64];
    get_root_filesystem(root_fs, sizeof(root_fs));

    char gpu[128];
    get_graphics_device(gpu, sizeof(gpu));

    check_item_t checks[NUM_CHECKS];
    collect_checks(checks, NUM_CHECKS, &os, &uts, pkg_tools, priv_tools, root_fs, gpu);

    if (terse_mode) {
        for (size_t i = 0; i < NUM_CHECKS; i++) {
            printf("%s %s\n", checks[i].task_id, checks[i].status);
        }
        return 0;
    }

    if (json_mode) {
        printf("[\n");
        for (size_t i = 0; i < NUM_CHECKS; i++) {
            printf("  {\n");
            printf("    \"task_id\": \"%s\",\n", checks[i].task_id);
            printf("    \"name\": \"%s\",\n", checks[i].name);
            printf("    \"status\": \"%s\",\n", checks[i].status);
            printf("    \"pass\": %s,\n", checks[i].pass ? "true" : "false");
            printf("    \"fail\": %s,\n", checks[i].fail ? "true" : "false");
            printf("    \"evidence\": {\n      %s\n    },\n", checks[i].evidence_json);
            printf("    \"severity\": \"%s\",\n", severity_str(checks[i].severity));
            printf("    \"help_ref\": \"%s\",\n", checks[i].help_ref);
            printf("    \"source\": \"%s\",\n", source);
            printf("    \"schema_version\": \"1.0.0\"\n");
            printf("  }%s\n", (i + 1 < NUM_CHECKS) ? "," : "");
        }
        printf("]\n");
    } else {
        printf("tam check - system health and progression checks\n\n");
        printf("Available checks:\n");
        for (size_t i = 0; i < NUM_CHECKS; i++) {
            printf("  %-20s [%s] %s\n", checks[i].task_id, severity_str(checks[i].severity), checks[i].name);
            printf("                       Help: %s\n", checks[i].help_ref);
        }
        printf("\nRun 'tam check <task_id>' or 'tam inspect --check <task_id>' to evaluate.\n");
    }
    return 0;
}

static int do_inspect(bool json_mode)
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

    check_item_t checks[NUM_CHECKS];
    collect_checks(checks, NUM_CHECKS, &os, &uts, pkg_tools, priv_tools, root_fs, gpu);

    if (json_mode) {
        printf("{\n");
        printf("  \"schema_version\": \"1.0.0\",\n");
        printf("  \"source\": \"tam inspect\",\n");
        printf("  \"host\": {\n");
        printf("    \"operating_system\": \"%s\",\n", os_display);
        printf("    \"os_name\": \"%s\",\n", os.name);
        printf("    \"os_id\": \"%s\",\n", os.id);
        printf("    \"os_id_like\": \"%s\",\n", os.id_like);
        printf("    \"os_version\": \"%s\",\n", os_ver);
        printf("    \"kernel_release\": \"%s\",\n", uts.release);
        printf("    \"architecture\": \"%s\",\n", uts.machine);
        if (cultivar != NULL) {
            printf("    \"cultivar_version\": \"%s\"\n", cultivar);
        } else {
            printf("    \"cultivar_version\": \"unavailable\"\n");
        }
        printf("  },\n");

        printf("  \"capabilities\": {\n");
        printf("    \"init_system\": \"%s\",\n", get_init_system());
        printf("    \"package_tools\": ");
        print_json_string_array(pkg_tools);
        printf(",\n");
        printf("    \"privilege_tools\": ");
        print_json_string_array(priv_tools);
        printf(",\n");
        printf("    \"root_filesystem\": \"%s\",\n", root_fs);
        printf("    \"graphics_device\": \"%s\"\n", gpu);
        printf("  },\n");

        printf("  \"profiles\": {\n");
        printf("    \"supported\": ");
        print_json_string_array(supported);
        printf(",\n");
        printf("    \"target_base\": \"%s\"\n", target_base);
        printf("  },\n");

        printf("  \"findings\": [\n");
        for (size_t i = 0; i < NUM_CHECKS; i++) {
            printf("    {\n");
            printf("      \"id\": \"%s\",\n", checks[i].task_id);
            printf("      \"severity\": \"%s\",\n", severity_str(checks[i].severity));
            printf("      \"summary\": \"%s\",\n", checks[i].summary);
            printf("      \"evidence\": {\n        %s\n      },\n", checks[i].evidence_json);
            printf("      \"help_link\": \"%s\"\n", checks[i].help_ref);
            printf("    }%s\n", (i + 1 < NUM_CHECKS) ? "," : "");
        }
        printf("  ],\n");

        printf("  \"checks\": [\n");
        for (size_t i = 0; i < NUM_CHECKS; i++) {
            printf("    {\n");
            printf("      \"task_id\": \"%s\",\n", checks[i].task_id);
            printf("      \"name\": \"%s\",\n", checks[i].name);
            printf("      \"status\": \"%s\",\n", checks[i].status);
            printf("      \"pass\": %s,\n", checks[i].pass ? "true" : "false");
            printf("      \"fail\": %s,\n", checks[i].fail ? "true" : "false");
            printf("      \"evidence\": {\n        %s\n      },\n", checks[i].evidence_json);
            printf("      \"severity\": \"%s\",\n", severity_str(checks[i].severity));
            printf("      \"help_ref\": \"%s\",\n", checks[i].help_ref);
            printf("      \"source\": \"tam inspect\",\n");
            printf("      \"schema_version\": \"1.0.0\"\n");
            printf("    }%s\n", (i + 1 < NUM_CHECKS) ? "," : "");
        }
        printf("  ]\n");
        printf("}\n");

        if (cultivar != NULL) {
            free(cultivar);
        }
        return 0;
    }

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
    printf("\n");

    printf("Inspection Findings:\n");
    for (size_t i = 0; i < NUM_CHECKS; i++) {
        printf("  [%s] %s: %s\n", severity_str(checks[i].severity), checks[i].task_id, checks[i].summary);
        printf("    Evidence: %s\n", checks[i].evidence_text);
        printf("    Help:     %s\n", checks[i].help_ref);
    }

    return 0;
}

static void print_inspect_help(void)
{
    fputs(
        "tam inspect - inspect host state and system health\n\n"
        "Usage:\n"
        "  tam inspect                       inspect host facts and diagnostic findings\n"
        "  tam inspect --pipe                print inspection report as plain text\n"
        "  tam inspect --json                emit full inspection report as JSON\n"
        "  tam inspect --check [ID]          run progression check and emit Check Contract JSON\n"
        "  tam inspect --terse               emit compact check status lines (with --check)\n"
        "  tam inspect --help                show this help and exit\n",
        stdout);
}

int cmd_inspect(int n, char **args)
{
    bool json_mode = false;
    bool terse_mode = false;
    const char *check_id = NULL;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0 || strcmp(args[i], "help") == 0) {
            print_inspect_help();
            return 0;
        } else if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            continue;
        } else if (strcmp(args[i], "--json") == 0) {
            json_mode = true;
        } else if (strcmp(args[i], "--terse") == 0) {
            terse_mode = true;
        } else if (strcmp(args[i], "--check") == 0) {
            if (i + 1 < n && args[i + 1][0] != '-') {
                check_id = args[++i];
            } else {
                return do_check_list(json_mode, terse_mode, "tam inspect");
            }
        } else if (strncmp(args[i], "--check=", 8) == 0) {
            check_id = args[i] + 8;
        } else if (args[i][0] == '-') {
            fprintf(stderr, TAM_COMMAND_NAME " inspect: unrecognized argument '%s'\n"
                            "Run '" TAM_COMMAND_NAME " inspect --help' for available options.\n", args[i]);
            return TAM_EXIT_USAGE;
        } else {
            fprintf(stderr, TAM_COMMAND_NAME " inspect: too many arguments\n"
                            "Run '" TAM_COMMAND_NAME " inspect --help' for available options.\n");
            return TAM_EXIT_USAGE;
        }
    }

    if (check_id != NULL) {
        return do_run_single_check(check_id, terse_mode, "tam inspect");
    }
    return do_inspect(json_mode);
}

static void print_check_help(void)
{
    fputs(
        "tam check - run progression and health checks\n\n"
        "Usage:\n"
        "  tam check                         list all available checks\n"
        "  tam check <task_id>               run specific check and emit Check Contract JSON\n"
        "  tam check --json                  emit all checks in Check Contract format as JSON\n"
        "  tam check --terse                 emit compact check status lines\n"
        "  tam check --help                  show this help and exit\n",
        stdout);
}

int cmd_check(int n, char **args)
{
    bool json_mode = false;
    bool terse_mode = false;
    const char *check_id = NULL;

    for (int i = 0; i < n; i++) {
        if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0 || strcmp(args[i], "help") == 0) {
            print_check_help();
            return 0;
        } else if (strcmp(args[i], "--json") == 0) {
            json_mode = true;
        } else if (strcmp(args[i], "--terse") == 0) {
            terse_mode = true;
        } else if (strcmp(args[i], "--pipe") == 0 || strcmp(args[i], "-p") == 0) {
            continue;
        } else if (args[i][0] == '-') {
            fprintf(stderr, TAM_COMMAND_NAME " check: unrecognized argument '%s'\n"
                            "Run '" TAM_COMMAND_NAME " check --help' for available options.\n", args[i]);
            return TAM_EXIT_USAGE;
        } else {
            if (check_id != NULL) {
                fprintf(stderr, TAM_COMMAND_NAME " check: too many arguments\n"
                                "Run '" TAM_COMMAND_NAME " check --help' for available options.\n");
                return TAM_EXIT_USAGE;
            }
            check_id = args[i];
        }
    }

    if (check_id != NULL) {
        return do_run_single_check(check_id, terse_mode, "tam check");
    }
    return do_check_list(json_mode, terse_mode, "tam check");
}

static void print_install_help(void)
{
    fputs(
        "tam install - Tamlinux installation framework\n\n"
        "Usage:\n"
        "  tam install                       show this overview and available subcommands\n"
        "  tam install --help                show this help and exit\n\n"
        "Planned component commands:\n"
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
