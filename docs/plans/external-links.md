# External links open in the reader's own browser

**Plan status:** In progress (handoff)
**Reviewed:** 2026-10-23
**Scope:** Command 0.44.0 (pager); no corpus change.
**Authority:** The developer's approval of this plan on 2026-10-23 (below).
**Next action:** TCK27: finish the docs, run `make test`, install the
development build, then merge `feature/external-links` into `develop`.

## Problem and agreed outcome

Following an external link (`https://...`) in the pager did nothing except
show "external link (not opened)". The 2026-10-09 offline-guide decision kept
tam from fetching anything; Lynx could stay optional but never a dependency.

The developer asked whether to hand the link to another program or to fetch
and render the page in tam. The research found that every comparable TUI
hands off: less's `^O^O` with `LESS_OSC8_OPEN_*`, w3m's `M`, Lynx's
`EXTERNAL`, newsboat's `browser`, aerc's openers with an xdg-open fallback,
and Python's `webbrowser` with `$BROWSER`. Fetching in tam would need TLS and
more dependencies, would render real web pages poorly through tam's own HTML
subset, would parse untrusted HTML in C, and would end the offline identity.
tam therefore hands off and still never opens a socket itself.

The developer's choices (2026-10-23):

- **Desktop:** a graphical session (`WAYLAND_DISPLAY` or `DISPLAY` set) opens
  the link in the graphical browser: each `$BROWSER` entry in turn, then
  `xdg-open`, started detached. The pager stays on screen.
- **Console:** with no display, a text browser runs in the same terminal:
  `$TAM_TEXT_BROWSER`, else the first of w3m, lynx, elinks and links. The
  pager suspends, then redraws when the browser exits.
- **Confirmation:** the first Enter, `g` or Right Arrow on an external link
  names the browser ("Enter again opens it in X"); a second press opens it.
  Any other key cancels.
- **OSC 8:** external links are OSC 8 hyperlinks in capable terminals, so
  foot's URL mode and mouse clicks work. None on `TERM=linux`, `vt*` or
  `cons*`, and none in piped output.

Limits: only `http:` and `https:` open. A URL with a space, a control byte, a
non-ASCII byte or more than 2048 bytes is refused. No shell is involved; the
URL is its own argument to `execvp`.

## Baseline and dependencies

- Branch `feature/external-links`, from `develop` at `a52c031`, holds the
  implementation and this plan.
- Command 0.43.2 to 0.44.0. Corpus unchanged at 0.42.5.
- No new library or package; w3m, lynx and xdg-open are optional at runtime.

## Implementation and handoff

**TCK26 (external link hand-off and OSC 8): done on the branch.**

- `src/external.c` and `src/external.h`, a new module covering URL checks,
  browser choice, the detached and foreground launches, and the
  no-browser messages.
- `src/pager_nav.c`: `follow_external()` replaces the old "not opened"
  branch.
- `src/pager_input.c`: `handle_key` forgets a pending link on any other key.
- `src/pager.c`: `pager_suspend()` and `pager_resume()`.
- `src/pager_int.h`: `pending_external`, `external_armed`, and the saved
  terminal modes.
- `src/terminal.c` and `src/terminal.h`: the `hyperlinks` flag.
- `src/pager_draw.c`: OSC 8 output, plus one help line. A status message
  now outranks the key hints on narrow rows, so the confirmation always
  shows.
- `Makefile`, `src/version.h` (0.44.0), and the version check in
  `tests/run.sh`.
- `tests/external_links.py`, the PTY test, with its fixture
  `tests/fixtures/external-links.html`. It uses stub browsers and covers:
  - confirm, then open;
  - cancel on another key;
  - refusals of `ftp:` and malformed URLs;
  - no installed browser, and a browser that cannot start;
  - the console hand-off, checking a real tty in cooked mode and that the
    pager comes back;
  - OSC 8 present on xterm and absent on `TERM=linux` and in `--pipe`.
- `tests/pager_screen.py`: the screen model now skips OSC strings.

Evidence on 2026-10-23:
- `make command` built without warnings.
- The new test passes, and it fails against the installed 0.43.x.
- Every code test in `make test` passed.

`make test` still stops at `tools/check-repo.py`, which wants the docs below.

**TCK27 (docs, install and merge): open.** The shared `tam` clone stays on
`develop`; never switch branches there. First action: after claiming the
paths in step 1, bring the branch's files onto `develop` without switching
with `git fetch origin && git checkout origin/feature/external-links --
<TCK26 paths>`, then:

1. Claim these paths: the files above, plus `ROADMAP.md`,
   `docs/plans/development.md`, `docs/plans/README.md`,
   `docs/plans/external-links.md`, `CHECKLISTS.md`, `man/tam.1`,
   `AI_PROVENANCE.md` and a `docs/ai/2026-10-2x-tam-external-links.md`
   record.
2. `ROADMAP.md`:
   - change the "Current Command Version" header and the `develop` row to
     0.44.0 / 0.42.5;
   - add a delivered-history row, "**0.44.0 command** | External links open
     in the reader's browser".
   `tools/check-repo.py` prints the exact fixes.
3. `docs/plans/development.md`:
   - add a decision-record entry, "Command 0.44.0 (minor): ...", and
     update the scope line;
   - amend "Offline identity and the `ask` boundary": tam never fetches;
     following an external link hands the URL to the user's own browser
     after a confirming second press.
4. `docs/plans/README.md`: add this plan to the index, and set its status
   when done.
5. `man/tam.1`:
   - under PAGER KEYBOARD CONTROLS, add "web links: press Enter twice";
   - under ENVIRONMENT, add `BROWSER` (graphical session, colon list, `%s`)
     and `TAM_TEXT_BROWSER` (console).
6. `CHECKLISTS.md`: set "Next free" to TCK28.
7. Provenance: `AI_PROVENANCE.md` row and `docs/ai/` record (public, no
   session IDs), then the private ledger row in `improvements-fred-tamlinux`
   per the `ai-provenance` skill.
8. Run `make test` (everything must pass), then
   `make install PREFIX=$HOME/.local/dev` and `type -a tam`.
9. Commit with TCK27 in the subject and push `develop`. Then delete
   `feature/external-links`, locally and on origin.

## Acceptance and limits

- **Desktop (foot under Hyprland):** in `tam show`, pressing Enter twice on
  a github.com link opens `$BROWSER` (`tam-launch-browser`).
- **foot URL mode:** Ctrl+Shift+O lists the link.
- **Console (Ctrl+Alt+F3):** the same link opens in w3m, and `q` returns to
  tam on the same page.
- **Piped output** (`tam show <page> --pipe`) is unchanged.
- **Not in scope:** opening `mailto:` or other schemes, copying URLs with
  OSC 52, and per-scheme handlers.
