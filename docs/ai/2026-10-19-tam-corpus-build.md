# Reliable corpus rebuilds (corpus 0.32.2)

- **Date:** 2026-10-19
- **Time:** 16:02 to 16:10, -04:00
- **Stage:** Develop — corpus tooling, tests and authoring instructions
- **CLI Tool:** Codex CLI 0.163.0, checked with the running runtime detector
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`), verified against turn metadata
- **Transcript:** Retained privately by the author.
- **Commits:** `fb8c1ac` and the commit updating the generated indexes and this record
- **Authorship:** Fred requested and approved the plan and target rename;
  Codex implemented the Make rules, generator, regression tests and documentation.
- **Checked against:** `Makefile`, `tools/build-index.py`,
  `tests/test_build_index.py`, `tests/run.sh`, `tests/guide.sh`,
  `docs/corpus-authoring.md`, `knowledge/tam.html`, `knowledge/version`,
  `src/version.h`, `CHECKLISTS.md` and `docs/plan-maintenance.md`.

## Guiding prompts

> Make a plan to fix the Makefile so make corpus reliably handles everything without needing to remember to run a separate python script.

> Can we change test-corpus to corpus-and-test?

> Can you clear context and implement it?

> Switch to default mode, clear context, and build the plan.

> Clear context and build plan.

## Key decisions

The developer selected a full recheck on every invocation rather than a cache.
TCK9 (reliable corpus builds) uses one phony corpus recipe shared by direct
index targets; unchanged outputs retain their timestamps. Tests rebuild before
checking. Manual versions, index schemas and Git metadata semantics remain.
The renamed target has no compatibility alias. Corpus 0.32.2 includes the
tooling correction and updated Make target reference; command 0.31.1 is unchanged.

The first integrated run exposed stale welcome-page assertions from an earlier
content rewrite. Navigation now tolerates additional links before the desktop
link; staged delivery is compared with current rendered corpus text rather
than an obsolete sentence. The history test compares its saved cursor-line
metric independently of the viewport status label. No reader code changed.
Only temporary staged installs were used; there was no session installation,
package assembly or promotion.

## Verification

- Focused index/Make regressions: 10 tests passed, covering Git-only changes,
  nested page inventory, unchanged outputs, missing outputs, direct/parallel
  targets, failure preservation, archives and rebuild-before-test ordering.
- `timeout 600 make check PREFIX=<development-prefix>
  LIBTAM_PREFIX=<development-prefix>` passed: command/PTY checks, 15 upgrade
  tests, six renderer goldens, 29 linter tests, 10 index/Make tests, and all
  55 page renders, search destinations, link and staged-install checks.
- `timeout 120 make -j4 corpus corpus-and-test PREFIX=<development-prefix>
  LIBTAM_PREFIX=<development-prefix>` passed and generated the indexes once.
- Live `tam ask` checks skipped because no local Ollama server was running.
- Changed Markdown links, complete plan inventory and `git diff --check` passed.
- After committing the source, `make corpus` refreshed the edited page's
  revision from 6 to 7 and its timestamp without calling Python separately.
  The post-commit `make corpus-and-test` run passed before recording those indexes.
