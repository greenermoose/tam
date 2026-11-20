# Optional component coverage gap check in `make dev` (no version change)

- **Date:** 2026-11-10
- **Time:** about 10:25 to 10:50 -05:00
- **Stage:** Develop; developer tooling only: `make dev` now asks whether to
  run the component coverage gap check, and skips it when declined.
  `Makefile`, `tools/dev.py`, its tests and README wording; no command or
  corpus version bump.
- **CLI Tool:** opencode v2.2.5
- **Model:** Big Pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this change and record.
- **Authorship:** Fred asked for the component coverage gap check to become
  optional in `make dev`, with a question to the developer instead of running
  on every make. OpenCode wrote the prompt in the developer loop, the make
  variable, the tests, the README wording and this record.
- **Checked against:** `Makefile` (`test` and `corpus-and-test` recipes),
  `tools/dev.py` (`prompt_coverage_gap_check`, `step_build_and_test`),
  `tests/test_dev.py`, `README.md`, `docs/plans/dev-loop.md` (plain `make`
  stays non-interactive), `docs/ai/2026-11-07-tam-footer-fixes-and-command-pages.md`
  (the gap check that stopped a `make dev` run), commit `34760c0` (tooling-only
  change without a version bump).

## Guiding prompts

> For make dev in the tam repo, make component coverage gap check optional.
> Ask the developer if he wants to run that check or not, don't run it on
> every make.

## Key decisions

- The question sits in step 5 of `tools/dev.py`, right before `make test`, and
  is asked on every run; the answer is not remembered. The default answer is
  **no**: a plain Enter skips the check, following "don't run it on every
  make". Declining passes `SKIP_COVERAGE_CHECK=1` to `make test`.
- Plain `make test` and `make corpus-and-test` still run the gap check when
  called directly: the dev-loop plan keeps plain make non-interactive, and
  the plan 26 coverage guard stays in force for agents and full runs. The new
  `SKIP_COVERAGE_CHECK` make variable (any non-empty value) skips it in those
  targets too and says so in the output.
- The component coverage *unit tests* and the `--json` map generation in
  `make corpus` are untouched; only the `--check` gap recipe became optional.
- Timing context from this checkout: the gap check was the slowest single test
  item (12.01 s of a 30.05 s `make test`; a skipped run measured 17.86 s).

## Verification

- `python3 -m unittest -q tests/test_dev.py`: 18 tests pass, including two
  new ones (the prompt's answers and retry; `step_build_and_test` passing
  `SKIP_COVERAGE_CHECK=1` to make only when the check is declined) and the
  fixture recording the variable both ways across full pty runs.
- `python3 -m unittest -q tests/test_install_guard.py tests/test_check_repo.py
  tests/test_build_index.py tests/test_component_coverage.py`: 50 tests pass.
- `make test`: passes with the gap check running (30.05 s total).
- `make test SKIP_COVERAGE_CHECK=1`: passes, prints `tam: component coverage
  gap check skipped (SKIP_COVERAGE_CHECK).` and lists no gap check in the
  timer summary (17.86 s total).
- `make -n test`, `make -n test SKIP_COVERAGE_CHECK=1` and the same for
  `corpus-and-test`: the recipes expand as intended both ways.
- `python3 tools/report-component-coverage.py --quiet --check` and
  `python3 tools/check-repo.py` pass on the changed tree; `git diff --check`
  is clean.
