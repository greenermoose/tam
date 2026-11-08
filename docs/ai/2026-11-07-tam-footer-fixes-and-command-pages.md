# Two-row footer fixes and pages for tam-boot and tam-meter

- **Date:** 2026-11-07
- **Stage:** Develop
- **CLI Tool:** Claude Code `2.1.319`
- **Model:** `claude-opus-5-5`
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred asked Claude to finish an OpenCode attempt (OpenCode
  `2.2.5`, Beam Free, high variant) to make `make dev` pass. Claude wrote the
  footer fixes, test updates and the two corpus pages. The Guide index
  regrouping in corpus 0.75.4 is OpenCode's work, committed unchanged apart
  from one trailing space.
- **Checked against:** `src/pager_draw.c`, `tests/run.sh`,
  `tests/pager_screen.py`, `tests/external_links.py`; in the `tamlinux`
  repository `commands/tam-boot/` (`journal.c`, `report.c`, `README.md`) and
  `commands/tam-meter/` (`sensors.c`, `tam-meter.c`, `report.c`, `live.c`,
  `README.md`); `tam-boot --help`, `tam-boot`, `tam-boot list`,
  `tam-meter --help`, `tam-meter now`, `tam-meter now --json` and a short
  `tam-meter reading`, run from the packaged commands on 2026-11-07.

## Guiding prompts

> Take a look at what opencode is trying to fix so make dev works for the tam
> repo. See if you can sort it out.

> Yes, draft those pages so make dev can finish.

## Command 0.50.2

The two-row footer had shipped without test updates. The tests that OpenCode
had disabled were catching three regressions, now fixed: a long link label
pushed the destination off the row and out of its buffer; on a narrow row the
title hid the line and range; and a pending external link replaced its
confirmation prompt with the bare address. The tests now read the new rows
with their original assertions restored.

## Corpus 0.76.0

`tools/report-component-coverage.py --check` failed because the product had
gained `tam-boot` and `tam-meter` without corpus mentions. Two Maintaining
pages now teach them at all three levels with captured output. One source
finding is reported rather than resolved: in `tam-boot` 0.1.0 the "Slowest
steps" list is the first qualifying kernel pauses in boot order, not the
longest; the page states that behavior.

Verification: `make test`, `python3 tools/lint-corpus.py`,
`python3 tools/report-component-coverage.py --check`, `make check-repo` and
`git diff --check` pass. The two new pages carry learning-path warnings from
`tools/report-coverage.py --warnings`; they are on no goal path yet.
