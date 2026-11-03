# Seventh guiding principle: make computing fun for humans (corpus 0.59.1)

- **Date:** 2026-11-02
- **Time:** 17:11 to 20:17 -05:00
- **Stage:** Develop, corpus 0.59.1
- **CLI Tool:** Antigravity CLI 1.4.12
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** will be recorded upon commit
- **Authorship:** Fred Horch requested: "Add a principle to the Tam project: make computing fun for humans. Don't ask a human to do boring work like test a bunch of things. Instead, have AI agents and scripts do the boring work and let the human play with his workstation. A human will find plenty of bugs just by playing, while scripts and AI agents can do the drudgery. Computing should be fun, not a chore!" Antigravity claimed paths with `tam-work`, added the seventh guiding principle to `knowledge/ecosystem/philosophy.html` and `knowledge/about.html`, bumped `knowledge/version` to 0.59.1, updated `ROADMAP.md` and `AI_PROVENANCE.md`, and regenerated corpus indexes.
- **Checked against:** `knowledge/ecosystem/philosophy.html`, `knowledge/about.html`, `ROADMAP.md`, `docs/corpus-authoring.md`, `tools/lint-corpus.py`, `tools/build-index.py`, `tools/check-repo.py`, and `tests/guide.sh`.

## Guiding prompts

> Add a principle to the Tam project: make computing fun for humans. Don't ask a human to do boring work like test a bunch of things. Instead, have AI agents and scripts do the boring work and let the human play with his workstation. A human will find plenty of bugs just by playing, while scripts and AI agents can do the drudgery. Computing should be fun, not a chore!

## Key decisions

- Incorporated Fred's seventh guiding principle into `knowledge/ecosystem/philosophy.html` under "Guiding principles for Tamlinux", updating the count from six to seven core engineering and design principles.
- Added "Make computing fun for humans" under "Core principles" in `knowledge/about.html` with a cross-reference to `ecosystem/philosophy.html`.
- Bumped corpus version in `knowledge/version` from 0.59.0 to 0.59.1, added delivered-history rows in `ROADMAP.md` for both 0.59.0 and 0.59.1, and regenerated `index.tsv`, `index.json`, and companion indexes.

## Verification

- `python3 tools/lint-corpus.py`: zero findings.
- `python3 tools/check-repo.py`: clean repository check.
- `python3 tools/build-index.py`: generated 231 entries across `index.tsv` and companion outputs.
- `tests/guide.sh build/tam`: all 12 guide test suites passed.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/philosophy`: verified plain text rendering of all seven guiding principles.
