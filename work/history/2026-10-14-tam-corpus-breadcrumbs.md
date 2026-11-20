# Breadcrumb trails and reading paths in the guide pages (corpus lane)

- **CLI Tool**: `opencode` `1.19.0`
- **Model**: `big-pickle`
- **Date**: 2026-10-14
- **Transcript**: Retained privately by the author.
- **Commits**: `8cf9ee8` (breadcrumb trails), `a8ca11d` (reading paths)
- **Scope**: the guide corpus only — `knowledge/`, `tools/build-index.py`,
  and `tests/`. No change to the `tam` command itself; no version bump.

Two related pieces of corpus work landed in one session. Fred directed both;
opencode implemented, tested, and installed them under its corpus-lane claim
while agy held the command lane (`src/`, `Makefile`, `completions/`).

## Part 1 — Breadcrumb trails (`8cf9ee8`)

Fred reported that it was hard to tell where a rendered page sits in the guide
hierarchy, and asked for a breadcrumb or top line, restricted to the corpus so
it would not collide with agy's command-lane work.

### Guiding prompt

> When the tam command displays files, I find it hard to know where I am
> in the hierarchy. Think about what we could display as a breadcrumb or
> top line that would help a user stay oriented and move around the
> knowledge corpus. I believe this is a change to the knowledge corpus
> only, not to the tam command itself. agy is working on the tam command
> now, so make sure to coordinate with it.

### Decisions and scope

- **Top line before the `<h1>`.** The renderer prints authored HTML in
  document order, so a `<nav aria-label="Breadcrumb">` placed before the
  `<h1>` becomes the page's first rendered line, above the depth line that
  `depth.c` injects after `</h1>`. No command change was needed.
- **Titles, slash-separated.** Fred chose the format from three rendered
  options: ancestor crumb labels are page/section titles joined by ` / `,
  e.g. `Guide / The Shell and Streams / Shell pipelines`. Ancestors are links
  (selectable in the pager, numbered in the pipe-mode `Links:` appendix); the
  page's own name is plain text.
- **Labels come from each page's `<h1>`,** so a crumb always matches the
  heading shown directly below it. Section crumbs use the section index's
  `<h1>`; the root crumb is `Guide` and links to `index.html`.
- **Root pages stayed plain** (this was revised in Part 2): `index.html` and
  `welcome.html` carried no trail at first.
- **`hardware/resources.html` has no section index,** so its trail is
  `Guide / System Resources and Performance`.
- **Redundant back-links removed.** The ecosystem pages' one-level
  `The Tamlinux ecosystem` paragraph and `ecosystem/index.html`'s
  `Guide index` paragraph were covered by the trail and were removed.
- **A check, not a generator.** `tools/build-index.py` validates every
  breadcrumb during the corpus build: an absent trail, a trail whose ancestor
  hrefs or labels do not match the page's place, a page-name crumb that
  differs from the page's `<h1>`, or a trail on an exempt page fails
  `make corpus`.

## Part 2 — Reading paths and welcome refinement (`a8ca11d`)

Fred reviewed the rendered welcome page and asked for three copy changes and a
more useful "Where to next", suggesting beginner/intermediate/expert tracks.
He then chose real track pages over experience-styled pointers.

### Guiding prompts

> Adjust Where to next first, install it so I can test it, then commit
> everything.

(Exploratory, no change made: Fred checked whether the pager's `g` key was
free for a future "return to the Guide" shortcut; `g` is already bound to
top-of-page, and `` ` `` is unbound. He decided to hand that separate,
command-lane change to agy, so nothing was changed here.)

### Decisions and scope

- **Three reading-path pages.** Added `knowledge/beginner.html`,
  `intermediate.html`, and `expert.html` — each an ordered list of topics for
  that experience level, with a breadcrumb and `level` metadata (track `use`).
  They are linked from a new "Reading paths" section in `index.html` (placed
  after "Start here" so the existing pager tests' link positions hold) and
  from the welcome page's "Where to next".
- **Welcome copy revised.** Each pager-key bullet now leads with a one-word
  subject (`Scrolling:`, `Links:`, `Breadcrumb:`, `Output:`); the scroll line
  names the up and down arrow keys (left and right do not scroll) and notes
  that the keys do nothing when the whole page already fits; "Where to next"
  now points at the three reading paths.
- **Breadcrumb rule relaxed.** Only `index.html` is now exempt. Every other
  page, including the top-level `welcome.html` and the new path pages, carries
  `Guide` and, for section pages, the section index. This supersedes the
  Part 1 choice to keep root pages plain.
- **Index depth tip.** "Using this guide" now notes per-page depth and the
  `tam show PAGE?level=expert` / `tam explain --level expert` forms.
- **Pager tests decoupled from the corpus.** The wrapped-link hyperlink test
  now runs against a `tests/fixtures` corpus (`link-wrap.html`,
  `index.html`) instead of the welcome page, and the forward-navigation test
  skips the welcome breadcrumb's two `index.html` links before following the
  desktop guide.

## Verification

- `make corpus` regenerated `knowledge/index.{tsv,json}` (29 entries) and
  passed the breadcrumb check.
- `tests/run.sh` and `tests/guide.sh` ran green. Because agy had uncommitted
  `src/` changes (a `0.17.0` version bump that broke the shared `--version`
  test), the suites were run against a clean `0.16.0` binary built from
  `HEAD` out-of-tree in `/tmp/opencode/tamclean`, not the shared `build/tam`;
  the staged-install check still ran `make install` in the tree and passed.
- `UPDATE=1` refreshed the index and welcome snapshots and created
  `beginner.txt`, `intermediate.txt`, and `expert.txt`;
  `tests/guide.sh` re-run clean confirms no drift.
- The breadcrumb check was mutation-tested against five breakages; each failed
  the build with a specific message (including a breadcrumb on `index.html`).
- `tam show` spot-checks confirmed the trail is the first rendered line and
  the three path pages render their ordered lists.

## Coordination

- opencode held the corpus lane (`knowledge/`, `tools/`, `tests/`) in both
  parts and messaged agy's inbox before editing Part 1; agy acknowledged,
  kept out of the corpus lane, and confirmed no dependency on line 1 or on
  appendix link numbers.
- agy owned the command lane throughout (`src/ask.c`, `src/ask.h`, milestone 9
  grounded ask); opencode did not touch its paths and installed its dev build
  from a clean out-of-tree binary to avoid agy's unfinished sources.
- Landed as `8cf9ee8` and `a8ca11d` on `develop`; no push, no version bump.
