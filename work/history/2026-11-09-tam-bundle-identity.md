# Bundle identity and ownership (command 0.51.0)

- **Date:** 2026-11-09
- **Time:** 23:20 to 23:30 -05:00
- **Stage:** Develop; command 0.50.4 to 0.51.0
- **CLI Tool:** Codex CLI 0.167.0
- **Model:** `gpt-6.1-sol`
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record.
- **Authorship:** Codex implements the approved bundle design's identity and ownership readers.
- **Checked against:** `src/identity.c`, `src/tam.c`, tam-bundle's format and `where` interface.

## Guiding prompt

> Pick up where claude left off and work on tam-bundle. The goal is to finish tam-bundle and start using it on [redacted: host].

## Decisions and verification

Only an executable installed inside a bundle reads that prefix's BUNDLE
version and cultivar. Installing a dormant bundle therefore leaves old
delivery and development commands reporting their own existing identity.
`tam where PATH` asks tam-bundle first, then pacman or xbps, passing paths
as arguments without a shell.

`make test-command` and `make check-repo` pass. New tests copy the command
into a bundle fixture with stale per-user metadata and exercise ownership
fallback with paths containing spaces and shell punctuation.
