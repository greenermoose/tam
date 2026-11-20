# Pager Hyperlink Navigation Keys (a/s/d/f) and Info Dir Rebind (i)

- **Date:** 2026-10-15
- **Stage:** Develop — Hyperlink navigation ergonomics: 2D directional navigation pad (`a` left, `s` up, `d` down, `f` right), first-visible-link selection, and rebind GNU Info directory from `d` to `i`.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred directed the keybinding changes and hyperlink navigation design. Antigravity claimed the repository paths using `tam-work`, implemented the 2D spatial hyperlink navigation algorithms (`src/pager.c`), rebound the GNU Info directory key from `d` to `i`, updated the pager status line, documented the new keybindings in `man/tam.1` and `src/info.c`, authored test fixture `tests/fixtures/nav-grid.html`, integrated test assertions in `tests/run.sh`, updated `ROADMAP.md` and `AI_PROVENANCE.md`, and verified clean builds and 100% test passes.

## Guiding prompt

> Let's change some of the keybindings for the tam command and add a better way to navigate among links.
> 1) Change the keybinding to get to the GNU Info directory from d to i.
> 2) Add keybindings for a, s, d, and f that allow navigation to the next or previous hyperlink.
> 3) If no hyperlink has been selected yet, any of these hyperlink selectors selects the first hyperlink visible on the screen. Then, after a hyperlink has been selected, they work as follows:
> a moves to a hyperlink to the left on this same line, wrapping to the line above. It eventually gets to the first hyperlink in the file, and wraps from the first to the last. It is functionally equivalent to Shift + Tab.
> s moves to a hyperlink above the selected hyperlink. It eventually gets to the first hyperlink in the file, and wraps around.
> d moves to a hyperlink below the selected hyperlink. It eventually gets to the last hyperlink in the file, and wraps around to the first.
> f moves to a hyperlink to the right on the same line, wrapping to the next line if there is no hyperlink on the same line. It eventually gets to the last hyperlink in the file, and wraps around to the first. It is functionally equivalent to Tab.

## Decisions and scope

- **Info Directory Key Rebinding (`i`):**
  - Changed the shortcut to jump to the root GNU Info directory catalog (`info:dir`) from `d` to `i` in `src/pager.c`.
  - Updated pager status line cues from `d dir` to `i dir`.
  - Updated help output in `tam info --help` (`src/info.c`) and man page documentation (`man/tam.1`).
- **2D Hyperlink Navigation Pad (`a`, `s`, `d`, `f`):**
  - **Initial Selection:** When no hyperlink is currently selected (`st->selected < 0`), pressing any of `a`, `s`, `d`, `f`, `Tab`, or `Shift-Tab` dynamically selects the first hyperlink visible within the terminal viewport (`[st->top, st->top + usable - 1]`). This prevents jumping back to line 1 when a user has scrolled deep into a document before initiating link selection.
  - **Horizontal Movement (`a` & `f`):**
    - `f`: Advances to the next hyperlink in document reading order (to the right on the current line, or wrapping to subsequent lines), wrapping from the last link to the first. Fully equivalent to `Tab`.
    - `a`: Circulates to the preceding hyperlink (to the left on the current line, or wrapping to preceding lines), wrapping from the first link to the last. Fully equivalent to `Shift-Tab`.
  - **Vertical Movement (`s` & `d`):**
    - Enhanced `linkref_t` and `pager_build_links` to track visual horizontal columns (`col_start`, `col_end`).
    - `s`: Jumps to the nearest line with hyperlinks strictly above the current line, selecting the link closest horizontally (minimizing horizontal center distance). When on the top-most line with links, wraps around to the bottom-most line with links.
    - `d`: Jumps to the nearest line with hyperlinks strictly below the current line, selecting the link closest horizontally. When on the bottom-most line with links, wraps around to the top-most line with links.
    - Gracefully handles degenerate cases (e.g. all links on a single line) by cycling through links without getting stuck.
- **Verification and Test Suite:**
  - Added fixture `tests/fixtures/nav-grid.html` with a 2x2 grid of links for testing multi-column 2D navigation.
  - Added PTY integration tests in `tests/run.sh` covering:
    - Initial link selection with `a`, `s`, `d`, `f` on initial screens and after scrolling down with `j`.
    - 2D grid navigation: left, right, up, down, column tracking, and wrapping boundaries.
    - Rebound `i` key verifying jump to `info:dir`.
    - Verification of `tam info --help` for `i` and `a/s/d/f` documentation.
