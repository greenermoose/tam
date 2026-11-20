# Selected link boundary spaces (0.29.3)

- **Date:** 2026-10-17
- **Time:** 12:45 to 12:47 -04:00
- **Stage:** Develop — kept the space before a selected hyperlink unhighlighted and bumped the command to 0.29.3.
- **CLI Tool:** Codex CLI `0.163.0`
- **Model:** GPT-6 (`gpt-6`)
- **Transcript:** Retained privately by the author.
- **Commits:** `2dc738b` (pager fix, regression fixture, and version records); this commit adds the record and provenance index entry.
- **Authorship:** Fred requested the boundary-space fix and development install; Codex wrote the implementation, regression test, version records, and this provenance entry.
- **Checked against:** `src/pager.c`, `tests/run.sh`, `tests/fixtures/link-leading-space.html`, `src/version.h`, `ROADMAP.md`, `docs/plans/development.md`, and `Makefile`.

## Guiding prompt

> Only highlight interior spaces, not the space before the hyperlink. See this bug in action in version 0.29.2. Fix this bug in version 0.29.3. Create and install the new version.

## Key decisions

Classify a selected link space as interior only when non-space text appears before and after it in the same rendered link span. This excludes the layout separator attached to the beginning of a hyperlink and trailing spaces. Added a PTY fixture matching the “Explore more in the desktop guide” case.

## Verification

- `git diff --check` — passed.
- `make test-command LIBTAM_PREFIX=$HOME/.local/dev` — passed, including the leading-boundary and interior-space PTY assertions and all six renderer goldens; live-server `tam ask` checks were skipped because Ollama was not running.
