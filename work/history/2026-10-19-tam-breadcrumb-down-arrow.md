# Down Arrow leaves the breadcrumb row (command 0.31.4)

- **Date:** 2026-10-19
- **Time:** 23:00 to 23:20 -04:00
- **Stage:** Develop; `src/pager.c`, pager tests, `tam(1)`, development contract
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer reported the stuck arrows and chose the
  behavior. Claude Code found the cause, wrote the fix and the regression test.
- **Checked against:** `src/pager.c` (`handle_key`, `select_vertical_link`,
  `set_cursor_line`, `link_is_visible`), `knowledge/ecosystem/os/fedora-family.html`
  (breadcrumb links only), `man/tam.1`, `docs/plans/development.md` 0.31.1
  entry, `tests/run.sh`, `make test`

## Guiding prompts

> I've got my tam pager wedged. Up and down arrows aren't working. Can you check
> to see what's happening with that? [...] Up and Down Arrow work on other
> pages. They just don't work on the page I'm on now (The Fedora Family). They
> were working, but then they stopped.

> Yes, make the change. You are right, the bug happens when you scroll all the
> way up and the Guide hyperlink gets selected. Then the Down Arrow only works
> if it can find a hyperlink on the page, and rather than taking you to the
> first line in the file, it takes you to the line with the first hyperlink.
> [...] I want the Up and Down Arrows to always allow you to move the attention
> line up and down. This Guide thing is a bug.

Asked whether Up on the first line should still focus the breadcrumb, the
developer answered: "Yes, keep it; fix Down".

## Cause

Up on the first line focuses the last breadcrumb link (0.31.1). While a
breadcrumb link was focused, Down called `select_vertical_link`, which jumps to
the first page link. A page with no page links left the breadcrumb focused,
and because breadcrumb links always count as visible, Home, End and the page
keys never cleared it, so Up and Down stayed stuck.

## Key decisions

- Down and `j` on the breadcrumb row clear the selection and return to the
  attention line where it was; that is the first line when Up got there, or the
  line the reader left when Tab or Shift-Tab reached the row.
- Up into the breadcrumb, `a`/`f` across it and `s`/`d` between breadcrumb and
  page links are unchanged.

## Verification

`make test` passed. New PTY case on `tests/fixtures/breadcrumb-only.html`:
Up focuses the breadcrumb, Down returns to L1, then Down and Up move the line;
from L3, Shift-Tab then Down or `j` returns to L3.
