# Use tam hi in the corpus (corpus 0.45.3)

- **Date:** 2026-10-27
- **Time:** 15:40 to 15:42 -04:00
- **Stage:** Develop, corpus correction
- **CLI Tool:** Codex CLI `0.164.0`
- **Model:** `gpt-6.1-terra`
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record and the subsequent index refresh.
- **Authorship:** Fred directed the change; Codex revised the corpus.
- **Checked against:** `src/tam.c`, corpus command references, and `docs/corpus-authoring.md`.

## Guiding prompts

> We are deprecating tam welcome. Use tam hi instead. Please rewrite the corpus to replace tam welcome with tam hi.

## Key decisions

The corpus recommends `tam hi` for returning to the welcome page. This change does not remove or alter command aliases.

## Verification

- Searched the corpus for the old command spelling; no references remain.
- `make corpus-and-test`: corpus lint, index tests and guide checks passed.
