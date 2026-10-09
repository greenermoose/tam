# tam orientation and explanation (0.4.0)

- **Date:** 2026-10-09
- **Stage:** Develop — milestone 3 (orientation and explanation).
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.2
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** `b01faf9` ("Implement tam orientation and curated explanation (0.4.0)").
- **Authorship:** Fred reviewed the slice proposal, selected a combined slice, confirmed the curated curriculum topics as sufficient, directed pager navigation with text fallback in pipes, and closed the review window. Antigravity implemented `tam welcome`, atomic `welcome_seen` state tracking, `tam explain <topic>`, test suites, documentation, and installed the binary. No delegation.

## Guiding prompts

> Take a look at the tam command and plans for it. What is the next slice to work on? Develop an implementation plan and share it with me so we can discuss.

> 1 combined slice
> 2 yes, sufficient
> 3 open in the pager, falling back to text in pipes
> Please close the micro review window and begin work on this implementation plan. Ask if you have any further questions.

## Decisions and scope

- **Combined milestone 3 slice:** Implemented both `welcome` (orientation) and `explain <topic>` (curated explanation) together.
- **`tam welcome`:**
  - Opens `welcome.html` from the knowledge root in the interactive pager when running in a terminal.
  - Emits clean text to stdout when stdout is redirected, piped, or when `--pipe` / `-p` is passed.
  - On successful display (exit code 0), writes an ISO 8601 UTC timestamp atomically to `$XDG_STATE_HOME/tamlinux/welcome_seen` (falling back to `~/.local/state/tamlinux/welcome_seen`) using `tam_fs_write_atomic`.
  - Ensures parent directory exists prior to writing.
  - Strict isolation: bare `tam`, `tam browse`, `tam --help`, `tam help`, `tam --version`, `tam explain`, and failed commands never write or touch `welcome_seen`.
- **`tam explain <topic>`:**
  - Strictly treats topic query strings as data; never spawns a shell or executes topic arguments. Shell metacharacters (`|`, `>`, `;`, `$()`) are handled harmlessly as literal strings.
  - Matches curated topics against existing curriculum pages: `welcome`/`orientation` (`welcome.html`), `install`/`installation`/`maintain`/`updates`/`recovery`/`layers` (`install/index.html`), `pipelines`/`pipes`/`redirection`/operators (`shell/pipelines.html`), and `man`/`manual`/`whatis`/`apropos` (`man/index.html`).
  - Opens matched page in interactive pager (or plain text when piped).
  - On unrecognized topics, prints `tam: no explanation found for '<topic>'`, points to `tam browse`, and exits 1.
  - Missing topic argument or unexpected flags exit 2 with usage diagnostics.
- **Shared infrastructure:**
  - Extracted common `view_knowledge_page` helper in `src/tam.c` shared across `browse`, `welcome`, and `explain`.
  - Command version bumped `0.3.0` -> `0.4.0` in `src/version.h` (`TAM_COMMAND_VERSION`) and tests.
  - Updated `print_intro` and `print_help` to document `welcome` and `explain`.
- **Standing install rule:**
  - Rebuilt and verified clean under `-Wall -Wextra -Wpedantic -std=c11`.
  - Installed to `/usr/local/bin/tam` via `pkexec make -C /home/fred/Code/tamlinux/tam install`.

## Verification

- `make` builds with zero warnings.
- `make test` runs `tests/run.sh` and `tests/guide.sh` with all checks passing:
  - `tam --version` asserts `0.4.0`.
  - `tam welcome` creates `welcome_seen` with valid ISO 8601 timestamp.
  - Isolated runs of bare `tam`, `browse`, `help`, `version`, and `explain` do not create `welcome_seen`.
  - `tam explain` resolves curated keywords, safely handles arbitrary shell strings as plain data, and emits clear diagnostics on misses.
  - Piped outputs across all commands contain no ANSI terminal escape codes.
- Installed binary `/usr/local/bin/tam` reports `0.4.0`, matches `tam help` and `tam --help`, renders welcome and explain text, and is ready for use.
