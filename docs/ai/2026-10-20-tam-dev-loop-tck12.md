# TCK12: repository drift checks, repository map, and drift fixes

- **Date:** 2026-10-20
- **Time:** 09:30 to 10:07 -04:00
- **Stage:** Develop; TCK12 (`make check-repo`, `docs/repo-map.md`, `tests/test_check_repo.py`, `Makefile`, `docs/plans/README.md`, and two `ROADMAP.md` corpus 0.34.0 drift fixes; repository tooling only, no version change)
- **CLI Tool:** Antigravity CLI 1.4.3
- **Model:** Gemini 4 Argon (High) (`gemini-4-argon-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `50ecf02`, `ac42b20`, `2b46bc3` and the commit that adds this record
- **Authorship:** The developer directed implementation of TCK12 from `docs/plans/dev-loop.md` and fixing each drift finding in its own commit. The AI tool wrote `tools/check-repo.py`, `docs/repo-map.md`, and `tests/test_check_repo.py`, updated `Makefile` and `docs/plans/README.md`, and fixed the two corpus 0.34.0 drift findings in `ROADMAP.md`.
- **Checked against:** `docs/plans/dev-loop.md`, `Makefile`, `tools/dev.py`, `tools/lint-corpus.py`, `src/version.h`, `knowledge/version`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`, `docs/plans/README.md`, `AGENTS.md`, commit `08b166a`

## Guiding prompts

> Develop TCK12 from docs/plans/dev-loop.md. Fix each drift finding in its own
> commit and ask me about any finding whose correct value is unclear.

## Key decisions

- The initial run of `tools/check-repo.py` reported two corpus version drift findings in `ROADMAP.md` left behind when commit `08b166a` bumped `knowledge/version` to `0.34.0`: the section 3 `develop` row still listed `0.32.0 / 0.33.6`, and section 2 had no delivered-history row for `0.34.0 corpus`. Both values were clear from `knowledge/version` and commit `08b166a`, and each was fixed in its own commit (`50ecf02`, `ac42b20`).

## Verification

`make check-repo` and `make test`: all command, corpus, render, upgrade, `tests/test_dev.py`, `tools/check-repo.py`, and `tests/test_check_repo.py` (9 fixture tests covering clean pass and planted errors for each of the seven drift checks) checks pass.
