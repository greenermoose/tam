# Pager bracket link selection keys (command 0.54.2 / corpus 0.78.22)

- **Date:** 2026-11-12
- **Time:** 06:29 to 10:55 -05:00
- **Stage:** Develop; command 0.54.2 maps `]` to select next link (Tab synonym) and `[` to select previous link (Shift+Tab synonym), and corpus 0.78.22 updates documentation.
- **CLI Tool:** Antigravity CLI 1.5.6
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit
- **Authorship:** Fred Horch directed making `]` work like Tab (select the next link) and `[` work like Shift+Tab (select the previous link). Antigravity CLI implemented the key bindings in `src/pager_input.c`, updated the status line hint in `src/pager_draw.c`, updated documentation in `man/tam.1`, `knowledge/help/index.html`, and `knowledge/learn/paths/first-steps.html`, updated test assertions in `tests/run.sh`, `tests/pager_screen.py`, and `tests/external_links.py`, bumped command version to 0.54.2 and corpus version to 0.78.22, and verified the test suites.
- **Checked against:** `src/pager_input.c`, `src/pager_draw.c`, `src/version.h`, `man/tam.1`, `knowledge/help/index.html`, `knowledge/learn/paths/first-steps.html`, `tests/run.sh`, `tests/pager_screen.py`, `tests/external_links.py`, `ROADMAP.md`, `docs/plans/development.md`.

## Guiding prompts

> For the tam pager, make ] work like Tab: select the next link. Make [ work like Shift + Tab: select the previous link.

## Key decisions

- Key `]` invokes `select_next_link(st, 1, 1)`: selects the next link in document order, circulating across all lines and breadcrumbs.
- Key `[` invokes `select_next_link(st, -1, 1)`: selects the previous link in document order, circulating backwards across all lines and breadcrumbs.
- Both keys match the existing Tab and Shift-Tab semantics completely, allowing convenient single-key link selection without two-key chords.
- Restored `"[] Select links"` on the status line in `src/pager_draw.c` and `man/tam.1`.
- Updated navigation help table (`knowledge/help/index.html`, `tests/fixtures/guide/help/index.html`, `tests/fixtures/help/index.html`) to show `Tab, ]` and `Shift-Tab, [`.
- Updated first steps guide (`knowledge/learn/paths/first-steps.html`) to explain `Tab or ]` and `Shift + Tab or [`.

## Verification

- `tests/run.sh build/tam`: All command integration tests passed, including link cycling with `]` and `[`.
- `python3 tests/pager_screen.py build/tam tests/fixtures`: Screen model test passed with `]` and `[` link selection.
- `python3 tests/external_links.py build/tam tests/fixtures`: External link tests passed.
- `make test-command`: Integration tests, upgrade tests, renderer goldens, and `check-repo.py` passed cleanly.
