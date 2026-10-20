# Home-row pager keys and attention column (command 0.32.0, corpus 0.33.5)

- **Date:** 2026-10-20
- **Time:** 06:36 to 07:10 -04:00
- **Stage:** Develop; TCK10 (home-row pager keys), decided as Tamlinux D175
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `a79e490`, `45c6683` and the commit that adds this record
- **Authorship:** The key layout, the attention column and its clearing rule
  are the developer's idea, from his own note. The AI tool read the pager,
  proposed the bottom-row top/bottom keys, asked the open design questions,
  and wrote the code, tests and documentation.
- **Checked against:** `src/pager.c` (`handle_key`, `set_cursor_line`, link
  collection, `emit_line`), `tests/run.sh`, `tests/pager_screen.py`,
  `man/tam.1`, `README.md`, `knowledge/welcome.html`, `src/info.c`

## Guiding prompts

> Read my idea in [redacted: private repository] about tam pager nav keys. What do you think? Do you understand it? Can you imagine how that would work?

> Please ponder G and suggest a solution. I'm wondering if there are other keys that could be used for home and end, preferably ones that don't require a shift. Are there a pair that are currently unused that could be pressed into service to be synonyms for Home and End? I want to retire G since g is now used for going to follow a link.

> I like it. Make it so.

> Build it.

## Key decisions

- The developer kept `j` = left and `k` = down as written: each finger does
  the same thing on both hands, rather than following the `less`/vim habit.
  The README warns vim users.
- The column moves from link to link (not cell by cell or word by word), and
  up/down move one row (not to the next line with a link). Both are the
  developer's answers.
- Top and bottom: `x`/`.` and `c`/`,`, the fingers that move one line reaching
  one row down. `,` = top and `.` = bottom, echoing `less`'s `<`/`>`, were
  rejected because they reverse the finger rule on the right hand.
- `s`/`l` and `d`/`k` share one function with Up and Down, so the breadcrumb
  row behaves as before. `select_vertical_link()` is removed.

## Verification

`make test`: all checks pass. Skipped: live `tam ask` (no local Ollama) and
the staged install (`make` unavailable inside that test). New checks cover
every key, wrapping, lines without links, retired `G`, `g` following, and the
band's cells and clearing in the screen model. `make corpus` was rerun after
the index commit. The developer's interactive review is pending.
