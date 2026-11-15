# Convert Explore menus to tables (corpus 0.93.4)

- **Date:** 2026-11-15
- **Time:** 12:29 to 12:32 -05:00
- **Stage:** Develop — corpus 0.93.4 Explore menu table conversion
- **CLI Tool:** Antigravity CLI `1.5.9`
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** recorded in `AI_PROVENANCE.md` and git history on `develop`.
- **Authorship:** Fred Horch directed converting `Explore` `<ul>` menus on pages like `desktop/appearance/index.html` into two-column `<table>` elements. Antigravity converted the `Explore` menus on all 30 non-glossary hub pages and updated `_render_letter_index` in `tools/glossary_index.py` so the 25 `glossary/<letter>/index.html` pages are generated as two-column `<table>` elements, bumped `knowledge/version` to `0.93.4`, and updated `ROADMAP.md` and provenance records.
- **Checked against:** `knowledge/desktop/appearance/index.html`, `tools/glossary_index.py`, `tests/test_glossary_index.py`, `knowledge/version`, `ROADMAP.md`, `build/tam show --pipe`.

## Guiding prompts

> Do the same for explore menus on pages like desktop/appearance/index.html

## Key decisions

- Updated `_render_letter_index` in `tools/glossary_index.py` so `make corpus` regenerates `knowledge/glossary/<letter>/index.html` as two-column `<table>` elements without reverting manual edits.
- Kept `knowledge/glossary/index.html` as a `<ul>` because it lists only letter-range links without descriptions (and `tests/test_glossary_index.py` checks that format).

## Verification

- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe desktop/appearance/index`: verified two-column table rendering.
- `make corpus-and-test`: passed (1136 pages parsed and rendered; 92 unit tests passed).
- `make check-repo`: passed.
