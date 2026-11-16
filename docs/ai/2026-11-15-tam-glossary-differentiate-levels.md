# Differentiate Novice, Learner, and Expert sections across all 830 glossary pages (corpus 0.94.2)

- **Date:** 2026-11-15
- **Time:** 19:06 to 19:12 -05:00
- **Stage:** Develop — rewrite all 830 glossary pages so General, Novice, Learner, and Expert lenses each add distinct value (`corpus 0.94.2`)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** `ed22de9` and follow-up index/record commit
- **Authorship:** Fred requested reviewing every glossary page so Novice, Learner, and Expert sections are not repetitive and follow the 4-part structure (general intro, simple Novice explanation, Learner text with questions to extend/retain knowledge, and precise Expert nuances/examples); Antigravity CLI orchestrated a 70-batch parallel workflow across all 830 pages under `knowledge/glossary/`, resolved 6 linter hits (`branch.html`, `character-encoding.html`, `color-temperature.html`, `rgb.html`, `unicode.html`, `utf-8.html`), bumped `knowledge/version` to `0.94.2`, rebuilt the corpus indexes, and updated `ROADMAP.md`.
- **Checked against:** `docs/corpus-pedagogy.md`, `knowledge/glossary/*/*.html`, `tools/lint-corpus.py`, `knowledge/version`, `ROADMAP.md`

## Guiding prompts

> Read each glossary page and make sure the novice, learner, and expert text is not simply repetitive. Each lens should add value, not simply repeat what the previous lens just said using fancier words that fewer people understand. The expert text should add sophisticated examples or extend a reader's understanding significantly.
>
> In general, every glossary page should have the following structure:
>
> 1) A general introduction for all audiences.
> 2) A simple explanation for novices that may not fully understand the general introduction and need more context or more help understanding the term.
> 3) Learner text with questions or other ways for someone interested in extending or retaining knowledge about the topic.
> 4) Expert text that explains nuances or provides detailed examples or summarizes using extremely precise technical language.

## Key decisions

None beyond the prompts.

## Verification

- `make corpus-and-test`: 1137 pages parsed and rendered, component coverage check passed, 92 corpus tooling unit tests passed, and `tools/check-repo.py` passed.
- Verified all 830 glossary pages include `<details class="q">` self-check questions in Learner.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe glossary/p/polkit` and `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe glossary/i/iso`: confirmed distinct general intro, Novice, Learner (with folded self-check), and Expert rendering.
