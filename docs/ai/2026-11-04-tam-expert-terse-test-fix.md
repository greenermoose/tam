# Expert level filtering assertion fix in tests/run.sh

- **Date:** 2026-11-04
- **Time:** 21:06 to 21:35 -05:00
- **Stage:** Develop; command test suite fix
- **CLI Tool:** Antigravity CLI 1.5.1
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** The commit that adds this record and fixes `tests/run.sh`
- **Authorship:** Fred reported a `make dev` failure on an expert terse test; Antigravity investigated the root cause, updated the test assertions to check a novice-only topic (`welcome`), verified `make test`, and wrote this record.
- **Checked against:** `tests/run.sh`, `tools/dev.py`, `knowledge/desktop/index.html`, `knowledge/welcome.html`, `ROADMAP.md`, and commit `f446e01`.

## Guiding prompts

> make dev in tam failed as follows. Please investigate and fix the root cause.
> 
> make: *** [Makefile:134: test] Error 1
> 
> FAIL: expert terse unexpectedly includes desktop
> The build failed before the tests ran.
> You changed only pages, and this is not a page check: it is a build, command or repository test. Your pages are probably fine. Ask an agent to fix the named test, then run make dev again; it keeps your version bump.
> Error: make test failed; version edits remain in the working tree. Nothing was committed.
> make: *** [Makefile:91: dev] Error 1

## Key decisions

In Plan 26 Phase 1 (commit `f446e01`), `knowledge/desktop/index.html` was updated to include an expert section and metadata `<meta name="level" content="novice, learner, expert">`. Consequently, `desktop` legitimately became an expert topic and is expected in `tam explain topics --level expert --terse`.

An older live-corpus assertion in `tests/run.sh` (from tam 0.18.0) had used `desktop` as a negative check to verify that non-expert topics are excluded by `--level expert`. With `desktop` now having expert content, that assertion failed.

Updated `tests/run.sh` to check `welcome` (a dedicated novice-only topic) instead of `desktop` for the negative `--level expert` filtering checks (`ex_expert_terse`, `ex_cat_expert`, and fallback `ex_no_exp`).

## Verification

- `tests/run.sh build/tam`: All tam command tests passed.
- `make test`: All test suites passed cleanly (command integration, upgrade tests, renderer goldens, corpus linter, unit tests, and guide verification).
