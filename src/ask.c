#include <tam.h>

#include "ask.h"
#include "index.h"
#include "version.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define TAM_EXIT_USAGE 2
#define TAM_EXIT_FAILURE 1

#ifndef TAM_DATADIR
#define TAM_DATADIR "/usr/local/share/tamlinux/knowledge"
#endif

#define DEFAULT_SERVER "http://127.0.0.1:11434"
#define DEFAULT_MODEL "qwen2.5-coder:1.5b"
#define FALLBACK_MODEL "llama3.2:1b"
#define DEFAULT_TIMEOUT_PROBE_MS 500
#define DEFAULT_TIMEOUT_GENERATE_MS 30000
#define MAX_CONTEXT_BYTES 4000

/* ------------------------------------------------------------------------- */
/* Knowledge root resolution                                                 */
/* ------------------------------------------------------------------------- */

static char *join_path(const char *dir, const char *rest)
{
    size_t dlen = strlen(dir);
    size_t rlen = strlen(rest);
    char *out = malloc(dlen + rlen + 1);
    if (out == NULL) return NULL;
    memcpy(out, dir, dlen);
    memcpy(out + dlen, rest, rlen + 1);
    return out;
}

static int knowledge_ok(const char *dir)
{
    if (dir == NULL || dir[0] == '\0') return 0;
    char *p = join_path(dir, "/index.html");
    int ok = p != NULL && tam_fs_file_exists(p);
    free(p);
    return ok;
}

static char *find_knowledge_root(void)
{
    const char *env = getenv("TAM_KNOWLEDGE_DIR");
    if (env != NULL && knowledge_ok(env)) {
        return strdup(env);
    }
    const char *compiled = getenv("TAM_DATADIR");
    if (compiled == NULL || compiled[0] == '\0') {
        compiled = TAM_DATADIR;
    }
    if (knowledge_ok(compiled)) {
        return strdup(compiled);
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* JSON String Helpers                                                       */
/* ------------------------------------------------------------------------- */

static void json_append_escaped(tam_str_t *dest, const char *src)
{
    if (src == NULL) return;
    for (const char *p = src; *p != '\0'; p++) {
        switch (*p) {
        case '"':  *dest = tam_str_cat(*dest, "\\\""); break;
        case '\\': *dest = tam_str_cat(*dest, "\\\\"); break;
        case '\b': *dest = tam_str_cat(*dest, "\\b"); break;
        case '\f': *dest = tam_str_cat(*dest, "\\f"); break;
        case '\n': *dest = tam_str_cat(*dest, "\\n"); break;
        case '\r': *dest = tam_str_cat(*dest, "\\r"); break;
        case '\t': *dest = tam_str_cat(*dest, "\\t"); break;
        default:
            if ((unsigned char)*p < 0x20) {
                *dest = tam_str_cat_printf(*dest, "\\u%04x", (unsigned char)*p);
            } else {
                char c[2] = {*p, '\0'};
                *dest = tam_str_cat(*dest, c);
            }
            break;
        }
    }
}

static char *extract_json_string(const char *json, const char *key)
{
    if (json == NULL || key == NULL) return NULL;
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(json, needle);
    if (p == NULL) return NULL;
    p += strlen(needle);
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == ':') p++;
    if (*p != '"') return NULL;
    p++; /* skip opening quote */

    tam_str_t res = tam_str_new("");
    while (*p != '\0') {
        if (*p == '"') {
            break;
        } else if (*p == '\\') {
            p++;
            if (*p == 'n') res = tam_str_cat(res, "\n");
            else if (*p == 'r') res = tam_str_cat(res, "\r");
            else if (*p == 't') res = tam_str_cat(res, "\t");
            else if (*p == '"') res = tam_str_cat(res, "\"");
            else if (*p == '\\') res = tam_str_cat(res, "\\");
            else if (*p != '\0') {
                char c[2] = {*p, '\0'};
                res = tam_str_cat(res, c);
            }
        } else {
            char c[2] = {*p, '\0'};
            res = tam_str_cat(res, c);
        }
        p++;
    }
    char *out = strdup(res);
    tam_str_free(res);
    return out;
}

/* ------------------------------------------------------------------------- */
/* Plain text extraction from HTML                                           */
/* ------------------------------------------------------------------------- */

static char *extract_plain_text_from_html(const char *html, size_t max_bytes)
{
    if (html == NULL) return strdup("");
    tam_str_t out = tam_str_new("");
    bool in_tag = false;
    bool in_script = false;
    bool in_style = false;
    bool prev_space = false;

    for (const char *p = html; *p != '\0' && tam_str_len(out) < max_bytes; p++) {
        if (*p == '<') {
            in_tag = true;
            if (strncasecmp(p, "<script", 7) == 0) in_script = true;
            if (strncasecmp(p, "<style", 6) == 0) in_style = true;
            if (strncasecmp(p, "</script", 8) == 0) in_script = false;
            if (strncasecmp(p, "</style", 7) == 0) in_style = false;
            if (strncasecmp(p, "<p", 2) == 0 || strncasecmp(p, "<br", 3) == 0 ||
                strncasecmp(p, "<li", 3) == 0 || strncasecmp(p, "<h", 2) == 0) {
                if (!prev_space) {
                    out = tam_str_cat(out, "\n");
                    prev_space = true;
                }
            }
            continue;
        }
        if (*p == '>') {
            in_tag = false;
            continue;
        }
        if (in_tag || in_script || in_style) {
            continue;
        }

        /* Entity decoding */
        if (*p == '&') {
            if (strncmp(p, "&lt;", 4) == 0) { out = tam_str_cat(out, "<"); p += 3; prev_space = false; continue; }
            if (strncmp(p, "&gt;", 4) == 0) { out = tam_str_cat(out, ">"); p += 3; prev_space = false; continue; }
            if (strncmp(p, "&amp;", 5) == 0) { out = tam_str_cat(out, "&"); p += 4; prev_space = false; continue; }
            if (strncmp(p, "&quot;", 6) == 0) { out = tam_str_cat(out, "\""); p += 5; prev_space = false; continue; }
            if (strncmp(p, "&#39;", 5) == 0) { out = tam_str_cat(out, "'"); p += 4; prev_space = false; continue; }
        }

        if (isspace((unsigned char)*p)) {
            if (!prev_space) {
                out = tam_str_cat(out, " ");
                prev_space = true;
            }
        } else {
            char c[2] = {*p, '\0'};
            out = tam_str_cat(out, c);
            prev_space = false;
        }
    }

    char *result = strdup(out);
    tam_str_free(out);
    return result;
}

/* ------------------------------------------------------------------------- */
/* HTTP Client via Non-blocking POSIX Sockets                                */
/* ------------------------------------------------------------------------- */

static bool parse_http_url(const char *url, char *host, size_t host_sz, int *port, char *path, size_t path_sz)
{
    if (url == NULL) return false;
    const char *p = url;
    if (strncmp(p, "http://", 7) == 0) {
        p += 7;
    } else if (strncmp(p, "https://", 8) == 0) {
        /* Simple HTTP client handles plain HTTP localhost/loopback */
        p += 8;
    }

    const char *slash = strchr(p, '/');
    const char *colon = strchr(p, ':');

    if (slash != NULL && colon != NULL && colon > slash) {
        colon = NULL; /* colon in path, not in authority */
    }

    if (colon != NULL) {
        size_t hlen = (size_t)(colon - p);
        if (hlen >= host_sz) hlen = host_sz - 1;
        memcpy(host, p, hlen);
        host[hlen] = '\0';
        *port = atoi(colon + 1);
    } else if (slash != NULL) {
        size_t hlen = (size_t)(slash - p);
        if (hlen >= host_sz) hlen = host_sz - 1;
        memcpy(host, p, hlen);
        host[hlen] = '\0';
        *port = 80;
    } else {
        snprintf(host, host_sz, "%s", p);
        *port = 80;
    }

    if (slash != NULL) {
        snprintf(path, path_sz, "%s", slash);
    } else {
        snprintf(path, path_sz, "/");
    }
    return true;
}

static int http_request(const char *host, int port, const char *method, const char *path,
                         const char *body, int timeout_ms, char **resp_body, int *status_code)
{
    if (resp_body) *resp_body = NULL;
    if (status_code) *status_code = 0;

    struct addrinfo hints, *res = NULL, *rp = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    int s = getaddrinfo(host, port_str, &hints, &res);
    if (s != 0) {
        return -1;
    }

    int sock = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) continue;

        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);

        int rc = connect(sock, rp->ai_addr, rp->ai_addrlen);
        if (rc == 0) {
            break; /* connected immediately */
        } else if (errno == EINPROGRESS) {
            struct pollfd pfd;
            pfd.fd = sock;
            pfd.events = POLLOUT;
            int poll_rc = poll(&pfd, 1, timeout_ms);
            if (poll_rc > 0 && (pfd.revents & POLLOUT)) {
                int err = 0;
                socklen_t len = sizeof(err);
                if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) == 0 && err == 0) {
                    break; /* connected successfully */
                }
            }
        }
        close(sock);
        sock = -1;
    }
    freeaddrinfo(res);

    if (sock == -1) {
        return -1;
    }

    /* Build HTTP request */
    tam_str_t req = tam_str_new("");
    req = tam_str_cat_printf(req, "%s %s HTTP/1.1\r\n", method, path);
    req = tam_str_cat_printf(req, "Host: %s:%d\r\n", host, port);
    req = tam_str_cat(req, "User-Agent: tam\r\n");
    req = tam_str_cat(req, "Connection: close\r\n");
    if (body != NULL) {
        req = tam_str_cat(req, "Content-Type: application/json\r\n");
        req = tam_str_cat_printf(req, "Content-Length: %zu\r\n\r\n", strlen(body));
        req = tam_str_cat(req, body);
    } else {
        req = tam_str_cat(req, "\r\n");
    }

    /* Send request with timeout */
    size_t total_sent = 0;
    size_t req_len = tam_str_len(req);
    while (total_sent < req_len) {
        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLOUT;
        int poll_rc = poll(&pfd, 1, timeout_ms);
        if (poll_rc <= 0) {
            tam_str_free(req);
            close(sock);
            return -1;
        }
        ssize_t n = write(sock, req + total_sent, req_len - total_sent);
        if (n <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            tam_str_free(req);
            close(sock);
            return -1;
        }
        total_sent += (size_t)n;
    }
    tam_str_free(req);

    /* Read response with timeout */
    tam_str_t raw_resp = tam_str_new("");
    char buf[4096];
    while (1) {
        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLIN;
        int poll_rc = poll(&pfd, 1, timeout_ms);
        if (poll_rc <= 0) {
            break; /* timeout or end */
        }
        ssize_t n = read(sock, buf, sizeof(buf));
        if (n <= 0) {
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
            break; /* connection closed or EOF */
        }
        raw_resp = tam_str_cat_len(raw_resp, buf, (size_t)n);
    }
    close(sock);

    if (tam_str_len(raw_resp) == 0) {
        tam_str_free(raw_resp);
        return -1;
    }

    /* Parse status code */
    if (status_code != NULL) {
        const char *proto = strstr(raw_resp, "HTTP/");
        if (proto != NULL) {
            const char *sp = strchr(proto, ' ');
            if (sp != NULL) {
                *status_code = atoi(sp + 1);
            }
        }
    }

    /* Find body separator \r\n\r\n */
    const char *hdr_end = strstr(raw_resp, "\r\n\r\n");
    if (hdr_end != NULL) {
        hdr_end += 4;
        if (resp_body != NULL) {
            *resp_body = strdup(hdr_end);
        }
    } else {
        if (resp_body != NULL) {
            *resp_body = strdup(raw_resp);
        }
    }

    tam_str_free(raw_resp);
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Model Check & Interactive Consent                                         */
/* ------------------------------------------------------------------------- */

static bool check_model_available(const char *tags_json, const char *model_name)
{
    if (tags_json == NULL || model_name == NULL) return false;
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", model_name);
    if (strstr(tags_json, needle) != NULL) return true;

    /* Check with :latest suffix if not provided */
    if (strchr(model_name, ':') == NULL) {
        snprintf(needle, sizeof(needle), "\"%s:latest\"", model_name);
        if (strstr(tags_json, needle) != NULL) return true;
    }
    return false;
}

/* ------------------------------------------------------------------------- */
/* Main Subcommand Implementation                                            */
/* ------------------------------------------------------------------------- */

int tam_cmd_ask(int argc, char **argv)
{
    const char *server_arg = NULL;
    const char *model_arg = NULL;
    bool json_mode = false;
    bool pipe_mode = false;
    char query_buf[1024] = {0};
    size_t qlen = 0;
    bool from_stdin = false;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf(
                "tam ask - opt-in grounded question answering over the Tamlinux guide\n\n"
                "Usage:\n"
                "  tam ask [OPTIONS] <question>\n"
                "  tam ask [OPTIONS] -\n\n"
                "Options:\n"
                "  --model <name>     local model name (default: $TAM_ASK_MODEL or '%s')\n"
                "  --server <url>     local inference server (default: $TAM_ASK_SERVER or '%s')\n"
                "  --json             emit structured machine-readable JSON output\n"
                "  --pipe             plain text output without terminal controls\n"
                "  -h, --help         show this help and exit\n\n"
                "The ask command connects only to an already-running local inference server\n"
                "and grounds its answers in the offline Tamlinux guide with mandatory citations.\n",
                DEFAULT_MODEL, DEFAULT_SERVER);
            return 0;
        } else if (strcmp(argv[i], "--json") == 0) {
            json_mode = true;
        } else if (strcmp(argv[i], "--pipe") == 0 || strcmp(argv[i], "-p") == 0) {
            pipe_mode = true;
        } else if (strcmp(argv[i], "--model") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "tam ask: option '--model' requires an argument\n");
                return TAM_EXIT_USAGE;
            }
            model_arg = argv[++i];
        } else if (strcmp(argv[i], "--server") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "tam ask: option '--server' requires an argument\n");
                return TAM_EXIT_USAGE;
            }
            server_arg = argv[++i];
        } else if (strcmp(argv[i], "-") == 0 && qlen == 0) {
            from_stdin = true;
        } else if (argv[i][0] == '-' && qlen == 0) {
            fprintf(stderr, "tam ask: unrecognized option '%s'\n", argv[i]);
            return TAM_EXIT_USAGE;
        } else {
            if (qlen > 0 && qlen + 1 < sizeof(query_buf)) {
                query_buf[qlen++] = ' ';
            }
            size_t alen = strlen(argv[i]);
            if (qlen + alen < sizeof(query_buf)) {
                memcpy(query_buf + qlen, argv[i], alen);
                qlen += alen;
                query_buf[qlen] = '\0';
            }
        }
    }

    if (from_stdin) {
        char buf[1024];
        while (fgets(buf, sizeof(buf), stdin) != NULL) {
            size_t blen = strlen(buf);
            if (qlen + blen < sizeof(query_buf)) {
                memcpy(query_buf + qlen, buf, blen);
                qlen += blen;
                query_buf[qlen] = '\0';
            }
        }
        /* Trim trailing whitespace */
        while (qlen > 0 && isspace((unsigned char)query_buf[qlen - 1])) {
            query_buf[--qlen] = '\0';
        }
    }

    if (qlen == 0) {
        fprintf(stderr, "tam ask: missing question\n");
        fprintf(stderr, "Run 'tam ask --help' for available options.\n");
        return TAM_EXIT_USAGE;
    }

    bool is_tty = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    if (pipe_mode) {
        is_tty = false;
    }

    /* Resolve server URL */
    const char *server_url = server_arg;
    if (server_url == NULL || server_url[0] == '\0') {
        server_url = getenv("TAM_ASK_SERVER");
    }
    if (server_url == NULL || server_url[0] == '\0') {
        server_url = DEFAULT_SERVER;
    }

    /* Resolve model name */
    const char *model_name = model_arg;
    bool model_explicit = (model_name != NULL && model_name[0] != '\0');
    if (!model_explicit) {
        model_name = getenv("TAM_ASK_MODEL");
        if (model_name != NULL && model_name[0] != '\0') {
            model_explicit = true;
        }
    }
    if (model_name == NULL || model_name[0] == '\0') {
        model_name = DEFAULT_MODEL;
    }

    /* Parse server URL */
    char host[256];
    int port = 11434;
    char path[256];
    if (!parse_http_url(server_url, host, sizeof(host), &port, path, sizeof(path))) {
        fprintf(stderr, "tam ask: invalid server URL '%s'\n", server_url);
        return TAM_EXIT_FAILURE;
    }

    /* 1. Health probe: probe /api/tags */
    char *tags_json = NULL;
    int status = 0;
    int req_rc = http_request(host, port, "GET", "/api/tags", NULL, DEFAULT_TIMEOUT_PROBE_MS, &tags_json, &status);

    if (req_rc != 0 || status != 200 || tags_json == NULL) {
        free(tags_json);
        if (!is_tty) {
            /* Exact non-interactive refusal contract */
            fprintf(stderr, "tam: ask requires a local model; run interactively to fetch or start Ollama\n");
            return TAM_EXIT_USAGE;
        } else {
            /* Interactive server-down refusal with search fallback */
            fprintf(stderr, "tam: no local model available (Ollama not running). Start Ollama or run 'tam search %s' instead.\n", query_buf);
            
            char *root = find_knowledge_root();
            if (root != NULL) {
                tam_index_t *idx = tam_index_load(root);
                if (idx != NULL) {
                    tam_search_result_t results[4];
                    size_t found = tam_index_search(idx, query_buf, results, 4);
                    if (found > 0 && results[0].score > 0) {
                        fprintf(stderr, "\nRelated topics in Tamlinux guide:\n");
                        for (size_t i = 0; i < found && i < 3; i++) {
                            fprintf(stderr, "  tam show %s  (%s)\n", results[i].entry->path, results[i].entry->title);
                        }
                    }
                    tam_index_free(idx);
                }
                free(root);
            }
            return TAM_EXIT_FAILURE;
        }
    }

    /* 2. Check model availability */
    bool model_ok = false;
    if (model_explicit) {
        model_ok = check_model_available(tags_json, model_name);
    } else {
        /* When no explicit model was requested, prefer DEFAULT_MODEL, then FALLBACK_MODEL, then any installed model */
        if (check_model_available(tags_json, DEFAULT_MODEL)) {
            model_name = DEFAULT_MODEL;
            model_ok = true;
        } else if (check_model_available(tags_json, FALLBACK_MODEL)) {
            model_name = FALLBACK_MODEL;
            model_ok = true;
        } else {
            char *first_model = extract_json_string(tags_json, "name");
            if (first_model != NULL && first_model[0] != '\0') {
                model_name = first_model;
                model_ok = true;
            } else {
                model_name = DEFAULT_MODEL;
                model_ok = false;
            }
        }
    }

    if (!model_ok) {
        free(tags_json);
        if (!is_tty) {
            fprintf(stderr, "tam: ask requires a local model; run interactively to fetch or start Ollama\n");
            return TAM_EXIT_USAGE;
        }

        const char *allow_fetch = getenv("TAM_ASK_ALLOW_FETCH");
        if (allow_fetch != NULL && strcmp(allow_fetch, "false") == 0) {
            fprintf(stderr, "tam: model '%s' is not installed and model fetching is disabled ($TAM_ASK_ALLOW_FETCH=false).\n", model_name);
            return TAM_EXIT_FAILURE;
        }

        /* Interactive Consent Prompt */
        printf("Model '%s' is not installed locally.\n", model_name);
        printf("URL: https://ollama.com/library/%s\n", model_name);
        if (strstr(model_name, "qwen2.5-coder:1.5b") != NULL) {
            printf("Estimated Size: ~986 MB\n");
        } else if (strstr(model_name, "llama3.2:1b") != NULL) {
            printf("Estimated Size: ~1.3 GB\n");
        } else {
            printf("Estimated Size: ~1-2 GB\n");
        }
        printf("Storage: ~/.ollama/models\n");
        printf("Fetch model now? [y/N] ");
        fflush(stdout);

        char resp[32] = {0};
        if (fgets(resp, sizeof(resp), stdin) == NULL || (resp[0] != 'y' && resp[0] != 'Y')) {
            fprintf(stderr, "tam: model fetch declined.\n");
            return TAM_EXIT_FAILURE;
        }

        printf("Fetching model '%s' via Ollama...\n", model_name);
        tam_str_t pull_body = tam_str_new("{\"name\": \"");
        json_append_escaped(&pull_body, model_name);
        pull_body = tam_str_cat(pull_body, "\"}");

        char *pull_resp = NULL;
        int pull_status = 0;
        int pull_rc = http_request(host, port, "POST", "/api/pull", pull_body, 120000, &pull_resp, &pull_status);
        tam_str_free(pull_body);
        free(pull_resp);

        if (pull_rc != 0 || pull_status != 200) {
            fprintf(stderr, "tam: failed to fetch model '%s'.\n", model_name);
            return TAM_EXIT_FAILURE;
        }
    } else {
        free(tags_json);
    }

    /* 3. Corpus Grounding via Index */
    char *root = find_knowledge_root();
    if (root == NULL) {
        fprintf(stderr, "tam: no offline guide installed\n");
        return TAM_EXIT_FAILURE;
    }

    tam_index_t *idx = tam_index_load(root);
    if (idx == NULL) {
        fprintf(stderr, "tam: failed to load guide index\n");
        free(root);
        return TAM_EXIT_FAILURE;
    }

    tam_search_result_t search_results[3];
    size_t found = tam_index_search(idx, query_buf, search_results, 3);

    /* Grounding check: if zero matches or top score <= 0 */
    if (found == 0 || search_results[0].score <= 0) {
        tam_index_free(idx);
        free(root);

        if (json_mode) {
            printf("{\n");
            printf("  \"question\": \"");
            tam_str_t qesc = tam_str_new("");
            json_append_escaped(&qesc, query_buf);
            printf("%s\",\n", qesc);
            tam_str_free(qesc);
            printf("  \"grounded\": false,\n");
            printf("  \"answer\": \"Not found in Tamlinux guide.\",\n");
            printf("  \"sources\": [],\n");
            printf("  \"model\": \"%s\",\n", model_name);
            printf("  \"server\": \"%s\",\n", server_url);
            printf("  \"schema_version\": \"1.0.0\"\n");
            printf("}\n");
        } else {
            printf("Not found in Tamlinux guide.\n");
            printf("Sources: (none)\n\n");
            printf("Try 'tam search %s' to find related topics.\n", query_buf);
        }
        return 0;
    }

    /* 4. Build Grounded Context from Excerpts */
    tam_str_t prompt_ctx = tam_str_new(
        "You are the Tamlinux offline guide assistant. Answer the user question concisely (in 2-5 sentences) based ONLY on the excerpts below.\n"
        "At the end of your answer, include a 'Sources:' line citing the source path(s).\n\n"
        "Documentation Excerpts:\n"
    );

    size_t total_excerpt_bytes = 0;
    for (size_t i = 0; i < found && i < 3; i++) {
        const tam_index_entry_t *entry = search_results[i].entry;
        char *fpath = join_path(root, "/");
        char *full_path = join_path(fpath, entry->path);
        free(fpath);

        size_t fsize = 0;
        char *html = tam_fs_read_file(full_path, &fsize);
        free(full_path);

        if (html != NULL) {
            char *plain = extract_plain_text_from_html(html, 1500);
            free(html);
            if (plain != NULL) {
                prompt_ctx = tam_str_cat_printf(prompt_ctx, "[Source: %s (%s)]\n%s\n\n", entry->path, entry->title, plain);
                total_excerpt_bytes += strlen(plain);
                free(plain);
            }
        }
        if (total_excerpt_bytes >= MAX_CONTEXT_BYTES) {
            break;
        }
    }

    prompt_ctx = tam_str_cat_printf(prompt_ctx, "Question: %s\nAnswer:\n", query_buf);

    /* 5. Query Model */
    tam_str_t req_json = tam_str_new("{\"model\": \"");
    json_append_escaped(&req_json, model_name);
    req_json = tam_str_cat(req_json, "\", \"prompt\": \"");
    json_append_escaped(&req_json, prompt_ctx);
    req_json = tam_str_cat(req_json, "\", \"stream\": false, \"options\": {\"num_predict\": 300, \"temperature\": 0.2}}");
    tam_str_free(prompt_ctx);

    char *gen_resp = NULL;
    int gen_status = 0;
    int gen_rc = http_request(host, port, "POST", "/api/generate", req_json, DEFAULT_TIMEOUT_GENERATE_MS, &gen_resp, &gen_status);
    tam_str_free(req_json);

    if (gen_rc != 0 || gen_status != 200 || gen_resp == NULL) {
        free(gen_resp);
        tam_index_free(idx);
        free(root);
        fprintf(stderr, "tam ask: inference query failed\n");
        return TAM_EXIT_FAILURE;
    }

    char *answer = extract_json_string(gen_resp, "response");
    free(gen_resp);

    if (answer == NULL || answer[0] == '\0') {
        free(answer);
        answer = strdup("Not found in Tamlinux guide.");
    }

    /* 6. Verify and Ensure Mandatory Citations */
    bool has_citations = (strstr(answer, "Sources:") != NULL);
    if (!has_citations && strstr(answer, "Not found in Tamlinux guide") == NULL) {
        tam_str_t enriched = tam_str_new(answer);
        enriched = tam_str_cat(enriched, "\n\nSources:\n");
        for (size_t i = 0; i < found && i < 3; i++) {
            enriched = tam_str_cat_printf(enriched, "  - %s (%s)\n",
                                          search_results[i].entry->path,
                                          search_results[i].entry->title);
        }
        free(answer);
        answer = strdup(enriched);
        tam_str_free(enriched);
    }

    /* 7. Output Result */
    if (json_mode) {
        printf("{\n");
        printf("  \"question\": \"");
        tam_str_t qesc = tam_str_new("");
        json_append_escaped(&qesc, query_buf);
        printf("%s\",\n", qesc);
        tam_str_free(qesc);

        printf("  \"grounded\": true,\n");
        printf("  \"answer\": \"");
        tam_str_t ansesc = tam_str_new("");
        json_append_escaped(&ansesc, answer);
        printf("%s\",\n", ansesc);
        tam_str_free(ansesc);

        printf("  \"sources\": [\n");
        for (size_t i = 0; i < found && i < 3; i++) {
            printf("    {\n");
            printf("      \"path\": \"%s\",\n", search_results[i].entry->path);
            printf("      \"title\": \"%s\"\n", search_results[i].entry->title);
            printf("    }%s\n", (i + 1 < found && i + 1 < 3) ? "," : "");
        }
        printf("  ],\n");
        printf("  \"model\": \"%s\",\n", model_name);
        printf("  \"server\": \"%s\",\n", server_url);
        printf("  \"schema_version\": \"1.0.0\"\n");
        printf("}\n");
    } else {
        printf("%s\n", answer);
    }

    free(answer);
    tam_index_free(idx);
    free(root);
    return 0;
}
