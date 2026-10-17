# Selected pager link spaces (0.29.2)

- **Date:** 2026-10-17
- **Time:** 12:38 to 12:44 -04:00
- **Stage:** Develop — made spaces inside selected hyperlinks visible and bumped the command to 0.29.2.
- **CLI Tool:** Codex CLI `0.163.0`
- **Model:** GPT-6 (`gpt-6`)
- **Transcript:** Retained privately by the author.
- **Commits:** `ced4840` (pager fix and version records); this commit adds the record and provenance index entry.
- **Authorship:** Fred requested the selected-link spacing fix and development install; Codex wrote the implementation, test, version records, and this provenance entry.
- **Checked against:** `src/pager.c`, `tests/run.sh`, `src/version.h`, `ROADMAP.md`, `docs/plans/development.md`, and `Makefile`.

## Guiding prompt

> This is good, except please highlight the interior spaces in a hyperlink as well as the non-space characters. See the example of how spaces are not being highlighted using tam command version 0.29.1. Bump the version to 0.29.2 in the new version you create and install.

## Key decisions

Apply selected emphasis to whitespace inside the selected hyperlink, while retaining the existing unselected-link whitespace behavior. Keep the corpus at 0.29.0.

## Verification

- `git diff --check` — passed.
- `make test-command LIBTAM_PREFIX=$HOME/.local/dev` — passed, including the PTY assertion that the reverse-video attribute covers spaces in the selected link and all six renderer goldens; live-server `tam ask` checks were skipped because Ollama was not running.
