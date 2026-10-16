# HTML Entity Whitelist and Display-Width Layout Fix (tam 0.25.2)

- **Date:** 2026-10-16
- **Stage:** Develop — renderer corrections. Finding F5 of the 2026-10-16 corpus
  audit, and the display-width defect found while reviewing that fix.
- **CLI Tool:** opencode 1.19.0
- **Model:** `big-pickle`
- **Transcript:** Retained privately by the author.
- **Commit:** `4d7610a` (entity whitelist), and the commit carrying this record
  (display-width layout).
- **Authorship:** Fred asked in a direct session to close finding F5 of the corpus
  audit — entities outside the renderer whitelist printing literally in
  `tam show`. Asked how to structure the change, he chose two commits in the one
  session: the whitelist alone first, then the layout correction that the
  snapshot review exposed. opencode claimed the command lane paths and the
  shared documentation paths under `tam-work`, verified the defects against the
  live command, made both changes, regenerated the golden snapshots, reviewed
  every diff, and ran the full test suite. No `knowledge/` page changed.

## Decisions and scope

- **Entity whitelist (`src/render.c`):** Added `larr`, `rarr`, `times`, `le`,
  `ge` and `asymp` to the named-entity table, so the renderer decodes every
  named entity the corpus uses. Numeric `&#…;` references were already decoded.
  Verified live before the change: `tam show shell/pipelines.html --pipe`
  printed `stdout &rarr; stdin` literally. LaTeX math (finding F6) is left
  alone: it is corpus markup, not a renderer gap.
- **Display-width measurement (`dwidth`):** Text width was measured with
  `strlen`, so every multi-byte glyph counted as two or three columns and a
  full-width line stopped short of the margin. Width is now counted in display
  columns, applied to body wrapping, table column widths (`printed_w`), cell
  wrapping and the flow column of `cell_text_len`.
- **Flow quality:** a space is no longer injected after an opening bracket
  (`storage checks ( tam` → `storage checks (tam`); a token made only of
  sentence punctuation (`.`, `,`, `:`, `)`, …) is kept with the word before it
  rather than stranded alone on a line; and the break decision looks ahead to
  such a token so a line is not pushed past the margin to keep it attached.
- **Version:** command `0.25.2` in `src/version.h`, the assertion in
  `tests/run.sh`, the `ROADMAP.md` header and the decision record of
  `docs/plans/development.md`. Corpus untouched.
- **Known limits, left as they are:** pre-existing lines that exceed the width
  (long URLs and code paths inside command examples) are not new and not
  touched; a token that begins with punctuation but contains letters (for
  example a file-extension cluster) still joins the preceding word, which
  matches the previous behaviour. The pager (`src/pager.c`) still counts bytes
  rather than display columns when it clips a line to the terminal width and
  when it pads the highlighted line, so a row whose right edge holds multi-byte
  glyphs can lose its last few columns on screen; that predates this change and
  is reported for a separate fix.

## Verification

- `make command PREFIX=$HOME/.local/dev` — clean, no warnings.
- `UPDATE=1 tests/guide.sh build/tam`, then a scripted review of all snapshots:
  33 changed; lines longer than 80 display columns 20 before and 20 after, with
  no newly over-width line; no line consisting only of punctuation; and a
  whitespace-insensitive comparison of every page confirmed no page gained or
  lost text (one guide re-flows its table cells with the new column widths).
- `make test PREFIX=$HOME/.local/dev` — both suites pass, including the
  golden snapshot comparison.
- Spot-checked `tam show` output for the pages named in the audit and for the
  two bracket cases above.
