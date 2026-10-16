# tam navigation and output ergonomics (tam 0.18.0)

- **Date:** 2026-10-14
- **Stage:** Develop — Milestone 10: Quick Guide index return (`\``), Goldilocks screen budget, interactive topics pager, and output modifiers implemented; version bumped to 0.18.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5
- **Correction (2026-10-15):** CLI version 1.3.5 is taken from the agy process log that served this session; this record originally said 1.3.4.
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested: "Claim and start work on the next milestone for the tam command. Let me know if you run into any difficulties." Antigravity identified Milestone 10 (Navigation & Output Ergonomics: Quick Guide Index Return Keybinding, Goldilocks Screen Budget, and Output Modifiers) as the next unstarted milestone in `docs/plans/development.md` and `ROADMAP.md`, claimed repository paths using `tam-work`, implemented the complete Milestone 10 deliverables across `src/pager.c`, `src/man.c`, `src/index.h`, `src/index.c`, `src/render.h`, `src/tam.c`, `src/install.c`, `src/version.h`, `completions/bash/tam`, and `tests/run.sh`, updated development plans and roadmaps, and verified the entire test suite (`make test-command`, `make test`).

## Guiding prompt

> Claim and start work on the next milestone for the tam command. Let me know if you run into any difficulties.

## Decisions and scope

- **Concurrency Claim & Workstream Discipline:**
  - Claimed command lane and documentation paths using `tam-work begin --owner agy --session <id> --scope src --scope completions --scope tests --scope docs --scope ROADMAP.md --scope AI_PROVENANCE.md --note 'Milestone 10: Navigation and Output Ergonomics'`.
  - Maintained strict path boundaries inside `tam`.
- **Quick Guide Index Return Keybinding (`` ` ``):**
  - In `src/pager.c`, bound backtick (`` ` ``) keypress to invoke `follow(st, "index.html")`. If the user is already on the root index page, it scrolls cleanly to line 0.
  - Pushes the current page path and reading line offset to the back history stack (`hist_push`), allowing users to explore deep subpages or man pages and immediately return to the index, with `u` returning back to where they left off.
  - Updated status line in `src/pager.c` to advertise the shortcut: `` ` index `` alongside existing navigation keys (`Tab/S-Tab link`, `/ search`, `q quit`).
  - In `src/man.c`, initialized `ctx.root` with `tam_find_knowledge_root()` in `tam_cmd_man`, and added support in `man_unified_load()` to resolve `index.html` and `tam:topics` documents through the corpus loader, enabling seamless index return directly from system man pages.
- **Goldilocks Screen Budget:**
  - Enforced strict ≤20 line screen budget for interactive orientation and usage commands so syntax never scrolls off an 80×24 terminal window.
  - Condensed `tam --help` from 32 lines down to 14 lines (≤20 lines) by grouping and consolidating redundant per-subcommand flag lines.
  - Refactored bare `tam explain` to emit strictly a 13-line usage overview (≤15 lines) explaining how to query topics and pointing users to `tam explain topics`, rather than dumping all 29 catalog topics.
- **Interactive Hyperlinked Topics Pager (`tam explain topics`):**
  - Implemented `tam_topics_document()` in `src/tam.c` (declared in `src/render.h`) generating dynamic HTML with `<a href="...">` hyperlinks to topic pages.
  - When invoked interactively in a TTY without `--pipe`, `tam explain topics` launches directly into `tam`'s built-in ANSI pager (`tam_pager_run()`), letting users navigate with Tab / arrow keys and hit Enter to jump directly to any topic explanation.
  - Taught `corpus_load()` in `src/tam.c` to recognize the virtual document URI `"tam:topics"`, ensuring that pressing `u` (back) or `` ` `` (home) inside the pager preserves history and seamlessly returns to the topics list.
  - Under `--pipe` or non-TTY, emits clean text catalog. Supports `--level LEVEL` depth filtering.
- **Output Modifiers (`--terse` and `--verbose`):**
  - Implemented `tam_index_print_catalog_ex()` in `src/index.c`:
    - `--terse`: Emits one topic slug per line, optimized for piping into `fzf`, `grep`, and shell loops.
    - `--verbose`: Emits exhaustive diagnostics, titles, paths, tracks, levels, prerequisites, and keyword arrays.
    - Default: Clean human-friendly title and summary overview.
  - Added `--terse` flag to `tam install check` (and `tam install inspect --check`), emitting compact `task_id status` pairs (e.g. `privilege_pkexec pass`).
- **Static Bash Completions:**
  - Updated `completions/bash/tam` to complete `topics`, `--terse`, and `--verbose` for `tam explain`, and `--terse` for `tam install`.
- **Version Bump:**
  - Bumped `TAM_COMMAND_VERSION` in `src/version.h` to `0.18.0`.

## Verification

- `make command` compiled cleanly with `gcc` with zero warnings under `-Wall -Wextra -Wpedantic`.
- `make test-command` passed all 22 test suites:
  - Line count assertions verified: `tam --help` is 14 lines (≤20 lines), bare `tam explain` is 13 lines (≤15 lines).
  - PTY interactive test verified: backtick (`` ` ``) key jumps immediately to `index.html` from `shell/pipelines.html`, and `u` returns back to pipelines.
  - PTY interactive test verified: `tam explain topics` launches interactive pager with live hyperlinks, follows link to topic explanation, and returns via `` ` `` to index.
  - Output modifier assertions verified: `tam explain topics --pipe`, `--terse`, `--verbose`, `--level expert --terse`.
  - Check status assertion verified: `tam install check --terse` emits compact token format.
- `make test` passed 100% across all command and guide tests.
