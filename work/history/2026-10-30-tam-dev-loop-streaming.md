# Stream dev-loop build output and remove pedantic repo drift checks

- **Date:** 2026-10-30
- **Time:** 12:38 to 13:16 -04:00
- **Stage:** Develop; repository tooling and build workflow, no component version bump
- **CLI Tool:** Antigravity CLI 1.4.11
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** Fred Horch requested real-time progress messages during `make dev` builds, followed by fixing the version replacement error in `ROADMAP.md` and removing unnecessary pedantic markdown drift tests from `make test`. Antigravity CLI implemented streaming via `subprocess.Popen` in `tools/dev.py`, made roadmap history replacement optional, updated `tests/test_dev.py`, and streamlined `Makefile` test targets.
- **Checked against:** `Makefile`, `tools/dev.py`, `tests/test_dev.py`, `ROADMAP.md`, `docs/plans/dev-loop.md`

## Guiding prompts

> When I'm do make dev for tam I want to see more results. There is too long a delay of no response while the project is building. Tell me what progress messages could be shown so a developer can see how the build is going.

> Of the two options, which is faster and more efficient?

> Implement option B.

> Commit

> Fix the make system so it works. Here's the error. Remove unnecessary tests. I want this to make faster without pedantic checks in the markdown documents.
>
> == 4. Versions ==
> Corpus version (0.50.0 -> patch 0.50.1 or minor 0.51.0) [p]atch (default) / [m]inor: 
> Command version (0.45.1 -> patch 0.45.2 or minor 0.46.0) [p]atch (default) / [m]inor: 
> One-line summary: Wordsmithing help screen and other pages
> Error: ROADMAP.md: expected pattern '(\\|[^\\n]+\\|\\n)(\\n+## 3\\. Current state and follow-up\\n)' exactly once, found 0; no version file was changed (run make check-repo) Nothing was committed.
> make: *** [Makefile:74: dev] Error 1

## Key decisions

- Added `stream=True` support to `run_make()` in `tools/dev.py` using `subprocess.Popen` to stream compiler, linter, and test suite execution to stdout in real time while capturing output for error reporting.
- Added `replace_optional()` to `tools/dev.py` and updated `apply_corpus_version_edits()` and `apply_command_version_edits()` so that inserting delivered-history table rows in `ROADMAP.md` is optional and supports renumbered sections (`## [0-9]+\. Current state and follow-up`), preventing build failure when delivered history is omitted.
- Streamlined `make test` and `make corpus-and-test` in `Makefile` by removing `python3 tools/check-repo.py` and `tests/test_check_repo.py` (pedantic markdown checks) as well as the slow dev-loop harness tests (`test_dev.py`, `test_install_guard.py`), keeping everyday builds fast and preserving those checks under `make check`.

## Verification

- `python3 -m unittest -v tests/test_dev.py`: 13 tests passed cleanly.
- Tested version bumping against live `ROADMAP.md` and repository version headers.
