# TCK11: `make dev` developer loop

- **Date:** 2026-10-20
- **Time:** 08:50 to 09:17 -04:00
- **Stage:** Develop; TCK11 (`make dev` developer loop, repository tooling only, no version change)
- **CLI Tool:** Antigravity CLI 1.4.3
- **Model:** Gemini 4 Argon (High) (`gemini-4-argon-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `ce4b617` and the commit that adds this record
- **Authorship:** The developer directed implementation of TCK11 from the
  approved plan (`docs/plans/dev-loop.md`). The AI tool wrote `tools/dev.py`,
  `tests/test_dev.py`, and the `Makefile`, `README.md`, and `AGENTS.md`
  updates.
- **Checked against:** `docs/plans/dev-loop.md`, `Makefile`,
  `tools/build-index.py`, `tests/test_build_index.py`, `src/version.h`,
  `knowledge/version`, `tests/run.sh`, `ROADMAP.md`,
  `docs/plans/development.md`, `README.md`, `AGENTS.md`, `tam-work`

## Guiding prompts

> Develop TCK11 from docs/plans/dev-loop.md. Read AGENTS.md first, then the plan.
> Claim only the paths TCK11 names, follow the plan exactly, and run make test.
> Stop after TCK11 is committed and show me `make dev DRY_RUN=1` output with one
> edited page so I can try the real `make dev` myself. Do not start TCK12, and
> ask me before changing anything the plan does not name.

> In a separate commit now.

## Key decisions

None beyond the prompts and the approved plan.

## Verification

`make test`: all command, corpus, render, upgrade, and `tests/test_dev.py`
checks pass. `make dev DRY_RUN=1` with one edited page (`knowledge/welcome.html`)
listed only that page as corpus, proposed `0.34.0 -> 0.34.1` (patch) or `0.35.0`
(minor), and left `git status` unchanged.
