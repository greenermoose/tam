# Hidden difficulty level notices (command 0.49.0)

- **Date:** 2026-11-06
- **Time:** 10:57 to 11:46 -05:00
- **Stage:** Develop; command 0.49.0 displays right-aligned hiding notices (`Hiding novice text (01 show)`, `Hiding learner text (02 show)`, `Hiding expert text (03 show)`) in subdued style with corresponding level color highlighting when difficulty levels are filtered out, and overrides Right Arrow, Return, and `g` on hiding lines to show all (0).
- **CLI Tool:** Antigravity CLI 1.5.2
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `c8c4630`, `36df1e7`, and the commit that updates this record
- **Authorship:** Fred Horch requested replacing the old hidden section headers (`-- 1 Novice: Novice definition (hidden; press 1 or 0) --`) on multi-level pages with right-aligned hiding notices (`Hiding novice text (01 show)`, `Hiding learner text (02 show)`, `Hiding expert text (03 show)`) styled like line Lb with subdued text and level colors, bumping tam command version to 0.49.0, and making Right Arrow, Return, and `g` act like `0` (show all) when the attention line is on a hiding line rather than following links. Antigravity designed the `<p class="level-line">` notice emission with `lvl-1`, `lvl-2`, `lvl-3` spans in `src/depth.c`, adjusted layout document block tightness in `src/render.c` so consecutive notices remain stacked while section content is spaced with blank lines, implemented `hiding_line_here` in `src/pager_nav.c` and wired Right Arrow, `\r`/`\n`, and `g` in `src/pager_input.c` to call `set_lens(st, '0')`, updated tests in `tests/run.sh`, bumped `TAM_COMMAND_VERSION` to 0.49.0 in `src/version.h`, and updated documentation.
- **Checked against:** `src/depth.c`, `src/render.c`, `src/version.h`, `src/pager_int.h`, `src/pager_nav.c`, `src/pager_input.c`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`.

## Guiding prompts

> In tam, change how hidden difficulty level text is displayed.
>
> When you are on a page that has novice, learner, and expert text sections and you are displaying just 1, 2, or 3 (novice, learner, or expert difficulty), instead of
>
> -- 1 Novice: Novice definition (hidden; press 1 or 0) --
>
> show aligned right a hiding notice:
>
>                                  Hiding novice text (01 show)
>
> Display that in the same style as the Showing 2: learner level text on line Lb. I.e., not in the same font color as regular text, but a little more subdued.
>
> The notices should read
>
>                   Hiding novice text (01 show)
>                  Hiding learner text (02 show)
>                   Hiding expert text (03 show)
>
> Put novice, learner, and expert in the corresponding color (same as the 1: novice, 2: learner, and 3: expert colors in line Lb).
>
> Ask if you have questions. Bump the version of tam command from 0.48.2 to 0.49.0.

> Also, if a hiding line has attention, right arrow or Return should have the same effect as 0, i.e. show all.

> I think you just need an override when attention is on a line that says hiding, so that the right arrow, g key, and Return key act like 0 in that context and don't do their regular function of following the selected link.

## Key decisions

- Reused `<p class="level-line">` with `<span>` classes `lvl-1`, `lvl-2`, and `lvl-3` for the notices. The existing renderer treats `level-line` paragraphs as right-aligned and dims text runs that lack level color styling, matching line Lb.
- Refined document block tightness rules in `src/render.c`: consecutive `B_META` blocks (such as adjacent hiding notices) remain tight without blank lines between them, and the `<h1>` title following leading level lines remains tight, but normal body blocks following a `B_META` notice have a blank line preceding them to preserve document structure.
- Removed unused `heading_text` helper function in `src/depth.c` since hidden section notices no longer include section heading titles.
- Added `hiding_line_here(st)` predicate inspecting the spans of `st->doc->lines[st->cursor_line]`. In `src/pager_input.c`, when `hiding_line_here` is true, Right Arrow (`\x1b[C`), Return (`\r`, `\n`), and `g` trigger `set_lens(st, '0')` instead of following links or advancing history.

## Verification

- `make command`: compiles cleanly with 0 warnings or errors under `-O2 -Wall -Wextra -Wpedantic -std=c11`.
- `make test-command`: passes all integration tests in `tests/run.sh`, upgrade tests in `test_upgrade.py` (15/15), and golden rendering tests in `tests/render.sh` (7/7).
- `make test`: full test suite passes completely (including corpus linter, component coverage, build index, and guide tests).
- Verified right alignment, subdued text (`TAM_STYLE_DIM`), and ANSI color escapes (`\x1b[32m`, `\x1b[33m`, `\x1b[35m`) for all three level notices across interactive PTY and piped output.
- Verified in `tests/run.sh` that positioning attention on a hiding line and pressing Return, Right Arrow, or `g` switches lens to 0 ("Showing 0: all text") and saves state.
