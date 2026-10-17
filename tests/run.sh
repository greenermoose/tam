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
# PTY pager fixtures need a real terminfo profile; separate cases exercise dumb.
export TERM=xterm-256color
unset NO_COLOR

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
[ $(printf '%s\n' "$("$TAM" --help)" | wc -l) -le 35 ] || fail "--help exceeds 35 lines"
"$TAM" -h >/dev/null || fail "-h failed"
"$TAM" help | grep -q 'Usage:' || fail "help is missing usage"
[ "$("$TAM" help)" = "$("$TAM" --help)" ] || fail "help differs from --help"
[ "$("$TAM")" = "$("$TAM" --help)" ] || fail "bare tam differs from --help"
if "$TAM" help extra >/dev/null 2>&1; then fail "help with extra args exited 0"; fi

# Terse help.
"$TAM" --terse | grep -q 'Essential Subcommands:' || fail "--terse missing Essential Subcommands"
[ $(printf '%s\n' "$("$TAM" --terse)" | wc -l) -le 15 ] || fail "--terse exceeds 15 lines"
[ "$("$TAM" --terse)" = "$("$TAM" --help --terse)" ] || fail "--terse differs from --help --terse"
[ "$("$TAM" --terse)" = "$("$TAM" help --terse)" ] || fail "--terse differs from help --terse"

# Verbose help.
"$TAM" --verbose | grep -q 'TAM_KNOWLEDGE_DIR' || fail "--verbose missing environment"
[ "$("$TAM" --verbose)" = "$("$TAM" --help --verbose)" ] || fail "--verbose differs from --help --verbose"
[ "$("$TAM" --verbose)" = "$("$TAM" help --verbose)" ] || fail "--verbose differs from help --verbose"
pass "--help, -h, help, --terse, and --verbose"

# Version: command, knowledge corpus, and library versions are reported; metadata is explicit.
# The command version is asserted here; the corpus version comes from the
# corpus itself, so a corpus release needs no change to this file.
corpus_version=$(cat "$knowledge/version")
"$TAM" --version | grep -q '^tam command 0.29.3' || fail "--version missing command version"
"$TAM" --version | grep -q "^knowledge corpus $corpus_version" || fail "--version missing corpus version"
"$TAM" --version | grep -q 'knowledge corpus' || fail "--version missing knowledge corpus"
"$TAM" --version | grep -q 'libtam' || fail "--version missing libtam"
"$TAM" -V >/dev/null || fail "-V failed"
iso "$TAM" --version | grep -q 'unavailable' || fail "missing cultivar metadata note"
pass "--version and -V"

# Decoupled versioning: knowledge corpus version can diverge from command version.
mock_corpus="$tmp/mock_corpus"
mkdir -p "$mock_corpus"
printf '0.99.1\n' > "$mock_corpus/version"
printf 'mock789\n' > "$mock_corpus/.build_id"
touch "$mock_corpus/index.html"
env TAM_KNOWLEDGE_DIR="$mock_corpus" "$TAM" --version | grep -q 'knowledge corpus 0.99.1 (dev mock789)' \
    || fail "decoupled corpus version not reported in --version"
env TAM_KNOWLEDGE_DIR="$mock_corpus" "$TAM" | grep -q 'showing corpus version 0.99.1 (dev mock789)' \
    || fail "decoupled corpus version not reported in bare tam"
rm "$mock_corpus/version"
env TAM_KNOWLEDGE_DIR="$mock_corpus" "$TAM" --version | grep -q 'knowledge corpus unversioned' \
    || fail "corpus without a version file not reported as unversioned"
pass "decoupled corpus version resolution"

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
if "$TAM" inspect | grep -q "$esc"; then fail "inspect contains escape sequences"; fi
if "$TAM" check | grep -q "$esc"; then fail "check contains escape sequences"; fi
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

# hello and hi aliases for welcome
hello_out=$("$TAM" hello --pipe)
printf '%s\n' "$hello_out" | grep -q 'Welcome to Tamlinux' || fail "tam hello did not render welcome"
hi_out=$("$TAM" hi --pipe)
printf '%s\n' "$hi_out" | grep -q 'Welcome to Tamlinux' || fail "tam hi did not render welcome"
pass "hello and hi aliases for welcome"

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
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" inspect >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" check >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" install --help >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" man --help >/dev/null
if [ -e "$iso_state/tamlinux/welcome_seen" ]; then
    fail "isolated commands wrote welcome_seen state"
fi
pass "state isolation: bare, show, help, version, search, explain, inspect, check, install, man do not touch welcome_seen"

# Show command: displays guide index or page; browse subcommand is retired.
sh_out=$("$TAM" show --pipe)
printf '%s\n' "$sh_out" | grep -q 'The Tamlinux User Guide' || fail "tam show --pipe failed"
sh_page=$("$TAM" show welcome --pipe)
printf '%s\n' "$sh_page" | grep -q 'Welcome to Tamlinux' || fail "tam show welcome failed"
sh_topics=$("$TAM" show topics --pipe)
printf '%s\n' "$sh_topics" | grep -q 'Available Explanation Topics' || fail "tam show topics failed"
sh_topics_pseudo=$("$TAM" show tam:topics --pipe)
printf '%s\n' "$sh_topics_pseudo" | grep -q 'Available Explanation Topics' || fail "tam show tam:topics failed"

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
    return lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].strip().decode("utf-8", "replace")

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
assert t4 == "info:dir", f"expected info:dir, got {t4}"
# 5. Shift-Tab variants (CSI 1;2Z, Kitty 9;2u, modifyOtherKeys 27;2;9~, N fallback)
t5 = send_and_get_target(b"\x1b[1;2Z")
assert t5 == "man/index.html", f"expected man/index.html, got {t5}"
t6 = send_and_get_target(b"\x1b[9;2u")
assert t6 == "man:tam(1)", f"expected man:tam(1), got {t6}"
t7 = send_and_get_target(b"\x1b[27;2;9~")
assert t7 == "tam:topics", f"expected tam:topics, got {t7}"
t8 = send_and_get_target(b"N")
assert t8 == "ecosystem/sustainability.html", f"expected ecosystem/sustainability.html, got {t8}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/knowledge" || fail "interactive pager link navigation failed"
    pass "pager link navigation: Tab forward and Shift-Tab backward circulation"

    # Pager hyperlink navigation with a, s, d, f and Info dir rebind to i
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl

# 1. Test 2D spatial link navigation on nav-grid fixture (a left, s up, d down, f right)
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
    os.execl(sys.argv[1], sys.argv[1], "show", "nav-grid")
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
    return lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].strip().decode("utf-8", "replace")

time.sleep(0.15)
# Initial: nothing selected. Press "f" -> selects first visible link: tl.html
t0 = send_and_get_target(b"f")
assert t0 == "tl.html", f"expected tl.html, got {t0}"

# Press "f" -> moves right on same line: tr.html
t1 = send_and_get_target(b"f")
assert t1 == "tr.html", f"expected tr.html, got {t1}"

# Press "d" -> moves down: br.html
t2 = send_and_get_target(b"d")
assert t2 == "br.html", f"expected br.html, got {t2}"

# Press "a" -> moves left on same line: bl.html
t3 = send_and_get_target(b"a")
assert t3 == "bl.html", f"expected bl.html, got {t3}"

# Press "s" -> moves up: tl.html
t4 = send_and_get_target(b"s")
assert t4 == "tl.html", f"expected tl.html, got {t4}"

# Press "s" again from top -> wraps around to bottom: bl.html
t5 = send_and_get_target(b"s")
assert t5 == "bl.html", f"expected bl.html, got {t5}"

# Press "d" again from bottom -> wraps around to top: tl.html
t6 = send_and_get_target(b"d")
assert t6 == "tl.html", f"expected tl.html, got {t6}"

# Move to tr.html
t7 = send_and_get_target(b"f")
assert t7 == "tr.html", f"expected tr.html, got {t7}"

# Press "s" from top right -> wraps around to bottom right: br.html
t8 = send_and_get_target(b"s")
assert t8 == "br.html", f"expected br.html, got {t8}"

# Press "d" from bottom right -> wraps around to top right: tr.html
t9 = send_and_get_target(b"d")
assert t9 == "tr.html", f"expected tr.html, got {t9}"

# a moves left to tl.html
t10 = send_and_get_target(b"a")
assert t10 == "tl.html", f"expected tl.html, got {t10}"

# a from tl.html wraps to last link: br.html
t11 = send_and_get_target(b"a")
assert t11 == "br.html", f"expected br.html, got {t11}"

# f from br.html wraps to first link: tl.html
t12 = send_and_get_target(b"f")
assert t12 == "tl.html", f"expected tl.html, got {t12}"

os.write(master, b"q")
os.waitpid(pid, 0)

# 2. Test initial selection of first visible link for keys a, s, d
for key in [b"a", b"s", b"d"]:
    m, s = pty.openpty()
    fcntl.ioctl(s, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 160, 0, 0))
    p = os.fork()
    if p == 0:
        os.close(m)
        os.dup2(s, 0)
        os.dup2(s, 1)
        os.dup2(s, 2)
        os.close(s)
        os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[2]
        os.execl(sys.argv[1], sys.argv[1], "show", "nav-grid")
    os.close(s)
    time.sleep(0.12)
    while select.select([m], [], [], 0.02)[0]:
        os.read(m, 1024)
    os.write(m, key)
    time.sleep(0.08)
    buf = b""
    while select.select([m], [], [], 0.02)[0]:
        buf += os.read(m, 1024)
    lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
    target = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].strip().decode("utf-8", "replace") if lines else ""
    assert target == "tl.html", f"expected tl.html for key {key}, got {target}"
    os.write(m, b"q")
    os.waitpid(p, 0)

# 3. Test key i jumps to info:dir
m, s = pty.openpty()
fcntl.ioctl(s, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 160, 0, 0))
p = os.fork()
if p == 0:
    os.close(m)
    os.dup2(s, 0)
    os.dup2(s, 1)
    os.dup2(s, 2)
    os.close(s)
    os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[3]
    os.execl(sys.argv[1], sys.argv[1], "show")
os.close(s)
time.sleep(0.15)
while select.select([m], [], [], 0.02)[0]:
    os.read(m, 1024)
os.write(m, b"i")
time.sleep(0.12)
buf = b""
while select.select([m], [], [], 0.02)[0]:
    buf += os.read(m, 4096)
lines = [l for l in buf.split(b"\x1b[24;1H") if b"|" in l]
title = lines[-1].split(b"|")[0].split(b"\x1b[K", 1)[0].strip().decode("utf-8", "replace") if lines else ""
assert "GNU Info Directory" in title or "dir" in title, f"expected Info Directory title, got {title}"
os.write(m, b"q")
os.waitpid(p, 0)

# 4. Test selecting first visible link when scrolled down in viewport
m, s = pty.openpty()
fcntl.ioctl(s, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 160, 0, 0))
p = os.fork()
if p == 0:
    os.close(m)
    os.dup2(s, 0)
    os.dup2(s, 1)
    os.dup2(s, 2)
    os.close(s)
    os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[3]
    os.execl(sys.argv[1], sys.argv[1], "show")
os.close(s)
time.sleep(0.15)
while select.select([m], [], [], 0.02)[0]:
    os.read(m, 1024)
for _ in range(10):
    os.write(m, b"j")
    time.sleep(0.01)
time.sleep(0.08)
while select.select([m], [], [], 0.02)[0]:
    os.read(m, 1024)
os.write(m, b"d")
time.sleep(0.08)
buf = b""
while select.select([m], [], [], 0.02)[0]:
    buf += os.read(m, 4096)
lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
target = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].strip().decode("utf-8", "replace") if lines else ""
assert target != "welcome.html" and target != "", f"unexpected target {target}"
os.write(m, b"q")
os.waitpid(p, 0)
' "$TAM" "$root/tests/fixtures" "$root/knowledge" || fail "interactive pager a/s/d/f navigation or i dir key failed"
    pass "pager hyperlink navigation: a/s/d/f 2D navigation, first visible link selection, and i dir rebind"

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

last_info = ("", "")
def parse_info(buf):
    global last_info
    lines = [l for l in buf.split(b"\x1b[24;1H") if b"|" in l]
    if not lines:
        return last_info
    last = lines[-1].split(b"\x1b[K", 1)[0].decode("utf-8", "replace")
    title = last.split("|")[0].strip()
    target = ""
    if "->" in last:
        target = last.split("->")[-1].split("\x1b", 1)[0].strip()
    last_info = (title, target)
    return last_info

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
assert t == "The Tamlinux User Guide" and tg == "", f"got {t!r} {tg!r} buf={buf!r}"

# 1. At tip of exploration and no link selected: right arrow does nothing
t, tg = send_and_get_info(b"\x1b[C")
assert t == "The Tamlinux User Guide" and tg == "", f"got {t!r} {tg!r}"

# 2. Select first link (welcome.html)
t, tg = send_and_get_info(b"\t")
assert t == "The Tamlinux User Guide" and tg == "welcome.html"

# 3. At tip of exploration with link selected: right arrow extends exploration
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Welcome" and tg == ""

# 4. At new tip with no link selected: right arrow does nothing
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Welcome" and tg == ""

# 5. Left arrow returns to index.html; no link selected
t, tg = send_and_get_info(b"\x1b[D")
assert t == "The Tamlinux User Guide" and tg == "", f"got {t!r} {tg!r}"

# 6. Arrived via left arrow and no link selected: right arrow proceeds toward tip (welcome.html)
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Welcome" and tg == ""

# 7. On welcome.html, skip the two guide-index links (the breadcrumb crumb
# and the inline link) and follow the desktop guide link
send_and_get_info(b"\t")
send_and_get_info(b"\t")
t, tg = send_and_get_info(b"\t")
assert tg == "desktop/index.html"
t, tg = send_and_get_info(b"\x1b[C")
assert t == "The Tamlinux Desktop" and tg == ""

# 8. Multi-step left arrow: back to welcome, then back to index
t, tg = send_and_get_info(b"\x1b[D")
assert t == "Welcome"
t, tg = send_and_get_info(b"\x1b[D")
assert t == "The Tamlinux User Guide"

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
assert t == "The Tamlinux User Guide"
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
    os.execl(sys.argv[1], sys.argv[1], "show", "link-wrap")
os.close(slave)

time.sleep(0.15)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

# 1. No unselected hyperlink underlines the leading space; underline starts with first non-space character
assert b"\x1b[4m " not in buf, "unselected link underlines leading space"

# 2. Tab selects the first link (one.html); its label wraps two lines and both are highlighted
os.write(master, b"\t")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
target = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].strip().decode("utf-8", "replace") if lines else ""
assert target == "one.html", f"expected one.html, got {target}"
assert b"\x1b[1m" in buf, "line 1 of wrapped link not bold"
assert b"\x1b[7m" in buf, "selected link lacks reverse-video emphasis"
assert b"\x1b[7mALPHA this anchor" in buf, "spaces inside selected link lost reverse-video emphasis"
assert buf.count(b"\x1b[1m") >= 2, "wrapped link continuation not highlighted"
assert b"\x1b[1m " not in buf, "leading space before wrapped link must not be highlighted"

# 3. Tab advances to the next link (two.html) — the wrapped link was one link, not two
os.write(master, b"\t")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
target = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].strip().decode("utf-8", "replace") if lines else ""
assert target == "two.html", f"expected two.html, got {target}"

# 4. Shift-Tab returns to the wrapped link, highlighting its lines again
os.write(master, b"\x1b[Z")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
target = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].strip().decode("utf-8", "replace") if lines else ""
assert target == "one.html", f"expected one.html, got {target}"
assert b"\x1b[7m" in buf, "wrapped link not reverse-highlighted after Shift-Tab"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures" || fail "hyperlink display test failed"
    pass "pager hyperlink display: selected-link reverse emphasis including spaces, leading-space protection, and wrapped continuation"

    # A layout separator before a hyperlink is attached to its span. It must
    # remain outside the selected highlight, while interior link spaces stay highlighted.
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl

master, slave = pty.openpty()
fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 120, 0, 0))

pid = os.fork()
if pid == 0:
    os.close(master)
    os.dup2(slave, 0)
    os.dup2(slave, 1)
    os.dup2(slave, 2)
    os.close(slave)
    os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[2]
    os.execl(sys.argv[1], sys.argv[1], "show", "link-leading-space")
os.close(slave)

time.sleep(0.15)
while select.select([master], [], [], 0.05)[0]:
    os.read(master, 4096)

os.write(master, b"\t")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

assert b"\x1b[7m the" not in buf, "space before hyperlink received selected reverse emphasis"
assert b"\x1b[7mthe desktop guide" in buf, "interior hyperlink spaces lost selected reverse emphasis"
os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures" || fail "selected hyperlink boundary spacing failed"
    pass "pager hyperlink boundary spacing: leading separator unhighlighted and interior space highlighted"

    # In-pager text search (/), hit jumping (n / N), Esc clear, and reading telemetry
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
    os.execl(sys.argv[1], sys.argv[1], "show", "shell/pipelines")
os.close(slave)

time.sleep(0.15)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

# 1. Reading telemetry: verify status line contains "L1/" and position metric
assert b"L1/" in buf and (b"(Top)" in buf or b"(All)" in buf), f"status line missing position metric: {buf}"
assert b"/ n/N" in buf, f"status line missing search hint: {buf}"

# 2. In-pager text search: search for "redirection"
os.write(master, b"/redirection\r")
time.sleep(0.10)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

assert b"redirection" in buf, "search term missing from status bar message"
assert b"[" in buf and b"]" in buf, "match indicator missing from status message"
assert b"\x1b[4mRedirection" in buf or b"\x1b[4mredirection" in buf, "search hit not highlighted"

# 3. Next match (n)
os.write(master, b"n")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)
assert b"redirection" in buf

# 4. Previous match (N)
os.write(master, b"N")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)
assert b"redirection" in buf

# 5. Esc clears search query and highlights
os.write(master, b"\x1b")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)
assert b"\x1b[4mRedirection" not in buf and b"\x1b[4mredirection" not in buf, "search highlights not cleared by Esc"

# 6. Miss search pattern
os.write(master, b"/nonexistent_pattern_12345\r")
time.sleep(0.10)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)
assert b"Pattern not found" in buf, f"expected Pattern not found message, got: {buf}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/knowledge" || fail "interactive pager search and telemetry test failed"
    pass "in-pager text search (/), match navigation (n/N), Esc clear, and reading telemetry"

    # Quick Guide index return (`): jump immediately to index.html from any depth, push history
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
    os.execl(sys.argv[1], sys.argv[1], "show", "shell/pipelines")
os.close(slave)

def get_title():
    time.sleep(0.08)
    buf = b""
    while select.select([master], [], [], 0.05)[0]:
        buf += os.read(master, 4096)
    lines = [l for l in buf.split(b"\x1b[24;1H") if b"|" in l]
    if not lines:
        return ""
    last = lines[-1].split(b"\x1b[K", 1)[0].decode("utf-8", "replace")
    return last.split("|")[0].strip()

# 1. Initial page is pipelines
t = get_title()
assert t == "Shell pipelines", f"unexpected initial title: {t}"

# 2. Press backtick (`) to jump to index.html
os.write(master, b"`")
t = get_title()
assert t == "The Tamlinux User Guide", f"expected index title after backtick, got: {t}"

# 3. Press u to return to shell/pipelines
os.write(master, b"u")
t = get_title()
assert t == "Shell pipelines", f"expected pipelines title after u, got: {t}"

# 4. Press backtick again, then quit
os.write(master, b"`")
t = get_title()
assert t == "The Tamlinux User Guide", f"expected index title after second backtick, got: {t}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/knowledge" || fail "interactive pager backtick return failed"
    pass "pager quick Guide index return (\`): immediate index jump and history push"

    # Interactive hyperlinked topics pager (tam explain topics)
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
    os.execl(sys.argv[1], sys.argv[1], "explain", "topics")
os.close(slave)

def get_status_info():
    time.sleep(0.08)
    buf = b""
    while select.select([master], [], [], 0.05)[0]:
        buf += os.read(master, 4096)
    lines = [l for l in buf.split(b"\x1b[24;1H") if b"|" in l]
    if not lines:
        return ("", "")
    last = lines[-1].split(b"\x1b[K", 1)[0].decode("utf-8", "replace")
    title = last.split("|")[0].strip()
    target = ""
    if "->" in last:
        target = last.split("->")[-1].split("\x1b", 1)[0].strip()
    return (title, target)

# 1. Initial page is Available Explanation Topics
title, _ = get_status_info()
assert title == "Available Explanation Topics", f"unexpected topics title: {title}"

# 2. Tab selects first topic link
os.write(master, b"\t")
title, target = get_status_info()
assert target != "", "expected link target to be selected by Tab"

# 3. Enter follows selected topic link into explanation
os.write(master, b"\r")
title_topic, _ = get_status_info()
assert title_topic != "Available Explanation Topics", f"did not follow link into topic explanation: {title_topic}"

# 4. u returns to topics page
os.write(master, b"u")
title_back, _ = get_status_info()
assert title_back == "Available Explanation Topics", f"did not return to topics page: {title_back}"

# 5. Backtick (`) jumps to root guide index.html
os.write(master, b"`")
title_home, _ = get_status_info()
assert title_home == "The Tamlinux User Guide", f"did not jump to guide index: {title_home}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/knowledge" || fail "interactive explain topics pager failed"
    pass "tam explain topics: interactive pager with live hyperlinks, back, and home shortcut"

    # Pager attention line, directional link catching, anchored Tab, and safe Enter activation
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl

# Part 1: Directional catching and visible retention on nav-grid
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
    os.execl(sys.argv[1], sys.argv[1], "show", "nav-grid")
os.close(slave)

def get_state():
    time.sleep(0.08)
    buf = b""
    while select.select([master], [], [], 0.02)[0]:
        buf += os.read(master, 4096)
    lines = [l for l in buf.split(b"\x1b[24;1H") if b"|" in l]
    last = lines[-1].split(b"\x1b[K", 1)[0].strip() if lines else b""
    parts = last.split(b"|")
    title = parts[0].strip().decode("utf-8", "replace") if len(parts) > 0 else ""
    pos = parts[1].strip().decode("utf-8", "replace") if len(parts) > 1 else ""
    target = ""
    if b"->" in last:
        target = last.split(b"->")[-1].split(b"\x1b", 1)[0].strip().decode("utf-8", "replace")
    msg = ""
    if b"[" in last and b"]" in last:
        msg = last.split(b"[")[-1].split(b"]")[0].strip().decode("utf-8", "replace")
    has_tint = b"\x1b[48;5;236m" in buf
    return title, pos, target, msg, has_tint

def send_key(key):
    while select.select([master], [], [], 0.02)[0]:
        os.read(master, 1024)
    os.write(master, key)
    return get_state()

time.sleep(0.15)
title, pos, target, msg, has_tint = get_state()
assert has_tint, "expected attention line background tint in output buffer"
assert "L1/4" in pos and "(All)" in pos, f"expected L1/4 in position metric, got: {pos}"
assert target == "", f"expected no initial selection, got: {target}"

# 1. First j moves to blank separator line: no link
title, pos, target, msg, has_tint = send_key(b"j")
assert "L2/4" in pos and target == "", f"expected blank line, got: {pos} {target}"

# 2. Second j moves onto links line 1: downward sweep catches FIRST link (tl.html)
title, pos, target, msg, has_tint = send_key(b"j")
assert "L3/4" in pos and target == "tl.html", f"expected downward sweep to catch first link tl.html, got: {target}"

# 3. Third j moves onto links line 2: downward sweep catches FIRST link (bl.html)
title, pos, target, msg, has_tint = send_key(b"j")
assert "L4/4" in pos and target == "bl.html", f"expected downward sweep to catch first link bl.html, got: {target}"

# 4. Tab advances on line 2 to br.html
title, pos, target, msg, has_tint = send_key(b"\t")
assert target == "br.html", f"expected Tab to advance to br.html, got: {target}"

# 5. Upward sweep (k) back to line 1: catches LAST link (tr.html)
title, pos, target, msg, has_tint = send_key(b"k")
assert "L3/4" in pos and target == "tr.html", f"expected upward sweep to catch last link tr.html, got: {target}"

# 6. Upward sweep (k) to blank line: tr.html stays selected because it is still visible!
title, pos, target, msg, has_tint = send_key(b"k")
assert "L2/4" in pos and target == "tr.html", f"expected tr.html to stay selected while visible, got: {target}"

# 7. Upward sweep (k) to heading line: tr.html stays selected while visible!
title, pos, target, msg, has_tint = send_key(b"k")
assert "L1/4" in pos and target == "tr.html", f"expected tr.html to stay selected while visible, got: {target}"

os.write(master, b"q")
os.waitpid(pid, 0)

# Part 2: Off-screen link de-selection and safe Enter
master, slave = pty.openpty()
fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 160, 0, 0))

pid = os.fork()
if pid == 0:
    os.close(master)
    os.dup2(slave, 0)
    os.dup2(slave, 1)
    os.dup2(slave, 2)
    os.close(slave)
    os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[3]
    os.execl(sys.argv[1], sys.argv[1], "show", "welcome")
os.close(slave)

time.sleep(0.15)
# Select first link (index.html on line 1)
title, pos, target, msg, has_tint = send_key(b"\t")
assert target == "index.html", f"expected index.html, got: {target}"
# Page down (Space): line 1 scrolls off screen
title, pos, target, msg, has_tint = send_key(b" ")
assert target == "", f"expected link to be cleared when scrolled off-screen, got: {target}"
# Enter when no link selected reports message and does not open off-screen link
title, pos, target, msg, has_tint = send_key(b"\r")
assert title == "Welcome", f"expected to remain on Welcome, got: {title}"
assert "no link selected" in msg, f"expected no link selected message, got: {msg}"

os.write(master, b"q")
os.waitpid(pid, 0)

# Part 3: Navigation history and forward stack restores cursor_line, top, and selected link
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
    os.execl(sys.argv[1], sys.argv[1], "show", "nav-grid")
os.close(slave)

time.sleep(0.15)
get_state()
# Move down to line 4 (L4/4) and select second link on line 4 (br.html)
send_key(b"j")
send_key(b"j")
send_key(b"j")
title, pos, target, msg, _ = send_key(b"\t")
assert "L4/4" in pos and target == "br.html", f"expected L4/4 br.html before jump, got: {pos} {target}"
# Jump to index.html via backtick (`), then scroll down 1 line on index.html
title, pos, target, msg, _ = send_key(b"`")
assert "L1/" in pos, f"expected L1 on index.html after backtick jump, got: {pos}"
title, pos_idx, _, _, _ = send_key(b"j")
assert "L2/" in pos_idx, f"expected L2 on index.html after j, got: {pos_idx}"
# Go back (Left Arrow \x1b[D): should restore L4/4 on nav-grid with no link selected so Right Arrow can advance forward
title, pos, target, msg, _ = send_key(b"\x1b[D")
assert "L4/4" in pos and target == "", f"expected back to restore L4/4 with no link selected, got: {pos} {target}"
# Right Arrow (\x1b[C) advances forward to index.html and restores L2/
title, pos, target, msg, _ = send_key(b"\x1b[C")
assert pos == pos_idx and target == "", f"expected forward to restore {pos_idx} on index.html, got: {pos} {target}"
os.write(master, b"q")
os.waitpid(pid, 0)

# Also verify forward navigation (\x1b[C when no link selected) restores saved line position
master, slave = pty.openpty()
fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 160, 0, 0))
pid = os.fork()
if pid == 0:
    os.close(master)
    os.dup2(slave, 0)
    os.dup2(slave, 1)
    os.dup2(slave, 2)
    os.close(slave)
    os.environ["TAM_KNOWLEDGE_DIR"] = sys.argv[3]
    os.execl(sys.argv[1], sys.argv[1], "show", "welcome")
os.close(slave)

time.sleep(0.15)
get_state()
# Page down on welcome so no link is selected (L24/...)
title, pos_w, target_w, _, _ = send_key(b" ")
assert target_w == "" and not pos_w.startswith("L1/"), f"expected scrolled welcome with no link, got: {pos_w} {target_w}"
# Jump to index.html with backtick (`), then page down on index.html
send_key(b"`")
title, pos_i, _, _, _ = send_key(b" ")
assert not pos_i.startswith("L1/"), f"expected scrolled index.html, got: {pos_i}"
# Left Arrow (\x1b[D) returns to welcome at saved line pos_w (with no link selected)
title, pos_back, target_back, _, _ = send_key(b"\x1b[D")
assert title == "Welcome" and pos_back == pos_w and target_back == "", f"expected Welcome at {pos_w}, got: {title} {pos_back} {target_back}"
# Right Arrow (\x1b[C) with no link selected goes forward to index.html at saved line pos_i
title, pos_fwd, _, _, _ = send_key(b"\x1b[C")
assert pos_fwd.split(" ->", 1)[0] == pos_i.split(" ->", 1)[0], f"expected forward to restore the index position, got: {pos_fwd}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures" "$root/knowledge" || fail "interactive pager attention line test failed"
    pass "attention line: charcoal tint, directional link catching, anchored Enter, and history position stack"

    python3 "$here/pager_screen.py" "$TAM" "$root/tests/fixtures" || fail "pager PTY screen-state checks failed"

    # Pager depth switching (0/1/2/3) and retired 1-9 link jumping
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl, re

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
    os.execl(sys.argv[1], sys.argv[1], "show", "shell/pipelines")
os.close(slave)

def strip_ansi(s):
    return re.sub(r"\x1b\[[0-9;?]*[A-Za-z]", "", s)

def send_and_read(key):
    while select.select([master], [], [], 0.02)[0]:
        os.read(master, 4096)
    os.write(master, key)
    time.sleep(0.08)
    buf = b""
    while select.select([master], [], [], 0.02)[0]:
        buf += os.read(master, 4096)
    return strip_ansi(buf.decode("utf-8", "replace"))

time.sleep(0.15)
init_buf = b""
while select.select([master], [], [], 0.02)[0]:
    init_buf += os.read(master, 4096)
assert b"Every level is shown below" in init_buf, f"expected initial all levels on pipelines, got: {init_buf}"

# 0 when already at all levels reports already at all levels
b0 = send_and_read(b"0")
assert "already at all levels" in b0, f"expected already at all levels message, got: {b0}"

# 1 switches to beginner depth (does NOT follow 1st hyperlink)
b1 = send_and_read(b"1")
assert "Depth: beginner." in b1, f"expected Depth: beginner. after pressing 1, got: {b1}"

# 2 switches to intermediate depth
b2 = send_and_read(b"2")
assert "Depth: intermediate." in b2, f"expected Depth: intermediate. after pressing 2, got: {b2}"

# 3 switches to expert depth
b3 = send_and_read(b"3")
assert "Depth: expert." in b3, f"expected Depth: expert. after pressing 3, got: {b3}"

# 0 switches back to all levels
b_all = send_and_read(b"0")
assert "Every level is shown below" in b_all, f"expected all levels banner after pressing 0, got: {b_all}"

# u goes back in history to expert depth
b_back = send_and_read(b"u")
assert "Depth: expert." in b_back, f"expected history back to restore expert depth, got: {b_back}"

# 4..9 no longer jump to links
b4 = send_and_read(b"4")
assert b4 == "", f"expected ignored key 4 to leave the display untouched, got: {b4}"

# Jump to index.html (`) where only beginner is offered; pressing 3 reports no expert level
send_and_read(b"`")
b_no_exp = send_and_read(b"3")
assert "no expert level on this page" in b_no_exp, f"expected no expert level message on index.html, got: {b_no_exp}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/knowledge" || fail "interactive pager depth switching test failed"
    pass "pager depth switching: 0/1/2/3 switch reading depth and 1-9 link jumping retired"
fi


# Explain command: curated topics resolve, bare explain lists topics, and errors report cleanly.
ex_out=$("$TAM" explain pipelines --pipe)
printf '%s\n' "$ex_out" | grep -q 'Shell pipelines' || fail "explain pipelines failed"
ex_pipe=$("$TAM" explain '>' --pipe)
printf '%s\n' "$ex_pipe" | grep -qi 'Redirection' || fail "explain '>' failed"
ex_pipe_sym=$("$TAM" explain '|' --pipe)
printf '%s\n' "$ex_pipe_sym" | grep -q 'Shell pipelines' || fail "explain '|' failed"
ex_pipe_q1=$("$TAM" explain "'|'" --pipe)
printf '%s\n' "$ex_pipe_q1" | grep -q 'Shell pipelines' || fail "explain \"'|'\" failed"
ex_pipe_q2=$("$TAM" explain '"|"' --pipe)
printf '%s\n' "$ex_pipe_q2" | grep -q 'Shell pipelines' || fail "explain '\"|\"' failed"
ex_browse=$("$TAM" explain browse --pipe)
printf '%s\n' "$ex_browse" | grep -q 'Tamlinux User Guide' || fail "explain browse failed"
ex_show=$("$TAM" explain show --pipe)
printf '%s\n' "$ex_show" | grep -q 'Tamlinux User Guide' || fail "explain show failed"
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
ex_lua=$("$TAM" explain lua --pipe)
printf '%s\n' "$ex_lua" | grep -q 'Lua: The Embedded Scripting Language' || fail "explain lua failed"
printf '%s\n' "$ex_lua" | grep -q 'Depth: beginner' || fail "explain lua missing depth line"

# Maintain track curated topics.
ex_maint=$("$TAM" explain maintain --pipe)
printf '%s\n' "$ex_maint" | grep -q 'System Maintenance and Reliability' || fail "explain maintain failed"
ex_upd=$("$TAM" explain updates --pipe)
printf '%s\n' "$ex_upd" | grep -q 'Non-Disruptive Updates and Package Lifecycle' || fail "explain updates failed"
ex_rb=$("$TAM" explain rollback --pipe)
printf '%s\n' "$ex_rb" | grep -q 'Snapshots and Generation Rollbacks' || fail "explain rollback failed"
ex_log=$("$TAM" explain logs --pipe)
printf '%s\n' "$ex_log" | grep -q 'Truthful System Diagnostics and Logs' || fail "explain logs failed"
ex_pwr=$("$TAM" explain power --pipe)
printf '%s\n' "$ex_pwr" | grep -q 'The Progressive Power Ladder' || fail "explain power failed"
ex_rec=$("$TAM" explain recovery --pipe)
printf '%s\n' "$ex_rec" | grep -q 'Emergency Disaster Recovery' || fail "explain recovery failed"

# Improve track curated topics.
ex_imp=$("$TAM" explain improve --pipe)
printf '%s\n' "$ex_imp" | grep -q 'Extending and Improving Your Workstation' || fail "explain improve failed"
ex_eff=$("$TAM" explain efficiency --pipe)
printf '%s\n' "$ex_eff" | grep -q 'Zero-Polling and Event-Driven Architecture' || fail "explain efficiency failed"
ex_zram=$("$TAM" explain zram --pipe)
printf '%s\n' "$ex_zram" | grep -q 'Memory Compression with ZRAM' || fail "explain zram failed"
ex_term=$("$TAM" explain terminal --pipe)
printf '%s\n' "$ex_term" | grep -q 'Terminal and Editor Ergonomics' || fail "explain terminal failed"
ex_scr=$("$TAM" explain scripting --pipe)
printf '%s\n' "$ex_scr" | grep -q 'Shell Scripting and Tool Conventions' || fail "explain scripting failed"
ex_tool=$("$TAM" explain toolchains --pipe)
printf '%s\n' "$ex_tool" | grep -q 'Native Toolchains and Multi-Agent Workflows' || fail "explain toolchains failed"
ex_aip=$("$TAM" explain ai-performance --pipe)
printf '%s\n' "$ex_aip" | grep -q 'Physics of Local AI Generation' || fail "explain ai-performance failed"

# Explain with - reads topic from stdin.
ex_stdin=$(printf 'jobs\n' | "$TAM" explain - --pipe)
printf '%s\n' "$ex_stdin" | grep -q 'Processes, Jobs, and Signals' || fail "explain - failed to read topic from stdin"

# Explain usage synopsis: explain --help, -h, and help display strictly concise usage synopsis (<=15 lines screen budget) and exit 0.
ex_help=$("$TAM" explain --help)
[ $(printf '%s\n' "$ex_help" | wc -l) -le 15 ] || fail "explain --help exceeds 15 lines"
printf '%s\n' "$ex_help" | grep -q 'tam explain topics' || fail "explain --help missing topics pointer"
printf '%s\n' "$ex_help" | grep -q 'tam explain TOPIC' || fail "explain --help missing usage"
printf '%s\n' "$ex_help" | grep -q 'tam show' || fail "explain --help missing show pointer"
[ "$("$TAM" explain help)" = "$ex_help" ] || fail "explain help differs from explain --help"
[ "$("$TAM" explain -h)" = "$ex_help" ] || fail "explain -h differs from explain --help"

# Bare explain displays dynamic topic keywords catalog.
ex_kw=$("$TAM" explain --pipe)
printf '%s\n' "$ex_kw" | grep -q 'Topic Keywords' || fail "bare explain missing Topic Keywords"
printf '%s\n' "$ex_kw" | grep -q 'Topic Keyword' || fail "bare explain missing table heading"
printf '%s\n' "$ex_kw" | grep -q 'tam' || fail "bare explain missing tam topic keyword"
printf '%s\n' "$ex_kw" | grep -q 'pipelines' || fail "bare explain missing pipelines topic keyword"

# tam explain TOPIC displays a Topic Guide (never takes user directly to a page)
ex_tg_tam=$("$TAM" explain tam --pipe)
printf '%s\n' "$ex_tg_tam" | grep -q 'Topic Guide: tam' || fail "explain tam missing Topic Guide header"
printf '%s\n' "$ex_tg_tam" | grep -q 'The tam Command' || fail "explain tam missing The tam Command page"
printf '%s\n' "$ex_tg_tam" | grep -q 'tam.html' || fail "explain tam missing tam.html link"

# tam show tam directly displays the dedicated tam page
sh_tam=$("$TAM" show tam --pipe)
printf '%s\n' "$sh_tam" | grep -q 'The tam Command' || fail "tam show tam missing heading"
printf '%s\n' "$sh_tam" | grep -qi 'C11' || fail "tam show tam missing C11 language"
printf '%s\n' "$sh_tam" | grep -q 'Makefile targets' || fail "tam show tam missing Makefile targets"
printf '%s\n' "$sh_tam" | grep -q 'libtam' || fail "tam show tam missing libtam"
printf '%s\n' "$sh_tam" | grep -qi 'develop -> test -> run -> release' || fail "tam show tam missing release cycle"

# tam explain topics lists available topics and exits 0.
ex_topics=$("$TAM" explain topics --pipe)
printf '%s\n' "$ex_topics" | grep -q 'Available topics:' || fail "explain topics missing heading"
printf '%s\n' "$ex_topics" | grep -q 'welcome' || fail "explain topics missing welcome"
printf '%s\n' "$ex_topics" | grep -q 'install' || fail "explain topics missing install"
printf '%s\n' "$ex_topics" | grep -q 'shell' || fail "explain topics missing shell"
printf '%s\n' "$ex_topics" | grep -q 'pipelines' || fail "explain topics missing pipelines"
printf '%s\n' "$ex_topics" | grep -q 'jobs' || fail "explain topics missing jobs"
printf '%s\n' "$ex_topics" | grep -q 'filesystem' || fail "explain topics missing filesystem"
printf '%s\n' "$ex_topics" | grep -q 'permissions' || fail "explain topics missing permissions"
printf '%s\n' "$ex_topics" | grep -q 'desktop' || fail "explain topics missing desktop"
printf '%s\n' "$ex_topics" | grep -q 'workspaces' || fail "explain topics missing workspaces"
printf '%s\n' "$ex_topics" | grep -q 'resources' || fail "explain topics missing resources"
printf '%s\n' "$ex_topics" | grep -q 'man' || fail "explain topics missing man"
printf '%s\n' "$ex_topics" | grep -q 'guide' || fail "explain topics missing guide"
printf '%s\n' "$ex_topics" | grep -q '  lua ' || fail "explain topics missing lua"

# tam explain topics --terse emits one topic slug per line.
ex_terse=$("$TAM" explain topics --terse)
printf '%s\n' "$ex_terse" | grep -qx 'pipelines' || fail "terse topics missing pipelines"
printf '%s\n' "$ex_terse" | grep -qx 'jobs' || fail "terse topics missing jobs"
printf '%s\n' "$ex_terse" | grep -qx 'welcome' || fail "terse topics missing welcome"
if printf '%s\n' "$ex_terse" | grep -q ' '; then fail "terse output contains spaces"; fi

# tam explain topics --verbose emits detailed metadata and keyword arrays.
ex_verbose=$("$TAM" explain topics --verbose)
printf '%s\n' "$ex_verbose" | grep -q 'Keywords:' || fail "verbose topics missing Keywords"
printf '%s\n' "$ex_verbose" | grep -q 'Path:' || fail "verbose topics missing Path"
printf '%s\n' "$ex_verbose" | grep -q 'Title:' || fail "verbose topics missing Title"
printf '%s\n' "$ex_verbose" | grep -Eq 'Revision: +Rev [1-9][0-9]* Last updated [0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}' \
    || fail "verbose topics missing Revision line"

# tam explain topics --level expert --terse emits only expert topics.
ex_expert_terse=$("$TAM" explain topics --level expert --terse)
printf '%s\n' "$ex_expert_terse" | grep -qx 'pipelines' || fail "expert terse missing pipelines"
printf '%s\n' "$ex_expert_terse" | grep -qx 'jobs' || fail "expert terse missing jobs"
if printf '%s\n' "$ex_expert_terse" | grep -qx 'desktop'; then fail "expert terse unexpectedly includes desktop"; fi

# Multi-word explain resolves via search fallback path.
ex_multi=$("$TAM" explain cpu load --pipe)
printf '%s\n' "$ex_multi" | grep -q 'System Resources' || fail "explain cpu load failed"

# Topic guide depth ladder: tam explain --level LEVEL TOPIC
ex_tg_lvl_b=$("$TAM" explain --level beginner pipelines --pipe)
printf '%s\n' "$ex_tg_lvl_b" | grep -q 'Topic Guide: pipelines (beginner Level)' || fail "explain --level beginner pipelines failed"
printf '%s\n' "$ex_tg_lvl_b" | grep -q 'shell/pipelines.html?level=beginner' || fail "explain --level beginner missing link target"

# Direct page depth ladder: tam show --level LEVEL PAGE
sh_lua_b=$("$TAM" show --level beginner lua --pipe)
printf '%s\n' "$sh_lua_b" | grep -q 'Why Lua is included' || fail "show --level beginner lua failed"
if printf '%s\n' "$sh_lua_b" | grep -q 'shared stack'; then fail "beginner lua contains expert section"; fi
sh_lua_e=$("$TAM" show --level expert lua --pipe)
printf '%s\n' "$sh_lua_e" | grep -q 'How a host embeds Lua' || fail "show --level expert lua failed"
if printf '%s\n' "$sh_lua_e" | grep -q 'Why Lua is included'; then fail "expert lua contains beginner section"; fi

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

# Content-depth ladder on direct show: tam show --level LEVEL PAGE
sh_lvl_b=$("$TAM" show --level beginner pipelines --pipe)
printf '%s\n' "$sh_lvl_b" | grep -q 'Beginner: Connecting commands' || fail "show --level beginner pipelines failed"
printf '%s\n' "$sh_lvl_b" | grep -q 'Expected output:' || fail "show --level beginner missing expected output"
if printf '%s\n' "$sh_lvl_b" | grep -q 'Expert: POSIX pipe'; then fail "beginner level contains expert section"; fi

sh_lvl_i=$("$TAM" show --level intermediate pipelines --pipe)
printf '%s\n' "$sh_lvl_i" | grep -q 'Intermediate: Redirection and Multi-Stage Filters' || fail "show --level intermediate pipelines failed"
printf '%s\n' "$sh_lvl_i" | grep -q 'Completion task' || fail "show --level intermediate missing completion task"
if printf '%s\n' "$sh_lvl_i" | grep -q 'Beginner: Connecting commands'; then fail "intermediate level contains beginner section"; fi

sh_lvl_e=$("$TAM" show --level expert pipelines --pipe)
printf '%s\n' "$sh_lvl_e" | grep -q 'Expert: POSIX pipe semantics' || fail "show --level expert pipelines failed"
printf '%s\n' "$sh_lvl_e" | grep -q 'Stream Redirection Operators' || fail "show --level expert missing operators table"
if printf '%s\n' "$sh_lvl_e" | grep -q 'Beginner: Connecting commands'; then fail "expert level contains beginner section"; fi

# Numeric and 'all' level synonyms (0|all, 1|beginner, 2|intermediate, 3|expert)
sh_lvl_all=$("$TAM" show pipelines --pipe)
[ "$("$TAM" show --level 1 pipelines --pipe)" = "$sh_lvl_b" ] || fail "show --level 1 differs from beginner"
[ "$("$TAM" show --level 2 pipelines --pipe)" = "$sh_lvl_i" ] || fail "show --level 2 differs from intermediate"
[ "$("$TAM" show --level 3 pipelines --pipe)" = "$sh_lvl_e" ] || fail "show --level 3 differs from expert"
[ "$("$TAM" show --level 0 pipelines --pipe)" = "$sh_lvl_all" ] || fail "show --level 0 differs from all levels"
[ "$("$TAM" show --level all pipelines --pipe)" = "$sh_lvl_all" ] || fail "show --level all differs from all levels"
[ "$("$TAM" show "pipelines?level=1" --pipe)" = "$sh_lvl_b" ] || fail "show pipelines?level=1 differs from beginner"
[ "$("$TAM" show "pipelines?level=2" --pipe)" = "$sh_lvl_i" ] || fail "show pipelines?level=2 differs from intermediate"
[ "$("$TAM" show "pipelines?level=3" --pipe)" = "$sh_lvl_e" ] || fail "show pipelines?level=3 differs from expert"
[ "$("$TAM" show "pipelines?level=0" --pipe)" = "$sh_lvl_all" ] || fail "show pipelines?level=0 differs from all levels"
[ "$("$TAM" show "pipelines?level=all" --pipe)" = "$sh_lvl_all" ] || fail "show pipelines?level=all differs from all levels"

[ "$("$TAM" explain --level 1 pipelines --pipe)" = "$ex_tg_lvl_b" ] || fail "explain --level 1 pipelines differs from beginner"
[ "$("$TAM" explain --level 0 pipelines --pipe)" = "$ex_out" ] || fail "explain --level 0 pipelines differs from all levels"
[ "$("$TAM" explain --level all pipelines --pipe)" = "$ex_out" ] || fail "explain --level all pipelines differs from all levels"
[ "$("$TAM" explain topics --level 3 --terse)" = "$ex_expert_terse" ] || fail "explain topics --level 3 --terse differs from expert"
[ "$("$TAM" explain topics --level 0 --terse)" = "$ex_terse" ] || fail "explain topics --level 0 --terse differs from all levels"
[ "$("$TAM" explain topics --level all --terse)" = "$ex_terse" ] || fail "explain topics --level all --terse differs from all levels"

# Catalog listing filtered by level
ex_cat_expert=$("$TAM" explain --level expert)
printf '%s\n' "$ex_cat_expert" | grep -q 'Topics with a expert level:' || fail "explain --level expert missing heading"
printf '%s\n' "$ex_cat_expert" | grep -q 'pipelines' || fail "expert catalog missing pipelines"
printf '%s\n' "$ex_cat_expert" | grep -q 'jobs' || fail "expert catalog missing jobs"
if printf '%s\n' "$ex_cat_expert" | grep -q '^  desktop '; then fail "expert catalog unexpectedly includes beginner-only desktop"; fi

# Invalid level argument produces exit code 2 (usage error)
if "$TAM" explain --level invalid pipelines >/dev/null 2>&1; then fail "explain --level invalid exited 0"; fi
"$TAM" explain --level invalid pipelines 2>&1 | grep -q "unknown level 'invalid'" || fail "missing unknown level message"

# Topic lacking requested level produces clear diagnostic and exits 1
if "$TAM" explain --level expert desktop >/dev/null 2>&1; then fail "explain --level expert desktop exited 0"; fi
"$TAM" explain --level expert desktop 2>&1 | grep -q "topic 'desktop' has no expert level" || fail "missing topic level diagnostic"

pass "explain resolves curated topics, handles misses, and treats shell strings as data"

# Search command: ranked search over corpus index, json output, canonical queries, and miss banner.
"$TAM" search >/dev/null || fail "bare search failed"
"$TAM" search --help | grep -q 'Usage:' || fail "search --help missing usage"

# Canonical query: "change file owner" -> permissions page
s_owner=$("$TAM" search "change file owner")
printf '%s\n' "$s_owner" | grep -q 'permissions' || fail "search 'change file owner' missing permissions topic"
printf '%s\n' "$s_owner" | grep -q 'filesystem/permissions.html' || fail "search 'change file owner' missing permissions page"
printf '%s\n' "$s_owner" | grep -q 'Depth:' || fail "search missing Depth indicator"
printf '%s\n' "$s_owner" | grep -q 'track: use' || fail "search missing track indicator"

# JSON output
s_json=$("$TAM" search --json "change file owner")
printf '%s\n' "$s_json" | grep -q '"topic": "permissions"' || fail "search --json missing topic field"
printf '%s\n' "$s_json" | grep -q '"page": "filesystem/permissions.html"' || fail "search --json missing page field"
printf '%s\n' "$s_json" | grep -q '"levels":' || fail "search --json missing levels field"
printf '%s\n' "$s_json" | grep -q '"track": "use"' || fail "search --json missing track field"

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
"$TAM" search lua | grep -q 'ecosystem/lua.html' || fail "search 'lua' failed"

# Out-of-corpus query produces clear miss banner and exits 1 (no auto-redirect)
if "$TAM" search cron >/dev/null 2>&1; then fail "search cron unexpectedly exited 0"; fi
"$TAM" search cron 2>&1 | grep -q "no topic found for 'cron'" || fail "search cron missing clear miss message"
"$TAM" search cron 2>&1 | grep -q "Top candidates:" || fail "search cron missing candidate banner"
"$TAM" search cron 2>&1 | grep -q "tam show" || fail "search cron missing tam show pointer"

# Bounded did-you-mean for typo
"$TAM" search permisions 2>&1 | grep -qi "Did you mean" || fail "search typo missing Did you mean"

pass "search subcommand, corpus index retrieval, json output, and canonical query set"

# Install command: bare overview, help, unimplemented verbs, and retirement of inspect/check under install.
"$TAM" install | grep -q 'Usage:' || fail "bare install missing usage"
"$TAM" install | grep -q 'Planned component commands:' || fail "bare install missing planned commands"
"$TAM" install --help | grep -q 'Usage:' || fail "install --help missing usage"
"$TAM" install help | grep -q 'Usage:' || fail "install help missing usage"
[ "$("$TAM" install help)" = "$("$TAM" install --help)" ] || fail "install help differs from install --help"

# Retired subcommands under install report usage error.
if "$TAM" install inspect >/dev/null 2>&1; then fail "install inspect exited 0"; fi
"$TAM" install inspect 2>&1 | grep -q "unrecognized argument 'inspect'" || fail "install inspect missing retirement error"
if "$TAM" install check >/dev/null 2>&1; then fail "install check exited 0"; fi
"$TAM" install check 2>&1 | grep -q "unrecognized argument 'check'" || fail "install check missing retirement error"

# Inspect subcommand: reports host facts, capabilities, profiles, and findings.
ins_out=$("$TAM" inspect)
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

# Inspection Findings in text report
printf '%s\n' "$ins_out" | grep -q 'Inspection Findings:' || fail "inspect missing Inspection Findings"
printf '%s\n' "$ins_out" | grep -q 'privilege_pkexec' || fail "inspect missing privilege_pkexec finding"
printf '%s\n' "$ins_out" | grep -q 'storage_btrfs' || fail "inspect missing storage_btrfs finding"
printf '%s\n' "$ins_out" | grep -q 'storage_root_space' || fail "inspect missing storage_root_space finding"
printf '%s\n' "$ins_out" | grep -q 'init_systemd' || fail "inspect missing init_systemd finding"
printf '%s\n' "$ins_out" | grep -q 'graphics_card' || fail "inspect missing graphics_card finding"
printf '%s\n' "$ins_out" | grep -q 'package_manager' || fail "inspect missing package_manager finding"
printf '%s\n' "$ins_out" | grep -q 'kernel_release' || fail "inspect missing kernel_release finding"
printf '%s\n' "$ins_out" | grep -q 'Help:' || fail "inspect missing Help deep-link"

# Cultivar resolution in inspect matches environment / isolated config.
iso "$TAM" inspect | grep -q 'Cultivar version:  7.7.7' || fail "inspect missing cultivar version from file"
rm -f "$config/tamlinux/version"
iso "$TAM" inspect | grep -q 'Cultivar version:  unavailable' || fail "isolated inspect missing unavailable cultivar"
env HOME="$home" XDG_CONFIG_HOME="$config" XDG_STATE_HOME="$state" \
    TAMLINUX_VERSION=8.8.8 "$TAM" inspect | grep -q '8.8.8' || fail "inspect ignored cultivar env"

# Inspect pipe flag.
"$TAM" inspect --pipe | grep -q 'Host Information:' || fail "inspect --pipe failed"

# Inspect JSON output: machine-readable findings and checks schema
ins_json=$("$TAM" inspect --json)
printf '%s\n' "$ins_json" | grep -q '"schema_version": "1.0.0"' || fail "inspect --json missing schema_version"
printf '%s\n' "$ins_json" | grep -q '"findings":' || fail "inspect --json missing findings"
printf '%s\n' "$ins_json" | grep -q '"checks":' || fail "inspect --json missing checks"
printf '%s\n' "$ins_json" | grep -q '"id": "privilege_pkexec"' || fail "inspect --json missing privilege_pkexec finding id"
printf '%s\n' "$ins_json" | grep -q '"help_link":' || fail "inspect --json missing help_link"

# Check contract via inspect: tam inspect --check <task_id>
chk_btrfs=$("$TAM" inspect --check storage_btrfs)
printf '%s\n' "$chk_btrfs" | grep -q '"task_id": "storage_btrfs"' || fail "--check storage_btrfs missing task_id"
printf '%s\n' "$chk_btrfs" | grep -q '"status": "pass"' || fail "--check storage_btrfs missing status pass"
printf '%s\n' "$chk_btrfs" | grep -q '"pass": true' || fail "--check storage_btrfs missing pass boolean"
printf '%s\n' "$chk_btrfs" | grep -q '"fail": false' || fail "--check storage_btrfs missing fail boolean"
printf '%s\n' "$chk_btrfs" | grep -q '"evidence":' || fail "--check storage_btrfs missing evidence"
printf '%s\n' "$chk_btrfs" | grep -q '"help_ref": "maintain/rollback.html"' || fail "--check storage_btrfs missing help_ref"
printf '%s\n' "$chk_btrfs" | grep -q '"source": "tam inspect"' || fail "--check storage_btrfs missing source"
printf '%s\n' "$chk_btrfs" | grep -q '"schema_version": "1.0.0"' || fail "--check storage_btrfs missing schema_version"

# Check subcommand: tam check lists available checks; --json emits check array
chk_list=$("$TAM" check)
printf '%s\n' "$chk_list" | grep -q 'Available checks:' || fail "tam check missing Available checks"
printf '%s\n' "$chk_list" | grep -q 'storage_btrfs' || fail "tam check missing storage_btrfs"
printf '%s\n' "$chk_list" | grep -q 'privilege_pkexec' || fail "tam check missing privilege_pkexec"

chk_json=$("$TAM" check --json)
printf '%s\n' "$chk_json" | grep -q '"task_id": "storage_root_space"' || fail "tam check --json missing storage_root_space"

# Direct check execution: tam check <task_id>
chk_space=$("$TAM" check storage_root_space)
printf '%s\n' "$chk_space" | grep -q '"task_id": "storage_root_space"' || fail "tam check storage_root_space failed"
printf '%s\n' "$chk_space" | grep -q '"status": "pass"' || fail "tam check storage_root_space status not pass"
printf '%s\n' "$chk_space" | grep -q '"source": "tam check"' || fail "tam check storage_root_space missing source"

# Terse check outputs: compact task_id status lines
chk_terse=$("$TAM" check --terse)
printf '%s\n' "$chk_terse" | grep -q '^privilege_pkexec pass' || fail "check --terse missing privilege_pkexec"
printf '%s\n' "$chk_terse" | grep -q '^storage_btrfs pass' || fail "check --terse missing storage_btrfs"

chk_single_terse=$("$TAM" check storage_root_space --terse)
[ "$chk_single_terse" = "storage_root_space pass" ] || fail "single check --terse incorrect: $chk_single_terse"

# Privilege detect-and-degrade: when pkexec is absent from PATH
no_pk_out=$(env PATH="$tmp" "$TAM" inspect)
printf '%s\n' "$no_pk_out" | grep -q 'privilege tool unavailable: limited inspection' \
    || fail "pkexec absence missing degrade warning finding"
no_pk_chk=$(env PATH="$tmp" "$TAM" inspect --check privilege_pkexec)
printf '%s\n' "$no_pk_chk" | grep -q '"status": "unknown"' || fail "degraded check missing status unknown"
printf '%s\n' "$no_pk_chk" | grep -q '"pass": false' || fail "degraded check pass should be false"
printf '%s\n' "$no_pk_chk" | grep -q '"severity": "warn"' || fail "degraded check severity should be warn"

# Unknown check diagnostic and error handling
if "$TAM" inspect --check nonexistent_check >/dev/null 2>&1; then
    fail "inspect --check nonexistent exited 0"
fi
"$TAM" inspect --check nonexistent_check 2>&1 | grep -q "unknown check 'nonexistent_check'" \
    || fail "inspect --check nonexistent missing unknown check diagnostic"

if "$TAM" check nonexistent_check >/dev/null 2>&1; then
    fail "check nonexistent exited 0"
fi
"$TAM" check nonexistent_check 2>&1 | grep -q "unknown check 'nonexistent_check'" \
    || fail "check nonexistent missing unknown check diagnostic"

# Unimplemented operations under install: plan, apply, verify exit 1 with clear diagnostic.
if "$TAM" install plan >/dev/null 2>&1; then fail "install plan exited 0"; fi
"$TAM" install plan 2>&1 | grep -q "'plan' is not implemented yet" || fail "plan missing unimplemented diagnostic"
if "$TAM" install apply >/dev/null 2>&1; then fail "install apply exited 0"; fi
"$TAM" install apply 2>&1 | grep -q "'apply' is not implemented yet" || fail "apply missing unimplemented diagnostic"
if "$TAM" install verify >/dev/null 2>&1; then fail "install verify exited 0"; fi
"$TAM" install verify 2>&1 | grep -q "'verify' is not implemented yet" || fail "verify missing unimplemented diagnostic"

# Argument errors under install.
if "$TAM" install bogus >/dev/null 2>&1; then fail "install bogus exited 0"; fi
"$TAM" install bogus 2>&1 | grep -q "unrecognized argument" || fail "install bogus missing error message"
if "$TAM" install --help extra >/dev/null 2>&1; then fail "install --help extra exited 0"; fi

# Argument errors under inspect and check.
if "$TAM" inspect extra >/dev/null 2>&1; then fail "inspect extra exited 0"; fi
"$TAM" inspect extra 2>&1 | grep -q "too many arguments" || fail "inspect extra missing error message"
if "$TAM" inspect --bogus >/dev/null 2>&1; then fail "inspect --bogus exited 0"; fi
if "$TAM" check extra1 extra2 >/dev/null 2>&1; then fail "check extra exited 0"; fi
"$TAM" check extra1 extra2 2>&1 | grep -q "too many arguments" || fail "check extra missing error message"
if "$TAM" check --bogus >/dev/null 2>&1; then fail "check --bogus exited 0"; fi

pass "install framework overview, inspect probes, check runner, and argument errors"

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

    # View tam(1) manual page
    man_tam=$("$TAM" man tam --pipe)
    printf '%s\n' "$man_tam" | grep -q 'tam - terminal entry point' || fail "tam man tam missing title"
    printf '%s\n' "$man_tam" | grep -q 'SUBCOMMANDS' || fail "tam man tam missing subcommands section"
else
    printf 'skip - host manual pages (man, groff or pages unavailable)\n'
fi

# Missing manual page reports clear diagnostic and exit 1.
if "$TAM" man nonexistent_xyz >/dev/null 2>&1; then fail "missing man page exited 0"; fi
"$TAM" man nonexistent_xyz 2>&1 | grep -q "no manual page found for 'nonexistent_xyz'" || fail "missing man page diagnostic unclear"
if "$TAM" man 1 nonexistent_xyz >/dev/null 2>&1; then fail "missing section man page exited 0"; fi
"$TAM" man 1 nonexistent_xyz 2>&1 | grep -q "in section 1" || fail "missing section diagnostic unclear"
pass "man subcommand, cross-reference hyperlinking, and error handling"

# -----------------------------------------------------------------------------
# Milestone 14: GNU Info Subsystem (tam info)
# -----------------------------------------------------------------------------
# Wrapper script bin/tam-info delegates to tam info.
if [ -x "$root/bin/tam-info" ]; then
    info_wrap=$("$root/bin/tam-info" --pipe)
    printf '%s\n' "$info_wrap" | grep -q 'GNU Info Directory' || fail "tam-info wrapper failed to render directory"
    pass "tam-info wrapper script executes and renders directory catalog"
fi

# Directory catalog rendering under --pipe
info_dir=$("$TAM" info --pipe)
printf '%s\n' "$info_dir" | grep -q 'GNU Info Directory' || fail "tam info --pipe missing title"
printf '%s\n' "$info_dir" | grep -q 'Coreutils' || fail "tam info --pipe missing Coreutils"
printf '%s\n' "$info_dir" | grep -q 'info:coreutils' || fail "tam info --pipe missing info:coreutils link"

# Manual top node rendering (coreutils)
info_core=$("$TAM" info coreutils --pipe)
printf '%s\n' "$info_core" | grep -q 'GNU Coreutils' || fail "tam info coreutils missing title"
printf '%s\n' "$info_core" | grep -q 'Node: Top' || fail "tam info coreutils missing Node: Top"

# Targeted node lookup (head invocation)
info_head=$("$TAM" info coreutils "head invocation" --pipe)
printf '%s\n' "$info_head" | grep -q 'Node: head invocation' || fail "tam info coreutils 'head invocation' missing node header"
printf '%s\n' "$info_head" | grep -q "Output the first part of files" || fail "tam info coreutils 'head invocation' missing body text"
printf '%s\n' "$info_head" | grep -q "info:coreutils/Common options" || fail "tam info coreutils missing linkified cross reference"

# Split manual lookup (libc)
info_libc=$("$TAM" info libc --pipe)
printf '%s\n' "$info_libc" | grep -q 'The GNU C Library Reference Manual' || fail "tam info libc missing title"
info_libc_sub=$("$TAM" info libc "Unconstrained Allocation" --pipe)
printf '%s\n' "$info_libc_sub" | grep -q 'Unconstrained Allocation' || fail "tam info libc indirect subfile node lookup failed"

# Virtual URI routing in tam show
info_show=$("$TAM" show info:coreutils --pipe)
printf '%s\n' "$info_show" | grep -q 'GNU Coreutils' || fail "tam show info:coreutils failed"
info_show_node=$("$TAM" show "info:coreutils/head invocation" --pipe)
printf '%s\n' "$info_show_node" | grep -q 'head invocation' || fail "tam show info:coreutils/head invocation failed"

# Piped output has no escape sequences
if "$TAM" info coreutils --pipe | grep -q "$esc"; then
    fail "piped info output contains escape sequences"
fi

# Error handling
set +e
"$TAM" info nonexistent_man_xyz >/dev/null 2>&1
rc_no_man=$?
"$TAM" info coreutils nonexistent_node_xyz >/dev/null 2>&1
rc_no_node=$?
"$TAM" info --unknown-flag >/dev/null 2>&1
rc_flag=$?
"$TAM" info a b c >/dev/null 2>&1
rc_args=$?
set -e
[ "$rc_no_man" -eq 1 ] || fail "expected exit 1 on nonexistent manual, got $rc_no_man"
[ "$rc_no_node" -eq 1 ] || fail "expected exit 1 on nonexistent node, got $rc_no_node"
[ "$rc_flag" -eq 2 ] || fail "expected exit 2 on unknown flag, got $rc_flag"
[ "$rc_args" -eq 2 ] || fail "expected exit 2 on too many args, got $rc_args"

# Help output
"$TAM" info --help | grep -q 'tam info - browse GNU Info manuals' || fail "tam info --help missing title"
"$TAM" info -h | grep -q 'Usage:' || fail "tam info -h missing usage"
"$TAM" info --help | grep -q 'i                         jump to root Info directory catalog' || fail "tam info --help missing i dir"
"$TAM" info --help | grep -q 'a/s/d/f' || fail "tam info --help missing a/s/d/f"

pass "tam info subcommand, node parsing, cross-references, and error handling"

# Shell completion scripts verification (Tamlinux supports bash)
bash -n "$root/completions/bash/tam" || fail "bash completion syntax error"

# Test bash completion behavior
bash_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam "")
COMP_CWORD=1
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_test" | grep -q 'show' || fail "bash completion missing show subcommand"
printf '%s\n' "$bash_test" | grep -q 'info' || fail "bash completion missing info subcommand"
printf '%s\n' "$bash_test" | grep -q 'inspect' || fail "bash completion missing inspect subcommand"
printf '%s\n' "$bash_test" | grep -q 'check' || fail "bash completion missing check subcommand"
printf '%s\n' "$bash_test" | grep -q 'install' || fail "bash completion missing install subcommand"
printf '%s\n' "$bash_test" | grep -q 'search' || fail "bash completion missing search subcommand"
printf '%s\n' "$bash_test" | grep -q -- '--help' || fail "bash completion missing --help"
printf '%s\n' "$bash_test" | grep -q -- '--terse' || fail "bash completion missing --terse"
printf '%s\n' "$bash_test" | grep -q -- '--verbose' || fail "bash completion missing --verbose"

bash_install_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam install "")
COMP_CWORD=2
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_install_test" | grep -q 'plan' || fail "bash completion missing install plan"
printf '%s\n' "$bash_install_test" | grep -q 'apply' || fail "bash completion missing install apply"
printf '%s\n' "$bash_install_test" | grep -q 'verify' || fail "bash completion missing install verify"

bash_inspect_flags=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam inspect "")
COMP_CWORD=2
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_inspect_flags" | grep -q -- '--json' || fail "bash completion missing inspect --json"
printf '%s\n' "$bash_inspect_flags" | grep -q -- '--check' || fail "bash completion missing inspect --check"

bash_inspect_check_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam inspect --check "")
COMP_CWORD=3
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_inspect_check_test" | grep -q 'storage_btrfs' || fail "bash completion missing inspect --check storage_btrfs"
printf '%s\n' "$bash_inspect_check_test" | grep -q 'privilege_pkexec' || fail "bash completion missing inspect --check privilege_pkexec"

bash_check_tasks=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam check "")
COMP_CWORD=2
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_check_tasks" | grep -q 'storage_root_space' || fail "bash completion missing check storage_root_space"
printf '%s\n' "$bash_check_tasks" | grep -q -- '--json' || fail "bash completion missing check --json"

bash_search_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam search "")
COMP_CWORD=2
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_search_test" | grep -q -- '--json' || fail "bash completion missing search --json"
printf '%s\n' "$bash_search_test" | grep -q -- '--pipe' || fail "bash completion missing search --pipe"

bash_explain_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam explain "")
COMP_CWORD=2
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_explain_test" | grep -q -- '--level' || fail "bash completion missing explain --level"

bash_level_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam explain --level "")
COMP_CWORD=3
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_level_test" | grep -q 'all' || fail "bash completion missing level all"
printf '%s\n' "$bash_level_test" | grep -q 'beginner' || fail "bash completion missing level beginner"
printf '%s\n' "$bash_level_test" | grep -q 'intermediate' || fail "bash completion missing level intermediate"
printf '%s\n' "$bash_level_test" | grep -q 'expert' || fail "bash completion missing level expert"
printf '%s\n' "$bash_level_test" | grep -qx '0' || fail "bash completion missing level 0"
printf '%s\n' "$bash_level_test" | grep -qx '1' || fail "bash completion missing level 1"
printf '%s\n' "$bash_level_test" | grep -qx '2' || fail "bash completion missing level 2"
printf '%s\n' "$bash_level_test" | grep -qx '3' || fail "bash completion missing level 3"

bash_ask_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam ask "")
COMP_CWORD=2
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_ask_test" | grep -q -- '--model' || fail "bash completion missing ask --model"
printf '%s\n' "$bash_ask_test" | grep -q -- '--server' || fail "bash completion missing ask --server"
printf '%s\n' "$bash_ask_test" | grep -q -- '--json' || fail "bash completion missing ask --json"
printf '%s\n' "$bash_ask_test" | grep -q -- '--pipe' || fail "bash completion missing ask --pipe"

bash_info_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam info "")
COMP_CWORD=2
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_info_test" | grep -q 'coreutils' || fail "bash completion missing info coreutils"
printf '%s\n' "$bash_info_test" | grep -q 'bash' || fail "bash completion missing info bash"
printf '%s\n' "$bash_info_test" | grep -q -- '--pipe' || fail "bash completion missing info --pipe"

pass "static bash shell auto-completion script"

# -----------------------------------------------------------------------------
# Milestone 9: Grounded ask surface, offline boundary, and non-interactive refusal
# -----------------------------------------------------------------------------
"$TAM" --help | grep -q 'tam ask' || fail "--help missing tam ask"

# Bare invocation missing question
set +e
ask_bare_err=$("$TAM" ask 2>&1)
ask_bare_rc=$?
set -e
[ "$ask_bare_rc" -eq 2 ] || fail "expected bare tam ask exit 2, got $ask_bare_rc"
echo "$ask_bare_err" | grep -q 'missing question' || fail "bare tam ask did not report missing question"

# tam ask --help
"$TAM" ask --help | grep -q 'Usage:' || fail "tam ask --help missing usage"
"$TAM" ask -h | grep -q 'Usage:' || fail "tam ask -h missing usage"

# Non-interactive refusal when server is unreachable (exit code 2)
set +e
ask_refusal_out=$("$TAM" ask --server http://127.0.0.1:59999 --pipe "what is a pipeline" 2>&1)
ask_refusal_code=$?
set -e
[ "$ask_refusal_code" -eq 2 ] || fail "expected exit 2 on unreachable server under --pipe, got $ask_refusal_code"
echo "$ask_refusal_out" | grep -q 'tam: ask requires a local model; run interactively to fetch or start Ollama' || fail "unexpected refusal message: $ask_refusal_out"

# The remaining checks talk to a running local Ollama server with the
# qwen2.5-coder:1.5b and llama3.2:1b models. Without a server they are skipped,
# visibly, instead of aborting the whole suite.
if python3 -c 'import socket, sys; s = socket.socket(); s.settimeout(1); sys.exit(s.connect_ex(("127.0.0.1", 11434)))' 2>/dev/null; then
    # Grounding miss on unrelated topic
    ask_miss_out=$("$TAM" ask --server http://127.0.0.1:11434 "what is the capital of Mars")
    echo "$ask_miss_out" | grep -q 'Not found in Tamlinux guide.' || fail "missing 'Not found' in miss output"
    echo "$ask_miss_out" | grep -q 'Sources: (none)' || fail "missing 'Sources: (none)' in miss output"

    # Grounding miss with --json
    ask_json_miss=$("$TAM" ask --json --server http://127.0.0.1:11434 "what is the capital of Mars")
    echo "$ask_json_miss" | grep -q '"grounded": false' || fail "json miss missing grounded: false"
    echo "$ask_json_miss" | grep -q '"sources": \[\]' || fail "json miss missing empty sources array"

    # Live inference query with default model (qwen2.5-coder:1.5b)
    ask_live_out=$("$TAM" ask --pipe "what is a pipeline")
    echo "$ask_live_out" | grep -qi 'pipeline' || fail "live answer missing pipeline text"
    echo "$ask_live_out" | grep -qi 'Sources:' || fail "live answer missing Sources:"

    # Live query with --json asserts default qwen2.5-coder:1.5b
    ask_live_json=$("$TAM" ask --json --pipe "what is a pipeline")
    echo "$ask_live_json" | grep -q '"grounded": true' || fail "live json missing grounded: true"
    echo "$ask_live_json" | grep -q 'shell/pipelines.html' || fail "live json missing shell/pipelines.html source"
    echo "$ask_live_json" | grep -q 'qwen2.5-coder:1.5b' || fail "live json missing qwen2.5-coder:1.5b model"

    # Live query with explicit model override (--model llama3.2:1b)
    ask_llama_json=$("$TAM" ask --model llama3.2:1b --json --pipe "what is a pipeline")
    echo "$ask_llama_json" | grep -q 'llama3.2:1b' || fail "explicit model json missing llama3.2:1b model"

    # Reading question from stdin via '-'
    ask_stdin_out=$(echo "how do I view background jobs?" | "$TAM" ask --pipe -)
    echo "$ask_stdin_out" | grep -qi 'job' || fail "stdin answer missing job text"
    echo "$ask_stdin_out" | grep -qi 'Sources:' || fail "stdin answer missing Sources:"

    # Piped output has no escape sequences
    if "$TAM" ask --pipe "what is a pipeline" | grep -q "$esc"; then
        fail "piped ask output contains escape sequences"
    fi

    pass "tam ask grounded retrieval, local server client, and non-interactive refusal"
else
    pass "tam ask offline boundary and non-interactive refusal"
    printf 'skip - tam ask live-server checks (no Ollama at 127.0.0.1:11434)\n'
fi

printf 'All tam command tests passed.\n'
