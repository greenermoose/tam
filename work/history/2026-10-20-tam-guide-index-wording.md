# Guide root index wording and corpus bump (corpus 0.33.6)

- **Date:** 2026-10-20
- **Time:** 08:14 to 08:26 -04:00
- **Stage:** Develop; `knowledge/index.html`, corpus version `0.33.6`, `tests/run.sh`, roadmap header, and development install under `~/.local/dev`
- **CLI Tool:** Antigravity CLI 1.4.3
- **Model:** Gemini 4 Argon (Medium) (`gemini-4-argon-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** `d022860`, `beda4e8` and the commit that updates this record and `tests/run.sh`
- **Authorship:** The developer edited `knowledge/index.html` and asked to bump the corpus version and install the updated version on the system. Antigravity bumped `knowledge/version` to `0.33.6`, updated `ROADMAP.md`, `tests/run.sh` and provenance, rebuilt and tested the corpus, and installed the clean build to the development prefix.
- **Checked against:** `knowledge/index.html`, `knowledge/version`, `tests/run.sh`, `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe guide`, `make corpus-and-test`, `make test`, `tam --version`

## Guiding prompts

> I just edited a few pages in the tam knowledge corpus. Bump the version of the corpus and do whatever else is necessary to install this new version on this system. Ask if you have questions.

> Make sure to commit changes before building so the version notices don't say dirty.

> index.html is the only page I edited. I was thinking about editing other pages but changed my mind.

## Key decisions

- Patch bump of `knowledge/version` from `0.33.5` to `0.33.6` for the Guide root index wording updates ("Your Tamlinux User Guide", adding the Welcome orientation link to the main list, and refining the description and footer prompt).
- Updated `tests/run.sh` assertions that checked for the previous root guide title `"The Tamlinux User Guide"` to match `"Your Tamlinux User Guide"`.
- Committed the page and version bump, then ran `make corpus` and committed the updated Git-derived page revision and timestamp in `knowledge/index.json` and `knowledge/index.tsv` before running `make corpus` and `make install PREFIX=$HOME/.local/dev`, ensuring both command and corpus version strings report clean commit hashes without `-dirty`.

## Verification

`make test` and `make corpus-and-test` passed (`tests/run.sh`, `test_upgrade.py`, `tests/render.sh`, `lint-corpus.py`, 29 lint unit tests, 10 index build unit tests, and `tests/guide.sh`). Installed to `~/.local/dev` after committing and verified `tam --version` reports clean `0.32.0` / `0.33.6` dev build identifiers without `-dirty`.
