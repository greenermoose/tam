# Lensed section hide/show notices (command 0.59.6)

- **Date:** 2026-11-15
- **Time:** 10:26 to 10:33 -05:00
- **Stage:** Develop — replace `[N Level]` section prefixes and `Hiding ... text (0N show)` markers with right-aligned `hide`/`show` notices in `tam`
- **CLI Tool:** agy 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending commit
- **Authorship:** Fred requested the right-aligned hide/show notices for lensed text sections; Antigravity updated `src/depth.c`, `src/render.c`, `src/pager_nav.c`, `src/version.h`, `tests/render/blocks.txt`, `tests/render/pedagogy.txt`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`, and provenance records.
- **Checked against:** `src/depth.c`, `src/render.c`, `src/pager_nav.c`, `src/version.h`, `tests/render/blocks.txt`, `tests/render/pedagogy.txt`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> When displaying a lensed text section, such as <section data-level="novice">, change the rendered text from
>
> [1 Novice] Choosing what to do from a text screen
>
> to
>
>                                                  23 hide novice text
> Choosing what to do from a text screen
>
> The novice text notice should be aligned right, and novice text should be in the lens color.
>
> If hidden, that lensed section should render
>
>                                                  01 show novice text
>
> with none of the text showing.

## Key decisions

- Replaced `[N Level]` heading prefix injection in `tam_depth_apply_rev` (`src/depth.c`) with `<p class="level-line">AB hide <span class="lvl-N"><level> text</span></p>` emitted before displayed `<section data-level="...">` elements, and `<p class="level-line">0N show <span class="lvl-N"><level> text</span></p>` for hidden `<section data-level="...">` elements.
- Updated `layout_document` in `src/render.c` so `B_META` (`<p class="level-line">`) is tight above any `B_HEAD` (not only `h1`), placing the hide notice directly above the section heading without a blank line.
- Updated `hiding_line_here` in `src/pager_nav.c` to match `01 show`, `02 show`, and `03 show`.

## Verification

- `make -C /home/fred/Code/tamlinux/tam check-repo`
- `make -C /home/fred/Code/tamlinux/tam command LIBTAM_CFLAGS="-I/home/fred/Code/tamlinux/libtam/include" LIBTAM_LIBS="/home/fred/Code/tamlinux/libtam/build/libtam.a"`
- `/home/fred/Code/tamlinux/tam/tests/render.sh /home/fred/Code/tamlinux/tam/build/tam`
- `tests/run.sh build/tam` (in `tam/`: all command/pager/level-lens tests passed; `storage_btrfs` check failed only because sandbox `/` is `tmpfs`)
