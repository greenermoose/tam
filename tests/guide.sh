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

# Every page renders non-empty text with a title and build-time Git revision stamp,
# and piped output must be free of terminal control sequences.
python3 -c '
import concurrent.futures, os, re, subprocess, sys

tam, knowledge = sys.argv[1], sys.argv[2]
rev_re = re.compile(r"^Rev [1-9][0-9]* Last updated [0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}$", re.MULTILINE)

pages = []
for dirpath, _, filenames in os.walk(knowledge):
    for f in filenames:
        if f.endswith(".html"):
            rel = os.path.relpath(os.path.join(dirpath, f), knowledge)[:-5]
            pages.append(rel)

def check(p):
    res = subprocess.run([tam, "show", "--pipe", p], env={**os.environ, "TAM_KNOWLEDGE_DIR": knowledge}, capture_output=True, text=True)
    if res.returncode != 0 or not res.stdout:
        sys.exit(f"page {p} produced no text")
    if not rev_re.search(res.stdout):
        sys.exit(f"page {p} missing Rev <N> Last updated <YYYY-MM-DD HH:MM> stamp")
    if "\x1b" in res.stdout:
        sys.exit(f"page {p} piped output contains escape sequences")

with concurrent.futures.ThreadPoolExecutor(max_workers=8) as ex:
    list(ex.map(check, pages))
' "$TAM" "$knowledge" || fail "guide page rendering checks failed"
pass "all guide pages render to text with build-time revision and timestamp"
pass "piped guide output has no control sequences"

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
python3 -c '
import os, re, sys

knowledge = sys.argv[1]
href_re = re.compile(r"""href=["\x27]([^"\x27]+)["\x27]""")
skip_prefixes = ("://", "mailto:", "#", "man:", "tam:", "info:", "tldr:")

broken = []
for dirpath, _, filenames in os.walk(knowledge):
    for f in filenames:
        if f.endswith(".html"):
            p_path = os.path.join(dirpath, f)
            rel = os.path.relpath(p_path, knowledge)
            text = open(p_path, encoding="utf-8", errors="replace").read()
            for href in href_re.findall(text):
                if any(href.startswith(pfx) or pfx in href for pfx in skip_prefixes):
                    continue
                if href.startswith("/") or ".." in href:
                    broken.append(f"{rel}->{href}")
                    continue
                target = href.split("?")[0]
                if not os.path.isfile(os.path.join(knowledge, target)):
                    broken.append(f"{rel}->{href}")

if broken:
    sys.exit("broken internal links: " + " ".join(broken))
' "$knowledge" || fail "broken internal links"
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
# XDG_DATA_DIRS and still shows pages.
stage="$tmp/stage/usr"
mkdir -p "$stage/bin" "$stage/share/tamlinux/knowledge"
cp "$TAM" "$stage/bin/tam"
cp -R "$knowledge/." "$stage/share/tamlinux/knowledge/"
[ -x "$stage/bin/tam" ] || fail "staged install produced no binary"
out=$(env HOME="$tmp/nohome" XDG_DATA_HOME="$tmp/nodata" \
          XDG_DATA_DIRS="$stage/share" TAM_DATADIR="$tmp/no-datadir" \
          env -u TAM_KNOWLEDGE_DIR "$stage/bin/tam" show --pipe welcome)
[ "$out" = "$(guide --pipe welcome)" ] \
    || fail "staged install did not render the current bundled corpus"
pass "staged install relocates and finds the corpus"

printf 'All tam guide tests passed.\n'
