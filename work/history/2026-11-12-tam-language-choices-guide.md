# Systems programming philosophy and language choices guide page (corpus 0.80.0)

- **Date:** 2026-11-12
- **Time:** 17:51 to 18:56 -05:00
- **Stage:** Develop; corpus 0.80.0 adds systems programming philosophy, language trade-offs (C11 vs Rust/Go/C++), and AI idea synthesis documentation.
- **CLI Tool:** Antigravity CLI 1.5.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit
- **Authorship:** Fred Horch directed discussing the pros and cons of the Tamlinux philosophy of writing programs in C11 rather than Rust, Go, or C++, including using AI agents to examine legacy and open source code across languages and implement ideas in C11, then directed drafting a new topic guide and expanding `ecosystem/c11.html`. Antigravity CLI claimed the scopes via `tam-work`, authored `knowledge/ecosystem/language-choices.html`, expanded `knowledge/ecosystem/c11.html`, updated `knowledge/ecosystem/index.html`, `knowledge/ecosystem/sanitizers.html`, and `knowledge/paths.json`, bumped corpus version to 0.80.0 in `knowledge/version` and `ROADMAP.md`, regenerated corpus indexes, and verified linting, component coverage, and test suites.
- **Checked against:** `size /usr/bin/tam`, `ldd /usr/bin/tam`, `build/tam --version`, `knowledge/ecosystem/c11.html`, `knowledge/ecosystem/sanitizers.html`, `knowledge/ecosystem/philosophy.html`, `knowledge/version`, `ROADMAP.md`, `tools/lint-corpus.py`, `tools/report-component-coverage.py`, `tools/check-repo.py`.

## Guiding prompts

> For the tam knowledge corpus, discuss the pros and cons of the Tamlinux philosophy of writing programs in C11 rather than rust or go or C++ or a variety of other programs. We are using AI agents to examine legacy code and getting ideas from open source projects that write in a variety of languages, and then implementing them in C11.
> Draft a new topic guide as well as expand ecosystem/c11.html.

## Key decisions

- Authored `knowledge/ecosystem/language-choices.html` with Novice, Learner, and Expert difficulty levels conforming to corpus pedagogy and style rules.
- Captured real workstation output comparing section sizes (`size`) and dynamic library linkages (`ldd`) for native C11 utilities.
- Detailed architectural trade-offs: C11 + `libtam` anti-bloat, millisecond startup, and 20-year source longevity versus the toolchain weight of Rust, the runtime and binary size floor of Go, and the template bloat and ABI fragility of C++.
- Documented the AI synthesis workflow: AI assistants survey open source projects across languages to extract algorithms and architecture under human direction, implementing lean, self-contained C11 code backed by `libtam` with zero dependency bloat.
- Expanded `knowledge/ecosystem/c11.html` with anti-bloat rationale in the Novice section and an architectural comparison subsection in the Expert section.
- Updated `knowledge/ecosystem/sanitizers.html` and `knowledge/paths.json` to link `ecosystem/language-choices.html` as the capstone step in the `c11-systems` learning path.
- Covered `tam-launch-journal`, closing the component coverage gap in `knowledge/component-coverage.json`.
- Bumped corpus version to 0.80.0 in `knowledge/version` and updated `ROADMAP.md`.

## Verification

- `python3 tools/lint-corpus.py`: All 592 corpus pages passed with zero defects.
- `python3 tools/report-component-coverage.py`: 251 components scanned, 181 covered, 70 exempt, 0 gaps.
- `make corpus`: Regenerated `index.tsv`, `index.json`, `index.parents.tsv`, `index.kinds.tsv`, `index.skills.tsv`, and `coverage.json`.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/language-choices`: Verified clean text rendering for Novice, Learner, and Expert levels.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam explain --pipe language-choices`: Verified topic guide creation and page discovery.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam explain --pipe c11`: Verified multi-page topic guide listing.
- `make corpus-and-test`: Rebuilt corpus, passed corpus checks, ran 78 tooling unit tests, and passed repository consistency check.
