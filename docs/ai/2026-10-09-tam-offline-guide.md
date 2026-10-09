# tam offline guide (milestone 2, 0.1.0)

- **Date:** 2026-10-09
- **Stage:** Develop — offline guide implemented.
- **Command version:** `0.2.0` — `0.1.0` for the new `browse` capability, then
  `0.2.0` for added pager navigation keys (`Esc` to quit, left/right arrows to
  go back/follow), per Fred's rule that adding a feature bumps the minor number.
- **CLI Tool:** opencode 1.18.35
- **Model:** `big-pickle`
- **Transcript:** Retained privately by the author.
- **Commits:** Not yet committed; the work continues in the shared checkout at
  Fred's request.
- **Authorship:** Fred set the goal and approved the design decisions below;
  opencode wrote the renderer, pager, command wiring, corpus pages, tests and
  documentation. No delegation.

## Guiding direction

Fred asked to develop the next slice of the `tam` command: the offline guide,
building on the milestone-1 foundation. The design was settled with him before
implementation, and the direction (paraphrased) was:

> Make `tam browse` open the bundled offline guide. It must work before the
> graphical desktop and on minimal systems, so render the guide natively
> rather than driving an external browser; Lynx may stay optional but must
> never be a dependency. Derive the plain text at runtime from the same HTML,
> and keep committed text as golden fixtures. File the plan first, then
> implement.

## Decisions and scope

- **Native renderer.** `tam` parses a bounded HTML subset in `src/render.c`
  and lays it out itself, so browsing needs no network and no external viewer
  and works on minimal / musl / legacy systems. `tam_exec` could not drive an
  interactive browser anyway (captured output plus a mandatory timeout).
- **One source of truth.** Semantic HTML is hand-authored under
  `tam/knowledge/`; text is derived at runtime. Committed snapshots under
  `tests/snapshots/` are golden fixtures with a regeneration path, not a
  second authored corpus.
- **Two front ends, one parse tree.** `src/pager.c` gives an interactive
  viewer (alternate screen, raw `termios`, scrolling, link selection and
  follow, history back, quit); non-TTY output and `--pipe` print text wrapped
  to a fixed 80 columns so snapshots are deterministic.
- **Corpus lookup.** Resolution order is `$TAM_KNOWLEDGE_DIR` →
  `$XDG_DATA_HOME/tamlinux/knowledge` →
  `$XDG_DATA_DIRS/tamlinux/knowledge` → the compiled install prefix (also
  overridable at runtime via `$TAM_DATADIR`). Relative `href`s resolve against
  the corpus root; `..` escapes, absolute paths and network URLs are refused.
  External links are labelled and never fetched.
- **Pages:** `index`, `welcome`, `install/index`, `shell/pipelines`,
  `man/index`.

## Verification

- `make` builds `src/tam.c src/render.c src/pager.c` against the installed
  `libtam` with `-Wall -Wextra -Wpedantic -std=c11`, no warnings.
- `make test` runs `tests/run.sh` (milestone-1 behavior, unchanged) and the new
  `tests/guide.sh`, which checks: every page renders non-empty; the index has a
  `Links:` appendix; piped output carries no escape sequences; extensionless
  page names resolve; every internal link points at a file; golden snapshots
  match; missing pages and a missing corpus fail with clear diagnostics; and a
  staged install relocated to a temporary prefix finds its corpus via
  `XDG_DATA_DIRS`. All checks passed.
- The renderer and pager were rebuilt and re-tested with
  `-fsanitize=address,undefined` with no findings, including a scripted pager
  session (follow a link, go back, quit).
- The interactive pager was exercised under a pseudo-terminal (foot-compatible
  SGR styling, alternate screen, status row).
