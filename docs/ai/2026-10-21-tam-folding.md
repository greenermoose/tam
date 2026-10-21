# TCK22 folding and self-quizzes (command 0.38.0 / corpus 0.37.1)

- **Date:** 2026-10-21
- **Time:** 06:30 to 07:28 -04:00
- **Stage:** Develop; implementation, fixtures, corpus corrections and documentation
- **CLI Tool:** Codex CLI `0.163.0` (live-checked)
- **Model:** `gpt-6.1-sol`
- **Transcript:** Retained privately by the author.
- **Commits:** `5a7af2e` (implementation) and the commit refreshing the
  corpus indexes and this record.
- **Authorship:** Fred requested TCK22; Codex implemented and verified the changes.
- **Checked against:** `docs/plans/learning-levels.md`, `docs/corpus-pedagogy.md`,
  `src/depth.c`, `src/render.c`, `src/pager.c`, `src/tam.c`, `Makefile`,
  `tools/build-index.py`, `tools/lint-corpus.py`, the affected corpus pages
  and fixture output.

## Guiding prompts

> Please work on TCK22—folding and self-quizzes from the tam repo.

> Install the new tam command and corpus.

## Key decisions

- Number folds before filtering, then transform only the remaining HTML. This
  prevents a lens change from moving an open answer to a different question
  and keeps filtered answers out of pipe output. Fold state stays in the
  pager and reaches the loader through a private query; addresses and history
  contain only the public address. Leaving the page clears the open set.
- Retain standard-input HTML in the loader context so its folds can also
  toggle. Width changes reflow the rendered document with fold state intact.
- Use generated level labels on section headings, without another gutter.
  Remove prefixes from the 23 affected pages and label their existing
  privileged examples; this adds no pilot pages or new exercises.
- Preserve the existing lint rules and add structural checks for quizzes,
  manual level prefixes and command risk labels. Extend index validation to
  smaller tagged blocks and the approved shared-body layered pattern, while
  retaining the one-section-per-level requirements for laddered pages.

## Verification

- `TAM_TEST_SKIP_OLLAMA=1 timeout 600 make test`: passed; command/pager,
  render goldens, lint, index, developer-loop, installation-guard, drift and
  corpus suites. Live Ollama checks were explicitly skipped.
- `timeout 120 make corpus-and-test`: passed after the final risk-label
  correction; all 70 pages render and corpus checks pass.
- Focused fixture checks: Enter toggle, `r`, `*`, quiz counts, address privacy,
  Back/Forward reset, lens-stable IDs, every supported block-level tag and
  standard-input folds. Seven renderer goldens pass; the intended existing
  golden changes are the generated section labels.
- Additional pipe check: nested folds/sections, comments before summaries,
  quoted `>` attributes and canonical numbering with an authored private
  attribute all pass. `git diff --check` and repository drift checks pass.

## Developer-requested development installation

Installed the clean `57e4c5f` command and corpus with `make install` using
`PREFIX=~/.local/dev` and the existing development `libtam` 0.3.0. The command
resolves first on the terminal PATH. With `TAM_KNOWLEDGE_DIR` unset, the
installed command reports command 0.38.0 and corpus 0.37.1, both at `57e4c5f`.
The installed executable and complete corpus match the build and checkout;
installed guide rendering and a pipe-mode quiz check pass.

This is the explicitly requested development override. Retire it with
`make uninstall PREFIX=~/.local/dev` immediately after a recorded package
deployment delivers the pinned version. No package acceptance or stage
promotion is implied.
