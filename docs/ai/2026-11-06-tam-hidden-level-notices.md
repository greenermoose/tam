# Hidden difficulty level notices (command 0.49.0)

- **Date:** 2026-11-06
- **Time:** 10:57 to 11:46 -05:00
- **Stage:** Develop; command 0.49.0 displays right-aligned hiding notices (`Hiding novice text (01 show)`, `Hiding learner text (02 show)`, `Hiding expert text (03 show)`) in subdued style with corresponding level color highlighting when difficulty levels are filtered out.
- **CLI Tool:** Antigravity CLI 1.5.2
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** and the commit that adds this record
- **Authorship:** Fred Horch requested replacing the old hidden section headers (`-- 1 Novice: Novice definition (hidden; press 1 or 0) --`) on multi-level pages with right-aligned hiding notices (`Hiding novice text (01 show)`, `Hiding learner text (02 show)`, `Hiding expert text (03 show)`) styled like line Lb with subdued text and level colors, and bumping tam command version to 0.49.0. Antigravity designed the `<p class="level-line">` notice emission with `lvl-1`, `lvl-2`, `lvl-3` spans in `src/depth.c`, adjusted layout document block tightness in `src/render.c` so consecutive notices remain stacked while section content is spaced with blank lines, updated tests in `tests/run.sh`, bumped `TAM_COMMAND_VERSION` to 0.49.0 in `src/version.h`, and updated documentation.
- **Checked against:** `src/depth.c`, `src/render.c`, `src/version.h`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`.

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

## Key decisions

- Reused `<p class="level-line">` with `<span>` classes `lvl-1`, `lvl-2`, and `lvl-3` for the notices. The existing renderer treats `level-line` paragraphs as right-aligned and dims text runs that lack level color styling, matching line Lb.
- Refined document block tightness rules in `src/render.c`: consecutive `B_META` blocks (such as adjacent hiding notices) remain tight without blank lines between them, and the `<h1>` title following leading level lines remains tight, but normal body blocks following a `B_META` notice have a blank line preceding them to preserve document structure.
- Removed unused `heading_text` helper function in `src/depth.c` since hidden section notices no longer include section heading titles.

## Verification

- `make command`: compiles cleanly with 0 warnings or errors under `-O2 -Wall -Wextra -Wpedantic -std=c11`.
- `make test-command`: passes all integration tests in `tests/run.sh`, upgrade tests in `test_upgrade.py` (15/15), and golden rendering tests in `tests/render.sh` (7/7).
- `make test`: full test suite passes completely (including corpus linter, component coverage, build index, and guide tests).
- Verified right alignment, subdued text (`TAM_STYLE_DIM`), and ANSI color escapes (`\x1b[32m`, `\x1b[33m`, `\x1b[35m`) for all three level notices across interactive PTY and piped output.
