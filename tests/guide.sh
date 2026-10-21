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

fail() { printf 'FAIL: %s\n' "$1" >&2; exit 1; }
pass() { printf 'ok - %s\n' "$1"; }

esc=$(printf '\033')
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

guide() {
    env TAM_KNOWLEDGE_DIR="$knowledge" "$TAM" show "$@"
}

# Pages that must resolve and render.
# Every page in the corpus, discovered rather than listed, so adding a page
# needs no change here. Renderer goldens live in tests/render.sh.
pages=$(cd "$knowledge" && find . -name '*.html' | sed 's#^\./##; s#\.html$##' | LC_ALL=C sort)

# Every page renders non-empty text with a title and build-time Git revision stamp.
for p in $pages; do
    out=$(guide --pipe "$p")
    [ -n "$out" ] || fail "page $p produced no text"
    printf '%s\n' "$out" | grep -Eq '^Rev [1-9][0-9]* Last updated [0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}$' \
        || fail "page $p missing Rev <N> Last updated <YYYY-MM-DD HH:MM> stamp"
done
pass "all guide pages render to text with build-time revision and timestamp"

# The index shows its own title and offers a link appendix. Order and wording
# are the author's; only the required destinations are fixed.
idx=$(guide --pipe)
idx_title=$(sed -n 's:.*<h1[^>]*>\(.*\)</h1>.*:\1:p' "$knowledge/index.html" | sed 's/<[^>]*>//g' | head -n 1)
[ -n "$idx_title" ] || fail "index.html has no <h1> heading"
printf '%s\n' "$idx" | grep -Fq "$idx_title" || fail "index missing its heading '$idx_title'"
printf '%s\n' "$idx" | grep -q '^Links:' || fail "index missing link appendix"
for entry in 'install/index.html' 'learn/index.html' 'use/index.html' \
             'maintain/index.html' 'improve/index.html' 'tam:prompt:guide-search' \
             'tam:prompt:page-search' 'tam:help' \
             'tam:topics' 'tam:man' 'info:dir' 'tam:tldr'; do
    printf '%s\n' "$idx" | grep -Fq "$entry" || fail "index missing $entry"
done
pass "index text includes links appendix"

# The five-part migration must keep the requested search vocabulary attached
# to useful, specific destinations rather than merely returning a hit.
search_destinations() {
    query=$1
    expected=$2
    out=$(env TAM_KNOWLEDGE_DIR="$knowledge" "$TAM" search "$query" --pipe)
    printf '%s\n' "$out" | grep -Fq "($expected)" \
        || fail "search '$query' missing destination $expected"
}
search_destinations install install/index.html
search_destinations learning learn/index.html
search_destinations workspaces desktop/workspaces.html
search_destinations permissions filesystem/permissions.html
search_destinations snapper maintain/snapper.html
search_destinations limine-snapper-sync maintain/snapper.html
search_destinations tty ecosystem/ttys-and-virtual-consoles.html
search_destinations vt ecosystem/ttys-and-virtual-consoles.html
search_destinations 'boot recovery' maintain/boot-recovery.html
search_destinations packaging ecosystem/packaging.html
search_destinations man man/index.html
search_destinations info learn/documentation.html
pass "acceptance queries reach their expected guide destinations"

# Curated topic lookup must reach the authored page. Read its actual title:
# teaching revisions may change wording without changing the command contract.
updates_title=$(sed -n 's:.*<h1[^>]*>\(.*\)</h1>.*:\1:p' "$knowledge/maintain/updates.html" | sed 's/<[^>]*>//g' | head -n 1)
[ -n "$updates_title" ] || fail "updates page has no heading"
updates=$(env TAM_KNOWLEDGE_DIR="$knowledge" "$TAM" explain updates --pipe)
printf '%s\n' "$updates" | grep -Fq "$updates_title" \
    || fail "explain updates did not reach the authored updates page"
pass "updates topic resolves without freezing its teaching-page title"

page_count=$(printf '%s\n' "$pages" | wc -l)
[ "$page_count" -ge 55 ] || fail "expected all 50 baseline and five new pages; found $page_count"
for p in learn/index.html use/index.html learn/tam.html \
         learn/documentation.html learn/practice.html; do
    [ -f "$knowledge/$p" ] || fail "missing required new page $p"
done
pass "all baseline pages and five required learning/navigation pages are present"

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
a=$(guide --pipe maintain)
b=$(guide --pipe maintain/index.html)
[ "$a" = "$b" ] || fail "maintain and maintain/index.html differ"
a=$(guide --pipe improve)
b=$(guide --pipe improve/index.html)
[ "$a" = "$b" ] || fail "improve and improve/index.html differ"
pass "extensionless page names resolve"

# Referenced links exist on disk. The corpus uses root-relative hrefs
# (resolved against the knowledge root, never the page's own directory).
broken=""
for p in $pages; do
    while IFS= read -r href; do
        [ -z "$href" ] && continue
        case $href in
            *://*|mailto:*|\#*|man:*|tam:*|info:*|tldr:*) continue ;;
        esac
        case $href in
            /*|*..*) broken="$broken $p->$href"; continue ;;
        esac
        href_path=${href%%\?*}
        [ -f "$knowledge/$href_path" ] || broken="$broken $p->$href"
    done <<EOF
$(grep -o 'href="[^"]*"' "$knowledge/$p.html" 2>/dev/null | sed 's/href="//; s/"$//')
EOF
done
[ -z "$broken" ] || fail "broken internal links:$broken"
pass "internal links resolve to files"

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
# XDG_DATA_DIRS and still shows pages. The Makefile finds libtam as it does
# for the binary under test, with pkg-config unless LIBTAM_PREFIX names it.
stage="$tmp/stage/usr"
if command -v make >/dev/null 2>&1; then
    make -C "$root" install DESTDIR="$tmp/stage" PREFIX=/usr BUILD="$tmp/build" \
            ${LIBTAM_PREFIX:+LIBTAM_PREFIX="$LIBTAM_PREFIX"} >"$tmp/stage.log" 2>&1 \
        || { cat "$tmp/stage.log" >&2; fail "staged install failed"; }
    [ -x "$stage/bin/tam" ] || fail "staged install produced no binary"
    out=$(env HOME="$tmp/nohome" XDG_DATA_HOME="$tmp/nodata" \
              XDG_DATA_DIRS="$stage/share" TAM_DATADIR="$tmp/no-datadir" \
              env -u TAM_KNOWLEDGE_DIR "$stage/bin/tam" show --pipe welcome)
    [ "$out" = "$(guide --pipe welcome)" ] \
        || fail "staged install did not render the current bundled corpus"
    pass "staged install relocates and finds the corpus"
else
    printf 'skip - staged install (make unavailable)\n'
fi

printf 'All tam guide tests passed.\n'
