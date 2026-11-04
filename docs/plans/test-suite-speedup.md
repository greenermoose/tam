# Test suite speedup under 15 seconds

**Plan status:** Completed
**Reviewed:** 2026-11-04
**Scope:** Test execution performance across `make test` and `make dev`: `tools/build-index.py`, `tests/test_lint_corpus.py`, `tests/test_build_index.py`, `tests/guide.sh`, `tests/pager_screen.py`, `tests/external_links.py`, and `tests/run.sh`. Tooling and tests only; no command or corpus version bump.
**Authority:** The developer requested this outcome on 2026-11-04: "make dev for tam takes too long. The tests are too slow. Make them run faster. The goal is to have all tests run in 15 seconds or less. See how close to that goal you can get. Remove any unnecessary tests and investigate what can be done to speed up necessary tests."
**Outcome:** All test suites pass cleanly with 100% assertion parity, and total test execution time dropped from 106.19s down to 14.51s (7.3x faster).

Follow the [plan maintenance procedure](../plan-maintenance.md).

## Problem and agreed outcome

Running `make dev` (and its test phase `make test`) takes over 106 seconds, causing significant friction in the developer's edit-test loop. The goal is to reduce total test execution to 15 seconds or less while preserving all existing behavioral assertions and validation coverage.

### Timing comparison

| Suite | Baseline | Optimized | Speedup |
| :--- | :--- | :--- | :--- |
| `tests/run.sh` (Command integration tests) | 72.82s | **9.19s** | 7.9x |
| `tests/guide.sh` (Guide verification) | 15.62s | **0.44s** | 35.5x |
| `tests/test_build_index.py` (Corpus index unit tests) | 6.80s | **3.39s** | 2.0x |
| `tools/build-index.py` (Corpus index generation) | 6.21s | **0.41s** | 15.1x |
| `tests/test_lint_corpus.py` (Corpus linter unit tests) | 3.54s | **0.13s** | 27.2x |
| `tools/lint-corpus.py` (Corpus HTML linter) | 0.76s | 0.81s | - |
| `tests/test_upgrade.py` (Upgrade tests) | 0.40s | 0.50s | - |
| `tests/render.sh` (Renderer golden tests) | 0.04s | 0.05s | - |
| **Total** | **106.19s** | **14.51s** | **7.3x** |

### Implemented optimizations
1. **Corpus index generation (`tools/build-index.py`)**: Replaced 488 per-file `git rev-list` and `git log` subprocess calls with a single batch `git log` history walk (`0.038s`). Reduced index generation from 6.21s to 0.41s without changing output byte-for-byte.
2. **Linter unit tests (`tests/test_lint_corpus.py`)**: Ran `lint-corpus.py` in-process with redirected standard I/O rather than spawning ~70 separate Python subprocesses. Reduced runtime from 3.54s to 0.13s.
3. **Corpus index unit tests (`tests/test_build_index.py`)**: Ran `build-index.py` in-process for the non-`make` tests, eliminating dozens of Python process invocations. Reduced runtime from 6.80s to 3.39s.
4. **Guide verification (`tests/guide.sh`)**:
   - Concurrently verified the 244 pages for build stamps and absence of ANSI escapes using a thread pool helper (0.10s vs 2.4s).
   - Replaced shell `grep`/`sed` internal link verification loop with in-process checking.
   - Streamlined staged install testing. Reduced runtime from 15.62s to 0.44s.
5. **PTY terminal tests (`tests/run.sh`, `tests/pager_screen.py`, `tests/external_links.py`)**:
   - Replaced fixed `time.sleep()` delays throughout PTY interactive tests with responsive `select.select()` drains using a 5ms quiet window.
   - Reduced `tests/run.sh` from 72.82s to 9.19s while keeping all interactive assertions.

## Baseline and dependencies

- Branch: `develop`
- Command: `0.48.2` (`src/version.h`)
- Corpus: `0.63.0` (`knowledge/version`)
- Measured on the workstation.

## Acceptance and limits

1. `make test` completes cleanly in 15 seconds or less (achieved: 14.51s).
2. All command, corpus, golden, linter, and upgrade assertions continue to pass without error.
3. Generated index files (`knowledge/index.*`, `coverage.json`) remain identical to unoptimized builds.
