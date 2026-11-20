# In-page links jump to their target (command 0.31.5)

- **Date:** 2026-10-20
- **Time:** 05:55 to 06:20 -04:00
- **Stage:** Develop; `src/render.c`, `src/pager.c`, pager tests, development contract
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer reported the dead links and approved the plan.
  Claude Code found the cause, wrote the fix and the regression test.
- **Checked against:** `src/man.c` (`render_man_to_html`), groff `-Thtml`
  output for `ls(1)` and `bash(1)`, `src/render.c` (`parser_handle_tag`,
  `layout_document`), `src/pager.c` (`follow`, `follow_selected`,
  `set_cursor_line`), `make test`

## Guiding prompts

> In tam, I can select relative links (such as in man pages) but when I type
> Enter or Right Arrow to follow them, nothing happens. I expected to jump to
> that target in the man page. Please investigate and create a plan to fix.

## Cause

groff writes a man page's contents list as `<a href="#DESCRIPTION">` and puts
the target in the heading as `<a name="DESCRIPTION">`. The renderer kept only
`href`, so `follow` searched the page text for the first span containing the
fragment. That span was the contents link itself, on the line the reader was
already on, so the jump went nowhere.

## Key decisions

- Anchors (`id=` on any element, `name=` on `<a>`) are recorded by block, not
  by line, so width reflow keeps them valid.
- The target goes to the first content row, as in `less` and browsers, so the
  reader sees the section start, not its last visible line.
- The text search stays as a fallback for pages without anchors, but a link no
  longer matches itself and headings win.

## Verification

`make test` passed. New PTY case on `tests/fixtures/pager-anchors.html`: Enter
on a groff-style link and Right Arrow on an `id=` link each put the heading on
row 1; Left Arrow returns, also after a resize. The case fails on 0.31.4. In a
PTY, `tam man ls` and `tam man bash` jump to section and subsection headings.
