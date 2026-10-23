# Checkpoint concept guide and glossary entry (corpus 0.43.0)

- **Date:** 2026-10-23
- **Time:** 12:26 to 13:30 -04:00
- **Stage:** Develop; corpus 0.43.0 adds `maintain/checkpoint.html` and `learn/glossary/checkpoint.html`.
- **CLI Tool:** Antigravity CLI 1.4.6
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** `39a0fd0`
- **Authorship:** Fred Horch directed creating a page or set of pages in the
  knowledge corpus explaining the checkpoint concept, identifying linking points
  and page count, after noting that `tam explain checkpoint` returned no results.
  Antigravity CLI authored `maintain/checkpoint.html` and `learn/glossary/checkpoint.html`,
  integrated cross-links across the maintenance and glossary pages, rebuilt index
  artifacts, and bumped the corpus version to 0.43.0.
- **Checked against:** `maintain/checkpoint.html`, `learn/glossary/checkpoint.html`,
  `maintain/index.html`, `maintain/rollback.html`, `maintain/snapper.html`,
  `learn/glossary/index.html`, `learn/glossary/snapshot.html`, `learn/glossary/rollback.html`,
  `knowledge/version`, `ROADMAP.md`

## Guiding prompts

> The codex session keeps saying things like, "Fresh checkpoints and private-history sync are still required." But tam explain checkpoint found nothing. Create a page or set of pages in the knowledge corpus explaing the checkpoint concept. Decide where this will be linked from existing content and how many pages are necessary to write. Tell me your plan and then let's discuss before you start writing that page or pages.

> Yes, good plan. Proceed.

## Key decisions

- Two pages are authored: one three-level ladder guide (`maintain/checkpoint.html`)
  and one companion glossary entry (`learn/glossary/checkpoint.html`).
- The main guide covers state boundaries across filesystem snapshots (Snapper),
  quiescent boot archives, mutable settings checkpoints, and declarative software
  generations.
- The expert level covers quiescence, explicit scope boundaries, cryptographic
  verification, and the anti-churn principle in plain language: capture state only
  before actual mutation of that layer, verify coverage, and reuse the established
  recovery anchor rather than churn intermediate checkpoints.
- `tam explain checkpoint` and `tam explain checkpoints` resolve directly to the
  new guide and glossary entry.

## Verification

- `make corpus`: cleanly generated index and sidecars with 110 entries.
- `python3 tools/lint-corpus.py`: zero errors or warnings.
- `python3 -m unittest tests/test_lint_corpus.py tests/test_build_index.py`: 52 tests passed (OK).
- `tam explain checkpoint --pipe`: displays Topic Guide listing both pages.
- `tam explain checkpoints --pipe`: displays Topic Guide.
- `tam show --pipe maintain/checkpoint.html`: cleanly renders all three learning levels.
