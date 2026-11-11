# Fast independent command and corpus builds

**Plan status:** Completed
**Reviewed:** 2026-11-11
**Scope:** Build and editing tools; no command or corpus version change.
**Authority:** The developer requested “Plan improvements to the tam build process. I want it going as fast as possible.” and then “Implement the plan.” He selected a separate fast editing loop alongside `make dev`, with an on-demand cache and no watcher.
**Next action:** None; source implementation and verification are complete. Development installation and packaged acceptance remain separate.

## Outcome and design

Keep Make and Python. `make command` builds the C program; `make corpus`
builds metadata independently of a compiler or libtam. `make corpus-check`
checks edited content and affected dependencies, while `make preview PAGE=…`
builds indexes and opens the source corpus with an existing reader. Neither
editing command commits, installs or pushes. `make dev` retains its finishing
workflow and existing approval/installation boundaries.

The shared pipeline reads authored inputs once, hashes contents even when
an editor restores timestamps, and persists page metadata, learning summaries,
lint findings and render-check successes in ignored `build/corpus/`. Global
hierarchy, prerequisite, bibliography, learning and link checks use the page
summaries. Changed tooling/rules invalidate their cached results; changed
reader identity invalidates rendering. Product inventory/exemptions affect
coverage; tldr Markdown has a separate index cache. Missing or altered outputs
are repaired. A lock serializes invocations, cache writes use atomic replacement,
and changing inputs trigger one retry before a clear failure.

`CORPUS_CACHE=0` recomputes everything without reading or updating cached
successes. Existing builder/report/linter interfaces remain available for
standalone use and reference comparison. Generated formats, independent
versions and page revision semantics stay the same.

`TAM_READER` selects an existing executable. Otherwise the pipeline selects
`build/tam`, then `tam` on PATH. A missing/incompatible reader asks the developer
to build/select one separately. Corpus-only preflight and install do not fall
back to compiling C. Full integrated tests build the command explicitly.

The finishing loop validates content once, refreshes page revision metadata
after its page commit, then refreshes corpus identity after the index commit.
Timing includes initial build prerequisites; developer-cycle output separates
local automated work, prompt waiting and network time.

C compilation uses generated header dependencies. Compile flags, link flags
and embedded command identity have separate stamps; only identity consumers
recompile when checkout identity changes. Installed static archive changes
still force relinking.

## Acceptance

On the current workstation, five-run median targets are ≤0.25 seconds for an
unchanged corpus build, ≤1 second for one changed page plus checks, ≤2 seconds
for 100 changed pages plus checks, and ≤3 seconds for cold build/content checks.
Also measure changing every page and checking/committing/staging a batch. The
benchmark creates and removes temporary fixtures, uses an existing reader,
and reports its exclusions; fixture history is shorter than production history.

Cached and uncached artifacts/findings/status must agree, and unchanged outputs
retain their mtimes. Verify additions, deletion/rename, restored timestamps,
invalid metadata, broken links, registry/rule/reader/inventory/tldr changes,
Git revisions, cache corruption, concurrent invocations and unstable inputs.
Verify actual compiler invocations for precise header dependencies, identity
changes, flag changes and static archive relinking. Full integrated checks
run once after focused tests, under an exclusive repository claim.

## Verification

Five-run medians: unchanged build **0.214s**, one edited page/checks **0.583s**,
100 edited pages/checks **1.443s**, cold build/checks **2.629s**, all 588 edited
pages/checks **2.668s**, and batch checking/commits/staged copy **1.927s**.
All four target medians pass. Slowest times and exclusions are in the
[implementation record](../ai/2026-11-11-tam-incremental-build.md).

Production artifacts match the uncached reference byte for byte. The complete
`make check SKIP_COVERAGE_CHECK=1` passes in 27.65s, including the new cache
and real compiler dependency tests. The default coverage gate remains active
and reports the pre-existing unmentioned `tam-launch-journal` component;
the unchanged standalone coverage checker confirms the same finding. That
content gap is separate from this tooling change.
No installation or Tamlinux stage transition is part of this work.
