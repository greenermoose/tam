# Subject library and unified discovery (command 0.57.0, corpus 0.90.0)

- **Date:** 2026-11-14
- **Time:** 16:47 to 17:27 -05:00
- **Stage:** Develop — reorganize the `tam` knowledge corpus into canonical subject folders (`hardware`, `systems`, `terminal`, `networking`, `security`, `programming`, `desktop`, `tamlinux`) and shared facilities (`paths`, `glossary`, `bibliography`, `skills`, `help`) in corpus `0.90.0`, and add subject browsing (`tam:topics`), alphabetical page catalog (`tam:catalog`), unified grouped search across the full index, and page-kind classification (`index.kinds.tsv`) in command `0.57.0`.
- **CLI Tool:** Codex CLI `0.169.0` and Antigravity CLI `1.5.9`
- **Model:** GPT-6 Astra (`gpt-6-astra`), Claude Opus 5.5 (High) (`claude-opus-5-5-high`), and Gemini 4 Argon (Medium) (`gemini-4-argon-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending
- **Authorship:** Fred directed the subject-first corpus reorganization, consolidation of the four overlapping catalog views into four clear reader entry points (`Browse subjects`, `Learn step by step`, `Search`, `Sources and further reading`), and the session handoff after Codex hit its usage limit; Codex CLI designed the subject-library plan, migrated the 1,081 baseline pages into canonical subject folders with 55 hubs, merged five duplicate pages, implemented `index.kinds.tsv` and grouped search in `src/`, and added `docs/plans/subject-library.md`, `docs/corpus-migration.tsv`, `docs/curriculum-gaps.md`, and `tests/test_library_discovery.py`; Antigravity CLI took over the work, removed empty legacy directories, replaced boilerplate sub-hub summaries and ordered subject-hub entries by prerequisite progression, updated remaining test assertions and render fixture `tests/render/blocks.html`, completed verification, and finalized the plan and roadmap records.
- **Checked against:** `AGENTS.md`, `ROADMAP.md`, `docs/plans/subject-library.md`, `docs/plans/development.md`, `docs/corpus-authoring.md`, `src/index.c`, `src/views.c`, `src/discovery.c`, `src/install.c`, `tools/build-index.py`, `tools/lint-corpus.py`, `tests/run.sh`, `tests/guide.sh`, `tests/test_library_discovery.py`

## Guiding prompts

> Organize the tam knowledge corpus. Consider all the knowledge that the corpus contains, then determine the best folder structure. Create or rename folders, move pages if necessary. If pages needed to be split, consolidated, renamed, or revised, do that. The goal should be an extremely well organized collection of folders and pages upon which we can continue to build a best-on-the-globe reference work that empowers readers without any background in a topic become PhD-level experts.

> Do we need learning paths, guide topics, keyword index, and glossary? The bibliography seems good: that is a list of external sources of information and sources for what's in our knowledge corpus. But I'm wondering if there's a way to consolidate or better organize the other four. What do readers really need? What is the most helpful additional structure and entry points can we provide. Maybe all four are great. I just get a little confused remembering what is what and how to find things.

> Add your recommended additional structure to the plan. Your plan should be the best way to organize all of the material and help readers find and understand the knowledge we've gathered.

> Implement the plan.

> Codex was reorganizing the tam knowledge corpus when it ran out of tokens. Find the codex session and take over it for it. See if you can complete its work before you run out of tokens.

> Try to finish this work before you run out of tokens

## Key decisions

- Replaced the former five-part track layout (`install`, `learn`, `use`, `maintain`, `improve`) and `ecosystem/` catch-all with eight canonical subject folders (`hardware`, `systems`, `terminal`, `networking`, `security`, `programming`, `desktop`, `tamlinux`) and five shared facilities (`paths`, `glossary`, `bibliography`, `skills`, `help`), with a clean address break recorded in `docs/corpus-migration.tsv`.
- Consolidated the root reader entry points into `Browse subjects` (`tam:topics`), `Learn step by step` (`tam:paths`), `Search` (`tam:search` / `tam:keywords`), and `Sources and further reading` (`bibliography/index.html`), while retaining `tam:catalog` (alphabetical pages A–Z) and `glossary/index.html` as secondary lookup views.
- Removed the 64-entry cutoff in `tam_index_search` so glossary growth cannot crowd out instructional articles, and grouped search results by reader purpose (`Definitions`, `Practical guidance`, `Explanations and reference`, `Learning paths`, `Subject guides`, `Sources`) backed by `index.kinds.tsv` with directory fallback for older corpora.
- Merged five duplicate pages (`desktop/clipboard.html`, `desktop/notifications.html`, `desktop/capture.html`, `shell/editors.html`, `learn/glossary/os.html`) into their canonical counterparts while preserving their expert sections, keywords, and path memberships; narrowed `security/hardware-authentication.html` to administrative privilege management and moved FIDO2/fingerprint and SSH command tables to `security/practice/`.

## Verification

- `make corpus`: rebuilt `knowledge/index.tsv`, `knowledge/index.json`, `knowledge/index.kinds.tsv`, `knowledge/index.parents.tsv`, `knowledge/index.paths.tsv`, `knowledge/index.skills.tsv`, `knowledge/coverage.json`, and `knowledge/component-coverage.json` with 0 warnings.
- `make test`: `tests/run.sh`, `test_upgrade.py` (15 tests), `tests/render.sh` (5 fixtures), `tests/guide.sh` and `tools/lint-corpus.py`, component coverage check, corpus tooling unit tests (87 tests including `test_library_discovery.py`), and `tools/check-repo.py` all passed.
