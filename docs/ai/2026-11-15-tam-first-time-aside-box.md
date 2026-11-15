# First-time aside display and open box border (command 0.59.10)

- **Date:** 2026-11-15
- **Time:** 11:14 to 11:25 -05:00
- **Stage:** Develop (command 0.59.10)
- **CLI Tool:** Antigravity CLI `1.5.9`
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** added in this commit
- **Authorship:** Fred specified the closed toggle wording and open box geometry; Antigravity CLI implemented the `fold.c` and `render.c` changes, updated the render golden and integration tests, bumped the version, and recorded provenance.
- **Checked against:** `src/fold.c`, `src/render.c`, `src/version.h`, `tests/render/pedagogy.txt`, `tests/run.sh`, `Makefile`

## Guiding prompts

> In the tam pager, change how asides are displayed:
>
> 1) For an <aside class="first-time">, do not display
>
> New to this?
>
> [+] New to this? (-> to show)
>
> Instead, show
>
> [+] You may want to know... (-> to show)
>
> 2) When the aside is open, surround it with
>
> [-] You may want to know... (<- to hide) ---------------------------------------------+
>  |                                                                                    |
>  | Text goes here, with one line above and below and one space padding left and right |
>  | Size of box depends on longest line. Long lines do word wrapping as normal.        |
>  |                                                                                    |
>  +------------------------------------------------------------------------------------+

> tam: refusing to install from a dirty tree:
>    M src/fold.c
>    M src/render.c
> Commit or discard these changes first. Do not install another session's uncommitted work.
> make[1]: *** [Makefile:197: check-clean-tree] Error 1
> make: *** [Makefile:219: install] Error 2
>
> make it yourself

## Key decisions

- Wrapped open interactive `<aside class="first-time">` blocks in `<tam-box>...</tam-box>` in `src/fold.c` and laid out `box > 0` block groups in `layout_box` (`src/render.c`) at `width - 5` so word wrapping and terminal-resize reflow (`tam_document_reflow`) work automatically.

## Verification

- `make command` (against `../libtam/build`): compiled with 0 warnings.
- `tests/render.sh build/tam`: `ok - render goldens match (7 fixtures)`.
- `tests/run.sh build/tam`: all fold, pager, and first-time aside PTY checks passed (`ok - first-time aside: single toggle label when closed and boxed layout when open`); `make check-repo` passed.
