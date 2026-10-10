#!/bin/sh
# Behavior checks for the offline guide: rendering to text, link graph,
# pipe safety, corpus resolution, and error handling.
set -eu

TAM=${1:-build/tam}
case $TAM in
    /*) ;;
    *) TAM=$(cd "$(dirname "$TAM")" && pwd)/$(basename "$TAM") ;;
esac

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/.." && pwd)
knowledge="$root/knowledge"
snapshots="$here/snapshots"

fail() { printf 'FAIL: %s\n' "$1" >&2; exit 1; }
pass() { printf 'ok - %s\n' "$1"; }

esc=$(printf '\033')
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

guide() {
    env TAM_KNOWLEDGE_DIR="$knowledge" "$TAM" show "$@"
}

# Pages that must resolve and render.
pages="index welcome install/index shell/index shell/pipelines shell/jobs filesystem/index filesystem/permissions desktop/index desktop/workspaces hardware/resources man/index"

# Every page renders non-empty text with a title.
for p in $pages; do
    out=$(guide --pipe "$p")
    [ -n "$out" ] || fail "page $p produced no text"
done
pass "all guide pages render to text"

# The index mentions its topics and offers a link appendix.
idx=$(guide --pipe)
printf '%s\n' "$idx" | grep -q 'Tamlinux guide' || fail "index missing title"
printf '%s\n' "$idx" | grep -q '^Links:' || fail "index missing link appendix"
printf '%s\n' "$idx" | grep -q 'welcome.html' || fail "index missing welcome link"
pass "index text includes links appendix"

# Piped output must be free of terminal control sequences.
for p in $pages; do
    if guide --pipe "$p" | grep -q "$esc"; then
        fail "page $p piped output contains escape sequences"
    fi
done
pass "piped guide output has no control sequences"

# Bare names (without .html) resolve to the same page.
a=$(guide --pipe welcome)
b=$(guide --pipe welcome.html)
[ "$a" = "$b" ] || fail "welcome and welcome.html differ"
a=$(guide --pipe install)
b=$(guide --pipe install/index.html)
[ "$a" = "$b" ] || fail "install and install/index.html differ"
a=$(guide --pipe shell)
b=$(guide --pipe shell/index.html)
[ "$a" = "$b" ] || fail "shell and shell/index.html differ"
a=$(guide --pipe desktop)
b=$(guide --pipe desktop/index.html)
[ "$a" = "$b" ] || fail "desktop and desktop/index.html differ"
a=$(guide --pipe filesystem)
b=$(guide --pipe filesystem/index.html)
[ "$a" = "$b" ] || fail "filesystem and filesystem/index.html differ"
pass "extensionless page names resolve"

# Referenced links exist on disk. The corpus uses root-relative hrefs
# (resolved against the knowledge root, never the page's own directory).
broken=""
for p in $pages; do
    for href in $(grep -o 'href="[^"]*"' "$knowledge/$p.html" 2>/dev/null \
                  | sed 's/href="//; s/"$//'); do
        case $href in
            *://*|mailto:*|\#*) continue ;;
        esac
        case $href in
            /*|*..*) broken="$broken $p->$href"; continue ;;
        esac
        [ -f "$knowledge/$href" ] || broken="$broken $p->$href"
    done
done
[ -z "$broken" ] || fail "broken internal links:$broken"
pass "internal links resolve to files"

# Golden text snapshots guard against unintended layout drift.
mkdir -p "$snapshots"
for p in $pages; do
    name=$(printf '%s' "$p" | tr '/' '-')
    snap="$snapshots/$name.txt"
    guide --pipe "$p" > "$tmp/out.txt"
    if [ "${UPDATE:-0}" = "1" ] || [ ! -f "$snap" ]; then
        cp "$tmp/out.txt" "$snap"
        continue
    fi
    if ! diff -u "$snap" "$tmp/out.txt" > "$tmp/diff.txt"; then
        cat "$tmp/diff.txt" >&2
        fail "rendered text for $p drifted from snapshot"
    fi
done
pass "golden text snapshots match"

# Missing page is a clear error.
if guide --pipe no-such-page >/dev/null 2>&1; then
    fail "missing page exited 0"
fi
guide --pipe no-such-page 2>&1 | grep -qi 'not found' \
    || fail "missing page lacks a clear message"
pass "missing page reports an error"

# With no corpus visible anywhere, the guide is reported as not installed.
if env HOME="$tmp/nohome" XDG_CONFIG_HOME="$tmp/nocfg" \
       XDG_DATA_HOME="$tmp/nodata" XDG_DATA_DIRS="$tmp/nodirs" \
       TAM_DATADIR="$tmp/nodatadir" \
       env -u TAM_KNOWLEDGE_DIR "$TAM" show --pipe >/dev/null 2>&1; then
    fail "show succeeded with no corpus installed"
fi
env HOME="$tmp/nohome" XDG_CONFIG_HOME="$tmp/nocfg" \
    XDG_DATA_HOME="$tmp/nodata" XDG_DATA_DIRS="$tmp/nodirs" \
    TAM_DATADIR="$tmp/nodatadir" \
    env -u TAM_KNOWLEDGE_DIR "$TAM" show --pipe 2>&1 \
    | grep -qi 'no offline guide' || fail "missing-corpus message unclear"
pass "absent corpus is reported clearly"

# The corpus installed with the binary wins over a user data directory, so a
# development install never renders another installation's pages.
mkdir -p "$tmp/otherdata/tamlinux/knowledge"
printf '<html><body><p>Other installation</p></body></html>\n' \
    > "$tmp/otherdata/tamlinux/knowledge/index.html"
out=$(env XDG_DATA_HOME="$tmp/otherdata" TAM_DATADIR="$knowledge" \
          env -u TAM_KNOWLEDGE_DIR "$TAM" show --pipe)
printf '%s\n' "$out" | grep -q 'Other installation' \
    && fail "user data directory shadowed the binary's own corpus"
pass "own corpus precedes user data directory"

# A staged install relocated away from the source tree finds its corpus via
# XDG_DATA_DIRS and still shows pages.
stage="$tmp/stage/usr"
if make -C "$root" install DESTDIR="$tmp/stage" PREFIX=/usr BUILD="$tmp/build" \
        LIBTAM_PREFIX="${LIBTAM_PREFIX:-/usr/local}" >/dev/null 2>&1; then
    [ -x "$stage/bin/tam" ] || fail "staged install produced no binary"
    out=$(env XDG_DATA_DIRS="$stage/share" TAM_DATADIR="$tmp/no-datadir" \
              env -u TAM_KNOWLEDGE_DIR "$stage/bin/tam" show --pipe welcome)
    printf '%s\n' "$out" | grep -q 'Tamlinux is an independent' \
        || fail "staged install did not find the bundled corpus"
    pass "staged install relocates and finds the corpus"
else
    printf 'skip - staged install (make unavailable)\n'
fi

printf 'All tam guide tests passed.\n'
