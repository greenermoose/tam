# Incremental tam builds and fast corpus editing (tooling only)

- **Date:** 2026-11-11
- **Time:** 18:23 to 18:54 -0500 (session includes investigation and planning).
- **Stage:** Develop; source tooling only, no component version change.
- **CLI Tool:** Codex CLI 0.167.1 (runtime-checked).
- **Model:** `gpt-6.1-sol` (runtime-checked).
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record and implementation.
- **Authorship:** Fred requested the design and implementation; Codex wrote the pipeline, build/workflow changes, tests and documentation.
- **Checked against:** `Makefile`, `tools/build-index.py`, `tools/corpus_pipeline.py`, `tools/dev.py`, existing speedup records, generated artifacts and the full test suite.

## Guiding prompts

> Do you think it's worth exploring other build tools or is make good enough?

> I'm specifically wondering about tam. Have we cleared up the problems building it? We had a major speed problem. I think it's better, but there is probably more work that could be done.
>
> I want to be able to work on the comman (a C program) and the corpus (a set of HTML files) separately. The most important thing is to be able to make lots of changes to lots of HTML files in the knowledge corpus and build that corpus super fast. It's still taking much longer than I would like, but it is moving in the right direction.

> Plan improvements to the tam build process. I want it going as fast as possible.

> Implement the plan.

## Decisions

Fred selected a separate fast editing loop alongside the existing finishing
workflow, and an on-demand cache without a watcher. Keep Make and Python;
reuse parsed summaries and validation successes instead of reparsing all pages.
Complete corpus verification bypasses cached successes. Page revisions and
all generated formats retain their established semantics.

`make corpus` needs no compiler/libtam. `make corpus-check` and
`make preview PAGE=…` use an existing reader; `TAM_READER` overrides selection.
The finishing loop refreshes metadata after the page commit, then identity
after the index commit. It measures local work, prompt waiting and network
separately. C dependencies track actual includes; identity and link flags
invalidate only the necessary work.

## Verification

- All production generated artifacts match the uncached reference byte for byte.
- Complete `make check SKIP_COVERAGE_CHECK=1`: passes, 27.65s wall, including
  78 corpus tooling tests and 49 developer/install/repository/C-build tests.
- The default gate correctly reports one pre-existing gap: `tam-launch-journal`.
  The standalone unmodified checking path reports the same gap. No exemption
  or corpus text was added to conceal it.
- Cache tests cover restored timestamps, failed checks, rename/deletion,
  links, registry/rule/reader/inventory changes, independent tldr inputs,
  output repair, corruption, concurrent invocation and changing inputs.
- Real compiler fixtures verify precise header dependencies, compile/link
  flags, checkout identity changes, archive relinking and command no-op builds.
- Corpus Make succeeds with nonexistent compiler and pkg-config commands.
- `make bench-corpus`: all four median targets met across five repetitions
  per workload, using the current 588 HTML pages and a temporary fixture history.

| Workload | Median | Slowest |
| --- | ---: | ---: |
| Cold build and content checks | 2.629s | 2.653s |
| Unchanged build | 0.214s | 0.223s |
| One edited page and checks | 0.583s | 0.588s |
| 100 edited pages and checks | 1.443s | 1.452s |
| All edited pages and checks | 2.668s | 2.754s |
| Batch check/commits/staged install | 1.927s | 2.256s |

Finishing measurement excludes interactive prompts, claims, version editing,
repository checks and network. It is not a measurement of Fred's entire
interactive `make dev` session. The unchanged standalone reference and full
checks remain available to audit the cache. Test fixtures and benchmark
staging were temporary; no live installation, deployment, stage move or
publication was performed.
