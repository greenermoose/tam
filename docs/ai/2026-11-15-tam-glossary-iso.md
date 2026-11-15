# ISO glossary page (corpus 0.94.0)

- **Date:** 2026-11-15
- **Time:** 18:56 to 18:59 -05:00
- **Stage:** Develop — add `glossary/i/iso.html` and link it from `glossary/i/index.html` (`corpus 0.94.0`)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** `23f7ddc` and follow-up index/record commit
- **Authorship:** Fred requested a glossary page for ISO explaining what ISO is and why it matters; Antigravity CLI authored `knowledge/glossary/i/iso.html`, linked it in `knowledge/glossary/i/index.html`, bumped `knowledge/version` to `0.94.0`, rebuilt the corpus indexes, and updated `ROADMAP.md`.
- **Checked against:** `knowledge/glossary/i/ietf.html`, `knowledge/glossary/a/ansi.html`, `knowledge/glossary/w/w3c.html`, `knowledge/glossary/i/iso-8859-1.html`, `knowledge/glossary/i/iso-iec-9899.html`, `knowledge/glossary/i/iso-iec-21031.html`, `knowledge/glossary/i/index.html`, `knowledge/version`, `ROADMAP.md`

## Guiding prompts

> I see several "ISO xxx" pages in the glossary, but no definition of ISO. Write a glossary page for ISO. What is ISO? Why does it matter?

## Key decisions

None beyond the prompts.

## Verification

- `make corpus-and-test`: 1137 pages parsed and rendered, component coverage check passed, 92 corpus tooling unit tests passed, and `tools/check-repo.py` passed.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe glossary/i/iso` and `env TAM_KNOWLEDGE_DIR=knowledge build/tam explain iso`: rendered the new glossary page and verified topic lookup.
