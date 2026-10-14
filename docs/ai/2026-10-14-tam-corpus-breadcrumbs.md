# Breadcrumb trails in the guide pages (corpus lane)

- **Date:** 2026-10-14
- **Stage:** Develop — the offline guide's corpus gained a breadcrumb
  trail on every page below the index, plus a corpus-build check that
  keeps the trails accurate. Corpus lane only (`knowledge/`, `tools/`,
  `tests/snapshots/`); no command change and no version bump.
- **CLI Tool:** `opencode` 1.19.0
- **Model:** `big-pickle`
- **Transcript:** Retained privately by the author.
- **Commit:** `8cf9ee8`
- **Authorship:** Fred asked opencode, in a direct (non-delegated)
  session, to think about a breadcrumb or top line that would help a
  reader stay oriented and move around the corpus, and directed that the
  change stay in the knowledge corpus because agy was working on the
  command. opencode chose the format with Fred (titles, slash-separated),
  authored the pages, the check, and the snapshots, verified the build and
  tests, and committed the change. Fred reviewed and approved the format
  before the commit.

## Guiding prompts

> When the tam command displays files, I find it hard to know where I am
> in the hierarchy. Think about what we could display as a breadcrumb or
> top line that would help a user stay oriented and move around the
> knowledge corpus. I believe this is a change to the knowledge corpus
> only, not to the tam command itself. agy is working on the tam command
> now, so make sure to coordinate with it.

## Decisions and scope

- **Top line before the `<h1>`.** The renderer prints authored HTML in
  document order, so a `<nav aria-label="Breadcrumb">` placed before the
  `<h1>` becomes the page's first rendered line, above the depth line
  that `depth.c` injects after `</h1>`. No command change was needed; the
  change is pure corpus HTML.
- **Titles, slash-separated.** Fred chose the format from three rendered
  options: ancestor crumb labels are page/section titles joined by ` / `,
  e.g. `Guide / The Shell and Streams / Shell pipelines`. Ancestors are
  links (so they are selectable in the pager and numbered in the pipe-mode
  `Links:` appendix); the page's own name is plain text.
- **Labels come from each page's `<h1>`,** so a crumb always matches the
  heading shown directly below it. Section crumbs use the section index's
  `<h1>`; the root crumb is `Guide` and links to `index.html`.
- **Root pages stay plain.** `index.html` and `welcome.html` are the guide
  roots and carry no trail.
- **`hardware/resources.html` has no section index,** so its trail is
  `Guide / System Resources and Performance`.
- **Redundant back-links removed.** The ecosystem pages' one-level
  `The Tamlinux ecosystem` paragraph and `ecosystem/index.html`'s
  `Guide index` paragraph are covered by the trail and were removed.
- **The index explains the trail.** A sentence was added to the guide
  index's "Using this guide" section so readers know the trail's early
  parts are links.
- **A check, not a generator.** `tools/build-index.py` now validates every
  breadcrumb in the corpus build: an absent trail, a trail whose ancestor
  hrefs or labels do not match the page's place (guide index, then its
  section index), a page-name crumb that differs from the page's `<h1>`,
  or a trail on a top-level page all fail `make corpus`.

## Verification

- `make corpus` regenerated `knowledge/index.{tsv,json}` (26 entries) and
  passed the new breadcrumb check.
- `make test` ran `tests/run.sh` and `tests/guide.sh` green; `UPDATE=1`
  refreshed the 24 page snapshots and the index snapshot, and
  `index.txt`/`welcome.txt` were unchanged.
- The check was mutation-tested against five breakages (wrong page-name
  crumb, wrong ancestor label, missing nav, nav on a root page, wrong
  ancestor href); each failed the build with a specific message.
- `tam show --pipe` spot-checks confirmed the trail is the first line,
  the `<h1>` and depth line follow, and the `Links:` appendix lists the
  ancestor crumbs first.
- Landed as commit `8cf9ee8` on `develop`; no push, no version bump.

## Coordination

- opencode held the corpus lane (`knowledge/`, `tools/`,
  `tests/snapshots/`) and messaged agy's inbox before editing.
- agy acknowledged, confirmed it would keep out of the corpus lane, and
  reported that neither its tests nor the planned milestone 9 work depend
  on line 1 of a rendered page or on exact appendix link numbers, and that
  the command lane is not adding a header/top-line feature.
