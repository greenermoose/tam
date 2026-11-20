# Pager reading depth keys and retired 1–9 link jumping (command 0.27.0)

- **Date:** 2026-10-16
- **Time:** 13:47 to 14:00 -04:00
- **Stage:** Develop — `tam` command 0.27.0 (`src/pager.c`, `src/version.h`, `man/tam.1`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`)
- **CLI Tool:** Antigravity CLI 1.4.0 (`agy 1.4.0`)
- **Model:** Gemini 4 Argon (`gemini-4-argon`)
- **Transcript:** Retained privately by the author.
- **Commits:** Recorded in git history on `develop`.
- **Authorship:** Fred directed retiring `1`–`9` hyperlink jumping in `src/pager.c` and binding `0`, `1`, `2`, `3` to switch the active page's reading depth (`all`, `beginner`, `intermediate`, `expert`); Antigravity CLI implemented the pager keybindings, man page update, and interactive PTY tests.
- **Checked against:** `src/pager.c`, `src/depth.h`, `src/tam.c`, `man/tam.1`, `tests/run.sh`, `knowledge/shell/pipelines.html`.

## Guiding prompts

> In pager.c, does pressing number keys 1–9 while reading a page still jump to the 1st–9th hyperlink on the page? Now that we have the attention line (0.25.0), Tab/Shift-Tab, and a/s/d/f for links, I would like to retire those number links. Instead, keys 0, 1, 2, 3 inside the interactive pager should switch the current page's reading depth (all, beginner, intermediate, expert) instead of jumping to links. Has that change already landed?

> Yes. Please see how quickly you can implement this change. It should be very straightfoward to do.

## Key decisions

- **Retiring `1`–`9` link activation and binding `0`/`1`/`2`/`3` to `switch_depth()`:** `switch_depth()` strips any existing `?level=` or `&level=` parameter from `st->cur_href`, compares the active depth bit against the requested depth (`0` = all levels, `1` = `beginner`, `2` = `intermediate`, `3` = `expert`), and loads the updated href through `st->load`, pushing the prior `{ href, top, cursor_line }` onto navigation history. Pages that do not support depth filtering (`man:`, `info:`, `-`, `<stdin>`) or do not offer the requested depth report a concise status message (`[depth levels not available on this page]` or `[no <level> level on this page]`) without leaving the current page.

## Verification

- `make test` (with `PREFIX` and `LIBTAM_PREFIX` set to the local development prefix): `tests/run.sh`, `tests/render.sh`, `tools/lint-corpus.py`, `tests/test_lint_corpus.py`, and `tests/guide.sh` all pass.
- Installed development build via `make install`.
