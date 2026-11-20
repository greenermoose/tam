# Rebuild tam corpus to clean dev build id (corpus 0.28.0)

- **Date:** 2026-10-17
- **Time:** 23:50 (2026-10-16) to 07:45 -04:00
- **Stage:** Develop — corpus rebuild and index validation (`knowledge/maintain/boot.html`, `knowledge/index.tsv`, `knowledge/index.json`)
- **CLI Tool:** Antigravity CLI 1.4.0 (`agy 1.4.0`)
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Recorded in git history on `develop`.
- **Authorship:** Fred requested rebuilding the tam corpus so the dev version does not report dirty; Antigravity CLI diagnosed the failed index generation from non-existent topic prerequisite 'hardware' and missing TSV header in `boot.html`/`index.tsv`, repaired the metadata, regenerated the indices and build ID, and verified the test suite.
- **Checked against:** `tools/build-index.py`, `knowledge/maintain/boot.html`, `knowledge/index.tsv`, `knowledge/index.json`, `tests/guide.sh`, `tests/test_lint_corpus.py`.

## Guiding prompts

> Rebuild the tam corpus so the dev version doesn't say dirty.

## Key decisions

- **Remove non-existent topic 'hardware' from `boot.html` prereqs:** `knowledge/maintain/boot.html` declared `<meta name="prereqs" content="maintain, hardware, filesystem">`, which caused `tools/build-index.py` to abort because `hardware` is not a declared corpus topic. Corrected to `maintain, filesystem` to match the declared body text and valid topics.
- **Restore `index.tsv` header and regenerated index:** The uncommitted/manual edits had dropped the header line from `knowledge/index.tsv` and left `index.json` with trailing duplicates, which broke `tam_index_load` parsing and caused `ecosystem/ai.html` to be skipped as a pseudo-header. Regenerating via `tools/build-index.py` restored the header, sorted order, and deterministic revisions.
- **Install clean dev corpus build ID:** Running index generation against a clean Git status removes the `-dirty` suffix from `knowledge/.build_id`, allowing `tam --version` to report a clean commit build ID.

## Verification

- `tools/build-index.py` generated `knowledge/index.tsv` and `knowledge/index.json` cleanly with 48 entries.
- `python3 tools/lint-corpus.py` and `python3 -m unittest -q tests/test_lint_corpus.py` (29 tests) pass.
- `tests/guide.sh` passes all tests, including verification of revision stamps on all pages.
- `make PREFIX=$HOME/.local/dev install` installed to `~/.local/dev`; `tam --version` reports `knowledge corpus 0.28.0 (dev <sha>)` without `-dirty`.
