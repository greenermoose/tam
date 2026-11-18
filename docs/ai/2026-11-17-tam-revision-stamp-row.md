# Pager revision stamp row (command 0.61.1)

- **Date:** 2026-11-16 to 2026-11-17
- **Time:** 22:46 -05:00 to 20:06 -05:00 (planning on 2026-11-16; implementation and verification on 2026-11-17)
- **Stage:** Develop. The `tam` pager draws the `Rev N Last updated` stamp as fixed chrome instead of page text.
- **CLI Tool:** opencode 2.2.5
- **Model:** Big Pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `3b75c70` (source), and the commit that adds this record (tests, fixtures, roadmap, decision record).
- **Authorship:** The developer asked for the stamp's presentation to change and decided where it sits, which version bump it takes, and that losing pager searchability is acceptable. The AI tool wrote the code, fixture, tests, and records.
- **Checked against:** `src/depth.c`, `src/depth.h`, `src/corpus.c`, `src/render.c`, `src/render.h`, `src/tam.c`, `src/pager_draw.c`, `src/pager_screen.c`, `tests/pager_screen.py`, `tests/run.sh`, `tests/guide.sh`, `tools/corpus_pipeline.py`, `tam show --pipe` output, and `tam --version`.

## Guiding prompts

> In the tam pager, change how the revision and last updated line is displayed. That line is not included in the HTML but is added automatically by tam command.
>
> It is currently displayed in the same color as the page contents, left aligned, and counts as the last line of the page. You can move the attention line over it.
>
> Change it so that it does not count in the line length of the page, you cannot move the attention line to it, and it is displayed in a more subtle color (same as the Showing <lens> line) aligned right.

> Stop being searchable is fine

> make it so

## Key decisions

- The stamp is pager chrome, not a page line: it is carried on the document
  (`tam_document_t.revision`) rather than injected as a `<p>`, so it is absent
  from `doc->lines`, the footer line total, the attention line, and search.
- It is drawn on the row just above the two footer rows, dim and right-aligned
  like the `Showing <lens>` line, on the normal background.
- `content_rows()` reserves that row only when the page has a stamp, and only
  when the page has a breadcrumb row; without a breadcrumb it uses the row that
  was already spare. Terminals shorter than five rows drop the stamp so the
  page keeps one line. A narrow terminal keeps the newest words after a leading
  ellipsis, falling back to tail characters.
- `tam_document_text()` still prints the stamp once, unchanged in position, so
  `--pipe`, the corpus build's reader check, and `tests/guide.sh` keep working.
- Renamed `tam_depth_apply_rev()` to `tam_depth_prepare()` because it no longer
  handles the revision.

## Verification

`make command` builds clean. `make test-command LIBTAM_PREFIX="$HOME/.local"`
passes the command suite, upgrade tests, renderer goldens, and `check-repo`.
The PTY screen model asserts the stamp's row, right alignment, dim attribute,
absence of the footer tint, that the footer total excludes it, that it does
not scroll, and that a four-row terminal omits it. `make test-command` without
`LIBTAM_PREFIX` still fails in `tests/test_documentation.py`, whose default
library prefix remains `~/.local/dev`; that is a pre-existing stale default,
not this change.
