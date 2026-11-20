# Compiler sanitizers and AddressSanitizer guide page (corpus 0.79.0)

- **Date:** 2026-11-12
- **Time:** 17:06 to 17:18 -05:00
- **Stage:** Develop; corpus 0.79.0 adds compiler sanitizers, AddressSanitizer (ASan), and runtime memory safety documentation.
- **CLI Tool:** Antigravity CLI 1.5.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit
- **Authorship:** Fred Horch directed adding pages about AddressSanitizer (ASan) to the tam knowledge corpus. Antigravity CLI authored `knowledge/ecosystem/sanitizers.html` covering ASan, UBSan, TSan, MSan, shadow memory, and redzones, authored glossary entries `knowledge/learn/glossary/addresssanitizer.html` and `knowledge/learn/glossary/sanitizer.html`, updated `knowledge/ecosystem/index.html`, `knowledge/ecosystem/c11.html`, `knowledge/improve/toolchains.html`, `knowledge/learn/glossary/index.html`, and `knowledge/paths.json`, bumped corpus version to 0.79.0 in `knowledge/version` and `ROADMAP.md`, rebuilt indexes, and verified corpus linting and tests.
- **Checked against:** `gcc --help`, GCC and Clang instrumentation manuals, `knowledge/ecosystem/c11.html`, `knowledge/improve/toolchains.html`, `knowledge/version`, `ROADMAP.md`, `tests/guide.sh`, `tools/lint-corpus.py`, `tools/check-repo.py`.

## Guiding prompts

> please add one or pages about this to the tam knowledge corpus

## Key decisions

- Authored `knowledge/ecosystem/sanitizers.html` with Novice, Learner, and Expert difficulty levels conforming to corpus pedagogy and style rules.
- Included real captured terminal session of a heap buffer overflow detected by AddressSanitizer on an Arch Linux workstation (`gcc -std=c11 -fsanitize=address -g`).
- Authored two glossary entries: `knowledge/learn/glossary/addresssanitizer.html` and `knowledge/learn/glossary/sanitizer.html`.
- Updated glossary index `knowledge/learn/glossary/index.html` with alphabetical placement.
- Added cross-links from `knowledge/ecosystem/index.html`, `knowledge/ecosystem/c11.html`, `knowledge/improve/toolchains.html`, and registered the step in `knowledge/paths.json` under path `c11-systems`.
- Bumped corpus version to 0.79.0 in `knowledge/version` and updated `ROADMAP.md`.

## Verification

- `python3 tools/lint-corpus.py`: All 591 corpus pages passed with zero defects.
- `make corpus`: Generated `index.tsv`, `index.json`, `index.parents.tsv`, `index.kinds.tsv`, `index.skills.tsv`, and `coverage.json` cleanly.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/sanitizers`: Verified clean text rendering for Novice, Learner, and Expert sections.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam search asan`: Confirmed search discovery for ASan and sanitizers.
- `bash tests/guide.sh build/tam`: All 12 guide test assertions passed.
- `python3 -B -m unittest -q tests/test_lint_corpus.py tests/test_build_index.py tests/test_component_coverage.py tests/test_corpus_pipeline.py`: All 78 tooling unit tests passed.
- `python3 tools/check-repo.py`: Repository consistency verified.
