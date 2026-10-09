#!/bin/sh
# Behavior checks for the tam command foundation (introduction, --help,
# --version, argument errors). None of these paths may write state.
set -eu

TAM=${1:-build/tam}
case $TAM in
    /*) ;;
    *) TAM=$(cd "$(dirname "$TAM")" && pwd)/$(basename "$TAM") ;;
esac

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
"$TAM" --version | grep -q '0.4.0' || fail "--version missing command version"
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
if "$TAM" explain pipelines --pipe | grep -q "$esc"; then fail "explain contains escape sequences"; fi
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

# Welcome isolation: bare tam, browse, help, version, explain NEVER write welcome state.
iso_state="$tmp/iso_state"
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" browse --pipe >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" --help >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" help >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" --version >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" explain pipelines --pipe >/dev/null
env HOME="$tmp/iso_home" XDG_STATE_HOME="$iso_state" "$TAM" explain nonexistent >/dev/null 2>&1 || true
if [ -e "$iso_state/tamlinux/welcome_seen" ]; then
    fail "isolated commands wrote welcome_seen state"
fi
pass "state isolation: bare, browse, help, version, explain do not touch welcome_seen"

# Explain command: curated topics resolve, missing topic and errors report cleanly.
ex_out=$("$TAM" explain pipelines --pipe)
printf '%s\n' "$ex_out" | grep -q 'Shell pipelines' || fail "explain pipelines failed"
ex_pipe=$("$TAM" explain '>' --pipe)
printf '%s\n' "$ex_pipe" | grep -q 'Redirection' || fail "explain '>' failed"
ex_man=$("$TAM" explain man --pipe)
printf '%s\n' "$ex_man" | grep -q 'Reading man pages' || fail "explain man failed"

if "$TAM" explain >/dev/null 2>&1; then fail "explain with no topic exited 0"; fi
"$TAM" explain 2>&1 | grep -qi 'missing topic' || fail "no message for missing topic"
if "$TAM" explain pipelines extra >/dev/null 2>&1; then fail "explain with extra args exited 0"; fi
"$TAM" explain pipelines extra 2>&1 | grep -qi 'too many' || fail "no message for extra args"

# Unmatched topic: clear diagnostic and reference to browse.
if "$TAM" explain no_such_topic >/dev/null 2>&1; then fail "unmatched topic exited 0"; fi
"$TAM" explain no_such_topic 2>&1 | grep -q "no explanation found for 'no_such_topic'" \
    || fail "missing clear miss diagnostic"
"$TAM" explain no_such_topic 2>&1 | grep -q "tam browse" \
    || fail "miss diagnostic does not point to tam browse"

# Safe input handling: arbitrary shell strings are treated strictly as data.
if "$TAM" explain 'cat foo | grep bar; rm -rf /' >/dev/null 2>&1; then
    fail "shell string topic unexpectedly exited 0"
fi
"$TAM" explain 'cat foo | grep bar; rm -rf /' 2>&1 | grep -q "no explanation found for 'cat foo | grep bar; rm -rf /'" \
    || fail "shell metacharacters not treated as plain query data"
pass "explain resolves curated topics, handles misses, and treats shell strings as data"

printf 'All tam command tests passed.\n'
