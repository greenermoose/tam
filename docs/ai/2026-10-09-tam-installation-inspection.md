# tam installation inspection (0.5.0)

- **Date:** 2026-10-09
- **Stage:** Develop — milestone 4 (installation inspection).
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.2
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred requested an implementation plan for the `tam install` subcommand, reviewed the draft in Micro, answered open questions regarding bare `tam install` overview behavior and output layout, and approved proceeding. Antigravity implemented `tam install` and `tam install inspect` in C against `libtam`, honest stubs for future installer verbs, guide updates, test suites, and documentation.

## Guiding prompts

> Take a look at the tam command repo and prepare an implementation plan for the next slice, the tam install subcommand. Ask if you have questions.

> I have answered those questions in the doc and saved it. Read it, then close the micro window and proceed.

## Decisions and scope

- **Milestone 4 implementation:**
  - Implemented modular `src/install.c` and `src/install.h` linked into `build/tam`.
  - **`tam install` (bare):** Prints an informative usage overview of available subcommands (`inspect`, `--help`) and upcoming lifecycle verbs (`plan`, `apply`, `verify`), exiting 0.
  - **`tam install --help` (and `tam install help`):** Exactly matches bare overview and exits 0. Extra arguments produce a clean usage error (exit 2).
  - **`tam install inspect`:**
    - Read-only inspection of host facts (`/etc/os-release` parsing, `uname()`, `$PATH` scanning for package and privilege tools, `/proc/mounts` root filesystem probe, `/dev/dri` graphics devices).
    - Resolves installed Tamlinux cultivar version from `$XDG_CONFIG_HOME/tamlinux/version`, `~/.config/tamlinux/version`, and `$TAMLINUX_VERSION` (falling back to "unavailable").
    - Maps supported profiles: `arch-hyprland, arch` on Arch/Omarchy; `void` on Void; `antix` on antiX/Debian; or `none (unsupported host distribution)`.
    - Sectioned key-value layout exactly matching §2.2 of the approved plan.
    - Zero ANSI escape sequences; respects `--pipe` and redirected stdout.
    - Strict state isolation: never writes `welcome_seen` or modifies any file.
  - **Unimplemented operations (`plan`, `apply`, `verify`):** Truthfully reports each operation is not implemented yet, explaining the installation framework milestone, and exits 1.
  - **Knowledge & Guide updates:**
    - Updated `knowledge/install/index.html` to document `tam install inspect` and the 4-step framework lifecycle (`inspect`, `plan`, `apply`, `verify`).
    - Updated `tests/snapshots/install-index.txt` golden fixture.
    - Added `inspect` and `inspection` keywords to `tam explain` curated topic map.
  - **Versioning:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` from `0.4.0` to `0.5.0`.

## Verification

- `make` compiles cleanly with zero warnings under `-O2 -Wall -Wextra -Wpedantic -std=c11`.
- `make test` runs all behavior checks in `tests/run.sh` and `tests/guide.sh`:
  - `tam --version` asserts `0.5.0`.
  - `tam install` prints usage and exits 0.
  - `tam install inspect` reports all required sections, fields, and profiles.
  - Cultivar version resolution in `inspect` accurately reports `unavailable`, file-configured versions, or environment overrides.
  - Unimplemented verbs `plan`, `apply`, and `verify` exit 1 with clear messages.
  - Negative isolation tests confirm `install` and `install inspect` never touch `welcome_seen`.
  - Golden snapshot matching for updated `knowledge/install/index.html` passes.
- Staged installation and live binary installation verified.
