# tam show subcommand and explain pipe quoting (0.7.0)

- **Date:** 2026-10-09
- **Stage:** Develop — subcommand renaming and explain query quoting.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.2
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** `6b2b3b9` ("Replace tam browse with tam show and add explain pipe quoting (0.7.0)").
- **Authorship:** Fred requested changing the guide subcommand from `tam browse` to `tam show` (with bare `tam show` showing the index page just like `tam browse` did), keeping `browse` as an explain topic handled by `tam explain browse`, and adding quoting support in `tam explain` for shell constructs like pipes (`|`) so users can invoke `tam explain '|'` or `tam explain "|"`. Antigravity implemented the feature in C, updated docs and plans across the project, updated tests and golden fixtures, bumped the command version, and verified all builds and tests.

## Guiding prompts

> Let's change how the tam command works. update the code and the plan and other docs. Instead of tam browse the subcommand should be tam show. tam show by itself shows the index page, just like tam browse does now. I just like "show" better than "browse". We can keep browse as a topic that tam explain browse can handle. Also, add to the explain command a way to quote things like pipes (|) so you can do something like tam explain '|' or tam explain "|". I assume tam explain | would not pass the shell parsing to give | to tam to process, right?

## Decisions and scope

- **Subcommand rename (`tam show`):**
  - Renamed `cmd_browse` to `cmd_show`. `tam show` by itself displays the guide index page in the terminal pager (or plain text when piped), preserving all previous options (`PAGE`, `--pipe` / `-p`).
  - `tam browse` is no longer a recognized subcommand and exits 2 with an unrecognized argument diagnostic pointing to `tam --help`.
  - Diagnostics for misses in `tam explain` now direct users to `tam show`.
  - Updated offline guide index page (`knowledge/index.html`) example to `tam show | head` and regenerated text test snapshot (`tests/snapshots/index.txt`).
- **Explain topic additions and quote stripping:**
  - Added `|` to `TOPIC_MAP` pointing to `shell/pipelines.html`.
  - Added `show` and `browse` to `TOPIC_MAP` pointing to `index.html`.
  - Implemented quote stripping in `match_curated_topic`: matching outer single or double quotes (`'...'` or `"..."`) are removed before keyword lookup.
  - Shell invocation note: in shells like bash, an unquoted `|` acts as a pipeline operator and causes a shell syntax error or pipeline split before `tam` is invoked; users must quote or escape it (`tam explain '|'`, `tam explain "|"`, or `tam explain \|`).
- **Tests and documentation:**
  - Added test coverage in `tests/run.sh` for `tam show`, retired `tam browse`, `tam explain '|'`, quoted forms (`"'|'"`, `'"|"'`), and `tam explain browse` / `tam explain show`.
  - Updated `tests/guide.sh` to use `tam show`.
  - Updated `tamlinux-packages/tests/installed-components.nix` test invocation.
  - Updated `README.md`, `tam/docs/plans/development.md`, `tam/docs/plans/offline-guide.md`, `tam/docs/plans/orientation-and-explanation.md`, and product plan `tamlinux/docs/plans/tamlinux-command.md`.
- **Versioning:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` from `0.6.0` to `0.7.0`.

## Verification

- `make clean && make && make test` passed cleanly with zero warnings:
  - `tam --version` asserts `0.7.0`.
  - `tam show --pipe` renders guide index.
  - `tam show welcome --pipe` renders welcome page.
  - `tam browse` exits 2 with unrecognized argument error.
  - `tam explain '|' --pipe`, `tam explain "'|'" --pipe`, and `tam explain '"|"' --pipe` render pipelines guide.
  - `tam explain browse --pipe` and `tam explain show --pipe` render guide index.
  - Golden snapshot match and guide integration tests pass.
