# Developer loop component coverage index follow-up (no version change)

- **Date:** 2026-11-10
- **Time:** 19:22 to 19:42 -05:00
- **Stage:** Develop; developer tooling only: `tools/dev.py` and test fixtures. No command or corpus version bump.
- **CLI Tool:** Antigravity CLI 1.5.5
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** Fred Horch reported the `make dev` failure caused by uncommitted `knowledge/component-coverage.json` and requested recording the bug and implementing the permanent fix. Antigravity updated `tools/dev.py`, test suites, repository map, and plan documentation.
- **Checked against:** `Makefile` (`INDEX`, `check-clean-tree`), `tools/dev.py` (`GENERATED_EXACT`, `INDEX_PATHS`), `tests/test_dev.py`, `tests/test_install_guard.py`, `docs/repo-map.md`, `docs/plans/dev-loop.md`.

## Guiding prompts

> Why did this make dev fail?
> [console output showing make install failure from dirty knowledge/component-coverage.json]

> Record the bug and do the permanent fix.

## Key decisions

- When component coverage map generation was added to `Makefile` (`$(INDEX)` and `check-clean-tree`), `tools/dev.py` was not updated to track `knowledge/component-coverage.json`.
- As a result:
  1. In Step 5 (`make test`), `make corpus` regenerates `knowledge/component-coverage.json` if corpus edits changed component references.
  2. In Step 6, only pre-test working tree changes are committed.
  3. In Step 7 (`Index follow-up`), `tools/dev.py` committed only the index files listed in `INDEX_PATHS`, leaving `knowledge/component-coverage.json` dirty.
  4. In Step 8 (`Install`), `make install` ran `check-clean-tree`, which aborted upon detecting the dirty `knowledge/component-coverage.json`.
- The fix adds `"knowledge/component-coverage.json"` to both `GENERATED_EXACT` (so it is treated as a generated build output and ignored in Step 3) and `INDEX_PATHS` (so it is staged and committed automatically during Step 7 index follow-up).
- Updated repository documentation in `docs/repo-map.md` and `docs/plans/dev-loop.md`, added unit tests in `tests/test_dev.py` and `tests/test_install_guard.py`.

## Verification

- `python3 -m unittest -q tests/test_dev.py tests/test_install_guard.py tests/test_check_repo.py`: all 42 tests pass.
- `python3 tools/check-repo.py`: passes with exit code 0.
- `make test-command`: all integration, upgrade, renderer, and consistency tests pass.
