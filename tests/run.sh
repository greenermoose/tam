#!/bin/sh
# Behavior checks for the tam command foundation (introduction, --help,
# --version, argument errors). None of these paths may write state.
set -eu

TAM=${1:-build/tam}
case $TAM in
    /*) ;;
    *) TAM=$(cd "$(dirname "$TAM")" && pwd)/$(basename "$TAM") ;;
esac

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/.." && pwd)
knowledge="$root/knowledge"
export TAM_KNOWLEDGE_DIR="$knowledge"

fail() { printf 'FAIL: %s\n' "$1" >&2; exit 1; }
pass() { printf 'ok - %s\n' "$1"; }

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

esc=$(printf '\033')
home="$tmp/home"
config="$tmp/config"
state="$tmp/state"

# Run the command with no cultivar metadata visible, whatever the caller's env.
iso() {
    env HOME="$home" XDG_CONFIG_HOME="$config" XDG_STATE_HOME="$state" \
        env -u TAMLINUX_VERSION "$@"
}

# Bare invocation: exit 0, useful output, no state written.
out=$(iso "$TAM")
[ -n "$out" ] || fail "bare tam produced no output"
if [ -n "$(find "$tmp" -mindepth 1 -print -quit 2>/dev/null)" ]; then
    fail "bare tam wrote state under an isolated home"
fi
pass "bare invocation prints an introduction and writes no state"

# Help.
"$TAM" --help | grep -q 'Usage:' || fail "--help is missing usage"
"$TAM" -h >/dev/null || fail "-h failed"
"$TAM" help | grep -q 'Usage:' || fail "help is missing usage"
[ "$("$TAM" help)" = "$("$TAM" --help)" ] || fail "help differs from --help"
if "$TAM" help extra >/dev/null 2>&1; then fail "help with extra args exited 0"; fi
pass "--help, -h and help"

# Version: command and library versions are reported; metadata is explicit.
"$TAM" --version | grep -q '0.13.0' || fail "--version missing command version"
"$TAM" --version | grep -q 'libtam' || fail "--version missing libtam"
"$TAM" -V >/dev/null || fail "-V failed"
iso "$TAM" --version | grep -q 'unavailable' || fail "missing cultivar metadata note"
pass "--version and -V"

# Cultivar version: env fallback, and the installed file wins over env.
env HOME="$home" XDG_CONFIG_HOME="$config" XDG_STATE_HOME="$state" \
    TAMLINUX_VERSION=9.9.9 "$TAM" --version | grep -q '9.9.9' \
    || fail "env override ignored"
mkdir -p "$config/tamlinux"
printf '7.7.7\n' > "$config/tamlinux/version"
env HOME="$home" XDG_CONFIG_HOME="$config" XDG_STATE_HOME="$state" \
    TAMLINUX_VERSION=9.9.9 "$TAM" --version | grep -q '7.7.7' \
    || fail "version file not preferred over env"
pass "cultivar version resolution"

# Piped output must not contain terminal control sequences.
if "$TAM" | grep -q "$esc"; then fail "bare output contains escape sequences"; fi
if "$TAM" --help | grep -q "$esc"; then fail "help contains escape sequences"; fi
if "$TAM" welcome --pipe | grep -q "$esc"; then fail "welcome contains escape sequences"; fi
if "$TAM" explain | grep -q "$esc"; then fail "bare explain contains escape sequences"; fi
if "$TAM" explain pipelines --pipe | grep -q "$esc"; then fail "explain contains escape sequences"; fi
if "$TAM" install | grep -q "$esc"; then fail "install contains escape sequences"; fi
if "$TAM" install inspect | grep -q "$esc"; then fail "install inspect contains escape sequences"; fi
pass "piped output has no control sequences"

# Unknown argument is an error that points at --help.
if "$TAM" --nope >/dev/null 2>&1; then fail "unknown argument exited 0"; fi
"$TAM" --nope 2>&1 | grep -q 'help' || fail "error does not point to --help"
if "$TAM" --help extra >/dev/null 2>&1; then fail "extra arguments exited 0"; fi
"$TAM" --help extra 2>&1 | grep -qi 'too many' || fail "no message for extra arguments"
pass "argument errors"

# Welcome command: displays orientation and records welcome_seen on success.
w_state="$tmp/welcome_state"
w_out=$(env HOME="$tmp/w_home" XDG_STATE_HOME="$w_state" "$TAM" welcome --pipe)
printf '%s\n' "$w_out" | grep -q 'Welcome to Tamlinux' || fail "welcome output missing title"
[ -f "$w_state/tamlinux/welcome_seen" ] || fail "welcome did not create welcome_seen"
grep -q '^[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}T' "$w_state/tamlinux/welcome_seen" \
    || fail "welcome_seen missing timestamp"
if env HOME="$tmp/w_home" XDG_STATE_HOME="$w_state" "$TAM" welcome extra >/dev/null 2>&1; then
    fail "welcome with extra arguments exited 0"
fi
pass "welcome displays orientation and records welcome_seen"

# Welcome isolation: bare tam, show, help, version, explain, install NEVER write welcome state.
iso_state="$tmp/iso_state"
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" show --pipe >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" --help >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" help >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" --version >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" explain >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" explain pipelines --pipe >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" explain nonexistent >/dev/null 2>&1 || true
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" search pipelines >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" install >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" install inspect >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" install --help >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" man --help >/dev/null
if [ -e "$iso_state/tamlinux/welcome_seen" ]; then
    fail "isolated commands wrote welcome_seen state"
fi
pass "state isolation: bare, show, help, version, search, explain, install, man do not touch welcome_seen"

# Show command: displays guide index or page; browse subcommand is retired.
sh_out=$("$TAM" show --pipe)
printf '%s\n' "$sh_out" | grep -q 'Welcome to the Tamlinux guide' || fail "tam show --pipe failed"
sh_page=$("$TAM" show welcome --pipe)
printf '%s\n' "$sh_page" | grep -q 'Welcome to Tamlinux' || fail "tam show welcome failed"

# Show with - reads HTML from stdin and renders tables and definition lists.
stdin_html='<h1>Custom HTML</h1><dl><dt>Alpha</dt><dd>First letter</dd></dl><table><tr><th>Name</th><th>Value</th></tr><tr><td>Foo</td><td>Bar</td></tr></table>'
sh_stdin=$(printf '%s\n' "$stdin_html" | "$TAM" show - --pipe)
printf '%s\n' "$sh_stdin" | grep -q 'Custom HTML' || fail "tam show - failed to render heading"
printf '%s\n' "$sh_stdin" | grep -q 'First letter' || fail "tam show - failed to render definition list"
printf '%s\n' "$sh_stdin" | grep -q 'Name  Value' || fail "tam show - failed to render table headers"
printf '%s\n' "$sh_stdin" | grep -q -- '----  -----' || fail "tam show - failed to render table separator"
printf '%s\n' "$sh_stdin" | grep -q 'Foo   Bar' || fail "tam show - failed to render table cells"

if "$TAM" browse >/dev/null 2>&1; then fail "tam browse unexpectedly succeeded"; fi
"$TAM" browse 2>&1 | grep -q "unrecognized argument 'browse'" || fail "tam browse missing unrecognized argument message"
pass "show subcommand and retired browse subcommand"

# Pager link navigation: Tab advances forward, Shift-Tab and N circulate backward
if python3 -c 'import pty' >/dev/null 2>&1; then
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl

master, slave = pty.openpty()
fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 160, 0, 0))

pid = os.fork()
if pid == 0:
    os.close(master)
    os.dup2(slave, 0)
    os.dup2(slave, 1)
    os.dup2(slave, 2)
    os.close(slave)
    os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[2]
    os.execl(sys.argv[1], sys.argv[1], "show")
os.close(slave)

def send_and_get_target(key):
    time.sleep(0.08)
    while select.select([master], [], [], 0.02)[0]:
        os.read(master, 1024)
    os.write(master, key)
    time.sleep(0.08)
    buf = b""
    while select.select([master], [], [], 0.02)[0]:
        buf += os.read(master, 1024)
    lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
    if not lines:
        return ""
    return lines[-1].split(b"->")[-1].strip().decode("utf-8", "replace")

time.sleep(0.15)
# 1. Tab forward selects first link
t1 = send_and_get_target(b"\t")
assert t1 == "welcome.html", f"expected welcome.html, got {t1}"
# 2. Tab forward selects second link
t2 = send_and_get_target(b"\t")
assert t2 == "install/index.html", f"expected install/index.html, got {t2}"
# 3. Shift-Tab (ANSI CSI Z) circulates backward
t3 = send_and_get_target(b"\x1b[Z")
assert t3 == "welcome.html", f"expected welcome.html, got {t3}"
# 4. Shift-Tab from first link wraps backward to last link
t4 = send_and_get_target(b"\x1b[Z")
assert t4 == "ecosystem/patches.html", f"expected ecosystem/patches.html, got {t4}"
# 5. Shift-Tab variants (CSI 1;2Z, Kitty 9;2u, modifyOtherKeys 27;2;9~, N fallback)
t5 = send_and_get_target(b"\x1b[1;2Z")
assert t5 == "ecosystem/index.html", f"expected ecosystem/index.html, got {t5}"
t6 = send_and_get_target(b"\x1b[9;2u")
assert t6 == "man/index.html", f"expected man/index.html, got {t6}"
t7 = send_and_get_target(b"\x1b[27;2;9~")
assert t7 == "hardware/resources.html", f"expected hardware/resources.html, got {t7}"
t8 = send_and_get_target(b"N")
assert t8 == "desktop/workspaces.html", f"expected desktop/workspaces.html, got {t8}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/knowledge" || fail "interactive pager link navigation failed"
    pass "pager link navigation: Tab forward and Shift-Tab backward circulation"

    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl

master, slave = pty.openpty()
fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 160, 0, 0))

pid = os.fork()
if pid == 0:
    os.close(master)
    os.dup2(slave, 0)
    os.dup2(slave, 1)
    os.dup2(slave, 2)
    os.close(slave)
    os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[2]
    os.execl(sys.argv[1], sys.argv[1], "show")
os.close(slave)

time.sleep(0.15)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 1024)

def parse_info(buf):
    lines = [l for l in buf.split(b"\x1b[24;1H") if b"|" in l]
    if not lines:
        return ("", "")
    last = lines[-1].replace(b"\x1b[K", b"").decode("utf-8", "replace")
    title = last.split("|")[0].strip()
    target = ""
    if "->" in last:
        target = last.split("->")[-1].strip()
    return (title, target)

def send_and_get_info(key):
    time.sleep(0.08)
    while select.select([master], [], [], 0.02)[0]:
        os.read(master, 1024)
    os.write(master, key)
    time.sleep(0.08)
    buf = b""
    while select.select([master], [], [], 0.02)[0]:
        buf += os.read(master, 1024)
    return parse_info(buf)

t, tg = parse_info(buf)
assert t == "tam - the Tamlinux guide" and tg == ""

# 1. At tip of exploration and no link selected: right arrow does nothing
t, tg = send_and_get_info(b"\x1b[C")
assert t == "tam - the Tamlinux guide" and tg == ""

# 2. Select first link (welcome.html)
t, tg = send_and_get_info(b"\t")
assert t == "tam - the Tamlinux guide" and tg == "welcome.html"

# 3. At tip of exploration with link selected: right arrow extends exploration
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Welcome" and tg == ""

# 4. At new tip with no link selected: right arrow does nothing
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Welcome" and tg == ""

# 5. Left arrow returns to index.html; no link selected
t, tg = send_and_get_info(b"\x1b[D")
assert t == "tam - the Tamlinux guide" and tg == ""

# 6. Arrived via left arrow and no link selected: right arrow proceeds toward tip (welcome.html)
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Welcome" and tg == ""

# 7. Select a link on welcome.html (desktop/index.html) and follow it
t, tg = send_and_get_info(b"\t")
assert tg == "desktop/index.html"
t, tg = send_and_get_info(b"\x1b[C")
assert t == "The Tamlinux Desktop" and tg == ""

# 8. Multi-step left arrow: back to welcome, then back to index
t, tg = send_and_get_info(b"\x1b[D")
assert t == "Welcome"
t, tg = send_and_get_info(b"\x1b[D")
assert t == "tam - the Tamlinux guide"

# 9. Multi-step right arrow: forward to welcome, then forward to desktop, then tip (does nothing)
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Welcome"
t, tg = send_and_get_info(b"\x1b[C")
assert t == "The Tamlinux Desktop"
t, tg = send_and_get_info(b"\x1b[C")
assert t == "The Tamlinux Desktop"

# 10. Return to index, then select DIFFERENT link (install/index.html), right arrow extends branch
t, tg = send_and_get_info(b"\x1b[D")
assert t == "Welcome"
t, tg = send_and_get_info(b"\x1b[D")
assert t == "tam - the Tamlinux guide"
# Tab twice to select install/index.html
send_and_get_info(b"\t")
t, tg = send_and_get_info(b"\t")
assert tg == "install/index.html"
# Right arrow extends this branch
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Installing and maintaining" and tg == ""
# At tip of new branch: right arrow does nothing
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Installing and maintaining" and tg == ""

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/knowledge" || fail "interactive pager forward navigation failed"
    pass "pager forward navigation: right arrow forward, referring page memory, and branch extension"

    # Hyperlink display: no leading-space underline; multiline wrapped links are single links with continuation highlight
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl

master, slave = pty.openpty()
fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 150, 0, 0))

pid = os.fork()
if pid == 0:
    os.close(master)
    os.dup2(slave, 0)
    os.dup2(slave, 1)
    os.dup2(slave, 2)
    os.close(slave)
    os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[2]
    os.execl(sys.argv[1], sys.argv[1], "show", "welcome")
os.close(slave)

time.sleep(0.15)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

# 1. No unselected hyperlink underlines the leading space; underline starts with first non-space character
assert b"\x1b[4;34m " not in buf, "unselected link underlines leading space"

# 2. Tab selects wrapped link (desktop/index.html) and highlights both line 1 and line 2 continuation
os.write(master, b"\t")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
target = lines[-1].split(b"->")[-1].strip().decode("utf-8", "replace") if lines else ""
assert target == "desktop/index.html", f"expected desktop/index.html, got {target}"
assert b"\x1b[7;1mthe" in buf, "line 1 of wrapped link not highlighted"
assert b"\x1b[7;1mdesktop guide" in buf, "line 2 continuation of wrapped link not highlighted"
assert b"\x1b[7;1m " not in buf, "leading space before wrapped link must not be highlighted"

# 3. Tab advances to next link (shell/index.html) — wrapped link was a single link, not two
os.write(master, b"\t")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
target = lines[-1].split(b"->")[-1].strip().decode("utf-8", "replace") if lines else ""
assert target == "shell/index.html", f"expected shell/index.html, got {target}"

# 4. Shift-Tab wraps/returns back to the wrapped link, highlighting both lines
os.write(master, b"\x1b[Z")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
target = lines[-1].split(b"->")[-1].strip().decode("utf-8", "replace") if lines else ""
assert target == "desktop/index.html", f"expected desktop/index.html, got {target}"
assert b"\x1b[7;1mthe" in buf and b"\x1b[7;1mdesktop guide" in buf

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/knowledge" || fail "hyperlink display test failed"
    pass "pager hyperlink display: leading-space underline prevention and wrapped link continuation highlight"
fi


# Explain command: curated topics resolve, bare explain lists topics, and errors report cleanly.
ex_out=$("$TAM" explain pipelines --pipe)
printf '%s\n' "$ex_out" | grep -q 'Shell pipelines' || fail "explain pipelines failed"
ex_pipe=$("$TAM" explain '>' --pipe)
printf '%s\n' "$ex_pipe" | grep -q 'Redirection' || fail "explain '>' failed"
ex_pipe_sym=$("$TAM" explain '|' --pipe)
printf '%s\n' "$ex_pipe_sym" | grep -q 'Shell pipelines' || fail "explain '|' failed"
ex_pipe_q1=$("$TAM" explain "'|'" --pipe)
printf '%s\n' "$ex_pipe_q1" | grep -q 'Shell pipelines' || fail "explain \"'|'\" failed"
ex_pipe_q2=$("$TAM" explain '"|"' --pipe)
printf '%s\n' "$ex_pipe_q2" | grep -q 'Shell pipelines' || fail "explain '\"|\"' failed"
ex_browse=$("$TAM" explain browse --pipe)
printf '%s\n' "$ex_browse" | grep -q 'Tamlinux guide' || fail "explain browse failed"
ex_show=$("$TAM" explain show --pipe)
printf '%s\n' "$ex_show" | grep -q 'Tamlinux guide' || fail "explain show failed"
ex_man=$("$TAM" explain man --pipe)
printf '%s\n' "$ex_man" | grep -q 'Reading man pages' || fail "explain man failed"

# Use track curated topics.
ex_shell=$("$TAM" explain shell --pipe)
printf '%s\n' "$ex_shell" | grep -q 'The Shell and Streams' || fail "explain shell failed"
ex_jobs=$("$TAM" explain jobs --pipe)
printf '%s\n' "$ex_jobs" | grep -q 'Processes, Jobs, and Signals' || fail "explain jobs failed"
ex_sig=$("$TAM" explain sigkill --pipe)
printf '%s\n' "$ex_sig" | grep -q 'Processes, Jobs, and Signals' || fail "explain sigkill failed"
ex_fs=$("$TAM" explain filesystem --pipe)
printf '%s\n' "$ex_fs" | grep -q 'The Linux Filesystem Hierarchy' || fail "explain filesystem failed"
ex_perm=$("$TAM" explain permissions --pipe)
printf '%s\n' "$ex_perm" | grep -q 'File Permissions and Ownership' || fail "explain permissions failed"
ex_desk=$("$TAM" explain desktop --pipe)
printf '%s\n' "$ex_desk" | grep -q 'The Tamlinux Desktop' || fail "explain desktop failed"
ex_ws=$("$TAM" explain workspaces --pipe)
printf '%s\n' "$ex_ws" | grep -q 'Workspaces and Multi-Display Layout' || fail "explain workspaces failed"
ex_res=$("$TAM" explain resources --pipe)
printf '%s\n' "$ex_res" | grep -q 'System Resources and Performance' || fail "explain resources failed"

# Explain with - reads topic from stdin.
ex_stdin=$(printf 'jobs\n' | "$TAM" explain - --pipe)
printf '%s\n' "$ex_stdin" | grep -q 'Processes, Jobs, and Signals' || fail "explain - failed to read topic from stdin"

# Bare explain lists available topics and exits 0.
ex_list=$("$TAM" explain)
printf '%s\n' "$ex_list" | grep -q 'Available topics:' || fail "explain with no topic missing topic list"
printf '%s\n' "$ex_list" | grep -q 'welcome' || fail "explain list missing welcome"
printf '%s\n' "$ex_list" | grep -q 'install' || fail "explain list missing install"
printf '%s\n' "$ex_list" | grep -q 'shell' || fail "explain list missing shell"
printf '%s\n' "$ex_list" | grep -q 'pipelines' || fail "explain list missing pipelines"
printf '%s\n' "$ex_list" | grep -q 'jobs' || fail "explain list missing jobs"
printf '%s\n' "$ex_list" | grep -q 'filesystem' || fail "explain list missing filesystem"
printf '%s\n' "$ex_list" | grep -q 'permissions' || fail "explain list missing permissions"
printf '%s\n' "$ex_list" | grep -q 'desktop' || fail "explain list missing desktop"
printf '%s\n' "$ex_list" | grep -q 'workspaces' || fail "explain list missing workspaces"
printf '%s\n' "$ex_list" | grep -q 'resources' || fail "explain list missing resources"
printf '%s\n' "$ex_list" | grep -q 'man' || fail "explain list missing man"
printf '%s\n' "$ex_list" | grep -q 'guide' || fail "explain list missing guide"
if printf '%s\n' "$ex_list" | grep -qi 'missing topic'; then
    fail "explain output unexpectedly mentions missing topic"
fi

# Piped bare explain also outputs topic list cleanly.
ex_pipe_list=$("$TAM" explain --pipe)
printf '%s\n' "$ex_pipe_list" | grep -q 'Available topics:' || fail "explain --pipe missing topic list"

# explain --help, -h, and help match bare explain topic listing.
[ "$("$TAM" explain help)" = "$ex_list" ] || fail "explain help differs from bare explain"
[ "$("$TAM" explain --help)" = "$ex_list" ] || fail "explain --help differs from bare explain"
[ "$("$TAM" explain -h)" = "$ex_list" ] || fail "explain -h differs from bare explain"

# Multi-word explain resolves via search fallback path.
ex_multi=$("$TAM" explain cpu load --pipe)
printf '%s\n' "$ex_multi" | grep -q 'System Resources' || fail "explain cpu load failed"

# Unmatched topic: clear diagnostic and reference to show.
if "$TAM" explain no_such_topic >/dev/null 2>&1; then fail "unmatched topic exited 0"; fi
"$TAM" explain no_such_topic 2>&1 | grep -q "no topic found for 'no_such_topic'" \
    || fail "missing clear miss diagnostic"
"$TAM" explain no_such_topic 2>&1 | grep -q "tam show" \
    || fail "miss diagnostic does not point to tam show"

# Safe input handling: arbitrary shell strings are treated strictly as data.
if "$TAM" explain 'cat foo | grep bar; rm -rf /' >/dev/null 2>&1; then
    fail "shell string topic unexpectedly exited 0"
fi
"$TAM" explain 'cat foo | grep bar; rm -rf /' 2>&1 | grep -q "no topic found for 'cat foo | grep bar; rm -rf /'" \
    || fail "shell metacharacters not treated as plain query data"
pass "explain resolves curated topics, handles misses, and treats shell strings as data"

# Search command: ranked search over corpus index, json output, canonical queries, and miss banner.
"$TAM" search >/dev/null || fail "bare search failed"
"$TAM" search --help | grep -q 'Usage:' || fail "search --help missing usage"

# Canonical query: "change file owner" -> permissions page
s_owner=$("$TAM" search "change file owner")
printf '%s\n' "$s_owner" | grep -q 'permissions' || fail "search 'change file owner' missing permissions topic"
printf '%s\n' "$s_owner" | grep -q 'filesystem/permissions.html' || fail "search 'change file owner' missing permissions page"

# JSON output
s_json=$("$TAM" search --json "change file owner")
printf '%s\n' "$s_json" | grep -q '"topic": "permissions"' || fail "search --json missing topic field"
printf '%s\n' "$s_json" | grep -q '"page": "filesystem/permissions.html"' || fail "search --json missing page field"

# Piped plain text has no escapes
if "$TAM" search --pipe "change file owner" | grep -q "$esc"; then
    fail "search piped output contains escape sequences"
fi

# Canonical queries from research/prototype
"$TAM" search "pipe output to grep" | grep -q 'pipelines' || fail "search 'pipe output to grep' failed"
"$TAM" search "redirect stderr" | grep -q 'pipelines' || fail "search 'redirect stderr' failed"
"$TAM" search "background a process" | grep -q 'jobs' || fail "search 'background a process' failed"
"$TAM" search "what is /etc" | grep -q 'filesystem' || fail "search 'what is /etc' failed"
"$TAM" search "check disk space" | grep -q 'resources' || fail "search 'check disk space' failed"
"$TAM" search "multi monitor" | grep -q 'workspaces' || fail "search 'multi monitor' failed"
"$TAM" search "man pages" | grep -q 'man' || fail "search 'man pages' failed"
"$TAM" search sway | grep -q 'ecosystem/compositors.html' || fail "search 'sway' failed"
"$TAM" search runit | grep -q 'ecosystem/init.html' || fail "search 'runit' failed"
"$TAM" search "carried patch" | grep -q 'ecosystem/patches.html' || fail "search 'carried patch' failed"

# Out-of-corpus query produces clear miss banner and exits 1 (no auto-redirect)
if "$TAM" search cron >/dev/null 2>&1; then fail "search cron unexpectedly exited 0"; fi
"$TAM" search cron 2>&1 | grep -q "no topic found for 'cron'" || fail "search cron missing clear miss message"
"$TAM" search cron 2>&1 | grep -q "Top candidates:" || fail "search cron missing candidate banner"
"$TAM" search cron 2>&1 | grep -q "tam show" || fail "search cron missing tam show pointer"

# Bounded did-you-mean for typo
"$TAM" search permisions 2>&1 | grep -qi "Did you mean" || fail "search typo missing Did you mean"

pass "search subcommand, corpus index retrieval, json output, and canonical query set"

# Install command: bare overview, help, inspect, unimplemented verbs, and error handling.
"$TAM" install | grep -q 'Usage:' || fail "bare install missing usage"
"$TAM" install --help | grep -q 'Usage:' || fail "install --help missing usage"
"$TAM" install help | grep -q 'Usage:' || fail "install help missing usage"
[ "$("$TAM" install help)" = "$("$TAM" install --help)" ] || fail "install help differs from install --help"

# Install inspect: reports host facts, capabilities, profiles.
ins_out=$("$TAM" install inspect)
printf '%s\n' "$ins_out" | grep -q 'Host Information:' || fail "inspect missing Host Information"
printf '%s\n' "$ins_out" | grep -q 'Operating system:' || fail "inspect missing Operating system"
printf '%s\n' "$ins_out" | grep -q 'Architecture:' || fail "inspect missing Architecture"
printf '%s\n' "$ins_out" | grep -q 'Kernel release:' || fail "inspect missing Kernel release"
printf '%s\n' "$ins_out" | grep -q 'Installation Capabilities:' || fail "inspect missing Capabilities"
printf '%s\n' "$ins_out" | grep -q 'Init system:' || fail "inspect missing Init system"
printf '%s\n' "$ins_out" | grep -q 'Package tools:' || fail "inspect missing Package tools"
printf '%s\n' "$ins_out" | grep -q 'Privilege tool:' || fail "inspect missing Privilege tool"
printf '%s\n' "$ins_out" | grep -q 'Root filesystem:' || fail "inspect missing Root filesystem"
printf '%s\n' "$ins_out" | grep -q 'Graphics device:' || fail "inspect missing Graphics device"
printf '%s\n' "$ins_out" | grep -q 'Installation Profiles:' || fail "inspect missing Profiles"
printf '%s\n' "$ins_out" | grep -q 'Supported:' || fail "inspect missing Supported"
printf '%s\n' "$ins_out" | grep -q 'Target base:' || fail "inspect missing Target base"

# Cultivar resolution in inspect matches environment / isolated config.
iso "$TAM" install inspect | grep -q 'Cultivar version:  7.7.7' || fail "inspect missing cultivar version from file"
rm -f "$config/tamlinux/version"
iso "$TAM" install inspect | grep -q 'Cultivar version:  unavailable' || fail "isolated inspect missing unavailable cultivar"
env HOME="$home" XDG_CONFIG_HOME="$config" XDG_STATE_HOME="$state" \
    TAMLINUX_VERSION=8.8.8 "$TAM" install inspect | grep -q '8.8.8' || fail "inspect ignored cultivar env"

# Inspect pipe flag.
"$TAM" install inspect --pipe | grep -q 'Host Information:' || fail "inspect --pipe failed"

# Unimplemented operations: plan, apply, verify exit 1 with clear diagnostic.
if "$TAM" install plan >/dev/null 2>&1; then fail "install plan exited 0"; fi
"$TAM" install plan 2>&1 | grep -q "'plan' is not implemented yet" || fail "plan missing unimplemented diagnostic"
if "$TAM" install apply >/dev/null 2>&1; then fail "install apply exited 0"; fi
"$TAM" install apply 2>&1 | grep -q "'apply' is not implemented yet" || fail "apply missing unimplemented diagnostic"
if "$TAM" install verify >/dev/null 2>&1; then fail "install verify exited 0"; fi
"$TAM" install verify 2>&1 | grep -q "'verify' is not implemented yet" || fail "verify missing unimplemented diagnostic"

# Argument errors under install.
if "$TAM" install bogus >/dev/null 2>&1; then fail "install bogus exited 0"; fi
"$TAM" install bogus 2>&1 | grep -q "unrecognized argument" || fail "install bogus missing error message"
if "$TAM" install inspect extra >/dev/null 2>&1; then fail "install inspect extra exited 0"; fi
"$TAM" install inspect extra 2>&1 | grep -q "too many arguments" || fail "inspect extra missing error message"
if "$TAM" install inspect --bogus >/dev/null 2>&1; then fail "install inspect --bogus exited 0"; fi
if "$TAM" install --help extra >/dev/null 2>&1; then fail "install --help extra exited 0"; fi
pass "install overview, inspect probes, honest unimplemented verbs, and argument errors"

# Man command: view man pages, section resolution, cross-reference hyperlinking, and tam-man wrapper.
"$TAM" man | grep -q 'Usage:' || fail "bare man missing usage"
"$TAM" man --help | grep -q 'Usage:' || fail "man --help missing usage"
"$TAM" man help | grep -q 'Usage:' || fail "man help missing usage"
[ "$("$TAM" man help)" = "$("$TAM" man --help)" ] || fail "man help differs from man --help"

# Pages installed on the host: a package build sandbox has no man pages or
# groff, so these checks run only where the pages can be rendered.
if command -v groff >/dev/null 2>&1 && man -w grep >/dev/null 2>&1 \
        && man -w 5 fstab >/dev/null 2>&1; then
    # View grep manual page: check content and hyperlinked cross references in SEE ALSO.
    man_grep=$("$TAM" man grep --pipe)
    printf '%s\n' "$man_grep" | grep -q 'grep - print lines that match patterns' || fail "tam man grep missing description"
    printf '%s\n' "$man_grep" | grep -q 'man:sed(1)' || fail "tam man grep missing hyperlinked sed(1) cross reference"
    printf '%s\n' "$man_grep" | grep -q 'man:awk(1)' || fail "tam man grep missing hyperlinked awk(1) cross reference"

    # View section 5 fstab manual page using both section argument and parenthesized syntax.
    man_fstab=$("$TAM" man 5 fstab --pipe)
    printf '%s\n' "$man_fstab" | grep -q 'static information about the filesystems' || fail "tam man 5 fstab failed"
    man_fstab_paren=$("$TAM" man "fstab(5)" --pipe)
    printf '%s\n' "$man_fstab_paren" | grep -q 'static information about the filesystems' || fail "tam man fstab(5) failed"

    # Wrapper script bin/tam-man delegates to tam man.
    if [ -x "$root/bin/tam-man" ]; then
        man_wrap=$("$root/bin/tam-man" grep --pipe)
        printf '%s\n' "$man_wrap" | grep -q 'man:sed(1)' || fail "tam-man wrapper missing live hyperlinked cross reference"
        pass "tam-man wrapper script executes and hyperlinks manual pages"
    fi
else
    printf 'skip - host manual pages (man, groff or pages unavailable)\n'
fi

# Missing manual page reports clear diagnostic and exit 1.
if "$TAM" man nonexistent_xyz >/dev/null 2>&1; then fail "missing man page exited 0"; fi
"$TAM" man nonexistent_xyz 2>&1 | grep -q "no manual page found for 'nonexistent_xyz'" || fail "missing man page diagnostic unclear"
if "$TAM" man 1 nonexistent_xyz >/dev/null 2>&1; then fail "missing section man page exited 0"; fi
"$TAM" man 1 nonexistent_xyz 2>&1 | grep -q "in section 1" || fail "missing section diagnostic unclear"
pass "man subcommand, cross-reference hyperlinking, and error handling"

printf 'All tam command tests passed.\n'
