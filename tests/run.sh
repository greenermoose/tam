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
# Pager cases change the reader's level lens; keep its state file here.
export XDG_STATE_HOME="$tmp/state-default"
# Rendered man pages and groff scratch files stay inside the test directory.
export XDG_CACHE_HOME="$tmp/cache-default" XDG_RUNTIME_DIR="$tmp/runtime-default"
unset TAM_LEVEL

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

# The additive parent column must not alter the existing updated column;
# a legacy ten-column index must continue to load as well.
compat="$tmp/index-compat"
mkdir -p "$compat/shell"
cp "$knowledge/shell/pipelines.html" "$compat/shell/pipelines.html"
cp "$knowledge/index.html" "$compat/index.html"
printf 'topic\tpath\ttitle\tsummary\tkeywords\tlevel\ttrack\tprereqs\trev\tupdated\n' > "$compat/index.tsv"
printf 'pipelines\tshell/pipelines.html\tPipelines\tSummary\tpipes\tbeginner\tuse\t\t7\t2026-10-18 10:25\n' >> "$compat/index.tsv"
printf 'path\tparent\nshell/pipelines.html\tshell/index.html\n' > "$compat/index.parents.tsv"
extended=$(TAM_KNOWLEDGE_DIR="$compat" "$TAM" show shell/pipelines --pipe)
printf '%s\n' "$extended" | grep -q 'Rev 7 Last updated 2026-10-18 10:25' \
    || fail "parent sidecar changed legacy updated metadata"
printf 'topic\tpath\ttitle\tsummary\tkeywords\tlevel\ttrack\tprereqs\trev\tupdated\n' > "$compat/index.tsv"
printf 'pipelines\tshell/pipelines.html\tPipelines\tSummary\tpipes\tbeginner\tuse\t\t7\t2026-10-18 10:25\n' >> "$compat/index.tsv"
rm "$compat/index.parents.tsv"
legacy=$(TAM_KNOWLEDGE_DIR="$compat" "$TAM" show shell/pipelines --pipe)
printf '%s\n' "$legacy" | grep -q 'Rev 7 Last updated 2026-10-18 10:25' \
    || fail "legacy ten-column index no longer loads"
pass "additive and legacy index columns preserve existing page metadata"

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
"$TAM" --version | grep -q '^tam command 0.43.1' || fail "--version missing command version"
"$TAM" --version | grep -q "^knowledge corpus $corpus_version" || fail "--version missing corpus version"
"$TAM" --version | grep -q 'knowledge corpus' || fail "--version missing knowledge corpus"
"$TAM" --version | grep -q 'libtam' || fail "--version missing libtam"
"$TAM" -V >/dev/null || fail "-V failed"
iso "$TAM" --version | grep -q 'unavailable' || fail "missing cultivar metadata note"
pass "--version and -V"

# Source upgrade help and argument validation remain offline and write-free.
upgrade_help=$(iso "$TAM" upgrade --help) || fail "tam upgrade --help failed"
printf '%s\n' "$upgrade_help" | grep -q 'Usage: tam upgrade' || fail "upgrade help missing usage"
switch_help=$(iso "$TAM" switch --help) || fail "tam switch --help failed"
printf '%s\n' "$switch_help" | grep -q 'tam switch --rollback' || fail "switch help missing rollback"
[ -z "$(find "$home" "$config" "$state" -mindepth 1 -print -quit 2>/dev/null)" ] || fail "upgrade help wrote state under isolated home"
set +e
iso "$TAM" upgrade --apply --json >/dev/null 2>&1
upgrade_conflict_rc=$?
iso "$TAM" switch --version 1.2.3 >/dev/null 2>&1
switch_conflict_rc=$?
set -e
[ "$upgrade_conflict_rc" -eq 2 ] || fail "upgrade conflicting options should exit 2"
[ "$switch_conflict_rc" -eq 2 ] || fail "switch version without release should exit 2"
[ -z "$(find "$home" "$config" "$state" -mindepth 1 -print -quit 2>/dev/null)" ] || fail "invalid upgrade invocation wrote state"
pass "upgrade and switch help are offline; invalid mutations fail before writes"

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
printf 'Incipiens\n' > "$config/tamlinux/cultivar"
iso "$TAM" --version | grep -q '^Tamlinux cultivar Incipiens 7.7.7$' \
    || fail "--version missing cultivar name"
rm -f "$config/tamlinux/cultivar"
iso "$TAM" --version | grep -q '^Tamlinux cultivar 7.7.7$' \
    || fail "--version without a name file should print the bare version"
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
[ -f "$w_state/tamlinux/tam/welcome_seen" ] || fail "welcome did not create tamlinux/tam/welcome_seen"
[ ! -e "$w_state/tamlinux/welcome_seen" ] || fail "welcome wrote the retired top-level welcome_seen"
grep -q '^[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}T' "$w_state/tamlinux/tam/welcome_seen" \
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
if [ -n "$(find "$iso_state" -mindepth 1 -print -quit 2>/dev/null)" ]; then
    fail "isolated commands wrote state: $(find "$iso_state" -mindepth 1 | head -3)"
fi
pass "state isolation: bare, show, help, version, search, explain, inspect, check, install, man do not touch welcome_seen"

# Show command: displays guide index or page; browse subcommand is retired.
sh_out=$(TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show --pipe)
printf '%s\n' "$sh_out" | grep -q 'Fixture Guide' || fail "tam show --pipe failed"
sh_page=$(TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show welcome --pipe)
printf '%s\n' "$sh_page" | grep -q 'Fixture Welcome' || fail "tam show welcome failed"
sh_topics=$("$TAM" show topics --pipe)
printf '%s\n' "$sh_topics" | grep -q 'Topics A to Z' || fail "tam show topics failed"
sh_topics_pseudo=$("$TAM" show tam:topics --pipe)
printf '%s\n' "$sh_topics_pseudo" | grep -q 'Topics A to Z' || fail "tam show tam:topics failed"

# page: addresses, tam:keywords, and tam:explain under show
sh_pipe1=$(TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show page:shell/pipelines --pipe) || fail "show page:shell/pipelines --pipe failed"
sh_pipe2=$(TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show shell/pipelines --pipe) || fail "show shell/pipelines --pipe failed"
[ "$sh_pipe1" = "$sh_pipe2" ] || fail "show page:shell/pipelines output differs from show shell/pipelines"
TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show tam:keywords --pipe >/dev/null || fail "show tam:keywords --pipe failed"
TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show 'tam:explain?topic=pipelines' --pipe >/dev/null || fail "show tam:explain?topic=pipelines --pipe failed"
set +e
sh_page_empty_err=$(TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show page: --pipe 2>&1)
sh_page_empty_rc=$?
set -e
[ "$sh_page_empty_rc" -eq 2 ] || fail "show page: expected exit 2, got $sh_page_empty_rc"
printf '%s\n' "$sh_page_empty_err" | grep -q "missing guide page after 'page:'" || fail "missing guide page error message missing"

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

# Pager link navigation: Tab advances forward, Shift-Tab and N circulate backward.
# Uses the root index in tests/fixtures/link-cycle/ (no breadcrumb) so corpus
# edits cannot change the link order.
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

last_target = ""
def send_and_get_target(key):
    global last_target
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
        return last_target
    # The link is the first word after "->"; the version follows at the right.
    words = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].split()
    last_target = words[0].decode("utf-8", "replace") if words else ""
    return last_target

time.sleep(0.15)
# 1. ] from no selection selects the same first link as Tab
t1 = send_and_get_target(b"]")
assert t1 == "first.html", f"expected first.html, got {t1}"
# 2. [ from first link wraps backward to last link, as Shift-Tab does
t_wrap = send_and_get_target(b"[")
assert t_wrap == "info:dir", f"expected info:dir, got {t_wrap}"
# 3. Tab forward wraps back to first link
t1_tab = send_and_get_target(b"\t")
assert t1_tab == "first.html", f"expected first.html, got {t1_tab}"
# 4. Tab forward selects second link
t2 = send_and_get_target(b"\t")
assert t2 == "second.html", f"expected second.html, got {t2}"
# 5. Shift-Tab (ANSI CSI Z) circulates backward
t3 = send_and_get_target(b"\x1b[Z")
assert t3 == "first.html", f"expected first.html, got {t3}"
# 6. Shift-Tab from first link wraps backward to last link
t4 = send_and_get_target(b"\x1b[Z")
assert t4 == "info:dir", f"expected info:dir, got {t4}"
# 7. Shift-Tab variants (CSI 1;2Z, Kitty 9;2u, modifyOtherKeys 27;2;9~)
t5 = send_and_get_target(b"\x1b[1;2Z")
assert t5 == "tam:man", f"expected tam:man, got {t5}"
t6 = send_and_get_target(b"\x1b[9;2u")
assert t6 == "docs/help.html", f"expected docs/help.html, got {t6}"
t7 = send_and_get_target(b"\x1b[27;2;9~")
assert t7 == "tam:topics", f"expected tam:topics, got {t7}"
# 8. N with no search active leaves the selected link unchanged
t8 = send_and_get_target(b"N")
assert t8 == "tam:topics", f"expected tam:topics, got {t8}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures/link-cycle" || fail "interactive pager link navigation failed"
    pass "pager link navigation: Tab forward and Shift-Tab backward circulation"

    # Pager home-row keys (D175) and Info dir rebind to i
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl

# 1. Home-row navigation on the nav-grid fixture
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
    return lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].split(b"  ")[0].strip().decode("utf-8", "replace")

def send_and_get_state(key):
    time.sleep(0.08)
    while select.select([master], [], [], 0.02)[0]:
        os.read(master, 1024)
    os.write(master, key)
    time.sleep(0.08)
    buf = b""
    while select.select([master], [], [], 0.02)[0]:
        buf += os.read(master, 1024)
    lines = [l for l in buf.split(b"\x1b[24;1H") if b"|" in l]
    last = lines[-1].split(b"\x1b[K", 1)[0] if lines else b""
    pos = last.split(b"|")[1].strip().decode() if b"|" in last else ""
    msg = last.split(b"[")[-1].split(b"]")[0].decode("utf-8", "replace") if b"]" in last else ""
    return pos, msg

time.sleep(0.15)
# The heading line has no links: left and right have nothing to catch.
assert send_and_get_target(b"f") == "", "f caught a link on a line without links"
assert send_and_get_target(b"j") == "", "j caught a link on a line without links"

# k moves the attention line down one row; the first link row catches tl.html.
assert send_and_get_target(b"k") == ""
t = send_and_get_target(b"k")
assert t == "tl.html", f"expected k to reach the first link row, got {t}"

# Right (f, ;) and left (a, j) move along the line and wrap at either end.
for key, want in ((b"f", "tr.html"), (b";", "tl.html"), (b"a", "tr.html"), (b"j", "tl.html")):
    t = send_and_get_target(key)
    assert t == want, f"expected {key!r} to catch {want}, got {t}"

# Down (d) catches the first link on the next row; right moves along it.
assert send_and_get_target(b"d") == "bl.html"
assert send_and_get_target(b"f") == "br.html"
# Up (l, s) moves one row; moving up catches the last link on the row.
assert send_and_get_target(b"l") == "tr.html"
assert send_and_get_target(b"s") == "tr.html", "the caught link stays selected on the blank row while visible"

# Bottom (c, ,) and top (x, .) jump to the ends of the page; G is retired.
pos, _ = send_and_get_state(b"x")
assert pos.startswith("L1 "), f"expected x to reach the top, got {pos}"
pos, _ = send_and_get_state(b"c")
assert pos.startswith("L4 "), f"expected c to reach the bottom, got {pos}"
pos, _ = send_and_get_state(b".")
assert pos.startswith("L1 "), f"expected . to reach the top, got {pos}"
pos, _ = send_and_get_state(b",")
assert pos.startswith("L4 "), f"expected , to reach the bottom, got {pos}"
send_and_get_state(b"x")
pos, _ = send_and_get_state(b"G")
assert pos == "", f"expected retired G to do nothing, got {pos}"

# g follows the caught link (the fixture targets do not exist).
send_and_get_target(b"c")
assert send_and_get_target(b"f") == "br.html"
_, msg = send_and_get_state(b"g")
assert msg == "cannot open br.html", f"expected g to follow br.html, got [{msg}]"

os.write(master, b"q")
os.waitpid(pid, 0)

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

' "$TAM" "$root/tests/fixtures" "$root/tests/fixtures/guide" || fail "interactive pager home-row keys or i dir key failed"
    pass "pager home-row keys: a/j f/; along the line, s/l d/k one row, x/. c/, top and bottom, g follows, i dir"

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
        target = last.split("->")[-1].split("\x1b", 1)[0].split("  ")[0].strip()
    last_info = (title, target)
    return last_info

last_buf = b""
def send_and_get_info(key):
    global last_buf
    time.sleep(0.15)
    while select.select([master], [], [], 0.02)[0]:
        os.read(master, 1024)
    os.write(master, key)
    time.sleep(0.15)
    buf = b""
    while select.select([master], [], [], 0.02)[0]:
        buf += os.read(master, 1024)
    last_buf = buf
    return parse_info(buf)

t, tg = parse_info(buf)
assert t == "Fixture Guide" and tg == "", f"got {t!r} {tg!r} buf={buf!r}"

# 1. At tip of exploration and no link selected: right arrow does nothing
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Guide" and tg == "", f"got {t!r} {tg!r}"

# 2. Select first link (welcome.html)
t, tg = send_and_get_info(b"\t")
assert t == "Fixture Guide" and tg == "welcome.html"

# 3. At tip of exploration with link selected: right arrow extends exploration
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Welcome" and tg == ""

# 4. At new tip with no link selected: right arrow does nothing
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Welcome" and tg == ""

# 5. Left arrow restores the index page and its selected link
t, tg = send_and_get_info(b"\x1b[D")
assert t == "Fixture Guide" and tg == "welcome.html", f"got {t!r} {tg!r}"

# 6. Arrived via left arrow with focus restored: right arrow follows welcome.html
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Welcome" and tg == ""

# 7. On welcome.html, skip navigation links and follow the desktop link;
# the fixed Guide breadcrumb remains available at the end of Tab traversal.
for _ in range(10):
    t, tg = send_and_get_info(b"\t")
    if tg == "desktop/index.html":
        break
assert tg == "desktop/index.html", f"desktop link not reached, got {tg!r}"
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Desktop" and tg == ""

# 8. Multi-step left arrow: back to welcome, then back to index
t, tg = send_and_get_info(b"\x1b[D")
assert t == "Fixture Welcome"
t, tg = send_and_get_info(b"\x1b[D")
assert t == "Fixture Guide"

# 9. Multi-step right arrow: forward to welcome, then forward to desktop, then tip (does nothing)
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Welcome"
# Following a selected root link starts a new branch and clears forward history.
# Find the desktop link without depending on the number of navigation links.
for _ in range(10):
    t, tg = send_and_get_info(b"\t")
    if tg == "desktop/index.html":
        break
assert tg == "desktop/index.html", f"desktop link not reached, got {tg!r}"
t, tg = send_and_get_info(b"\r")
assert t == "Fixture Desktop", f"got {t!r} {tg!r} raw={last_buf!r}"
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Desktop"

# 10. Return to index, then select DIFFERENT link (install/index.html), right arrow extends branch
t, tg = send_and_get_info(b"\x1b[D")
assert t == "Fixture Welcome"
t, tg = send_and_get_info(b"\x1b[D")
assert t == "Fixture Guide"
# The previously focused welcome link is restored; one Tab selects install/index.html.
t, tg = send_and_get_info(b"\t")
assert tg == "install/index.html"
# Right arrow extends this branch
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Installing" and tg == ""
# At tip of new branch: right arrow does nothing
t, tg = send_and_get_info(b"\x1b[C")
assert t == "Fixture Installing" and tg == ""

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures/guide" || fail "interactive pager forward navigation failed"
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
target = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].split(b"  ")[0].strip().decode("utf-8", "replace") if lines else ""
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
target = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].split(b"  ")[0].strip().decode("utf-8", "replace") if lines else ""
assert target == "two.html", f"expected two.html, got {target}"

# 4. Shift-Tab returns to the wrapped link, highlighting its lines again
os.write(master, b"\x1b[Z")
time.sleep(0.08)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)

lines = [l for l in buf.split(b"\x1b[24;1H") if b"->" in l]
target = lines[-1].split(b"->")[-1].split(b"\x1b", 1)[0].split(b"  ")[0].strip().decode("utf-8", "replace") if lines else ""
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
import pty, os, re, time, sys, select, termios, struct, fcntl

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

# 1. Reading telemetry: verify status line contains position metric
assert b"L1 1-" in buf and b"/" in buf, f"status line missing position metric: {buf}"
# The status row lists only Help, Quit and link selection, then the version.
assert b"h Help  Esc Quit  [] Select links" in buf, f"status line missing key hints: {buf}"
assert b"/ n/N" not in buf and b"q Quit" not in buf, f"status line kept old hints: {buf}"
assert re.search(rb"v\d+\.\d+\.\d+ \([^)]*\)\x1b\[K", buf), f"status line missing version: {buf}"

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
' "$TAM" "$root/tests/fixtures/guide" || fail "interactive pager search and telemetry test failed"
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
assert t == "Fixture pipelines", f"unexpected initial title: {t}"

# 2. Press backtick (`) to jump to index.html
os.write(master, b"`")
t = get_title()
assert t == "Fixture Guide", f"expected index title after backtick, got: {t}"

# 3. Press u to return to shell/pipelines
os.write(master, b"u")
t = get_title()
assert t == "Fixture pipelines", f"expected pipelines title after u, got: {t}"

# 4. Press backtick again, then quit
os.write(master, b"`")
t = get_title()
assert t == "Fixture Guide", f"expected index title after second backtick, got: {t}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures/guide" || fail "interactive pager backtick return failed"
    pass "pager quick Guide index return (\`): immediate index jump and history push"

    # Pager '=' address display and toggle
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

last_status = ""
def get_status_line():
    global last_status
    time.sleep(0.08)
    buf = b""
    while select.select([master], [], [], 0.02)[0]:
        buf += os.read(master, 4096)
    if b"\x1b[24;1H" in buf:
        part = buf.split(b"\x1b[24;1H")[-1]
        last = part.split(b"\x1b[K", 1)[0].decode("utf-8", "replace")
        last_status = last.strip()
    return last_status

time.sleep(0.15)
# Initial state shows position metric
status0 = get_status_line()
assert "L1 1-" in status0, f"expected L1 position at start, got: {status0}"

# 1. = shows Path = page:shell/pipelines.html
os.write(master, b"=")
status = get_status_line()
assert "Path = page:shell/pipelines.html" in status, f"expected Path = page:shell/pipelines.html, got: {status}"

# 2. A line-down key (d) keeps it
os.write(master, b"d")
status = get_status_line()
assert "Path = page:shell/pipelines.html" in status, f"expected Path = ... to persist after line down, got: {status}"

# 3. Esc removes it and the position appears again
os.write(master, b"\x1b")
status = get_status_line()
assert "Path = " not in status and "L2 1-" in status, f"expected position restored after Esc, got: {status}"

# 4. = twice shows and then hides it
os.write(master, b"=")
status = get_status_line()
assert "Path = page:shell/pipelines.html" in status, f"expected Path = ... after first =, got: {status}"
os.write(master, b"=")
status = get_status_line()
assert "Path = " not in status and "L2 1-" in status, f"expected position restored after second =, got: {status}"

# 5. = and then following a link removes it on the new page
os.write(master, b"=")
status = get_status_line()
assert "Path = page:shell/pipelines.html" in status
# [ selects the breadcrumb link (index.html), Enter follows it
os.write(master, b"[")
time.sleep(0.08)
os.write(master, b"\r")
time.sleep(0.15)
buf = b""
while select.select([master], [], [], 0.05)[0]:
    buf += os.read(master, 4096)
lines = [l for l in buf.split(b"\x1b[24;1H") if l]
last = lines[-1].split(b"\x1b[K", 1)[0].decode("utf-8", "replace").strip() if lines else ""
assert "Path = " not in last, f"expected address cleared on new page, got: {last}"
assert "Fixture Guide" in buf.decode("utf-8", "replace"), "expected to be on Fixture Guide"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures/guide" || fail "interactive pager address toggle failed"
    pass "pager address toggle (=): path display, persistence, Esc clear, and navigation reset"

    # Pager guide search ('\') and in-page search ('/') prompts; show quick-keys links
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl

binary, corpus = sys.argv[1], sys.argv[2]

def launch(args):
    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 160, 0, 0))
    pid = os.fork()
    if pid == 0:
        os.close(master)
        os.dup2(slave, 0); os.dup2(slave, 1); os.dup2(slave, 2); os.close(slave)
        env = os.environ.copy()
        env["TAM_KNOWLEDGE_DIR"] = corpus
        env["TERM"] = "xterm-256color"
        os.execve(binary, [binary] + args, env)
    os.close(slave)
    return pid, master

def drain(fd, delay=0.1):
    time.sleep(delay)
    buf = b""
    while select.select([fd], [], [], 0.02)[0]:
        buf += os.read(fd, 8192)
    return buf

# 1. \ shows Search in guide \, typing pipe and Enter shows Results for and Fixture pipelines
pid, master = launch(["show", "welcome"])
drain(master, 0.15)
os.write(master, b"\\")
buf = drain(master, 0.1)
assert b"Search in guide \\" in buf, f"expected guide search prompt, got: {buf}"
os.write(master, b"pipe\r")
buf = drain(master, 0.3)
assert b"Results for" in buf and b"Fixture pipelines" in buf, f"expected search results, got: {buf}"
os.write(master, b"q")
os.waitpid(pid, 0)
os.close(master)

# 2. / shows Search in page /
pid, master = launch(["show", "welcome"])
drain(master, 0.15)
os.write(master, b"/")
buf = drain(master, 0.1)
assert b"Search in page /" in buf, f"expected page search prompt, got: {buf}"
os.write(master, b"\x1b")
drain(master, 0.1)
os.write(master, b"q")
os.waitpid(pid, 0)
os.close(master)

# 3. show quick-keys: following each link opens page search, guide search, and help overlay
pid, master = launch(["show", "quick-keys"])
drain(master, 0.15)

# First link: Search in page /
os.write(master, b"]")
drain(master, 0.05)
os.write(master, b"\r")
buf = drain(master, 0.1)
assert b"Search in page /" in buf, f"expected page search prompt from link, got: {buf}"
os.write(master, b"\x1b")
drain(master, 0.1)

# Second link: Search in guide \
os.write(master, b"]")
drain(master, 0.05)
os.write(master, b"\r")
buf = drain(master, 0.1)
assert b"Search in guide \\" in buf, f"expected guide search prompt from link, got: {buf}"
os.write(master, b"\x1b")
drain(master, 0.1)

# Third link: Navigation help
os.write(master, b"]")
drain(master, 0.05)
os.write(master, b"\r")
buf = drain(master, 0.1)
assert b"Navigation help" in buf, f"expected help overlay from link, got: {buf}"
os.write(master, b"\x1b")
drain(master, 0.1)

os.write(master, b"q")
os.waitpid(pid, 0)
os.close(master)

# 4. tam search on a terminal opens the results page in the pager; its links follow
pid, master = launch(["search", "pipe"])
buf = drain(master, 0.3)
assert b"Results for" in buf and b"Fixture pipelines" in buf, f"expected search results in pager, got: {buf}"
os.write(master, b"=")
buf = drain(master, 0.1)
assert b"tam:search?query=pipe" in buf, f"expected search address, got: {buf}"
os.write(master, b"\x1b")
drain(master, 0.1)
os.write(master, b"]")
buf = drain(master, 0.05)
assert b"-> shell/pipelines.html" in buf, f"expected result link selected, got: {buf}"
os.write(master, b"\r")
buf = drain(master, 0.3)
assert b"Fixture pipelines |" in buf and b"Results for" not in buf, f"expected pipelines page from result link, got: {buf}"
os.write(master, b"q")
os.waitpid(pid, 0)
os.close(master)

# 5. A search miss stays on the terminal with suggestions and a failing status
pid, master = launch(["search", "zzzqqq"])
_, status = os.waitpid(pid, 0)
buf = b""
while True:
    try:
        chunk = os.read(master, 8192)
    except OSError:
        break
    if not chunk:
        break
    buf += chunk
os.close(master)
assert b"no topic found" in buf, f"expected miss banner, got: {buf}"
assert os.WEXITSTATUS(status) == 1, f"expected exit 1 on miss, got {status}"
' "$TAM" "$root/tests/fixtures/guide" || fail "interactive pager prompt and quick-keys tests failed"
    pass "pager search prompts (\\ and /), quick-keys links, and tam search in the pager"

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
        target = last.split("->")[-1].split("\x1b", 1)[0].split("  ")[0].strip()
    return (title, target)

# 1. Initial page is Topics A to Z
title, _ = get_status_info()
assert title == "Topics A to Z", f"unexpected topics title: {title}"

# 2. Tab selects first topic link
os.write(master, b"\t")
title, target = get_status_info()
assert target != "", "expected link target to be selected by Tab"

# 3. Enter follows selected topic link into explanation
os.write(master, b"\r")
title_topic, _ = get_status_info()
assert title_topic != "Topics A to Z", f"did not follow link into topic explanation: {title_topic}"

# 4. u returns to topics page
os.write(master, b"u")
title_back, _ = get_status_info()
assert title_back == "Topics A to Z", f"did not return to topics page: {title_back}"

# 5. Backtick (`) jumps to root guide index.html
os.write(master, b"`")
title_home, _ = get_status_info()
assert title_home == "Fixture Guide", f"did not jump to guide index: {title_home}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures/guide" || fail "interactive explain topics pager failed"
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
        target = last.split(b"->")[-1].split(b"\x1b", 1)[0].split(b"  ")[0].strip().decode("utf-8", "replace")
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
assert "L1 1-4/4" in pos, f"expected L1 1-4/4 in position metric, got: {pos}"
assert target == "", f"expected no initial selection, got: {target}"

# 1. First k moves to blank separator line: no link
title, pos, target, msg, has_tint = send_key(b"k")
assert "L2 1-4/4" in pos and target == "", f"expected blank line, got: {pos} {target}"

# 2. Second k moves onto links line 1: downward sweep catches FIRST link (tl.html)
title, pos, target, msg, has_tint = send_key(b"k")
assert "L3 1-4/4" in pos and target == "tl.html", f"expected downward sweep to catch first link tl.html, got: {target}"

# 3. Third k moves onto links line 2: downward sweep catches FIRST link (bl.html)
title, pos, target, msg, has_tint = send_key(b"k")
assert "L4 1-4/4" in pos and target == "bl.html", f"expected downward sweep to catch first link bl.html, got: {target}"

# 4. Tab advances on line 2 to br.html
title, pos, target, msg, has_tint = send_key(b"\t")
assert target == "br.html", f"expected Tab to advance to br.html, got: {target}"

# 5. Upward sweep (l) back to line 1: catches LAST link (tr.html)
title, pos, target, msg, has_tint = send_key(b"l")
assert "L3 1-4/4" in pos and target == "tr.html", f"expected upward sweep to catch last link tr.html, got: {target}"

# 6. Upward sweep (l) to blank line: tr.html stays selected because it is still visible!
title, pos, target, msg, has_tint = send_key(b"l")
assert "L2 1-4/4" in pos and target == "tr.html", f"expected tr.html to stay selected while visible, got: {target}"

# 7. Upward sweep (l) to heading line: tr.html stays selected while visible!
title, pos, target, msg, has_tint = send_key(b"l")
assert "L1 1-4/4" in pos and target == "tr.html", f"expected tr.html to stay selected while visible, got: {target}"

os.write(master, b"q")
os.waitpid(pid, 0)

# Part 2: Fixed breadcrumb links remain reachable while the page scrolls
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
# Shift-Tab from no selection wraps to the final canonical parent breadcrumb.
title, pos, target, msg, has_tint = send_key(b"\x1b[Z")
assert target == "learn/index.html", f"expected Learning breadcrumb, got: {target}"
# Page down: the fixed parent breadcrumb remains selected and available.
title, pos, target, msg, has_tint = send_key(b" ")
# Enter follows the visible parent breadcrumb.
title, pos, target, msg, has_tint = send_key(b"\r")
assert title == "Fixture Learning", f"expected Learning after following breadcrumb, got: {title}"
# The parent page still exposes Guide as a selectable ancestor.
title, pos, target, msg, has_tint = send_key(b"\x1b[Z")
assert target == "index.html", f"expected Guide breadcrumb, got: {target}"
title, pos, target, msg, has_tint = send_key(b"\r")
assert title == "Fixture Guide", f"expected Guide after following breadcrumb, got: {title}"

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
send_key(b"k")
send_key(b"k")
send_key(b"k")
title, pos, target, msg, _ = send_key(b"\t")
assert "L4 1-4/4" in pos and target == "br.html", f"expected L4 1-4/4 br.html before jump, got: {pos} {target}"
# Jump to index.html via backtick (`), then scroll down 1 line on index.html
title, pos, target, msg, _ = send_key(b"`")
assert "L1 " in pos, f"expected L1 on index.html after backtick jump, got: {pos}"
title, pos_idx, _, _, _ = send_key(b"k")
assert "L2 " in pos_idx, f"expected L2 on index.html after k, got: {pos_idx}"
# Go back (Left Arrow \x1b[D): restore L4 1-4/4 and its focused br.html link.
title, pos, target, msg, _ = send_key(b"\x1b[D")
assert "L4 1-4/4" in pos and target == "br.html", f"expected back to restore L4 1-4/4 and br.html focus, got: {pos} {target}"
# The restored focus remains actionable.
title, pos, target, msg, _ = send_key(b"\x1b[C")
assert title != "Pager fixture", f"expected Right Arrow to follow restored focus, got: {title}"
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
assert target_w == "" and not pos_w.startswith("L1 "), f"expected scrolled welcome with no link, got: {pos_w} {target_w}"
# Jump to index.html with backtick (`), then page down on index.html
send_key(b"`")
title, pos_i, _, _, _ = send_key(b" ")
assert not pos_i.startswith("L1 "), f"expected scrolled index.html, got: {pos_i}"
# Left Arrow (\x1b[D) returns to welcome at saved line pos_w (with no link selected)
title, pos_back, target_back, _, _ = send_key(b"\x1b[D")
# Compare the cursor line metric independently of the viewport status label.
assert title == "Fixture Welcome" and pos_back.split()[0] == pos_w.split()[0] and target_back == "", f"expected Welcome at {pos_w}, got: {title} {pos_back} {target_back}"
# Right Arrow (\x1b[C) with no link selected goes forward to index.html at saved line pos_i
title, pos_fwd, target_fwd, _, _ = send_key(b"\x1b[C")
assert title == "Fixture Guide" and target_fwd == "", f"expected forward to restore the Guide root, got: {title} {pos_fwd} {target_fwd}"

os.write(master, b"q")
os.waitpid(pid, 0)

# Part 4: Down leaves the breadcrumb on a page whose only links are breadcrumbs
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
    os.execl(sys.argv[1], sys.argv[1], "show", "breadcrumb-only")
os.close(slave)

time.sleep(0.15)
_, pos, target, _, _ = get_state()
assert pos.startswith("L1 ") and target == "", f"expected L1 with no selection, got: {pos} {target}"
_, pos, target, _, _ = send_key(b"\x1b[A")
assert target == "index.html", f"expected Up on the first line to focus the breadcrumb, got: {target}"
_, pos, target, _, _ = send_key(b"\x1b[B")
assert pos.startswith("L1 ") and target == "", f"expected Down to return to the first line, got: {pos} {target}"
_, pos, target, _, _ = send_key(b"\x1b[B")
assert pos.startswith("L2 ") and target == "", f"expected Down to move the attention line, got: {pos} {target}"
_, pos, target, _, _ = send_key(b"\x1b[A")
assert pos.startswith("L1 ") and target == "", f"expected Up to move the attention line, got: {pos} {target}"
# From mid-page, Shift-Tab focuses the breadcrumb; Down and k return to the same line.
_, pos_mid, _, _, _ = send_key(b"kk")
assert pos_mid.startswith("L3 "), f"expected kk to reach L3, got: {pos_mid}"
_, _, target, _, _ = send_key(b"\x1b[Z")
assert target == "index.html", f"expected Shift-Tab to focus the breadcrumb, got: {target}"
_, pos, target, _, _ = send_key(b"\x1b[B")
assert pos.split()[0] == pos_mid.split()[0] and target == "", f"expected Down to return to {pos_mid}, got: {pos} {target}"
send_key(b"\x1b[Z")
_, pos, target, _, _ = send_key(b"k")
assert pos.split()[0] == pos_mid.split()[0] and target == "", f"expected k to return to {pos_mid}, got: {pos} {target}"

os.write(master, b"q")
os.waitpid(pid, 0)
' "$TAM" "$root/tests/fixtures" "$root/tests/fixtures/guide" || fail "interactive pager attention line test failed"
    pass "attention line: charcoal tint, directional link catching, anchored Enter, and history position stack"

    python3 "$here/pager_screen.py" "$TAM" "$root/tests/fixtures" || fail "pager PTY screen-state checks failed"

    # Level lens (TCK21): 0-3 set one lens that holds across links, Back and
    # a restart, is saved in tamlinux/tam/level, and never dead-ends a page.
    python3 -c '
import pty, os, time, sys, select, termios, struct, fcntl, re

tam, guide, tldr, tmp = sys.argv[1:5]

def strip_ansi(s):
    # terminfo emits character-set selection (ESC ( B) around styled links
    # as well as CSI controls; neither is visible text.
    return re.sub(r"\x1b(?:\[[0-?]*[ -/]*[@-~]|[()][0-~])", "", s)

class Pager:
    def __init__(self, args, state, cols=160, env=None, stdin=None):
        self.master, slave = pty.openpty()
        fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, cols, 0, 0))
        self.pid = os.fork()
        if self.pid == 0:
            os.close(self.master)
            # Standard input is a pipe: the pager reads keys from /dev/tty.
            os.setsid()
            fcntl.ioctl(slave, termios.TIOCSCTTY, 0)
            if stdin is not None:
                r, w = os.pipe()
                os.write(w, stdin)
                os.close(w)
                os.dup2(r, 0)
            else:
                os.dup2(slave, 0)
            os.dup2(slave, 1)
            os.dup2(slave, 2)
            os.close(slave)
            os.environ["TAM_KNOWLEDGE_DIR"] = guide
            os.environ["TAM_TLDR_DIR"] = tldr
            os.environ["XDG_STATE_HOME"] = state
            os.environ.pop("TAM_LEVEL", None)
            os.environ.update(env or {})
            os.execl(tam, tam, *args)
        os.close(slave)
        time.sleep(0.2)
        self.first = self.drain()

    def drain(self):
        buf = b""
        while select.select([self.master], [], [], 0.05)[0]:
            buf += os.read(self.master, 65536)
        return strip_ansi(buf.decode("utf-8", "replace"))

    def key(self, k):
        self.drain()
        os.write(self.master, k)
        time.sleep(0.1)
        return self.drain()

    def quit(self):
        os.write(self.master, b"q")
        os.waitpid(self.pid, 0)
        os.close(self.master)

def saved(state):
    try:
        with open(os.path.join(state, "tamlinux", "tam", "level")) as f:
            return f.read().strip()
    except FileNotFoundError:
        return None

state = os.path.join(tmp, "lens-state")

# A fresh reader starts at All; 0 says so and writes nothing.
p = Pager(["show", "shell/pipelines"], state)
assert "All text" in p.first, p.first
assert "All text 0123" in p.first, f"status row lacks the lens: {p.first}"
# The page opens on its title, line 1; the level line above it is Lb and the
# breadcrumb row La, and the range and total count only numbered lines.
assert "L1 1-" in p.first, f"page did not open on its title as L1: {p.first}"
b = p.key(b"\x1b[A")
assert "Lb 1-" in b, f"level line is not Lb: {b}"
b = p.key(b"\x1b[A")
assert "La 1-" in b, f"breadcrumb row is not La: {b}"
b = p.key(b"\x1b[B")
assert "Lb 1-" in b, f"down from the breadcrumb did not return to Lb: {b}"
b = p.key(b"\x1b[B")
assert "L1 1-" in b, f"down from the level line did not reach L1: {b}"
# The top of the page is the title, not the level line above it.
for top in (b"x", b"."):
    p.key(b"c")
    b = p.key(top)
    assert "L1 1-" in b, f"{top!r} did not go to L1: {b}"
b = p.key(b"0")
assert "already at Level 0 All" in b, b
assert saved(state) is None, "pressing the current lens wrote state"

# 2 re-renders in place: level line, hidden-section markers, status, message.
b = p.key(b"2")
assert "Intermediate text on Intermediate path" in b, b
assert "-- 1 Beginner: Beginner (hidden; press 1 or 0) --" in b, b
assert "-- 3 Expert: Expert (hidden; press 3 or 0) --" in b, b
assert "Level 2 Intermediate: 2 sections hidden" in b, b
assert "Intermediate text 0123" in b, b
assert saved(state) == "intermediate", saved(state)

# The lens holds across a link to a single-level page, which opens under a
# truthful level line instead of failing, and the status row says so.
b = p.key(b"`")
assert "Beginner text on Intermediate path (no Intermediate text)" in b, b
# The relatives line links children of the page at the lens level, from
# index.parents.tsv.
assert "Intermediate text: Fixture pipelines" in b, b
assert "(not on page)" in b, b
# = shows the address with the lens.
b = p.key(b"=")
assert "Path = page:index.html?level=intermediate" in b, b
p.key(b"=")

# Back returns to the page at the current lens; it does not undo the lens.
b = p.key(b"\x1b[D")
assert "Intermediate text on Intermediate path" in b, b
b = p.key(b"3")
assert "Expert text on Expert path" in b, b
b = p.key(b"\x1b[D")
assert "Expert text 0123" in b, f"Back undid the lens change: {b}"
p.quit()

# The lens survives a restart.
p = Pager(["show", "shell/pipelines"], state)
assert "Expert text on Expert path" in p.first, p.first
# A link that names a level sets the lens.
for _ in range(12):
    b = p.key(b"]")
    if "-> shell/pipelines.html?level=beginner" in b:
        break
else:
    raise AssertionError("no beginner link on the page")
b = p.key(b"\r")
assert "Beginner text on Beginner path" in b, b
assert saved(state) == "beginner", saved(state)
p.quit()

# TAM_LEVEL overrides the file and is never written back.
p = Pager(["show", "shell/pipelines"], state, env={"TAM_LEVEL": "expert"})
assert "Expert text on Expert path" in p.first, p.first
b = p.key(b"2")
assert "Intermediate text on Intermediate path" in b, b
p.quit()
assert saved(state) == "beginner", f"TAM_LEVEL run wrote the lens: {saved(state)}"

# --level overrides both for this run only.
p = Pager(["show", "--level", "3", "shell/pipelines"], state)
assert "Expert text on Expert path" in p.first, p.first
p.quit()
assert saved(state) == "beginner", saved(state)

# An unreadable or invalid state file means All, silently.
with open(os.path.join(state, "tamlinux", "tam", "level"), "w") as f:
    f.write("seven\n")
p = Pager(["show", "shell/pipelines"], state)
assert "All text" in p.first and "All text on" not in p.first, p.first
p.quit()

# One-level pages: tldr and standard input keep the lens and say so.
p = Pager(["show", "tldr:tar"], state)
b = p.key(b"2")
assert "Level 2 Intermediate set; this page has one level" in b, b
p.quit()
p = Pager(["show", "-"], state, stdin=b"<html><body><h1>Piped</h1><p>Text.</p></body></html>")
b = p.key(b"1")
assert "Level 1 Beginner set; this page has one level" in b, b
p.quit()

# Status row: "Beginner text 0123" from 100 columns, "Beginner 0123" from 60,
# the digits alone below 60.
for cols, want, absent in ((120, "Beginner text 0123", None), (80, "Beginner 0123", "text 0123"), (40, "0123", "Beginner")):
    p = Pager(["show", "shell/pipelines"], state, cols=cols)
    p.key(b"1") if "Beginner text on" not in p.first else None
    b = p.key(b"x")
    text = p.first + b
    at = max(text.rfind("L1 "), text.rfind("Lb "))
    status = text[at:at + 40] if at >= 0 else ""
    assert want in status, f"{cols} columns: {status!r}"
    assert absent is None or absent not in status, f"{cols} columns: {status!r}"
    p.quit()

# 4..9 do nothing.
p = Pager(["show", "shell/pipelines"], state)
assert p.key(b"4") == "", "key 4 changed the display"
p.quit()

# TCK22: fold keys use ordinary links and never navigate or expose private
# state in the address. Leaving via a link, Back or Forward hides answers.
p = Pager(["show", "folds.html"], state, env={"TAM_LEVEL": "all"})
assert "SECRET_ALPHA" not in p.first and "SECRET_BETA" not in p.first, p.first
assert "[+] Answer (-> to show)" in p.first, p.first
assert "[+] Model answer (-> to show)" in p.first, p.first
# + and - act on the answer on the attention line or selected; elsewhere they
# say where to go.
b = p.key(b"+")
assert "Move to a [+] line to show an answer" in b and "SECRET_ALPHA" not in b, b
p.key(b"]")
b = p.key(b"+")
assert "SECRET_ALPHA" in b and "[-] Answer (<- to hide)" in b and "Quiz 1/2 revealed" in b, b
b = p.key(b"+")
assert "Quiz 0/2 revealed" not in b, f"+ on a shown answer hid it: {b}"
b = p.key(b"-")
assert "SECRET_ALPHA" not in b and "[+] Answer (-> to show)" in b and "Quiz 0/2 revealed" in b, b
assert "Quiz" not in p.key(b"-"), "- on a hidden answer changed it"
# Left Arrow hides a shown answer and stays on the page; on a hidden answer
# it goes back, and the first page has nowhere to go.
p.key(b"+")
b = p.key(b"\x1b[D")
assert "SECRET_ALPHA" not in b and "[+] Answer (-> to show)" in b and "Quiz 0/2 revealed" in b, b
b = p.key(b"\x1b[D")
assert "no earlier page" in b and "SECRET_ALPHA" not in b, b
# The hints name the arrows: Right Arrow shows, Left Arrow hides.
b = p.key(b"\x1b[C")
assert "SECRET_ALPHA" in b and "[-] Answer (<- to hide)" in b, b
b = p.key(b"\x1b[D")
assert "SECRET_ALPHA" not in b and "[+] Answer (-> to show)" in b, b
# Enter (and Right Arrow) toggle silently.
b = p.key(b"\r")
assert "SECRET_ALPHA" in b and "SECRET_BETA" not in b and "Quiz 1/2 revealed" in b, b
b = p.key(b"\r")
assert "SECRET_ALPHA" not in b and "Quiz 0/2 revealed" in b, b
b = p.key(b"r")
assert "SECRET_ALPHA" in b and "SECRET_BETA" not in b, b
b = p.key(b"r")
assert "SECRET_BETA" in b and "Quiz 2/2 revealed" in b, b
assert "all answers shown" in p.key(b"r")
b = p.key(b"*")
assert "SECRET_ALPHA" not in b and "SECRET_BETA" not in b and "Quiz 0/2 revealed" in b, b
b = p.key(b"*")
assert "SECRET_ALPHA" in b and "SECRET_BETA" in b and "Quiz 2/2 revealed" in b, b
b = p.key(b"=")
assert "fold=" not in b and "Path = page:folds.html" in b, b
p.key(b"n")
b = p.key(b"\x1b[D")
assert "[+] Answer" in b and "SECRET_ALPHA" not in b and "SECRET_BETA" not in b, b
p.key(b"*")
p.key(b"\x1b[C")
b = p.key(b"\x1b[D")
assert "SECRET_ALPHA" not in b and "SECRET_BETA" not in b, b
p.quit()

# Stable numbering across level filtering: the shared quiz stays fold 2.
p = Pager(["show", "layered.html"], state, env={"TAM_LEVEL": "all"})
p.key(b"1")
b = p.key(b"r")
assert "SHARED_ANSWER" in b and "EXPERT_ANSWER" not in b and "Quiz 1/1 revealed" in b, b
b = p.key(b"0")
assert "EXPERT_ANSWER" not in b, b
# The expert blocks push the shared answer below the initial viewport.
b = p.key(b"c")
assert "SHARED_ANSWER" in b and "EXPERT_ANSWER" not in b, b
p.quit()

# Standard-input HTML has the same folding behavior and can be reloaded.
p = Pager(["show", "-"], state, env={"TAM_LEVEL": "all"},
          stdin=b"<h1>Input quiz</h1><details class=\"q\"><summary>Input?</summary>INPUT_SECRET</details>")
assert "INPUT_SECRET" not in p.first, p.first
assert "INPUT_SECRET" in p.key(b"r")
p.quit()
' "$TAM" "$root/tests/fixtures/guide" "$root/tests/fixtures/tldr" "$tmp" || fail "interactive pager level lens test failed"
    pass "level lens: 0-3 persist across links, Back and restarts; TAM_LEVEL and --level override; level line, markers and status row"
fi


# TCK22 pipe output: answers follow the questions and are collected per
# section; block filtering must also hide answers from filtered details.
python3 - "$TAM" "$root/tests/fixtures/guide" <<'PYFOLD'
import os, subprocess, sys
tam, guide = sys.argv[1:]
def show(page, *args):
    return subprocess.check_output([tam, "show", page, *args], text=True,
        env={**os.environ, "TAM_KNOWLEDGE_DIR": guide})
s = show("folds.html", "--pipe")
assert s.index("Question beta?") < s.index("Answers") < s.index("SECRET_ALPHA") < s.index("SECRET_BETA"), s
assert "1. Question alpha?" in s and "1. Answer" in s and "2. Model answer" in s, s
assert "tam:fold:" not in s and "(Enter)" not in s, s
assert show("folds.html") == s, "non-terminal show did not use pipe folding"
s = show("layered.html", "--pipe", "--level", "1")
assert "SHARED_CONTENT" in s and "SHARED_ITEM" in s and "SHARED_ANSWER" in s, s
assert "EXPERT_" not in s and "NESTED_DIV" not in s and "COMMENT_" not in s, s
assert "2. SHARED_QUESTION" in s and "2. Answer" in s, s
s = show("layered.html", "--pipe", "--level", "3")
for block in ("ASIDE", "PARAGRAPH", "ITEM", "PRE", "QUESTION", "ANSWER", "DIV", "TABLE"):
    assert "EXPERT_" + block in s, s
assert "NESTED_DIV" in s and "SHARED_ANSWER" in s, s
PYFOLD
pass "folds: pipe answers collected, non-terminal show, stable IDs and block-level filtering"

# Explain command: curated topics resolve, bare explain lists topics, and errors report cleanly.
ex_out=$("$TAM" explain pipelines --pipe)
printf '%s\n' "$ex_out" | grep -q 'Shell pipelines' || fail "explain pipelines failed"
ex_pipe=$("$TAM" explain '>' --pipe)
printf '%s\n' "$ex_pipe" | grep -Fq 'shell/pipelines.html' || fail "explain '>' failed"
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
printf '%s\n' "$ex_lua" | grep -q 'Levels: beginner' || fail "explain lua missing levels line"

# Maintain track curated topics.
ex_maint=$("$TAM" explain maintain --pipe)
printf '%s\n' "$ex_maint" | grep -q 'System Maintenance and Reliability' || fail "explain maintain failed"
ex_upd=$("$TAM" explain updates --pipe)
[ -n "$ex_upd" ] || fail "explain updates produced no output"
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
printf '%s\n' "$ex_kw" | grep -q 'Keyword Index' || fail "bare explain missing Keyword Index"
printf '%s\n' "$ex_kw" | grep -q 'Keyword  *Pages  *Covers' || fail "bare explain missing table heading"
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
printf '%s\n' "$ex_topics" | grep -q 'Topics A to Z:' || fail "explain topics missing heading"
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
fixture_show() {
    env TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show "$@"
}
sh_lvl_b=$(fixture_show --level beginner pipelines --pipe)
printf '%s\n' "$sh_lvl_b" | grep -q '^Beginner' || fail "show --level beginner pipelines failed"
printf '%s\n' "$sh_lvl_b" | grep -q 'Use a pipe to count lines' || fail "show --level beginner missing fixture content"
if printf '%s\n' "$sh_lvl_b" | grep -q '^Expert'; then fail "beginner level contains expert section"; fi
printf '%s\n' "$sh_lvl_b" | grep -q '^-- 3 Expert: Expert' || fail "beginner level missing hidden-section marker"

sh_lvl_i=$(fixture_show --level intermediate pipelines --pipe)
printf '%s\n' "$sh_lvl_i" | grep -q '^Intermediate' || fail "show --level intermediate pipelines failed"
printf '%s\n' "$sh_lvl_i" | grep -q 'Redirection of standard error' || fail "show --level intermediate missing fixture content"
if printf '%s\n' "$sh_lvl_i" | grep -q '^Beginner'; then fail "intermediate level contains beginner section"; fi

sh_lvl_e=$(fixture_show --level expert pipelines --pipe)
printf '%s\n' "$sh_lvl_e" | grep -q '^Expert' || fail "show --level expert pipelines failed"
printf '%s\n' "$sh_lvl_e" | grep -q 'Every redirection is applied' || fail "show --level expert missing fixture content"
if printf '%s\n' "$sh_lvl_e" | grep -q '^Beginner'; then fail "expert level contains beginner section"; fi

# A page without the requested level opens under its level line, exit 0.
sh_banner=$(TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show --level 2 welcome --pipe) \
    || fail "show --level 2 on a single-level page failed"
printf '%s\n' "$sh_banner" | grep -qE '^ +Beginner text on Intermediate path \(no Intermediate text\)$' \
    || fail "single-level page missing its right-aligned level line"
printf '%s\n' "$sh_banner" | grep -q 'Fixture Welcome' || fail "level-line page lost its content"
# Related pages come from parent links: a page with no relative at the level
# shows only the level path; siblings, then the parent, are offered next.
if printf '%s\n' "$sh_banner" | grep -q 'Intermediate text:'; then
    fail "level line without relatives at the level must show only the level path"
fi
sh_sib=$(TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show --level 3 learn/index --pipe) \
    || fail "show --level 3 on the learning hub failed"
printf '%s\n' "$sh_sib" | grep -q 'Expert text: Fixture pipelines' \
    || fail "relatives line did not link a sibling at the level"
if printf '%s\n' "$sh_sib" | grep 'Expert text:' | grep -q 'Welcome'; then
    fail "relatives line linked a child without the level"
fi
# Exercise the ranking contract independently of live page titles and tracks.
python3 - "$TAM" "$tmp/banner-ranking" <<'PY' || fail "banner parent ranking failed"
import csv
import os
from pathlib import Path
import subprocess
import sys

tam, corpus = sys.argv[1], Path(sys.argv[2])
corpus.mkdir()
(corpus / "index.html").write_text(
    '<html><head><meta name="level" content="beginner"></head>'
    '<body><h1>Banner fixture</h1><section data-level="beginner">'
    '<h2>Beginner</h2><p>Read this page.</p></section></body></html>'
)
header = "topic path title summary keywords level track prereqs rev updated".split()

def entry(topic, title, parent, prereqs="", level="intermediate", track="other"):
    path = "index.html" if topic == "current" else topic + ".html"
    return [topic, path, title, "Summary", topic, level, track, prereqs,
            "1", "2026-10-21 06:00"], parent

current = entry("current", "Current", "parent.html", "shell, pipes", "beginner", "use")
parent = entry("parent", "Parent", "root.html", "shell, pipes", track="use")
sibling = entry("sibling", "Sibling", "parent.html", "shell, pipes", track="use")
child = entry("child", "Child", "index.html")
# A stronger prerequisite match in the same track is still unrelated.
unrelated = entry("unrelated", "Unrelated", "elsewhere.html", "shell, pipes", track="use")
wrong_level = entry("wrong-level", "Wrong level", "index.html", level="expert")

def banner(entries, parents=True):
    rows = [current, unrelated, wrong_level, *entries]
    with (corpus / "index.tsv").open("w") as f:
        writer = csv.writer(f, delimiter="\t", lineterminator="\n")
        writer.writerow(header)
        writer.writerows(row for row, _ in rows)
    sidecar = corpus / "index.parents.tsv"
    with sidecar.open("w") as f:
        writer = csv.writer(f, delimiter="\t", lineterminator="\n")
        writer.writerow(["path", "parent"])
        writer.writerows((row[1], p) for row, p in rows)
    if not parents:
        sidecar.unlink()
    out = subprocess.check_output(
        [tam, "show", "--level", "2", "current", "--pipe"], text=True,
        env={**os.environ, "TAM_KNOWLEDGE_DIR": str(corpus)},
    )
    return next((line.strip() for line in out.splitlines()
                 if line.strip().startswith("Intermediate text:")), None)

assert banner([parent, sibling, child]) == "Intermediate text: Child | Sibling | Parent"
assert banner([parent, sibling]) == "Intermediate text: Sibling | Parent"
assert banner([parent]) == "Intermediate text: Parent"
# Ties use shared prerequisites, then title, and stop at three links.
children = [entry("zulu", "Zulu", "index.html", "shell"),
            entry("alpha", "Alpha", "index.html"),
            entry("beta", "Beta", "index.html", "shell"),
            entry("delta", "Delta", "index.html", "shell, pipes")]
assert banner([parent, sibling, *children]) == "Intermediate text: Delta | Beta | Zulu"
assert banner([]) is None
assert banner([parent, sibling, child], parents=False) is None
PY
pass "level-line relatives: children, siblings, parent, tie breaks, cap and fallback"
# Piped output ignores the saved lens.
mkdir -p "$tmp/pipe-lens/tamlinux/tam"
printf 'expert\n' > "$tmp/pipe-lens/tamlinux/tam/level"
[ "$(XDG_STATE_HOME="$tmp/pipe-lens" fixture_show pipelines --pipe)" = "$(fixture_show pipelines --pipe)" ] \
    || fail "piped output followed the saved lens"

# Numeric and 'all' level synonyms (0|all, 1|beginner, 2|intermediate, 3|expert)
sh_lvl_all=$(fixture_show pipelines --pipe)
[ "$(fixture_show --level 1 pipelines --pipe)" = "$sh_lvl_b" ] || fail "show --level 1 differs from beginner"
[ "$(fixture_show --level 2 pipelines --pipe)" = "$sh_lvl_i" ] || fail "show --level 2 differs from intermediate"
[ "$(fixture_show --level 3 pipelines --pipe)" = "$sh_lvl_e" ] || fail "show --level 3 differs from expert"
[ "$(fixture_show --level 0 pipelines --pipe)" = "$sh_lvl_all" ] || fail "show --level 0 differs from all levels"
[ "$(fixture_show --level all pipelines --pipe)" = "$sh_lvl_all" ] || fail "show --level all differs from all levels"
[ "$(fixture_show "pipelines?level=1" --pipe)" = "$sh_lvl_b" ] || fail "show pipelines?level=1 differs from beginner"
[ "$(fixture_show "pipelines?level=2" --pipe)" = "$sh_lvl_i" ] || fail "show pipelines?level=2 differs from intermediate"
[ "$(fixture_show "pipelines?level=3" --pipe)" = "$sh_lvl_e" ] || fail "show pipelines?level=3 differs from expert"
[ "$(fixture_show "pipelines?level=0" --pipe)" = "$sh_lvl_all" ] || fail "show pipelines?level=0 differs from all levels"
[ "$(fixture_show "pipelines?level=all" --pipe)" = "$sh_lvl_all" ] || fail "show pipelines?level=all differs from all levels"

[ "$("$TAM" explain --level 1 pipelines --pipe)" = "$ex_tg_lvl_b" ] || fail "explain --level 1 pipelines differs from beginner"
[ "$("$TAM" explain --level 0 pipelines --pipe)" = "$ex_out" ] || fail "explain --level 0 pipelines differs from all levels"
[ "$("$TAM" explain --level all pipelines --pipe)" = "$ex_out" ] || fail "explain --level all pipelines differs from all levels"
[ "$("$TAM" explain topics --level 3 --terse)" = "$ex_expert_terse" ] || fail "explain topics --level 3 --terse differs from expert"
[ "$("$TAM" explain topics --level 0 --terse)" = "$ex_terse" ] || fail "explain topics --level 0 --terse differs from all levels"
[ "$("$TAM" explain topics --level all --terse)" = "$ex_terse" ] || fail "explain topics --level all --terse differs from all levels"

# Catalog listing filtered by level
ex_cat_expert=$("$TAM" explain --level expert)
printf '%s\n' "$ex_cat_expert" | grep 'Keyword Index (expert)' >/dev/null || fail "explain --level expert missing heading"
printf '%s\n' "$ex_cat_expert" | grep 'pipelines' >/dev/null || fail "expert catalog missing pipelines"
printf '%s\n' "$ex_cat_expert" | grep 'jobs' >/dev/null || fail "expert catalog missing jobs"
if printf '%s\n' "$ex_cat_expert" | grep '^  desktop ' >/dev/null; then fail "expert catalog unexpectedly includes beginner-only desktop"; fi

# Invalid level argument produces exit code 2 (usage error)
if "$TAM" explain --level invalid pipelines >/dev/null 2>&1; then fail "explain --level invalid exited 0"; fi
"$TAM" explain --level invalid pipelines 2>&1 | grep -q "unknown level 'invalid'" || fail "missing unknown level message"

# A topic lacking the requested level lists every level under its level line.
ex_no_exp=$("$TAM" explain --level expert desktop --pipe) || fail "explain --level expert desktop failed"
printf '%s\n' "$ex_no_exp" | grep -q "All text on Expert path (no Expert text)" || fail "missing topic level line"

pass "explain resolves curated topics, handles misses, and treats shell strings as data"

# Search command: ranked search over corpus index, json output, canonical queries, and miss banner.
"$TAM" search >/dev/null || fail "bare search failed"
"$TAM" search --help | grep -q 'Usage:' || fail "search --help missing usage"

# Canonical query: "change file owner" -> permissions page
s_owner=$("$TAM" search "change file owner")
printf '%s\n' "$s_owner" | grep -q 'permissions' || fail "search 'change file owner' missing permissions topic"
printf '%s\n' "$s_owner" | grep -q 'filesystem/permissions.html' || fail "search 'change file owner' missing permissions page"
printf '%s\n' "$s_owner" | grep -q 'Levels:' || fail "search missing Levels indicator"
if printf '%s\n' "$s_owner" | grep -q 'Depth'; then fail "search still says Depth"; fi
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
iso "$TAM" inspect | grep -q 'Tamlinux cultivar: 7.7.7' || fail "inspect missing cultivar version from file"
printf 'Incipiens\n' > "$config/tamlinux/cultivar"
iso "$TAM" inspect | grep -q 'Tamlinux cultivar: Incipiens 7.7.7' || fail "inspect missing cultivar name"
iso "$TAM" inspect --json | grep -q '"tamlinux_cultivar": "Incipiens"' || fail "inspect --json missing tamlinux_cultivar"
iso "$TAM" inspect --json | grep -q '"tamlinux_version": "7.7.7"' || fail "inspect --json missing tamlinux_version"
iso "$TAM" inspect | grep -q 'tamlinux_version: Tamlinux 7.7.7 is deployed for' || fail "inspect missing deployment finding"
rm -f "$config/tamlinux/version" "$config/tamlinux/cultivar"
if [ "$(id -u)" -ne 0 ]; then
    iso "$TAM" inspect | grep -q 'Tamlinux cultivar: not deployed for .* (per-user install)' \
        || fail "isolated inspect missing not-deployed row"
    iso "$TAM" inspect | grep -q '\[info\] tamlinux_version: Tamlinux is not deployed for' \
        || fail "isolated inspect missing not-deployed finding"
    iso "$TAM" inspect --json | grep -q '"tamlinux_version": null' || fail "inspect --json version should be null"
fi
env HOME="$home" XDG_CONFIG_HOME="$config" XDG_STATE_HOME="$state" \
    TAMLINUX_VERSION=8.8.8 "$TAM" inspect | grep -q '8.8.8' || fail "inspect ignored cultivar env"

# Operating system identity from fixture os-release files (TAM_INSPECT_ROOT).
os_fixtures="$root/tests/fixtures/os-release"
arch_out=$(env TAM_INSPECT_ROOT="$os_fixtures/arch" "$TAM" inspect)
printf '%s\n' "$arch_out" | grep -q 'Operating system:  Arch Linux (rolling)$' || fail "inspect missing Arch Linux (rolling)"
if printf '%s\n' "$arch_out" | grep -q 'OS version:'; then fail "OS version row shown without VERSION_ID"; fi
printf '%s\n' "$arch_out" | grep -q '\[info\] os_identity: os-release names the installed distribution' \
    || fail "inspect missing passing os_identity"
over_out=$(env TAM_INSPECT_ROOT="$os_fixtures/override" "$TAM" inspect)
printf '%s\n' "$over_out" | grep -q 'Operating system:  Overlay (arch-like)$' || fail "inspect missing override name"
printf '%s\n' "$over_out" | grep -q 'OS version:        1.2.3' || fail "inspect missing OS version row"
printf '%s\n' "$over_out" | grep -q 'ID=overlay over /usr/lib/os-release ID=arch' || fail "inspect missing os_identity override evidence"
env TAM_INSPECT_ROOT="$os_fixtures/override" "$TAM" check os_identity | grep -q '"status": "fail"' \
    || fail "check os_identity should fail on an override"
pass "inspect operating system identity"

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
man_landing=$("$TAM" man)
printf '%s\n' "$man_landing" | grep -q '^Guide > Man pages > Manual lookup$' || fail "bare man missing landing location"
printf '%s\n' "$man_landing" | grep -q 'Open a page' || fail "bare man missing open-page action"
printf '%s\n' "$man_landing" | grep -q 'Find by keyword' || fail "bare man missing keyword action"
printf '%s\n' "$man_landing" | grep -q 'Browse a section' || fail "bare man missing section action"
"$TAM" man --help | grep -q 'Usage:' || fail "man --help missing usage"
"$TAM" man help | grep -q 'Usage:' || fail "man help missing usage"
[ "$("$TAM" man help)" = "$("$TAM" man --help)" ] || fail "man help differs from man --help"
manual_uri=$("$TAM" show tam:man --pipe)
printf '%s\n' "$manual_uri" | grep -q 'Find by keyword' || fail "tam:man URI did not open manual discovery"
search_uri=$("$TAM" show 'tam:search?query=pipe' --pipe)
printf '%s\n' "$search_uri" | grep -q 'Results for pipe' || fail "tam:search URI did not return Guide results"

# Deterministic manual lookup helper: argv remains literal, section browsing
# is paged, and missing tools/no matches have different recoverable messages.
lookup_bin="$tmp/lookup-bin"
mkdir -p "$lookup_bin"
cat > "$lookup_bin/apropos" <<'EOF'
#!/bin/sh
printf '<%s>\n' "$@" >> "$TAM_TEST_APROPOS_LOG"
if [ "$1" = "-e" ]; then
    printf '%s\n' 'printf (1) - format and print data' 'printf (3) - formatted output conversion'
    exit 0
fi
if [ "$1" = "-s" ]; then
    section=$2
    i=0
    while [ "$i" -lt 25 ]; do
        printf 'demo%02d (%s) - section browser fixture\n' "$i" "$section"
        i=$((i + 1))
    done
    exit 0
fi
if [ "$2" = "flood" ]; then
    i=0
    while [ "$i" -lt 5000 ]; do
        printf 'flood%04d (1) - bounded output fixture\n' "$i"
        i=$((i + 1))
    done
    exit 0
fi
if [ "$2" = "printf" ]; then
    printf '%s\n' 'printf (1) - format and print data'
    exit 0
fi
exit 16
EOF
chmod +x "$lookup_bin/apropos"
export TAM_TEST_APROPOS_LOG="$tmp/apropos-argv.log"
lookup_keyword=$(PATH="$lookup_bin:$PATH" "$TAM" show 'tam:man:keyword?query=printf' --pipe)
printf '%s\n' "$lookup_keyword" | grep -q 'printf(1)' || fail "manual keyword lookup missing result"
lookup_section=$(PATH="$lookup_bin:$PATH" "$TAM" show 'tam:man:section?section=3' --pipe)
printf '%s\n' "$lookup_section" | grep -q 'demo23(3)' || fail "section page missing result"
printf '%s\n' "$lookup_section" | grep -q 'offset=24' || fail "section results missing pagination"
lookup_open=$(PATH="$lookup_bin:$PATH" "$TAM" show 'tam:man:open?query=printf' --pipe)
printf '%s\n' "$lookup_open" | grep -q 'Several installed sections match' || fail "ambiguous manual sections were not listed"
lookup_flood=$(PATH="$lookup_bin:$PATH" "$TAM" show 'tam:man:keyword?query=flood' --pipe)
printf '%s\n' "$lookup_flood" | grep -q 'Output reached the safe lookup limit' || fail "manual lookup output limit was not reported"
hostile_marker="$tmp/tam-lookup-injection-marker"
hostile_uri="tam:man:keyword?query=%24%28touch%20$hostile_marker%29"
: > "$TAM_TEST_APROPOS_LOG"
lookup_miss=$(PATH="$lookup_bin:$PATH" "$TAM" show "$hostile_uri" --pipe)
printf '%s\n' "$lookup_miss" | grep -q 'No manual entries matched this keyword' || fail "manual keyword no-hit message missing"
[ ! -e "$hostile_marker" ] || fail "manual lookup executed hostile query text"
grep -Fq 'tam-lookup-injection-marker' "$TAM_TEST_APROPOS_LOG" \
    || { cat "$TAM_TEST_APROPOS_LOG" >&2; fail "manual helper did not receive hostile query as one literal argv element"; }
[ "$(wc -l < "$TAM_TEST_APROPOS_LOG")" -eq 2 ] \
    || fail "manual helper received split query arguments"
missing_apropos=$(PATH="$tmp/no-apropos" "$TAM" show 'tam:man:keyword?query=printf' --pipe)
printf '%s\n' "$missing_apropos" | grep -q 'apropos tool is unavailable' \
    || fail "missing apropos tool message missing"
cat > "$lookup_bin/man" <<'EOF'
#!/bin/sh
[ "$1" = "-w" ] || exit 1
printf '%s\n' "$TAM_TEST_MAN_PAGE"
EOF
cat > "$lookup_bin/groff" <<'EOF'
#!/bin/sh
printf '%s\n' '<html><head><title>Fixture printf(1)</title></head><body><h1>Fixture printf(1)</h1><p>Loaded fixture manual.</p></body></html>'
EOF
chmod +x "$lookup_bin/man" "$lookup_bin/groff"
printf 'fixture\n' > "$tmp/printf.1"
export TAM_TEST_MAN_PAGE="$tmp/printf.1"

python3 -c '
import fcntl, os, pty, select, struct, sys, termios, time

binary, helper, corpus = sys.argv[1:]
def launch(args):
    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 120, 0, 0))
    pid = os.fork()
    if pid == 0:
        os.close(master)
        os.dup2(slave, 0); os.dup2(slave, 1); os.dup2(slave, 2); os.close(slave)
        env = os.environ.copy()
        env["TAM_KNOWLEDGE_DIR"] = corpus
        env["PATH"] = helper + os.pathsep + env.get("PATH", "")
        os.execve(binary, [binary] + args, env)
    os.close(slave)
    return pid, master

def drain(fd, delay=0.15):
    time.sleep(delay)
    data = b""
    while select.select([fd], [], [], 0.02)[0]: data += os.read(fd, 8192)
    return data

def parse_info(buf):
    lines = [line for line in buf.split(b"\x1b[24;1H") if b"|" in line]
    last = lines[-1].split(b"\x1b[K", 1)[0] if lines else b""
    title = last.split(b"|")[0].strip().decode("utf-8", "replace")
    target = last.split(b"->")[-1].split(b"\x1b", 1)[0].split(b"  ")[0].strip().decode("utf-8", "replace") if b"->" in last else ""
    return title, target

# The fixed breadcrumb row is keyboard reachable from the first document line.
pid, master = launch(["man"])
buf = drain(master)
title, target = parse_info(buf)
assert title == "Manual lookup" and target == "", (title, target)
os.write(master, b"\x1b[A")
title, target = parse_info(drain(master))
assert title == "Manual lookup" and target == "tam:man", f"Up did not focus the last breadcrumb: {target}"
os.write(master, b"a")
title, target = parse_info(drain(master))
assert target == "index.html", f"a did not move left across breadcrumbs: {target}"
os.write(master, b"f")
title, target = parse_info(drain(master))
assert target == "tam:man", f"f did not move right across breadcrumbs: {target}"
os.write(master, b"s")
title, target = parse_info(drain(master))
assert target == "index.html", f"s did not move to the previous breadcrumb: {target}"
os.write(master, b"d")
title, target = parse_info(drain(master))
assert target != "index.html" and target != "tam:man", f"d did not return from the breadcrumb to the page: {target}"
os.write(master, b"x\x1b[A")
title, target = parse_info(drain(master))
assert target == "tam:man", f"Up did not return from the document to the breadcrumb: {target}"
os.write(master, b"\x1b[Z")
title, target = parse_info(drain(master))
assert target == "index.html", f"Shift-Tab did not move to the previous breadcrumb: {target}"
os.write(master, b"\t")
title, target = parse_info(drain(master))
assert target == "tam:man", f"Tab did not move to the next breadcrumb: {target}"
os.write(master, b"a\r")
title, target = parse_info(drain(master, 0.3))
assert title == "Fixture Guide", f"Enter did not follow the Guide breadcrumb: {title}"
os.write(master, b"q"); os.waitpid(pid, 0); os.close(master)

pid, master = launch(["man"])
buf = drain(master)
os.write(master, b"\t\t\r")
buf += drain(master)
os.write(master, b"printf\r")
buf += drain(master, 0.3)
assert b"Several installed sections match" in buf, buf
os.write(master, b"q"); os.waitpid(pid, 0); os.close(master)

pid, master = launch(["show", "tam:search"])
buf = drain(master)
# First Tab is the Man pages utility link; the next is Start a Guide search.
os.write(master, b"\t\t\r")
buf += drain(master)
os.write(master, b"pipe\r")
buf += drain(master, 0.3)
assert b"Results for" in buf and b"Fixture pipelines" in buf, buf
os.write(master, b"q"); os.waitpid(pid, 0); os.close(master)

pid, master = launch(["show", "tam:man:keyword?query=printf"])
buf = drain(master)
os.write(master, b"\t\r")
buf += drain(master, 0.3)
assert b"Fixture printf(1)" in buf and b"Loaded fixture manual" in buf, buf
os.write(master, b"q"); os.waitpid(pid, 0); os.close(master)
' "$TAM" "$lookup_bin" "$root/tests/fixtures/guide" || fail "manual and Guide search prompts failed"
pass "manual discovery, breadcrumb keyboard focus, bounded literal helper calls, pagination, and Guide Search prompts"

# Pages installed on the host: a package build sandbox has no man pages or
# groff, so these checks run only where the pages can be rendered.
if command -v groff >/dev/null 2>&1 && man -w grep >/dev/null 2>&1 \
        && man -w 5 fstab >/dev/null 2>&1; then
    # View grep manual page: check content and hyperlinked cross references in SEE ALSO.
    man_grep=$("$TAM" man grep --pipe)
    printf '%s\n' "$man_grep" | grep -q '^Guide > Man pages > ' || fail "tam man pipe missing location trail"
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

# Man pages render the same from every compression format, through groff run
# without a shell, and leave no scratch files behind.
mkdir -p "$tmp/manz/man1" "$tmp/manz-run"
for z in gz bz2 xz zst plain; do
    cp "$root/man/tam.1" "$tmp/manz/man1/tamz$z.1"
done
gzip "$tmp/manz/man1/tamzgz.1"
command -v bzip2 >/dev/null && bzip2 "$tmp/manz/man1/tamzbz2.1"
command -v xz >/dev/null && xz "$tmp/manz/man1/tamzxz.1"
command -v zstd >/dev/null && zstd -q --rm "$tmp/manz/man1/tamzzst.1"
want=$(MANPATH="$tmp/manz" XDG_RUNTIME_DIR="$tmp/manz-run" "$TAM" man tamzplain --pipe | sed 1,3d)
printf '%s\n' "$want" | grep -q 'SYNOPSIS' || fail "plain man page did not render"
for z in gz bz2 xz zst; do
    [ -e "$tmp/manz/man1/tamz$z.1.$z" ] || continue
    got=$(MANPATH="$tmp/manz" XDG_RUNTIME_DIR="$tmp/manz-run" "$TAM" man "tamz$z" --pipe | sed 1,3d)
    [ "$got" = "$want" ] || fail "$z-compressed man page renders differently"
done
leftover=$(find "$tmp/manz-run" -mindepth 3 2>/dev/null)
[ -z "$leftover" ] || fail "man rendering left scratch files: $leftover"
pass "man pages render alike from gzip, bzip2, xz and zstd with no scratch files left"

# A rendered man page is cached, one file per source, reused while the source
# is unchanged and rendered again when it changes.
cache_dir="$tmp/manz-cache/tamlinux/tam/man"
first=$(MANPATH="$tmp/manz" XDG_CACHE_HOME="$tmp/manz-cache" "$TAM" man tamzplain --pipe)
[ "$(find "$cache_dir" -name '*.html' | wc -l)" -eq 1 ] || fail "man cache did not store one page"
head -1 "$cache_dir"/*.html | grep -q '^<!-- tam-man-cache 1 ' || fail "man cache entry has no stamp"
second=$(MANPATH="$tmp/manz" XDG_CACHE_HOME="$tmp/manz-cache" "$TAM" man tamzplain --pipe)
[ "$first" = "$second" ] || fail "cached man page renders differently"
sed -i 's/^\.SH NAME$/.SH NAME\nCACHE-CHANGED/' "$tmp/manz/man1/tamzplain.1"
touch -d '+1 minute' "$tmp/manz/man1/tamzplain.1"
MANPATH="$tmp/manz" XDG_CACHE_HOME="$tmp/manz-cache" "$TAM" man tamzplain --pipe | grep -q 'CACHE-CHANGED' \
    || fail "a changed man page was served from the cache"
[ "$(find "$cache_dir" -name '*.html' | wc -l)" -eq 1 ] || fail "a changed man page left a second cache file"
pass "man page cache: one file per source, reused, refreshed when the source changes"

# One loader serves every address. "man:" alone (the man breadcrumb's link)
# opens the Man pages view; --level reaches every view that takes the lens;
# a guide address never leaves the guide.
"$TAM" show man: --pipe | grep -q 'Man pages' || fail "man: did not open the Man pages view"
TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show tam:keywords --level 3 --pipe \
    | grep -q 'Keyword Index (expert)' || fail "--level did not reach tam:keywords"
TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show "tam:explain?topic=pipelines" --level 3 --pipe \
    | grep -q 'Topic Guide: pipelines (expert Level)' || fail "--level did not reach tam:explain"
for bad in "page:../run.sh" "page:shell/../../run.sh" "../run.sh"; do
    if TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show "$bad" --pipe >/dev/null 2>&1; then
        fail "guide address $bad left the guide"
    fi
done
pass "one address loader: man: view, --level on views, guide paths stay inside"

# One tag scanner reads attributes as name=value pairs: text inside another
# value, or a longer name such as data-href, never counts; <abbr> in the
# breadcrumb is not a link.
scan_out=$(printf '%s' '<html><head><title>T</title></head><body><nav aria-label="Breadcrumb"><p><abbr title="x">G</abbr> <a href="index.html">Guide</a> / Here</p></nav><h1>T</h1><p><a data-href="wrong.html" href="right.html">one</a> and <a title="href=bad.html" href="good.html">two</a>.</p></body></html>' \
    | TAM_KNOWLEDGE_DIR="$root/tests/fixtures/guide" "$TAM" show - --pipe)
printf '%s\n' "$scan_out" | head -1 | grep -qx 'Guide > Here' || fail "breadcrumb misread around <abbr>: $scan_out"
printf '%s\n' "$scan_out" | grep -q 'right.html' || fail "href lost beside data-href: $scan_out"
printf '%s\n' "$scan_out" | grep -q 'good.html' || fail "href lost after a title holding href=: $scan_out"
if printf '%s\n' "$scan_out" | grep -q 'wrong.html\|bad.html'; then
    fail "attribute text read as a link target: $scan_out"
fi
pass "one tag scanner: attributes are name=value pairs, breadcrumb links are <a> only"

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
printf '%s\n' "$info_dir" | grep -q '^Guide > Info manuals$' || fail "tam info pipe missing location trail"
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
"$TAM" info --help | grep -q 'a/j f/;' || fail "tam info --help missing the home-row keys"

pass "tam info subcommand, node parsing, cross-references, and error handling"

# -----------------------------------------------------------------------------
# Native tldr Pages Subsystem (tam tldr)
# -----------------------------------------------------------------------------
tldr_fix="$here/fixtures/tldr"
guide_fix="$here/fixtures/guide"

[ -x "$root/bin/tam-tldr" ] || fail "bin/tam-tldr is missing or not executable"
tldr_wrap=$(TAM_TLDR_DIR="$tldr_fix" "$root/bin/tam-tldr" tar --pipe)
printf '%s\n' "$tldr_wrap" | grep -q 'Archiving utility' || fail "bin/tam-tldr did not delegate to tam tldr"

tldr_landing=$(TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr --pipe)
printf '%s\n' "$tldr_landing" | grep -q '^Guide > tldr pages$' || fail "tam tldr landing missing location trail"
printf '%s\n' "$tldr_landing" | grep -q 'tam:prompt:tldr-open' || fail "tam tldr landing missing open prompt link"
printf '%s\n' "$tldr_landing" | grep -q 'tam:prompt:tldr-keyword' || fail "tam tldr landing missing keyword prompt link"
printf '%s\n' "$tldr_landing" | grep -q 'tam:tldr:list' || fail "tam tldr landing missing list link"

tldr_tar=$(TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr tar --pipe)
printf '%s\n' "$tldr_tar" | grep -q '^Guide > tldr pages > tar$' || fail "tam tldr tar missing location trail"
printf '%s\n' "$tldr_tar" | grep -Fq '[-C|--directory]' || fail "tam tldr tar missing both-form option [-C|--directory]"
printf '%s\n' "$tldr_tar" | grep -q 'tldr:gzip' || fail "tam tldr tar missing See also link to tldr:gzip"
printf '%s\n' "$tldr_tar" | grep -q 'tam:tldr' || fail "tam tldr tar missing link back to tam:tldr"
printf '%s\n' "$tldr_tar" | grep -q 'index.html' || fail "tam tldr tar missing link back to Guide index.html"

tldr_gc=$(TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr git commit --pipe)
printf '%s\n' "$tldr_gc" | grep -q '^Guide > tldr pages > git-commit$' || fail "multi-word tam tldr git commit failed"
printf '%s\n' "$tldr_gc" | grep -Fq '[-a|--all]' || fail "git-commit missing [-a|--all]"
printf '%s\n' "$tldr_gc" | grep -Fq 'stash@{0}' || fail "git-commit nested braces {{stash@{0}}} failed"
printf '%s\n' "$tldr_gc" | grep -Fq '{{literal}}' || fail "git-commit escaped braces \{\{literal\}\} failed"

tldr_cat_l=$(TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr cat --pipe)
printf '%s\n' "$tldr_cat_l" | grep -Fq 'cat (linux)' || fail "tam tldr cat did not prefer linux section"
printf '%s\n' "$tldr_cat_l" | grep -Fq 'tldr:common/cat' || fail "tam tldr cat missing alternate platform link"

tldr_cat_c=$(TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr --platform common cat --pipe)
printf '%s\n' "$tldr_cat_c" | grep -Fq 'cat (common)' || fail "tam tldr --platform common cat failed"
printf '%s\n' "$tldr_cat_c" | grep -Fq 'tldr:linux/cat' || fail "tam tldr common/cat missing linux/cat link"

tldr_alias=$(TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr ll --pipe)
printf '%s\n' "$tldr_alias" | grep -q 'tldr:ls' || fail "tam tldr alias page ll did not link to tldr:ls"

tldr_list=$(TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr --list --pipe)
printf '%s\n' "$tldr_list" | grep -q '^Guide > tldr pages > tldr index$' || fail "tam tldr --list missing location trail"
printf '%s\n' "$tldr_list" | grep -q 'tldr:common/tar' || fail "tam tldr --list missing tar entry"

tldr_kw=$(TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr --search Archiving --pipe)
printf '%s\n' "$tldr_kw" | grep -q 'tldr:common/tar' || fail "tam tldr --search Archiving did not find tar"

show_tldr=$(TAM_KNOWLEDGE_DIR="$guide_fix" TAM_TLDR_DIR="$tldr_fix" "$TAM" show tldr:tar --pipe)
printf '%s\n' "$show_tldr" | grep -q '^Guide > tldr pages > tar$' || fail "tam show tldr:tar failed"

show_tldr_idx=$(TAM_KNOWLEDGE_DIR="$guide_fix" TAM_TLDR_DIR="$tldr_fix" "$TAM" show tam:tldr --pipe)
printf '%s\n' "$show_tldr_idx" | grep -q '^Guide > tldr pages$' || fail "tam show tam:tldr failed"

"$TAM" tldr --help | grep -q 'tam tldr - view concise command-example pages' || fail "tam tldr --help missing title"
set +e
TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr no-such-tldr-cmd >/dev/null 2>&1
rc_tldr_miss=$?
TAM_TLDR_DIR="$tldr_fix" "$TAM" tldr --bad-flag >/dev/null 2>&1
rc_tldr_flag=$?
set -e
[ "$rc_tldr_miss" -eq 1 ] || fail "expected exit 1 on missing tldr page, got $rc_tldr_miss"
[ "$rc_tldr_flag" -eq 2 ] || fail "expected exit 2 on invalid tldr flag, got $rc_tldr_flag"

pass "tam tldr subcommand, index discovery, option formatting, and hyperlinks"

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
printf '%s\n' "$bash_test" | grep -q 'tldr' || fail "bash completion missing tldr subcommand"
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

bash_tldr_test=$(bash -c '
source "'"$root"'/completions/bash/tam"
COMP_WORDS=(tam tldr "")
COMP_CWORD=2
_tam_completions
printf "%s\n" "${COMPREPLY[@]}"
')
printf '%s\n' "$bash_tldr_test" | grep -q 'tar' || fail "bash completion missing tldr tar"
printf '%s\n' "$bash_tldr_test" | grep -q -- '--list' || fail "bash completion missing tldr --list"
printf '%s\n' "$bash_tldr_test" | grep -q -- '--pipe' || fail "bash completion missing tldr --pipe"

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
if [ "${TAM_TEST_SKIP_OLLAMA:-0}" = 1 ]; then
    pass "tam ask live-server checks explicitly skipped for source validation"
elif python3 -c 'import socket, sys; s = socket.socket(); s.settimeout(1); sys.exit(s.connect_ex(("127.0.0.1", 11434)))' 2>/dev/null; then
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
