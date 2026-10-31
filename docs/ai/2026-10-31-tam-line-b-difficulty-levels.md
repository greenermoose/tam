# Line b difficulty level display and highlighting (command 0.48.0)

- **Date:** 2026-10-31
- **Time:** 09:48 to 10:22 -04:00
- **Stage:** Develop; command 0.48.0 displays `Showing 0: all text`, `Showing 1: beginner level text`, `Showing 2: intermediate level text`, and `Showing 3: expert level text` on line b depending on selected difficulty level; highlights level tokens in cyan, green, yellow, and magenta with bold monochrome fallback; preserves right alignment and line c related links.
- **CLI Tool:** Antigravity CLI 1.4.12
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit
- **Authorship:** Fred Horch requested the difficulty level display strings on line b, key selection behavior, and distinct color highlighting, and decided to maintain right alignment, drop the old path and missing notes on line b, and preserve line c related links. Antigravity designed the implementation, inline style flags, span parser handling, terminal color emitter, tests, and documentation.
- **Checked against:** `src/depth.c`, `src/depth.h`, `src/render.h`, `src/render.c`, `src/pager_draw.c`, `src/version.h`, `tests/run.sh`, `tests/render.sh`, `tests/render/pedagogy.txt`

## Guiding prompts

> Change the tam command to display
>
> Showing 0: all text
> Showing 1: beginner level text
> Showing 2: intermediate level text
> Showing 3: expert level text
>
> On line b depending on which difficulty level has been selected. Pressing 0, 1, 2, or 3 selects that level of difficulty.
>
> Highlight 0: all, 1: beginner, 2: intermediate, and 3: expert with a different color if possible.
>
> Ask if you have questions about this project.

> 1 right aligned
> 2 let's discuss more. I don't understand the on <Level> path link and no <Level> text note
> 3 line b should only appear on pages with declared levels
> 4 yes
>
> Let's talk about 2 before you show me the plan file.

> I accept your recommendation for question 2.
>
> Keep related links on line c.

> ready

> Satisfied. Close the plan window. Proceed.

## Key decisions

- Line b is right-aligned above the heading, matching the existing `level-line` paragraph convention.
- The previous `on <Level> path` hyperlink and `(no <Level> text)` disclaimer on line b are dropped in favor of the clean `Showing X: ... text` indicator.
- Line c (`See also -> <related>`) is retained when a page lacks text at the selected level and related pages exist.
- Line b appears only on pages declaring `<meta name="level" ...>` metadata.
- Colors: ANSI 6 (Cyan) for `0: all`, ANSI 2 (Green) for `1: beginner`, ANSI 3 (Yellow) for `2: intermediate`, and ANSI 5 (Magenta) for `3: expert`, with monochrome fallback to bold. The surrounding words `Showing ` and ` text` / ` level text` remain in dimmed metadata style.

## Verification

- `make command`: clean compile with zero warnings or errors.
- `make test-command`: all integration tests in `tests/run.sh` pass, upgrade tests pass (15/15), and renderer golden snapshot for `pedagogy.txt` updated and matches.
- PTY testing confirms ANSI color escape sequences (`\x1b[36m`, `\x1b[32m`, `\x1b[33m`, `\x1b[35m`) for the four levels, and interactive keypresses `0`, `1`, `2`, `3` immediately update line b.
