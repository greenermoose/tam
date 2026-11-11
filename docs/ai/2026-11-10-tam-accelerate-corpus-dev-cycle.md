# Accelerate Corpus Development Cycle (tooling only)

- **Date:** 2026-11-10
- **Time:** 18:10 to 20:15 -05:00
- **Stage:** Develop; repository build tools and developer loop
- **CLI Tool:** agy 1.5.5
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending
- **Authorship:** The developer asked to accelerate the development cycle when editing knowledge corpus pages and running `make dev`. The AI tool implemented single-pass in-memory component coverage scanning, lane-targeted testing and tooling unit-test skipping in `tools/dev.py`, single-page linting in `tools/lint-corpus.py`, and a `make preview` target in `Makefile`.
- **Checked against:** `Makefile`, `tools/dev.py`, `tools/report-component-coverage.py`, `tools/lint-corpus.py`, `tests/test_dev.py`, `tests/test_build_index.py`, `tests/test_component_coverage.py`.

## Guiding prompts

> Would writing tests for tam command and tam corpus in C speed up development cycle? I'm waiting too long for tests and the build process. Suggest ways to accelerate the development cycle when editing pages in the corpus.
> /plan Plan how to do everything possible to accelerate the development cycle. I will be editing many pages in the knowledge corpus. I want to be able to run make dev often to check my work and install updated pages in the development version for testing. Consider all possible ways to contain to benefit from testing but at a much faster tempo.
> /plan 1 give an interactive prompt with default set to skip
> 2 yes, provide a convenience target
> 
> Show the plan in micro
> /plan ready
> /plan Approve, proceed

## Key decisions

- In `tools/report-component-coverage.py`, replaced repeated disk scanning ($250 \times 588 = 147{,}000$ file I/O operations) with pre-loading all HTML pages once and scanning with a single compiled union regex of all component identifiers. This accelerates coverage generation from 11.9s to 0.3s (a 40x speedup) with byte-for-byte identical output.
- In `tools/dev.py`, Step 5 detects when changes are strictly corpus pages (`knowledge/*.html` plus version bumps in `ROADMAP.md`). For corpus-only changes, it selects `make corpus-and-test` instead of full `make test` (skipping command integration tests in `tests/run.sh`).
- Added an interactive prompt to `tools/dev.py` for corpus-only runs asking whether to run tooling unit tests (`test_build_index.py`, etc.), defaulting to `[N]` (skip via `SKIP_UNIT_TESTS=1`).
- Added `make preview [PAGE=...]` to `Makefile`, enabling live zero-commit preview of uncommitted working-tree knowledge pages directly in `tam`.
- Added optional page arguments to `tools/lint-corpus.py` to support instant single-page linting during editing.

## Verification

- `python3 -m unittest -v tests/test_component_coverage.py`: all 6 tests pass.
- `python3 tools/report-component-coverage.py --quiet --check`: exit 0; output matches baseline with zero difference.
- `python3 -m unittest -v tests/test_lint_corpus.py`: all 35 tests pass.
- `python3 -m unittest -v tests/test_build_index.py`: all 22 tests pass.
- `python3 -m unittest -v tests/test_dev.py`: all 21 tests pass.
- `make corpus-and-test SKIP_UNIT_TESTS=1 SKIP_COVERAGE_CHECK=1`: live corpus checks pass in 2.01s.
- `make check`: all integration and unit tests pass in 18.61s (down from ~55s).
- `python3 tools/check-repo.py`: passes cleanly.
