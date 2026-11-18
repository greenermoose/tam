# Retire the ~/.local/dev prefix (command 0.61.2, corpus 0.96.1)

- **Date:** 2026-11-17
- **Time:** 20:06 to 20:30 -05:00
- **Stage:** Develop. Development prefixes and the text that names them move from `~/.local/dev` to `~/.local`.
- **CLI Tool:** opencode 2.2.5
- **Model:** Big Pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `bb31587`, `d408b98`, and the commit that adds this record.
- **Authorship:** The developer asked for the documentation test fix and then for the retired prefix to be replaced everywhere, stating where development and released software now live. The AI tool found the remaining references, changed the source, tests, manual, corpus pages and current plans, and wrote this record.
- **Checked against:** `bin/tam-upgrade`, `tests/test_upgrade.py`, `tests/test_dev.py`, `tests/test_documentation.py`, `man/tam.1`, `README`, the six corpus pages that named the prefix, `docs/plans/development.md`, `docs/plans/upgrade.md`, `docs/plans/dev-loop.md`, `tools/dev.py`, and `tam --version`.

## Guiding prompts

> fix tests/test_documentation.py

> Fix ~/.local/dev everywhere. We are now using ~/.local/bin and ~/.local/lib for development work. We install to /usr/bin and /usr/lib for each new version of Tamlinux.

## Key decisions

- Development builds use `~/.local`: executables in `~/.local/bin`, libraries in
  `~/.local/lib`, headers in `~/.local/include`. Every released Tamlinux
  version installs to `/usr/bin` and `/usr/lib`; the corpus pages now say so.
- `bin/tam-upgrade`'s `safe_root()` builds and switches source generations under
  `~/.local/opt/tam`, so `tam upgrade --apply` and `tam switch` share the same
  development prefix as `make install PREFIX=$HOME/.local`. Receipts stay in
  `~/.local/state/tam-upgrade`.
- A plain `~/.local/bin/tam` from `make install` is an unowned endpoint for the
  switcher; remove it with `make uninstall PREFIX=$HOME/.local` before the first
  source switch. The upgrade tests cover that refusal.
- Dated records stay as written: `docs/ai/`, `docs/handoffs/`, delivered-history
  rows, completed or superseded plans, and the decision entries that describe
  what happened at the time still name `~/.local/dev`. Only current behaviour,
  instructions and examples changed.

## Verification

`make command` builds clean and reports `tam command 0.61.2`.
`make test` passes the command suite, upgrade tests, renderer goldens, corpus
checks, component coverage, corpus tooling tests and `check-repo`. The upgrade
tests now exercise the `.local` prefix, including the refusal of a pre-existing
unowned `~/.local/bin/tam` and of a symlinked `~/.local`.
