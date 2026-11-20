# Step through multiple links on a line with Down and Up (command 0.56.0)

- **Date:** 2026-11-14
- **Time:** 11:39 to 11:51 -05:00
- **Stage:** Develop — step through multiple active elements on the attention line with `Down` and `Up` in `src/pager_links.c` (`command 0.56.0`)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending
- **Authorship:** Fred directed the change to `Down` and `Up` navigation and confirmed breadcrumb and page-scroll behavior; Antigravity CLI implemented `step_link_on_line` and updated `move_line_up`/`move_line_down` in `src/pager_links.c`, updated tests in `tests/run.sh`, bumped the command version to `0.56.0`, and updated `ROADMAP.md` and `docs/plans/development.md`.
- **Checked against:** `src/pager_links.c`, `src/pager_input.c`, `src/pager_int.h`, `src/fold.c`, `src/version.h`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> /plan Change how down works in the tam command. Currently, when you move the attention line down, it moves down and highlights the first link on that line. If you are on a line with multiple lines, moving down does not jump and highlight the next link. Instead, it simply moves down to the next line.
> ...
> 1) If the current attention line has zero or one link or other active element, move to the next line down if there is a next line in the page.
> 2) If the current line has more than one active element, select the next element on that line, unless the last active element is already selected, in which case, move to the next line down if there is one.
> Have up work the same way in reverse.
> The result is that you can navigate through every page and select every active element (questions, links, etc.) simply by using the down and up keys.

## Key decisions

- `step_link_on_line` scans `st->links` on `st->cursor_line` so both ordinary links and folded questions (`tam:fold:<id>`) are visited in order by `Down`/`d`/`k` and in reverse by `Up`/`s`/`l` before advancing to the next/previous line.
- When the breadcrumb row has multiple links, `Down` steps forward through breadcrumb links and leaves to the page line after the last one (mirroring `Up`).
- `PgDn`/`Space` and `PgUp`/`b` remain full-page scroll keys.

## Verification

- `make command`: built `build/tam` with zero warnings (linked against local `libtam/build`).
- `make test-command`: `tests/run.sh` (all command integration tests) and `test_upgrade.py` (15 tests) passed; `tests/render.sh` failed on pre-existing `blocks.txt` (`[1 Novice] Beginner section` vs `Beginner section`), unrelated to this change.
- `make check-repo`: passed.
