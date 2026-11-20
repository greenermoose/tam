# Highlight breadcrumb row as attention line when focused (command 0.59.7)

- **Date:** 2026-11-15
- **Time:** 10:36 to 10:41 -05:00
- **Stage:** Develop; `src/pager_draw.c`, `src/version.h`, `tests/pager_screen.py`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`, `AI_PROVENANCE.md`
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit for this record
- **Authorship:** Fred directed the change; Antigravity CLI implemented the fix, tests, version bump, and documentation updates.
- **Checked against:** `src/pager_draw.c`, `src/pager_links.c`, `tests/pager_screen.py`, `tests/run.sh`, `src/version.h`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> Change the tam command pager so that you can move the attention line all the way to the breadcrumb line. Right now it stops at second to top line, even though you can select links in breadcrumb. Make it so top line gets highlighted when you move attention line all the way up.

## Key decisions

None beyond the prompts. When `breadcrumb_focused(st)` is true (with `height > 2` and `location.nspans > 0`), `emit_location_line` in `src/pager_draw.c` renders the top row with `emit_attention(1)` across its full width (and `>` in the fallback gutter when enabled), while `emit_line` and `draw` suppress the attention highlight and gutter marker on content rows so only the top row is highlighted.

## Verification

`make -C tam test-command`: passed (all command integration tests, `pager_screen.py`, `test_upgrade.py`, renderer goldens, and `check-repo.py`).
