# Pager page scrolling and screen overlap (command 0.47.0)

- **Date:** 2026-10-31
- **Time:** 2026-10-30 21:58 to 2026-10-31 07:05 -04:00
- **Stage:** Develop; command 0.47.0
- **CLI Tool:** Antigravity CLI 1.4.12
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending shared-checkout commit
- **Authorship:** Fred Horch directed scrolling a page with one line of screen overlap on Space/Page Down and Shift-Space/Page Up, showing the last/first full page when not enough lines remain, and confirmed attention row placement, `b` parity, Shift-Space escape decoding, and single-screen document behavior. Antigravity CLI implemented `pager_page_down` and `pager_page_up` in `src/pager_links.c`, bound keys and escape sequences in `src/pager_input.c`, updated `man/tam.1`, bumped command version to 0.47.0, and added automated tests in `tests/pager_screen.py`.
- **Checked against:** `src/pager_links.c`, `src/pager_input.c`, `src/pager_draw.c`, `src/pager_int.h`, `man/tam.1`, `tests/pager_screen.py`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`.

## Guiding prompts

> In tam, when you press space or Page Down, scroll down a page so that if there are enough lines in the file, the last line showing on the screen becomes the first line. If there aren't enough remaining lines in the file, show the last full page of the file. Make Shift-Space and Page Up work the same way going toward the beginning of the file.
>
> Currently, space or Page Down just moves the attention row down, it doesn't scroll on first press. These keys should scroll the lines displayed on first press on a screen.
>
> Ask if you have questions about this change in functionality for paging through a page.

> 1 A
> 2 yes
> 3 let's find out
> 4 move attention line

> Are you doing anything or just pretending?

## Key decisions

- **Step and overlap math:** For `usable` content rows, step size is `usable - 1` (minimum 1). For page down, `new_top = top + step`; if `new_top > max_top`, `top = max_top` to display the last full page. For page up, `new_top = top - step`; if `new_top <= 0`, `top = 0` to display the first full page.
- **Attention row placement:** On scrolling, attention row lands on the first visible line (`top`) of the new screen (which is the overlap line on page down). When already at the boundary or on single-screen documents, page down moves attention to the document end, and page up moves attention to the document top.
- **Key bindings:** Bound `Space` and `\x1b[6~` (Page Down) to `pager_page_down`. Bound `\x1b[5~` (Page Up), `b`, and Shift-Space escape sequences (Kitty CSI u `\x1b[32;2u` / `\x1b[32;2:1u`, xterm modifyOtherKeys `\x1b[27;2;32~` / `\x1b[32;2~`) to `pager_page_up`.

## Verification

- `make command`: Clean build with zero compiler warnings.
- `python3 tests/pager_screen.py build/tam tests/fixtures`: Verified interactive page-down, page-up, overlap line assertions, boundary clamping, Shift-Space escape sequences, and single-screen documents.
- `make test-command`: Full command test suite (`tests/run.sh`, `test_upgrade.py`, `tests/render.sh`) passed completely.
