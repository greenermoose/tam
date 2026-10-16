#!/bin/sh
# Renderer goldens: each fixture page in tests/render/ must render exactly
# as its .txt golden. UPDATE=1 rewrites the goldens after an intended
# rendering change; read the diff before committing.
set -eu

TAM=${1:-build/tam}
case $TAM in
    /*) ;;
    *) TAM=$(cd "$(dirname "$TAM")" && pwd)/$(basename "$TAM") ;;
esac

here=$(cd "$(dirname "$0")" && pwd)
fixtures="$here/render"

fail() { printf 'FAIL: %s\n' "$1" >&2; exit 1; }
pass() { printf 'ok - %s\n' "$1"; }

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

count=0
for page in "$fixtures"/*.html; do
    name=$(basename "$page" .html)
    golden="$fixtures/$name.txt"
    env TAM_KNOWLEDGE_DIR="$fixtures" "$TAM" show --pipe "$name" > "$tmp/out.txt"
    if [ "${UPDATE:-0}" = "1" ]; then
        cp "$tmp/out.txt" "$golden"
    elif [ ! -f "$golden" ]; then
        fail "no golden for $name; create it with UPDATE=1 and read it"
    elif ! diff -u "$golden" "$tmp/out.txt" > "$tmp/diff.txt"; then
        cat "$tmp/diff.txt" >&2
        fail "rendered text for $name differs from its golden"
    fi
    count=$((count + 1))
done
[ "$count" -gt 0 ] || fail "no fixture pages in $fixtures"
pass "render goldens match ($count fixtures)"
