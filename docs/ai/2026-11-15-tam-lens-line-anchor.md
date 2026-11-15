# Keep attention line and viewport anchored on lens lines (command 0.59.9)

- **Date:** 2026-11-15
- **Time:** 11:01 to 11:04 -05:00
- **Stage:** Develop; `src/pager_nav.c`, `src/pager_input.c`, `src/version.h`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`, `AI_PROVENANCE.md`
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit for this record
- **Authorship:** Fred reported the viewport jump; Antigravity CLI diagnosed the stale `max_top` clamp and generalized lens-line anchoring to `set_lens`.
- **Checked against:** `src/pager_nav.c`, `src/pager_input.c`, `src/pager_links.c`, `src/version.h`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> If you change lens when you are on a lens line in the content of a page, keep attention on the same line and keep the screen showing the attention line. Otherwise, it gets confusing. I had attention on the expert text lens line in desktop/session/start.html, and when I rotated through lenses with right and left the screen display jumped all around and I lost my place.

## Key decisions

None beyond the prompts. `handle_key_inner` in `src/pager_input.c` previously computed `max_top` before `switch (c)`, so reloading a longer page (e.g. switching from Expert to All on `desktop/session/start.html`) clamped `st->top` back to the shorter page's `max_top` after `cycle_lens` had scrolled to the lens line. Moving the `max_top` calculation after `switch (c)` eliminates the stale clamp, and moving lens-line anchoring into `set_lens` in `src/pager_nav.c` keeps the attention line and viewport anchored for both cycling keys and `0–3`.

## Verification

`make -C tam check-repo`, `tests/render.sh build/tam`, PTY verification on `desktop/session/start.html`, and `tests/run.sh build/tam` (passing all interactive pager, lens, render, and CLI checks up to the sandboxed `storage_btrfs` host check).
