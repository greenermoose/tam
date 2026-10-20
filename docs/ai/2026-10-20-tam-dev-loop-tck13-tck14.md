# TCK13 and TCK14: pager tests on fixtures, and `make dev` failure guidance (no version change)

- **Date:** 2026-10-20
- **Time:** 12:05 to 12:50 -04:00
- **Stage:** Develop; TCK13 (`tests/fixtures/guide/`, `tests/run.sh`, `tests/guide.sh`, `tools/check-repo.py` check 8, `tests/test_check_repo.py`, `AGENTS.md`) and TCK14 (`tools/dev.py`, `tests/test_dev.py`, plan, plan index, decision record and roadmap); repository tests and tooling only, no version change
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `ee845af`, `e8da257` and the commit that adds this record
- **Authorship:** The developer approved a three-slice plan after `make dev` stopped on a pager test that asserted the live index's link order; slice 1 is [the link-cycle fixture record](2026-10-20-tam-pager-link-cycle-fixture.md). He then directed slices 2 and 3. The AI tool wrote the fixture guide, the test, checker and `dev.py` changes, and the plan updates.
- **Checked against:** `tests/run.sh`, `tests/guide.sh`, `src/pager.c`, `src/depth.c`, `src/index.c`, `src/tam.c`, `tools/dev.py`, `tools/check-repo.py`, `Makefile`, `AGENTS.md`, `docs/plans/dev-loop.md`, `docs/plan-maintenance.md`

## Guiding prompts

> Worked great. Continue with slices 2 and 3.

## Key decisions

- One small fixture guide (`tests/fixtures/guide/`) stands in for the corpus
  for every interactive test. It includes a hand-written `index.tsv`, so topics,
  search and explain work without a build step. It lives in its own directory so
  it does not change how the existing `tests/fixtures/` pages resolve.
- Breadcrumbs come from each page's `<nav aria-label="Breadcrumb">`, and depth
  levels come from `<section data-level>` and the `level` meta, so the fixture
  pages copy those structures rather than the corpus text.
- One nav-grid assertion (`title != "Your Tamlinux User Guide"`) could never
  fail, because the fixture root has a different title. It now compares against
  the fixture root's title.
- Check 8 covers interactive tests only. About 200 non-interactive
  `tam explain`/`tam search` routing assertions in `tests/run.sh` still read
  the live corpus. Whether they are corpus contracts or command tests is the
  developer's decision; the plan records them as out of scope.
- `make dev` prints its guidance after the 40-line tail, where the terminal
  leaves it in view, rather than before it as the plan first said.

## Verification

`make test` passes: command, upgrade, render, lint, corpus unit tests,
`tests/test_dev.py` (9), `tools/check-repo.py`, `tests/test_check_repo.py` (10)
and `tests/guide.sh`. On a scratch copy whose live index has its links
reversed and whose index and Welcome are retitled, `tests/run.sh` and
`tests/guide.sh` both pass. Check 8 reports all eight live-corpus interactive
tests in the previous `tests/run.sh` and none in the new one.
