# make dev version drift (corpus 0.46.1)

- **Date:** 2026-10-28
- **Time:** 08:47 to 09:05 -04:00
- **Stage:** Develop; developer loop, corpus checks and a corpus patch
- **CLI Tool:** Claude Code 2.1.312
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** The commit that adds this record and the index follow-up.
- **Authorship:** Fred reported a `make dev` failure and asked for the root cause to be fixed; Claude Code wrote the changes and this record.
- **Checked against:** `tools/dev.py`, `tools/check-repo.py`, `tools/build-index.py`, `tests/run.sh`, `Makefile`, `ROADMAP.md`, the corpus 0.46.0 commit and its session record.

## Guiding prompts

> I just tried make dev for tam and ran into this error. Please investigate and fix the root cause.

## Key decisions

The corpus 0.46.0 commit bumped `knowledge/version` and the roadmap header by hand but left the `develop` row at 0.45.5 and added no history row. It was verified with `make corpus-and-test`, which did not run `tools/check-repo.py`, so the drift reached `develop`. The same commit's topic `boot phases` (a space) and its `desktop` keyword broke two `tests/run.sh` checks.

- `make dev` stages every version edit in memory and writes only after all of them match. Before, a failed edit left the earlier ones on disk although it reported nothing committed.
- `make corpus-and-test` now runs `tools/check-repo.py`, and `tools/build-index.py` rejects a topic that is not a slug.
- Corpus 0.46.1 renames the topic to `boot-phases` and drops the `desktop` keyword; Fred chose that over moving the live-corpus check to a fixture.

## Verification

`make test`: both suites pass, including the new `tests/test_dev.py` and `tests/test_build_index.py` cases. `tools/check-repo.py` reports no findings.
