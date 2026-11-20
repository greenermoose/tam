# tam explain topic listing and scaling roadmap (0.6.0)

- **Date:** 2026-10-09
- **Stage:** Develop — explain topic discovery and scaling plan.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.2
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** `c3f0719` ("List available topics on bare tam explain (0.6.0)").
- **Authorship:** Fred requested updating `tam explain` so that invoking it without topic arguments displays a list of available explainable topics rather than reporting a missing topic argument error, along with a design plan for scaling when the explainable catalog grows large. Antigravity implemented the feature in C, created the scaling plan document, updated tests, bumped the command version, and verified build and tests.

## Guiding prompts

> For the tam command, make it so when you do tam explain, rather than saying missing topic arguments, it shows a list of topics. Have a plan for what should happen when the list of explainable topics gets really long. For now, the topic list is pretty short so you can just list them all.

## Decisions and scope

- **Bare `tam explain` behavior:**
  - Removed `missing_topic_error`. Invoking `tam explain` (or `tam explain --pipe`, `tam explain help`, `tam explain --help`) without a topic argument now prints a clean, organized listing of available topics and their keywords directly to stdout, exiting 0.
  - Topics are grouped by primary domain with 1-line summaries and their accepted aliases/keywords (`welcome`, `install`, `pipelines`, `man`, `guide`).
  - Output is formatted to fit comfortably on any standard 80x24 terminal window without scrolling.
  - Strict output safety: piped or redirected output contains zero ANSI terminal escape sequences.
  - Strict isolation: bare `tam explain` never touches `$XDG_STATE_HOME` or modifies `welcome_seen`.
  - Diagnostics for unmatched topics (`tam explain nonexistent`) now reference both `tam explain` (to discover topics) and `tam browse` (to explore the guide).
- **Topic scaling plan:**
  - Authored `docs/plans/explain-topic-scaling.md` outlining a 5-phase scaling roadmap:
    1. Phase 1 (Current, <20 topics / <50 keywords): Direct listing to stdout in a clean grouped format.
    2. Phase 2 (20–60 topics): Functional domain grouping and progressive disclosure (`tam explain` domain summary + `tam explain <domain>/` + `tam explain --all`).
    3. Phase 3 (60–200 topics): Interactive TUI pager integration (`isatty(STDOUT_FILENO)` opens interactive topic catalog with navigable links; `--pipe` outputs compact text).
    4. Phase 4 (200+ topics): Search / "apropos" matching (`tam explain search <query>`), "Did you mean?" suggestions on miss, disambiguation for partial matches.
    5. Phase 5: Corpus-driven metadata decoupling (topic definitions generated from HTML metadata / JSON) and shell auto-completion (`tam explain <TAB>`).
- **Versioning:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` from `0.5.0` to `0.6.0`.

## Verification

- `make` compiles cleanly with zero warnings under `-O2 -Wall -Wextra -Wpedantic -std=c11`.
- `make test` runs all tests in `tests/run.sh` and `tests/guide.sh`:
  - `tam --version` asserts `0.6.0`.
  - `tam explain` exits 0 and prints topic list with all primary topics.
  - `tam explain --pipe` prints clean topic list without escape sequences.
  - `tam explain help`, `--help`, `-h` match bare explain output.
  - Unmatched topics report error and exit 1.
  - Shell strings as topic input are treated strictly as plain data.
  - Negative isolation confirms bare `tam explain` does not write `welcome_seen`.
