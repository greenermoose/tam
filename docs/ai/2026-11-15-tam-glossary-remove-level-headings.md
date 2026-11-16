# Remove redundant level-definition headings from glossary pages (corpus 0.94.1)

- **Date:** 2026-11-15
- **Time:** 19:00 to 19:03 -05:00
- **Stage:** Develop — remove `<h2>Novice definition</h2>`, `<h2>Learner definition</h2>`, and `<h2>Expert definition</h2>` from all 830 glossary pages (`corpus 0.94.1`)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** `358dac0` and follow-up index/record commit
- **Authorship:** Fred requested sweeping every glossary page to remove the extraneous `Novice definition`, `Learner definition`, and `Expert definition` lines; Antigravity CLI removed all 2,490 headings across the 830 pages under `knowledge/glossary/`, bumped `knowledge/version` to `0.94.1`, rebuilt the corpus indexes, and updated `ROADMAP.md`.
- **Checked against:** `knowledge/glossary/*/*.html`, `tools/lint-corpus.py`, `knowledge/version`, `ROADMAP.md`

## Guiding prompts

> Several pages in the glossary have this format.
>
> Remove the lines
>
> Novice definition
> Learner definition
> Expert definition
>
> They are unnecessary. We have the Showing novice text, etc. lines already to alert the reader that this is a section for novices, learners, or experts. Do a thorough sweep through every glossary page to remove these extraneous lines.

## Key decisions

None beyond the prompts.

## Verification

- `make corpus-and-test`: 1137 pages parsed and rendered, component coverage check passed, 92 corpus tooling unit tests passed, and `tools/check-repo.py` passed.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe glossary/p/polkit` and `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe glossary/i/iso`: confirmed the redundant headings are gone while `Showing <level> text` notices remain.
