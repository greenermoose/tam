# tam corpus index and search (milestone 6, tam 0.10.0)

- **Date:** 2026-10-13
- **Stage:** Develop — milestone 6 implemented; version bumped to 0.10.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.3
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred asked to examine the plan for the `tam` command, decide what the most important slice to work on is, claim it, and begin working on it. Antigravity identified Milestone 6 (Corpus index and `tam search`) as the prerequisite foundation for milestones 7–9, claimed the owning checkout scopes via `tam-work`, and implemented the complete milestone: authored metadata in the HTML corpus, built the build-time index projection, wrote the C index and retrieval engine, added `tam search` with `--json`, wired search fallback into `tam explain`, and updated test suites and development installs.

## Guiding prompts

> Take a look at the plan for the tam command, decide what the most important slice to work on is, then claim it and begin working on it.

## Decisions and scope

- **Corpus metadata & build projection:** Authored `<meta name="topic">`, `<meta name="summary">`, `<meta name="keywords">`, `<meta name="level">`, and `<meta name="track">` in all 12 knowledge HTML pages. Added `tools/build-index.py` generating `knowledge/index.tsv` and `knowledge/index.json` at build time. Added missing aliases identified in R-C3 (`ls`, `uninstall`, `redirect stderr`, `multi monitor`, `disk space`).
- **Corpus index C module:** Implemented `src/index.h` and `src/index.c` with TSV loading, case-insensitive alias lookup, query tokenization, stopword filtering, word stemming, field-weighted scoring, prefix matching, and Levenshtein edit distance for bounded fuzzy suggestions.
- **Single search verb (`tam search`):** Implemented `tam search [--json] [--pipe] TERM...` returning a ranked list of matches or machine-readable JSON for scripting.
- **Miss banner and suggestions:** On miss, stderr starts with `tam: no topic found for '<query>'`, presents bounded fuzzy suggestion (`Did you mean '<keyword>'?` with edit distance ≤ 1; never auto-applied), lists top candidate topics with reopen commands, and offers `tam man <query>` when a system manual page is available. Exits 1.
- **Explain search fallback:** `tam explain` retains single-topic direct opening semantics, but resolves multi-word queries (e.g. `tam explain cpu load`) through the search fallback path. Bare `tam explain` prints the catalog projected dynamically from the index.
- **HTML parser robustness:** Fixed tag scanning in `src/render.c` to skip quoted attribute values when detecting the closing `>` delimiter, preventing attribute contents containing `>` from leaking into rendered body text.
- **Version bump:** Minor bump to `0.10.0` in `src/version.h` for the new subcommand and corpus indexing capabilities. Development build installed under `~/.local/dev`.

## Verification

- `tests/run.sh` passes all command tests, including bare search, search `--help`, search `--pipe`, search `--json`, canonical query retrieval (`change file owner`, `pipe output to grep`, `redirect stderr`, `background a process`, `what is /etc`, `check disk space`, `multi monitor`, `man pages`), miss banners (`tam search cron`), bounded fuzzy suggestions (`tam search permisions`), multi-word explain fallback (`tam explain cpu load`), safe shell string data handling, and state isolation.
- `tests/guide.sh` passes all guide tests; golden text snapshots match with 0 drift.
- `make test` passes cleanly.
- `~/.local/dev/bin/tam` installed, tested from PATH, reports `tam command 0.10.0 (dev ...)`.
