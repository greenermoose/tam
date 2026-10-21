# Repository map

What each top-level entry and `docs/` subdirectory in `tam` is for, and how it
changes. `tools/dev.py` reads the category on each entry line during `make dev`,
and `tools/check-repo.py` verifies that every entry is listed here.

Categories:

- *you edit* — authored pages, source code, documentation, build files and active plans
- *script edits* — version files and roadmap version places updated by the developer loop
- *generated, never edit* — build and index outputs recreated by the build
- *history, read-only* — completed session records and handoff notes

## Entries

- `knowledge/` — *you edit*. Offline HTML guide pages (`*.html`); `knowledge/version` is updated by `make dev` and `knowledge/index.*` is built by `make corpus`.
- `src/` — *you edit*. C source and headers for the `tam` binary; `TAM_COMMAND_VERSION` in `src/version.h` is updated by `make dev`.
- `bin/` — *you edit*. Wrapper scripts (`tam-man`, `tam-info`, `tam-tldr`, `tam-upgrade`) installed alongside `tam`.
- `completions/` — *you edit*. Static Bash tab-completion script (`completions/bash/tam`).
- `man/` — *you edit*. Section 1 manual page (`man/tam.1`).
- `tools/` — *you edit*. Python build, lint, developer-loop and drift-check scripts (`build-index.py`, `build-tldr-index.py`, `lint-corpus.py`, `dev.py`, `check-repo.py`).
- `tests/` — *you edit*. Command, corpus, rendering, upgrade, developer-loop and drift-check suites (`tests/run.sh` command version assertion is updated by `make dev`).
- `docs/` — *you edit*. Corpus authoring, corpus audit, plan-maintenance and repository-map guides.
- `docs/plans/` — *you edit*. Feature plans, the living development contract (`development.md`), templates and the plan index (`README.md`); closed plans remain in place as historical records.
- `docs/ai/` — *history, read-only*. Per-session AI provenance records; add a new file for a new AI session without rewriting earlier records.
- `docs/handoffs/` — *history, read-only*. Historical cross-session handoff notes kept at stable paths.
- `docs/templates/` — *you edit*. Page skeletons (ladder, layered, single, path) that new corpus pages start from; see `docs/corpus-pedagogy.md`.
- `build/` — *generated, never edit*. Compiler output (`build/tam` and build configuration stamp) created by `make command`.
- `Makefile` — *you edit*. Build, test, `make dev`, `make check-repo` and install targets; keep `SRC` and `DEPS` aligned with `src/`.
- `README.md` — *you edit*. Project overview, build prerequisites, developer loop and command reference.
- `ROADMAP.md` — *script edits*. Current version header, delivered-history rows and the `develop` branch row updated by `make dev`; narrative sections change with plan updates.
- `AGENTS.md` — *you edit*. Concurrency rules, lane scopes, build/test commands and provenance guidance for AI agents.
- `AI_PROVENANCE.md` — *you edit*. Chronological table of AI-assisted sessions; append a row when adding a record under `docs/ai/`.
- `CHECKLISTS.md` — *you edit*. Monotonic `TCK<N>` checklist counter and minting rules across plans.
- `ACKNOWLEDGEMENTS.md` — *you edit*. Third-party and upstream attributions.
- `COPYRIGHT` — *you edit*. Repository copyright statement.
- `LICENSE` — *you edit*. GNU General Public License v3 text.
- `source-build.json` — *you edit*. Declarative build manifest read by `tam upgrade` when building source generations.
