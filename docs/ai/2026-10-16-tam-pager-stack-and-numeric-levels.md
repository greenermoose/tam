# Pager navigation state stack, numeric level synonyms, and renderer punctuation spacing (command 0.26.0)

- **Date:** 2026-10-16
- **Time:** 13:16 to 13:45 -04:00
- **Stage:** Develop — `tam` command 0.26.0 (`src/pager.c`, `src/depth.c`, `src/depth.h`, `src/tam.c`, `src/render.c`, `src/version.h`, `completions/bash/tam`, `man/tam.1`, `tests/render/`, `tests/run.sh`)
- **CLI Tool:** Antigravity CLI 1.4.0 (`agy 1.4.0`)
- **Model:** Gemini 4 Argon (`gemini-4-argon`)
- **Transcript:** Retained privately by the author.
- **Commits:** Recorded in git history on `develop`.
- **Authorship:** Fred directed the implementation of the command-lane improvements (pager navigation state stack and numeric/`all` depth level synonyms) and the renderer punctuation spacing fix; Antigravity CLI wrote the C changes, test cases, and documentation updates.
- **Checked against:** `src/pager.c`, `src/depth.c`, `src/depth.h`, `src/tam.c`, `src/render.c`, `completions/bash/tam`, `man/tam.1`, `tests/run.sh`, `tests/render/blocks.html`, `knowledge/improve/toolchains.html`.

## Guiding prompts

> Start implementing the tam command-lane improvements right now.

> Check your inbox for Claude's renderer-bug message and fix it.

## Key decisions

- **Restoring viewport and cursor line while clearing active link selection on back/forward:** `nav_state_t` records `{ href, top, cursor_line }` before following any page or intra-page `#anchor` link. When `go_back()` or `go_forward()` restores a stack entry, `top` and `cursor_line` are restored (clamped to current terminal height and document length) while `selected` is reset to `-1`. Keeping `selected = -1` on history arrival preserves the Right Arrow contract (`Right Arrow` advances along the `forward` stack when no link is selected, and follows a link when one is explicitly selected), while `Tab` and `Enter` remain anchored to the restored `cursor_line`.
- **Whitespace-aware punctuation attachment in `src/render.c`:** `layout_flow()` and `cell_wrap()` now track `had_space` across spans and tokens so a token starting with `,.;:!?%)]}` only attaches without a leading space (`tight = 1`) when no whitespace separated it from the previous token in the HTML source (`<code>x</code>.` renders as `x.`, whereas `<code>git add .</code>` keeps its space as `git add .`). `peek_following_punct()` accounts for the separating space column (`sp + dwidth(...)`) so space-preceded punctuation tokens neither overflow the line width nor wrap alone to the start of the next line.

## Verification

- `make test` (with `PREFIX` and `LIBTAM_PREFIX` set to the local development prefix): `tests/run.sh`, `tests/render.sh` (6 fixtures), `tools/lint-corpus.py`, `tests/test_lint_corpus.py` (29 tests), and `tests/guide.sh` all pass.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe improve/toolchains` renders `Never use git add -A, git add . or git commit -a`.
- Installed development build via `make install`.
