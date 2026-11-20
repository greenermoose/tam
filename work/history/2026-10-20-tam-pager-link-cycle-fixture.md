# Pager link-cycle test on a fixture (no version change)

- **Date:** 2026-10-20
- **Time:** 11:40 to 11:55 -04:00
- **Stage:** Develop; test-only fix in `tests/run.sh` plus a new fixture `tests/fixtures/link-cycle/index.html`; nothing installed changes, so no version change
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer reordered the guide index links and ran `make dev`, which stopped in `make test`. Antigravity diagnosed the failing assertion; the developer asked for a fix plan and approved it. The AI tool wrote the fixture, the test change and this record.
- **Checked against:** `tests/run.sh`, `tests/guide.sh`, `tools/dev.py`, `docs/plans/dev-loop.md`, `AGENTS.md`, `Makefile`

## Guiding prompts

> make dev for a simple update to a the knowledge corpus I tried failed due to
> a stale assertion. It appears either our plan for make dev or the
> implementation of it did not anticipate this problem. Please see the report
> from agy for you about this issue. Ponder and plan a fix.

## Key decisions

- The Tab/Shift-Tab circulation test asserted the order of the last five links
  on the live guide index, so a correct corpus edit failed a command test. It
  now runs on a fixed fixture; `tests/guide.sh` still checks that the live
  index carries every required link, in any order.
- The fixture is the root `index.html` of its own directory. On any other page
  Shift-Tab from the first link focuses the breadcrumb before wrapping, which
  would change what the test exercises.
- Other `tests/run.sh` pager tests still read live page titles, link order and
  breadcrumbs. Moving them to fixtures, a `check-repo` guard, and a clearer
  `make dev` failure message are planned as separate slices.

## Verification

`make test` passes with the developer's uncommitted `knowledge/index.html`
edit in the tree. Swapping two fixture links makes the test fail
(`expected info:dir, got tam:man`); restoring them passes.
