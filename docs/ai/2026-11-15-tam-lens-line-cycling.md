# Simplify lens lines and cycle lenses 0–3 on lens lines (command 0.59.8)

- **Date:** 2026-11-15
- **Time:** 10:51 to 10:56 -05:00
- **Stage:** Develop; `src/depth.c`, `src/depth.h`, `src/pager_int.h`, `src/pager_nav.c`, `src/pager_input.c`, `src/version.h`, `tests/run.sh`, `tests/render/blocks.txt`, `tests/render/pedagogy.txt`, `ROADMAP.md`, `docs/plans/development.md`, `AI_PROVENANCE.md`
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit for this record
- **Authorship:** Fred directed the change; Antigravity CLI implemented the changes, tests, version bump, and documentation updates.
- **Checked against:** `src/depth.c`, `src/depth.h`, `src/pager_nav.c`, `src/pager_input.c`, `src/pager_int.h`, `src/version.h`, `tests/run.sh`, `tests/render/blocks.txt`, `tests/render/pedagogy.txt`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> Change the tam pager when it is on a lens line as follows:
>
> 1) On line Lb, instead of displaying
>
> 0 Showing all text
>
> with all in the lens color, display
>
> Showing all text
>
> with all text in the lens color.
>
> 2) When the attention line is on a lens line, change what go back (Left Arrow, a, ;) and activate (Right Arrow key, f, j, Return key) do to cycle through lenses 0 to 3. Go back decrements and wraps 0 to 3; active increments and wraps 3 to 0.
>
> 3) For lens lines in the page, e.g.
>
> 23 hide novice text
>
> display it instead as
>
> Showing novice text
>
> or
>
> Hiding novice text
>
> Go back or activate keys do the same cycling, in effect toggling this section on or off depending on lens value.

## Key decisions

None beyond the prompts. `src/depth.c` emits `Showing <span class="lvl-%u">%s text</span>` on line Lb and `Showing <span class="lvl-%d">%s text</span>` / `Hiding <span class="lvl-%d">%s text</span>` on in-page section lens lines. `lens_line_here` in `src/pager_nav.c` detects any line carrying `TAM_STYLE_LVL_MASK` (when the breadcrumb row is not focused), and `cycle_lens` steps the lens 0–3 in either direction while keeping the attention line anchored to the same lens line across the in-place reload.

## Verification

`make -C tam check-repo`, `tests/render.sh build/tam`, and `tests/run.sh build/tam` (with `tests/run.sh` passing all interactive pager, lens, render, and CLI checks up to the sandboxed `storage_btrfs` host check).
