# Dynamic Topic Guides and Dedicated tam Command Page (tam 0.23.0)

- **Date:** 2026-10-15
- **Stage:** Develop — Milestone 15: Dynamic Topic Guides, tam explain Topic Keywords Catalog, and Dedicated `tam` Command Page (`knowledge/tam.html`), version bump to 0.23.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred directed work on Version 0.23.0 for `tam` in the `tam` repository. Antigravity claimed the repository paths using `tam-work`, updated the Roadmap and Development Plan, authored the dedicated `tam` command page (`knowledge/tam.html`), dynamically generated topic guides and topic keyword catalogs (`src/index.c`, `src/index.h`, `src/tam.c`), integrated pager display for `tam explain`, added depth support to `tam show`, updated bash completions (`completions/bash/tam`), manual page (`man/tam.1`), updated test suites and snapshots, and verified clean test pass across all lanes.

## Guiding prompt

> Fix tam explain tam to display a topic guide to tam. tam explain should always display a topic guide, never take you directly to a page. tam show is for going directly to a page.
>
> Topic guides are lists of pages, with each page explained. The list of topics should be dynamically created based on meta tags. Each tam page should have a list of keywords. When you type tam explain, you should get a list of all the topic keywords that you can page through.
>
> tam explain should always present its results in the pager, just like tam show.
>
> Work on this project by first updating the plan and ROADMAP for tam. Insert this as version 0.23.0 and renumber all the future versions accordingly.
>
> There should also be a page devoted to tam, which you can find by either:
> - `tam explain tam` -> you'll get a topic guide for tam showing pages that have tam as a keyword in their meta data
> - `tam show tam` -> you'll get the page for tam
>
> The page for tam must include a section that describes how to code tam. I need to know what language it is written in (I know it is C), and how to make and install it. All the targets for the makefiles must be described. I also need to know the libraries and how to link them in (I need to know the compiler chain and the commands that the makefile uses to build the command). Furthermore, the tam page must explain how the tam command gets included in a version of Tamlinux. The whole develop -> test -> run -> release cycle must be fully explained (or a link to a page that does explain that cycle must be provided on the tam page).

## Decisions and scope

- **Roadmap & Planning Update:**
  - Inserted Milestone 15: Dynamic Topic Guides & Dedicated `tam` Command Page as version `0.23.0` in `ROADMAP.md` and `docs/plans/development.md`.
  - Renumbered all future milestones: 0.24.0 (Why Axis), 0.25.0 (Depth Ladder), 0.26.0 (Learn Mode), 0.27.0 (Strict Semantic HTML Parser), 0.28.0 (System Installation Orchestrator), and 1.0.0 (Production Stable).
  - Authored dedicated architecture and contract document in `docs/plans/topic-guides-and-tam-page.md`.
  - Bumped `TAM_COMMAND_VERSION` to `"0.23.0"` in `src/version.h`.
- **Command Subcommand Invariant:**
  - `tam explain` strictly opens a Topic Guide; it never jumps directly into a single page.
  - `tam show` is the sole direct page navigation subcommand. Added `--level` support to `tam show` (`tam show --level LEVEL [PAGE]`) to view pages filtered directly by depth.
- **Dynamic Topic Guides & Metadata Extraction (`src/index.c`, `src/index.h`, `src/tam.c`):**
  - Added `tam_index_find_topic_pages()` to collect all corpus pages matching a topic or keyword in metadata (`<meta name="topic">`, `<meta name="keywords">`).
  - Added `tam_index_get_keywords()` to dynamically extract and sort all unique topic keywords across the corpus, counting matching pages and extracting primary summaries.
  - Implemented `tam_topic_guide_document()` to render interactive topic guides with breadcrumbs, descriptions, page links, tracks, depth levels, and explanations.
  - Implemented `tam_topic_keywords_document()` to display the full dynamic topic keywords catalog.
  - Wired virtual URI schemes `tam:explain?topic=...` and `tam:keywords` into `corpus_load()` for seamless pager history and link navigation.
  - Routed both bare `tam explain` and `tam explain TOPIC` through the interactive ANSI pager (`tam_pager_run`), falling back to clean text under `--pipe`.
  - Maintained `tam explain --help`, `-h`, and `help` within the <=15 line screen budget.
- **Dedicated tam Knowledge Page (`knowledge/tam.html`):**
  - Written with semantic HTML across Beginner, Intermediate, and Expert depth levels:
    - Language: Standard C (C11 standard, ISO/IEC 9899:2011).
    - Build and install: prerequisites, quick build commands, local prefix installation (`PREFIX=$HOME/.local/dev`).
    - Complete Makefile targets reference: `all`, `command`, `corpus`, `test`, `test-command`, `test-corpus`, `check`, `install`, `uninstall`, `clean`, `FORCE`.
    - Compiler chain: `cc`, `CFLAGS` (`-O2 -Wall -Wextra -Wpedantic -std=c11`), `CPPFLAGS`, `LDFLAGS` (`-Wl,-rpath,/usr/local/lib -L/usr/local/lib`), libraries (`-ltam`, `-lz`), and exact build command.
    - Deployment lifecycle: disciplined 4-stage develop -> test -> run -> release cycle via `tamlinux-packages`, immutable `/nix/store` package outputs, Home Manager activation, and generational rollback.
  - Added `tam` keyword across related pages (`welcome.html`, `improve/toolchains.html`, `improve/scripting.html`) and linked `tam.html` in `knowledge/index.html`.
- **Manual, Completions, and Verification:**
  - Documented `tam show --level` and updated `tam explain` topic guide contract in `man/tam.1`.
  - Added `--level` completion for `tam show` in `completions/bash/tam`.
  - Updated `tests/run.sh` to test version 0.23.0, explain synopsis, bare explain keyword listing, `tam explain tam` (Topic Guide), and `tam show tam` (page contents).
  - Added `tam` to `tests/guide.sh` and refreshed text snapshots (`tests/snapshots/index.txt`, `tests/snapshots/tam.txt`).
  - Verified full test suite (`make check`) passes 100% with zero regressions.

## Verification

- `make all`: Clean build under C11 with zero warnings.
- `make corpus`: Generated `knowledge/index.tsv` and `knowledge/index.json` with 44 entries and zero errors.
- `make test-command`: 100% of command tests passed (`tests/run.sh`).
- `make test-corpus`: 100% of guide tests passed (`tests/guide.sh`).
- `make check`: All suites passed green.
