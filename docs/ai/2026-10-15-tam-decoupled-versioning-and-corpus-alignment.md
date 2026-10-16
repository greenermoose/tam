# Decoupled Versioning and Knowledge Corpus Alignment (tam 0.24.0)

- **Date:** 2026-10-15
- **Stage:** Develop — Milestone 16: Decoupled Command & Corpus Versioning, Knowledge Corpus Subcommand Review & Alignment, Orientation Shortcuts, and Ecosystem Enrichments, version bump to 0.24.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred directed inserting a new Milestone 0.24.0 into `tam` to decouple the command and knowledge corpus version numbers, review every page in the corpus for `tam` subcommands (`tam inspect`, `tam check`, etc.), and discern/incorporate Fred's private idea notes. Antigravity claimed the respective scopes under `tam-work`, executed the codebase and corpus changes, updated test suites, renumbered future roadmap versions (0.25.0 through 0.29.0), and filed/removed the raw ideas in the private planning notes.

## Guiding prompt

> For the tam command, we added tam inspect and tam check. But I don't think those changes got carefully incorporated in the knowledge corpus. Please insert a new version 0.24.0 milestone and increment all the existing versions to slot this new one ahead of them.
>
> For 0.24.0, do the following:
>
> 1) Decouple the version number of the tam command itself (the parser and displayer) and the knowledge corpus. They can both become version 0.24.0, but may diverge in the future if we iterate on one or the other faster.
>
> 2) Carefully review every page in the knowledge corpus for tam subcommands. We have added tam inspect and tam check. There may be others that were added, or functionality in the tam command changed, and these changes may not be reflected perfectly in the knowledge corpus. Attempt to perfect the knowledge corpus.
>
> 3) Take a look at the ideas in [redacted: private repository path]. Look for any that relate to tam. Review the ideas and discern whether they are good or bad. Ask me if you have questions about them. Then choose good ideas (if any) to incorporate into this version 0.24.0 of the tam command and the knowledge corpus. Remove the ideas from the fred folder and file them elsewhere in the [redacted: private repository] repo, and leave a removal record called HISTORY in the fred ideas folder.
>
> Ask if you have questions.

## Decisions and scope

- **Ideas Discernment & Filing (private planning notes):**
  - Evaluated the 4 ideas in the maintainer's private ideas folder:
    1. *ideas for tam*: Decoupling command binary version from corpus version. Discernment: Decisively good; core requirement for 0.24.0.
    2. *aliases for Tamlinux*: "One place to look" in `~/.bashrc`, `tam hello`, `tam hi`, `h`. Discernment: Accepted `tam hello` and `tam hi` built-ins and user guidance on single-file alias customization. Added caution on single-letter `h` colliding with `history` in full developer environments.
    3. *nut root stock cultivar*: Completing the botanical metaphor and command taxonomy (`*-nut`, `*-root`, `*`). Discernment: Decisively sound taxonomy. Added **Fruit** as the 6th layer (end-user applications). Binary separation deferred until nut UKI is built.
    4. *guiding principles*: 6 core philosophical tenets. Discernment: Foundational architecture principles. Incorporated into `knowledge/ecosystem/philosophy.html`.
  - Recorded the discernment, filing locations and incorporation details in a private audit record, removed the original notes, and committed the change in the private planning repository.

- **Decoupled Command and Corpus Versioning:**
  - `src/version.h` defines `TAM_COMMAND_VERSION` ("0.24.0") and `TAM_DEFAULT_CORPUS_VERSION` ("0.24.0").
  - `knowledge/version` stores `"0.24.0"`.
  - `tools/build-index.py` validates `knowledge/version` and stamps build ID to `knowledge/.build_id`. Added `knowledge/.build_id` to `.gitignore`.
  - Added runtime discovery functions in `src/tam.c`: `tam_corpus_version(root)` and `tam_corpus_build_id(root)`.
  - Bare `tam` and `tam --help` report command version alongside corpus version.
  - `tam --version` reports command version, corpus version, `libtam` version, and installed cultivar metadata.

- **Orientation Shortcuts:**
  - Added `tam hello` and `tam hi` built-in CLI aliases routing directly to first-run orientation (`cmd_welcome`).

- **Corpus Review & Alignment:**
  - Updated all knowledge corpus pages to use top-level `tam inspect` and `tam check`:
    - `knowledge/maintain/index.html`: Added top-level `tam inspect` and `tam check` to the quick-reference maintenance card.
    - `knowledge/maintain/updates.html`: Replaced `tam install inspect` with `tam inspect`.
    - `knowledge/maintain/rollback.html`: Highlighted `tam check storage_btrfs` and `tam check storage_root_space`.
    - `knowledge/maintain/power.html`: Highlighted `tam check graphics_card`.
    - `knowledge/maintain/recovery.html`: Highlighted `tam check privilege_pkexec`.
    - `knowledge/install/index.html`: Highlighted top-level `tam inspect` and `tam check` alongside `tam install inspect`.
    - `knowledge/hardware/resources.html`: Added memory and storage diagnostics using `tam inspect` and `tam check`.
    - `knowledge/welcome.html`: Documented `tam inspect` and `tam check`, plus `tam hello` / `tam hi`.
    - `knowledge/shell/index.html`: Added section on user shell aliases and the one-place-to-look principle.
    - `knowledge/tam.html`: Added exhaustive table of all 14 `tam` subcommands, documented decoupled versioning architecture.
    - `knowledge/index.html`: Highlighted `tam inspect` and `tam check` across User Guide Part IV and Part VII.
  - Enriched ecosystem documentation:
    - Added **Fruit** (6th layer) and command varieties (`*-nut`, `*-root`, `*`) to `knowledge/ecosystem/terminology.html`.
    - Added Fred's 6 Guiding Principles to `knowledge/ecosystem/philosophy.html`.

- **Roadmap & Planning:**
  - Inserted Milestone 16 as version `0.24.0` in `ROADMAP.md` and `docs/plans/development.md`.
  - Renumbered all future milestones: 0.25.0 (Why Axis), 0.26.0 (Depth Ladder), 0.27.0 (Learn Mode), 0.28.0 (Syntax Dissection), 0.29.0 (Installation Framework), and 1.0.0 (Production Stable).
  - Authored dedicated plan in `docs/plans/decoupled-versioning-and-corpus-alignment.md`.

- **Completions, Man Pages, and Tests:**
  - Updated `completions/bash/tam` with `hello` and `hi`.
  - Updated `man/tam.1` synopsis and descriptions for `hello`, `hi`, and decoupled corpus versioning.
  - Updated `tests/run.sh` to test 0.24.0, dual version strings, mock corpus version override, and `hello`/`hi` aliases.
