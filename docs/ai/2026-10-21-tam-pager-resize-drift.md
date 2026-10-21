# Pager resize cursor preservation (command 0.38.1)

- **Date:** 2026-10-21
- **Time:** approximately 14:32 to 14:45 -04:00
- **Stage:** Develop; preserve the pager's logical reading location through resize reflow
- **CLI Tool:** Codex CLI 0.163.0
- **Model:** GPT-6 Luna (`gpt-6-luna`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer asked to finish the `tam` handoff and verify
  whether the fix had already landed. Codex confirmed the bug on the current
  build, implemented the cursor-kind tie-break and regression tests, and updated
  the command version and plan records.
- **Checked against:** `src/pager.c`, `src/render.c`, `src/version.h`,
  `tests/pager_screen.py`, `tests/fixtures/guide/welcome.html`,
  `docs/plans/pager-display-portability.md`, `docs/plans/development.md`,
  `ROADMAP.md`, and the October 20 pager resize handoff.

## Guiding prompts

> Finish up the work in the tam repo. I believe there is a handoff file you can find.

> Check to make sure this is the correct handoff. This work may have already been done. Are you sure no prior agent has done this already?

## Key decisions

The handoff's proposed correction was still needed: `develop` was clean, the
reflow search still selected the first line at an equal logical offset, and
the existing 0.38.0 build moved the focused row from `L7` to `L6` on resize.
During reflow, the pager now records whether the cursor line is blank and uses
that only to break equal-distance ties. The renderer's logical offsets remain
unchanged. The pager portability plan remains in progress because the exact
packaged Foot and Linux-console review is still pending.

## Verification

- `make command` — compiled with the repository's warning flags.
- `env -u TAM_LEVEL python3 tests/pager_screen.py build/tam tests/fixtures` —
  passed, including text-row and blank-row resize coverage.
- `make test-command` — passed; the live Ollama probe was skipped because no
  server was available.
- `make test` — passed, including command, corpus lint/index, upgrade,
  developer-loop, install-guard, drift, renderer-golden and guide checks. The
  live Ollama probe was skipped because no server was available.
- `git diff --check` — passed.
