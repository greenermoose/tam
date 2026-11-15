# Fix inline tag extra spacing in `src/render.c` (command 0.59.3)

- **Date:** 2026-11-15
- **Time:** 06:36 to 06:40 -05:00
- **Stage:** Develop — fix inline tag word-separator spacing in `src/render.c` (command 0.59.3)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending (current commit)
- **Authorship:** Fred directed fixing the `tam` command pager so it does not inject extra spaces around `<i>` tags such as `"<i>synonyms</i>"` on `welcome.html`; Antigravity fixed `layout_flow` and `wrap_cell` in `src/render.c`, added a render golden case in `tests/render/blocks.html`, bumped `TAM_COMMAND_VERSION` to `0.59.3`, and updated roadmap/provenance records.
- **Checked against:** `src/render.c`, `src/version.h`, `knowledge/welcome.html`, `tests/render/blocks.html`, `tests/render/blocks.txt`, `tests/run.sh`, `ROADMAP.md`, `docs/plans/development.md`, `AI_PROVENANCE.md`, `agy --version`

## Guiding prompts

> Fix the tam command pager so it does not inject extra spaces for <i> tags. On the welcome.html page,
>
> "<i>synonyms</i>"
>
> is rendered with spaces in it. Fix that. The render should be "synonyms" with synonyms in italics and no spaces around it.

## Key decisions

- Changed `layout_flow` and `wrap_cell` in `src/render.c` to compute `sep` directly from `had_space` (`(col > (int)lead && had_space)` and `(col > 0 && had_space)`), removing the punctuation/bracket heuristics (`tight`, `attach`, `prev`) that injected spaces between adjacent inline runs with no HTML whitespace (such as `"` and `<i>synonyms</i>`).

## Verification

- `make -C tam command LIBTAM_CFLAGS=... LIBTAM_LIBS=...`: built `build/tam` with zero warnings.
- `tam/tests/render.sh tam/build/tam`: `ok - render goldens match (7 fixtures)`.
- `env TAM_KNOWLEDGE_DIR=.../tam/knowledge tam/build/tam show --pipe welcome`: verified `"synonyms"` renders without surrounding spaces.
- `make -C tam check-repo`: passed.
- `make -C tam test-command LIBTAM_CFLAGS=... LIBTAM_LIBS=...`: all command/pager tests passed except pre-existing `FAIL: --check storage_btrfs missing status pass` in the sandboxed environment.
