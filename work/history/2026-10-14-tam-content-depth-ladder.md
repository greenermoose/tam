# tam content-depth ladder and fading templates (tam 0.15.0)

- **Date:** 2026-10-14
- **Stage:** Develop — Milestone 7: content-depth ladder and fading templates implemented; version bumped to 0.15.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5
- **Correction (2026-10-15):** CLI version 1.3.5 is taken from the agy process log that served this session; this record originally said 1.3.4.
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested: "Take a look in the tam repo at the plan for the tam command. Implement the next milestone. I believe that is version 0.15.0, but double check." Antigravity confirmed Milestone 7 (0.15.0) from `ROADMAP.md` and `docs/plans/development.md`, claimed the required paths via `tam-work`, and implemented the complete content-depth ladder and fading templates: authored fading templates across core curated topics (`shell/jobs.html`, `shell/pipelines.html`, `filesystem/permissions.html`, `hardware/resources.html`), implemented dynamic depth section extraction and in-page depth links (`src/depth.c`, `src/depth.h`), added `--level` options and catalog filtering to `tam explain`, projected depth metadata and prerequisites into `knowledge/index.tsv` and `knowledge/index.json` via `tools/build-index.py` and `src/index.c`, added depth indicators to `tam search`, added Bash shell completions for `--level`, updated automated tests (`tests/run.sh` and `tests/guide.sh`) and golden snapshots, and bumped `TAM_COMMAND_VERSION` to `0.15.0`.

## Guiding prompts

> Take a look in the tam repo at the plan for the tam command. Implement the next milestone. I believe that is version 0.15.0, but double check.
> Can you keep going on this project?

## Decisions and scope

- **Fading Template Architecture:**
  - Standardized three pedagogical depth rungs:
    - **Beginner:** One-line summary, prerequisites (links to introductory guide pages), single annotated command sequence with realistic expected output, "what just happened" explanation, common pitfall, and next-step link.
    - **Intermediate:** Summary, worked example fragment or scaffold, completion task (user fills in missing step) with target output, and verification hints.
    - **Expert:** Terse reference, compact option/flag tables, links to system manual pages (`tam man`), zero redundant scaffolding.
  - Authored complete fading templates for four core topics:
    - `knowledge/shell/pipelines.html` (`topic: pipelines`, `prereqs: shell`)
    - `knowledge/shell/jobs.html` (`topic: jobs`, `prereqs: shell`)
    - `knowledge/filesystem/permissions.html` (`topic: permissions`, `prereqs: filesystem`)
    - `knowledge/hardware/resources.html` (`topic: resources`, `prereqs: filesystem`)
- **Depth Module & Rendering (`src/depth.h`, `src/depth.c`):**
  - Implemented level bitmask enum (`TAM_LEVEL_BEGINNER`, `TAM_LEVEL_INTERMEDIATE`, `TAM_LEVEL_EXPERT`).
  - Added `tam_depth_split_href` to extract `?level=NAME` query parameters while preserving base file paths for loader and history stacks.
  - Added `tam_depth_apply` to parse `<meta name="level">`, remove unrequested `<section data-level="...">` blocks when a level is selected, and insert an interactive linked depth banner (`Depth: beginner, intermediate, expert...`) after the first `<h1>` on laddered pages.
  - Updated `src/render.c` to end blocks cleanly on `section`, `article`, and `div` tags.
- **Corpus Metadata Projection:**
  - Extended `tools/build-index.py` to extract `level`, `track`, and `prereqs` metadata, validate declared levels against `<section data-level="...">` blocks, ensure prerequisites refer to valid topics, and generate 8-column `knowledge/index.tsv` and structured `knowledge/index.json`.
  - Normalized `knowledge/ecosystem/libc.html` metadata from non-standard `advanced` to `expert`.
  - Updated `src/index.h` and `src/index.c` to load level bitmasks, track strings, and prerequisite arrays.
- **Command Surface & Completion:**
  - Added `--level LEVEL` option to `tam explain`:
    - `tam explain --level <level> <topic>` opens the page at the chosen depth (or gives truthful diagnostics if the level is not offered).
    - `tam explain --level <level>` lists only topics offering that specific depth.
    - Emits usage error (exit 2) for unknown level names.
  - Extended `tam search` and `tam search --json` to report depth rungs (`levels`), `track`, and `prereqs` for every result.
  - Updated `completions/bash/tam` to complete `--level` and its valid arguments (`beginner`, `intermediate`, `expert`).
- **Version Bump:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` to `0.15.0`.
- **Testing & Verification:**
  - Updated `tests/run.sh` with automated assertions for `--version 0.15.0`, depth-specific rendering, catalog filtering, invalid level handling, search depth indicators, and Bash completions.
  - Updated `tests/guide.sh` to recognize `man:` links and `?level=` query parameters in internal link verification.
  - Refreshed golden snapshots in `tests/snapshots/` using `UPDATE=1 tests/guide.sh`.

## Verification

- `make test` runs both `tests/run.sh` and `tests/guide.sh` cleanly with exit code 0.
- Staged installation and uninstallation tested with `make install DESTDIR=... PREFIX=/usr`.
