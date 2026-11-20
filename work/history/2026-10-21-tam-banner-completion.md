# Complete banner parent ranking (command 0.37.1)

- **Date:** 2026-10-21
- **Time:** 06:23 to 06:28 -04:00
- **Stage:** Develop. Complete the interrupted command 0.37.1 patch; corpus unchanged.
- **CLI Tool:** Codex CLI 0.163.0 (checked from the running binary)
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer authorized taking over the exhausted Antigravity
  session's claims and completing its work. Antigravity wrote the ranking code,
  initial fixture/tests and documentation, recorded in its
  [session record](2026-10-21-tam-banner-parent-ranking.md). Codex reviewed those
  edits, corrected the failing PTY assertion helper, added ranking regression
  checks, synchronized the plan index, and completed verification and commits.
- **Checked against:** the previous session's retained transcript,
  `src/tam.c`, `src/index.c`, `src/terminal.c`, `tests/run.sh`,
  `tests/fixtures/guide/`, `docs/plans/learning-levels.md`, `man/tam.1`,
  and `tamlinux` commit `bfd03c7` (the completed welcome-state documentation fix).

## Guiding prompts

> agy ran out of tokens in the middle of the work it was doing in the tam repo. Please investigate, take over its work claims, and finish up what it was doing.

## Key decisions

The accepted parent-ranking design is unchanged. The interrupted test failure
came from the PTY helper leaving `B` from terminfo's `ESC ( B` character-set
selection in captured text. Strip that sequence along with CSI controls before
checking the visible title. Disposable guide fixtures check relation priority,
shared-prerequisite and title ordering, the three-link limit, level filtering,
unrelated-page exclusion, and fallback when no relative or parent sidecar exists.

This completes TCK21's banner follow-up. TCK22–TCK25 remain in the learning-levels
plan; no development installation or package-stage promotion is part of this patch.

## Verification

`timeout 180 make test` passes: command and PTY checks, six renderer goldens,
upgrade tests, corpus lint and index checks, developer-loop and install-guard
tests, repository drift checks, and the staged guide installation. The optional
live Ollama checks skip when the server is unavailable. The new ranking cases
also pass independently. Checkout-corpus spot checks confirm Welcome and About
at Intermediate link "Intermediate path", the Learning hub at Expert links
"Expert path", and the Using hub links its Expert child pages. Changed planning
and provenance links resolve; `git diff --check` and the public-provenance scan
are clean.
