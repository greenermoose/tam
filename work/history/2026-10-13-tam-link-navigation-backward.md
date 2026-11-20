# tam backward link navigation (tam 0.11.0)

- **Date:** 2026-10-13
- **Stage:** Develop — backward link circulation implemented; version bumped to 0.11.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.4
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested improving the `tam` command so that `Shift-Tab` circulates backward among links in the pager, noting that `Tab` moved forward with no documented or discoverable way to go back. Antigravity confirmed `Tab` (forward) and `Shift-Tab` (backward) as the standard, accessible HCI convention, implemented backward link navigation supporting ANSI CSI Z (`\033[Z`), CSI `1;2Z`, Kitty `9;2u`, modifyOtherKeys `27;2;9~`, and Alt-Tab (`\033\t`) while keeping `N` as a fallback, updated the pager status line to `Tab/S-Tab link` with visual column truncation to prevent UTF-8 slicing, updated corpus documentation and snapshots, and added comprehensive PTY-based automated test coverage.

## Guiding prompts

> Improve the tam command so that shift-Tab circulates backward among links. Right now, Tab goes to the next link but there is no way to go back. (Or if there is a way to go back, it is not documented or easy to find.) I have worked with other programs that allow Tab for next link and shift-Tab for previous link. If there is a better convention and good HCI reason for using a different keybinding for navigation among links, please let me know and we can discuss. Otherwise, if you accept my idea of Tab forward and shift-Tab backward, please implement for tam 0.11.0. Thank you!

## Decisions and scope

- **HCI convention:** Affirmed `Tab` for next link and `Shift-Tab` for previous link as the universal, gold standard convention for keyboard link navigation and focus traversal across web browsers, terminal pagers (w3m, lynx), TUIs, and W3C WAI-ARIA accessibility standards.
- **Escape sequence decoding:** In `src/pager.c`, expanded escape sequence handling to recognize standard ANSI/ECMA-48 CBT (`\033[Z`), CSI `1;2Z` (`\033[1;2Z`), Kitty CSI u Shift-Tab (`\033[9;2u`), xterm modifyOtherKeys Shift-Tab (`\033[27;2;9~`), and Alt-Tab (`\033\t`), mapping them to backward link navigation (`select_next_link(st, -1)`).
- **Backward link circulation:** Retained wraparound semantics in `select_next_link`: pressing `Shift-Tab` wraps backwards from the first link to the last link on the page, and scrolls the pager viewport to ensure the selected link is visible. Kept `N` as a backwards-compatible fallback.
- **Status bar & UTF-8 safety:** Updated the pager status line hint from `Tab link` to `Tab/S-Tab link`. Added `status_byte_limit` helper to measure display columns and truncate status output cleanly at UTF-8 character boundaries, avoiding multibyte slicing (such as in `→` and `←`) on narrow terminals.
- **Documentation & snapshots:** Updated `README.md`, `knowledge/index.html`, `tests/snapshots/index.txt`, `src/man.c`, and plans to document `Tab`/`Shift-Tab` link navigation.
- **Version bump:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` to `0.11.0`.

## Verification

- `tests/run.sh` passes all command tests, including a new automated PTY test that exercises `Tab` forward selection, `Shift-Tab` backward selection, first-to-last link wraparound, CSI `1;2Z`, Kitty `9;2u`, modifyOtherKeys `27;2;9~`, and `N` fallback navigation.
- `tests/guide.sh` passes all guide tests and validates golden text snapshots without drift.
- `make test` passes cleanly.
- Development build installed under `~/.local/dev` via `make install PREFIX=$HOME/.local/dev`.
