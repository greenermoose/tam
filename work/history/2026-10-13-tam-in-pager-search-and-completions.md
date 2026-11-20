# tam in-pager search, reading telemetry and completions (tam 0.14.0)

- **Date:** 2026-10-13
- **Stage:** Develop — in-pager text search, reading progress telemetry, and static shell completions implemented; version bumped to 0.14.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.4
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested implementing `tam` version 0.14.0 based on the work plan reviewed in Micro. Fred reviewed the security implications of shell auto-completions and selected Option 2 (pure static completions for subcommands and flags with zero dynamic file I/O). Antigravity implemented in-pager keyword search (`/`) with in-buffer hit highlighting, forward (`n`) and backward (`N`) match navigation, and `Esc` clearing; enhanced pager status line with reading telemetry metrics (`L...`, percentage, `Top`, `Bot`, `All`); created static shell completion scripts for Bash (`completions/bash/tam`), Zsh (`completions/zsh/_tam`), and Fish (`completions/fish/tam.fish`); updated `Makefile` installation rules; added automated PTY and shell completion tests in `tests/run.sh`; bumped `TAM_COMMAND_VERSION` to `0.14.0`; and updated `ROADMAP.md` and `docs/plans/development.md`.

## Guiding prompts

> Please create a work plan for version 0.14.0, then let me review it before you start on it.
> Please show me the work plan in a micro window.
> The work plan looks good, but tell me the security implications of the shell auto-completions. Are these a good idea? Why does tam need to provide this functionality?
> Let's go with option 2 for now. I like the idea of static completions for tam subcommands. Is that what option 2 would provide?
> I've refreshed the review window. Plan looks good. Please close the review window, claim the paths, and begin implementation!
> Tamlinux will only support bash shell. So we can remove code for fish and zsh. If you install Tamlinux, you will have the bash shell.

## Decisions and scope

- **In-Pager Text Search (`src/pager.c`):**
  - Added interactive search prompt activated by `/`, with cursor positioning at the status line, character echoing, backspace editing, and Enter/Esc handling.
  - Implemented case-insensitive line search across rendered document lines, recording line indices and byte offsets in `st->hits`.
  - Added in-buffer standout hit highlighting in `emit_line` without breaking hyperlink underlines or multi-line link continuation styling.
  - Added `n` (jump to next match) and `N` (jump to previous match) with automatic viewport scrolling and match count messaging (`[1/5] pattern`).
  - Added `Esc` search clearing that resets search highlights and query while keeping the pager open. Page transitions (`follow`, `go_back`, `go_forward`) also clear active search state.
  - Retained `N` backward link circulation fallback when no search query is active.
- **Reading Progress Telemetry (`src/pager.c`):**
  - Updated status line rendering to compute exact line position metrics: `L1/N (All)` when the document fits on screen, `L1/N (Top)` at the beginning, `LN/N (Bot)` at the end, and `Lk/N (X%)` in between.
  - Updated control hint section to include `/ find` and `n/N next` alongside link, navigation, and exit keys.
- **Static Bash Auto-Completions (Option 2):**
  - Following Fred's confirmation that Tamlinux will only support Bash, completions were scoped strictly to Bash (`completions/bash/tam`); Fish and Zsh completion scripts were removed.
  - Restricts completions strictly to hardcoded subcommand names (`show`, `welcome`, `explain`, `search`, `man`, `install`, `help`) and flags (`--help`, `-h`, `--version`, `-V`, `--pipe`, `--json`).
  - Strict zero-file-I/O security invariant: completion script performs no filesystem access, executes no dynamic subshells, and parses no untrusted data on `<TAB>`.
  - Added `Makefile` install/uninstall targets for the standard bash-completion directory.
- **Version Bump:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` to `0.14.0`.
- **Testing & Verification:**
  - Added automated Python PTY interactive tests in `tests/run.sh` covering search activation, hit highlighting, match cycling, `Esc` clearing, and reading progress telemetry.
  - Added completion verification tests asserting `bash -n` syntax and execution of completion functions.
  - Verified `make clean && make test` and `make install DESTDIR=...` clean pass.

## Verification

- `tests/run.sh` passes all command foundation, interactive PTY, and completion checks.
- `tests/guide.sh` passes all guide rendering and golden snapshot checks.
- Staged installation and uninstallation tested with `DESTDIR`.
