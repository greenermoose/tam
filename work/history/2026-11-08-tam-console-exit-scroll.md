# Console exit leaves a fresh prompt line (command 0.50.3)

- **Date:** 2026-11-08
- **Time:** 20:37 to 20:50 -05:00
- **Stage:** Develop; command 0.50.2 to 0.50.3 on `develop`
- **CLI Tool:** Claude Code 2.1.319
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `786c2eb` and the commit that adds this record
- **Authorship:** Fred Horch reported the behavior from a real Linux VT and
  chose the exit behavior; the AI tool diagnosed it and wrote the change and
  its test.
- **Checked against:** `src/terminal.c`, `src/pager.c`, `src/pager_draw.c`,
  `infocmp linux` (no `smcup`/`rmcup`), `tests/pager_screen.py`

## Guiding prompts

> hi opens tam without starting a desktop. One bug: when you quit, it does not clear the screen.

> I actually prefer on a kernel console when you quit a program that the screen not clear, but that it cleanly scrolls up and you get a prompt. In other words, it's nice to be able to see what you were just doing, but also be sure that you are at a prompt with the cursor and clear space to type your next command. Can you make tam work like that? I think more does that sometimes and maybe not always.

## Key decisions

The Linux console's terminfo entry has no alternate screen, so the pager's
last frame stays on the console after exit, and the cursor was left wherever
that frame put it. The developer rejected clearing the screen in favor of
keeping the page visible. Without an alternate screen, `tam_terminal_leave`
now moves to the bottom row and writes a newline: the page scrolls up one
line and the prompt starts on an empty bottom line. The same path runs when
the pager hands the terminal to another program, which then also starts on
a fresh line. Terminals with an alternate screen are unchanged.

## Verification

`tests/pager_screen.py` gained a `TERM=linux` exit check, and its screen
model now scrolls on a newline at the bottom row. The check fails against
the 0.50.2 `terminal.c` and passes with the change. `make check-repo` and
`make test` pass (61 checks).
