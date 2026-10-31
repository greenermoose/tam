# HTML help page and back-on-help navigation (command 0.46.0)

- **Date:** 2026-10-30
- **Time:** 15:10 to 17:15 -04:00
- **Stage:** Develop; pager help architecture and navigation
- **CLI Tool:** Antigravity CLI 1.4.11
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** Fred Horch directed changing how the help screen works for tam command 0.46.0: making h show help/index.html, pulling the help screen out of pager_draw.c and rewriting it as a regular HTML file that the help keys show (h, ?), making the content display and navigation work like any other page, with the exception that pressing a help key from the help page takes the reader back to the page they were on when you invoked help. Antigravity CLI implemented the changes across the C pager codebase, created `knowledge/help/index.html` and fixture files, updated manual pages, roadmap and decision records, and wrote integration and screen model tests.
- **Checked against:** `src/pager_draw.c`, `src/pager_input.c`, `src/pager_nav.c`, `src/pager_int.h`, `src/version.h`, `man/tam.1`, `knowledge/help/index.html`, `tests/fixtures/guide/help/index.html`, `tests/fixtures/help/index.html`, `tests/run.sh`, `tests/pager_screen.py`, `tools/check-repo.py`

## Guiding prompts

> For tam command 0.46.0, change how the help screen works. Make h show help/index.html. Pull the help screen out of pager_draw.c and rewrite it as a regular HTML file that the help keys show (h, ?). Make the content dispay and navigation work like any other page, with the exception that pressing a help key from the help page takes you back to the page you were on when you invoked help. Ask if you have any questions.

## Key decisions

- **Decoupled HTML help page:** Removed the hardcoded modal help overlay array and loop skip from `src/pager_draw.c` and retired `st->help_open` in `src/pager_int.h`. Authored `knowledge/help/index.html` as a structured HTML document with breadcrumb, table of keys, and descriptions. Added matching test fixtures in `tests/fixtures/guide/help/index.html` and `tests/fixtures/help/index.html`.
- **Help key toggle & return navigation:** In `src/pager_input.c`, keys `h` and `?` now inspect `is_help_page(st->cur_href)`. When on any other page, pressing `h` or `?` invokes `follow(st, "help/index.html")`, which pushes the originating page and viewport coordinates onto `st->history`. When already on the help page, pressing `h` or `?` invokes `pager_return_from_help(st)`, unwinding any intermediate help history entries and restoring the exact page, attention line, and scroll position from when help was invoked.
- **Regular navigation on help:** Navigation keys (arrows, `d`/`k`, `s`/`l`, `x`/`c`, `Tab`/`]`/`[`, `/` search) function identically on the help page as on any other page. `tam:help` action links route to `help/index.html`.
- **Check-repo roadmap validation:** Updated `tools/check-repo.py` to recognize `ROADMAP.md`'s `## 3. Delivered history` section heading alongside legacy `## 2. Delivered history` following earlier roadmap reorganization.

## Verification

- `make command`: Clean build of binary `build/tam` at command version 0.46.0.
- `python3 tests/pager_screen.py build/tam tests/fixtures`: Verified interactive screen model tests, including cursor movement on the help page and return to the caller on `?`.
- `tests/run.sh build/tam`: All integration checks passed, including interactive PTY tests for help invocation, navigation on the help page, return via `h` and `?`, and version assertions.
- `tools/check-repo.py`: Clean validation of copied facts and delivered history versions.
- `make test`: Completed successfully.
