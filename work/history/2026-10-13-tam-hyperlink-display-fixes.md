# tam hyperlink display fixes (tam 0.13.0)

- **Date:** 2026-10-13
- **Stage:** Develop — hyperlink first-space underline and wrapped line continuation fixes implemented; version bumped to 0.13.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.4
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested fixing two bugs in how hyperlinks are displayed for `tam` version 0.13.0:
  1. Do not underline the first space in a hyperlink; start the underline with the first non-space character.
  2. A hyperlink that wraps lines should be a single hyperlink, not two separate hyperlinks, highlighting the continuation of the hyperlinked text on the next line rather than ending the highlighting at the line break for wrapped text.
  Antigravity tagged each `<a>` anchor with a discrete `link_id` in `src/render.c` and `src/render.h`, updated `src/pager.c` to merge multi-line spans of the same hyperlink into single link references, highlighted all lines of a wrapped hyperlink upon selection, skipped leading whitespace when activating hyperlink underline/highlight styles in `emit_line`, bumped `TAM_COMMAND_VERSION` to `0.13.0` in `src/version.h`, added automated PTY interactive tests in `tests/run.sh`, and updated documentation.

## Guiding prompts

> For tam version 0.13.0, fix two bugs in how hyperlinks are displayed:
> 
> 1) Do not underline the first space in a hyperlink. Start the underline with the first non-space character.
> 2) A hyperlink that wraps lines should be a single hyperlink, not two separate hyperlinks. You will need to highlight the continuation of the hyperlinked text on the next line rather than ending the highlighting at the line break for wrapped text.
> 
> Ask if you quesions about this project.

## Decisions and scope

- **Hyperlink Identity Tracking (`link_id`):** In `src/render.h`, added `int link_id` to `tam_span_t`. In `src/render.c`, `parser_t` assigns an incrementing `link_id` on every opening `<a>` tag and resets it to `0` upon `</a>`. `spanlist_append()` propagates `link_id` to spans, and layout engines (`layout_flow`, `cell_wrap`, `layout_table`, `layout_pre`) preserve `link_id` across lines when text wraps.
- **Single Hyperlink for Wrapped Links:** In `src/pager.c`, `pager_build_links()` checks if an incoming link span belongs to the same anchor instance as the prior link (`sp->link_id == prev->link_id`, or matching `href` across consecutive lines). If so, it updates `line_end` on the existing link instead of appending a new link entry. This guarantees:
  1. Wrapped hyperlinks are counted as a single hyperlink.
  2. Tab and Shift-Tab navigate to next/previous hyperlinks across the document without stopping on wrapped line continuations.
- **Continuation Highlighting Across Lines:** In `src/pager.c`, `emit_line()` determines whether a span is selected by comparing `sp->link_id == sel->link_id` (or matching `href`). When a wrapped link is selected, the starting line and all continuation lines receive the selection highlight (`\033[7;1m`), rather than truncating at the line break.
- **Leading-Space Underline Prevention:** In `src/pager.c`, `emit_line()` prints any leading whitespace in a hyperlink span unstyled before calling `emit_style()`. Underline (`\033[4;34m`) and selection reverse-video (`\033[7;1m`) begin strictly on the first non-space character, preventing underlined spaces before link text and inside reverse-video selection blocks.
- **Version bump:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` to `0.13.0`.
- **Documentation:** Updated `AI_PROVENANCE.md` and `docs/plans/development.md`.

## Verification

- `tests/run.sh` passes all command tests, including an automated PTY test verifying:
  1. No unselected hyperlink underlines leading whitespace; underline begins with the first non-space character.
  2. Tab selects the wrapped link and highlights both the initial line and the second line continuation.
  3. Leading space before the wrapped link is not highlighted in reverse video.
  4. Tab advances to the subsequent link, proving the wrapped link is treated as a single entity.
  5. Shift-Tab circulates backward to the wrapped link, highlighting all lines.
- `tests/guide.sh` passes all guide rendering and golden snapshot checks.
- `make` and `make test` pass cleanly with zero warnings or errors.
