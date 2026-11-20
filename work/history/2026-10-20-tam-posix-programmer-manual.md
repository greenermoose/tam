# POSIX Programmer's Manual guide page (corpus 0.34.0)

- **Date:** 2026-10-20
- **Time:** 08:46 to 09:05 -04:00
- **Stage:** Develop; new corpus page, parent link, roadmap header
- **CLI Tool:** Antigravity CLI 1.4.3
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer asked whether a page existed for the POSIX Programmer's Manual and directed creating a dedicated page. Antigravity CLI authored `knowledge/man/posix.html`, linked it from `knowledge/man/index.html`, bumped `knowledge/version` to 0.34.0, regenerated corpus indexes, and verified with `make corpus-and-test`.
- **Checked against:** `man -w` for `0p`, `1p`, and `3p` sections from installed `man-pages` 6.19-1, `tam man 1p sh`, `tam man 3p fork`, `tam show --pipe man/posix`, `tam explain posix --pipe`, `tools/lint-corpus.py`, `tests/test_lint_corpus.py`, `tests/test_build_index.py`, `tests/guide.sh`.

## Guiding prompts

> Create the dedicatd page if you can. There is another agy session doing some work in the tam repo now. But I hope it isn't blocking your lane.

## Key decisions

- Added `knowledge/man/posix.html` under the `learn` track, with parent `man/index.html`.
- Documents the origin of the POSIX Programmer's Manual (IEEE Std 1003.1 / The Open Group Base Specifications), the standard sections (`0p`, `1p`, `3p`), standard interface specifications versus GNU/Linux implementations, anatomy of POSIX pages (highlighting APPLICATION USAGE and RATIONALE), and practical ways to browse and query POSIX pages directly inside `tam`.
- Updated `knowledge/man/index.html` to link to `man/posix.html`.
- Regenerated `knowledge/index.tsv`, `knowledge/index.json`, and `knowledge/index.parents.tsv`.
- Minor version bump from 0.33.6 to 0.34.0 for new corpus capability and page addition.

## Verification

- `python3 tools/lint-corpus.py` reported zero findings.
- `python3 -m unittest -q tests/test_lint_corpus.py` passed (29 tests).
- `python3 -m unittest -q tests/test_build_index.py` passed (10 tests).
- `tests/guide.sh build/tam` passed all tests.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe man/posix` verified clean terminal layout, breadcrumbs, and link targets.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam explain posix --pipe` verified discovery of the new page.
