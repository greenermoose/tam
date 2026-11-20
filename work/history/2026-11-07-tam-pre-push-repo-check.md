# Pre-push repository consistency check and version-bump rules

- **Date:** 2026-11-07
- **Time:** 14:32 to 15:00 -05:00
- **Stage:** Develop
- **CLI Tool:** Antigravity CLI (`agy`) 1.5.3
- **Model:** Gemini 4 Argon (High) (`gemini-4-argon-high`) and Claude Opus 5.5 (High) (`claude-opus-5.5-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** Fred directed the workflow and documentation fixes after `check-repo.py` caught missing `ROADMAP.md` delivered-history rows during packaging; Antigravity wrote the hook, Makefile targets, `AGENTS.md` version checklist, and unit test.
- **Checked against:** `tools/check-repo.py`, `Makefile`, `AGENTS.md`, `tests/test_check_repo.py`

## Guiding prompts

> Yes, go ahead. And fix the runbook so we don't keep getting blocked like this. What changes to the process and documentation can be made so it's smoother? Codex ran into a simple stupid niggling error that blocked it, too. I want to smooth these things out so we're not getting tripped up by nothing burgers and only stop when something real is wrong.

> The issue I'm thinking about was two lines of documentation that codex had to update.

> Yes, include these fixes. Regarding tam, no, do not uninstall the development version I'm working on now. What other questions do you have?

> Keep going.

## Key decisions

- `AGENTS.md` previously listed only `src/version.h`, `tests/run.sh`, `docs/plans/development.md` and the `ROADMAP.md` header under `## Versions`, omitting the section 2 delivered-history row and section 3 `develop` row that `tools/check-repo.py` enforces, and single-lane test targets (`make test-command`, `make corpus-and-test`, `make test`) did not invoke `tools/check-repo.py`.
- Added `.githooks/pre-push` (enabled via `git config core.hooksPath .githooks` in `make check-repo`), wired `tools/check-repo.py` into `make test`, `make test-command`, and `make corpus-and-test`, and listed all version locations in `AGENTS.md` so missing roadmap rows are caught before commit or push.

## Verification

- `make check-repo`: passes with zero findings and configures `core.hooksPath` to `.githooks`.
- `python3 -m unittest -q tests/test_check_repo.py`: all tests pass, including `test_pre_push_hook_enforces_check_repo`.
