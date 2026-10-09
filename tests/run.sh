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
"$TAM" --version | grep -q '0.3.0' || fail "--version missing command version"
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
pass "piped output has no control sequences"

# Unknown argument is an error that points at --help.
if "$TAM" --nope >/dev/null 2>&1; then fail "unknown argument exited 0"; fi
"$TAM" --nope 2>&1 | grep -q 'help' || fail "error does not point to --help"
if "$TAM" --help extra >/dev/null 2>&1; then fail "extra arguments exited 0"; fi
"$TAM" --help extra 2>&1 | grep -qi 'too many' || fail "no message for extra arguments"
pass "argument errors"

printf 'All tam command tests passed.\n'
