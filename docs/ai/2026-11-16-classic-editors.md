# Classic text editors: ex, vi, GNU Emacs, and the Emacs versus vi debate (corpus 0.96.0)

- **Date:** 2026-11-16
- **Time:** 16:30 to 20:15 -05:00
- **Stage:** Develop — add `knowledge/terminal/editors/` (five pages), glossary terms, bibliography entries, and context links (`corpus 0.96.0`)
- **CLI Tool:** OpenCode 2.2.5
- **Model:** Big Pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `66a2a6c` and follow-up index/record commit
- **Authorship:** Fred requested `tam` corpus pages for vi, ex, and GNU Emacs plus a standalone explanation of the Emacs versus vi debate, with glossary and bibliography support; OpenCode authored the five editor pages, two glossary pages, four bibliography pages, the bibliography index entries and People rows, the hub and context links, the version and roadmap bumps, and this record.
- **Checked against:** POSIX `ex` and `vi` specifications at pubs.opengroup.org; GNU Emacs 31.1 Info manual (`emacs.info`); Vim 9.2 documentation (`vi_diff.txt`, `uganda.txt`, `ex` mode behaviour); the Jargon File entries EMACS, vi, and holy wars; captured output from unpacked Arch packages for `vim` 9.2 and GNU Emacs 31.1; `pacman -Si` package sizes; `knowledge/terminal/configuration.html`, `knowledge/desktop/apps/editor.html`, `knowledge/bibliography/index.html`, `knowledge/version`, `ROADMAP.md`

## Guiding prompts

> make it so. coordinate with codex if you get blocked claiming work.

## Key decisions

- Placed the pages under `terminal/editors/` because the subject library already treats terminals and shells as the owning terminal subject; no editor pages existed there before.
- Made the four tool pages ladders (`kind="tutorial"`) and the debate a standalone ladder (`kind="explanation"`) so it can be found and read without opening both tool pages first.
- Used only captured output in Learner examples, taken from Arch packages unpacked in a scratch directory; nothing was installed or removed.
- Kept the debate page neutral. Tamlinux's baseline editor is `micro`; vi, Vim, Neovim, and Emacs are optional packages. The page compares and does not declare a winner.
- Defined Emacs Lisp inline rather than linking a glossary page that does not exist.
- Added `line-editor` and `modal-editing` as glossary entries because the tool pages rely on both terms at Novice level.

## Verification

- `make corpus`: built 1148 pages with indexes current.
- `make corpus-and-test`: corpus checks, component coverage, 92 corpus tooling unit tests, and `tools/check-repo.py` all passed.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe` on all eleven new or changed pages: each rendered its breadcrumb, title, summary, and level sections correctly.
- Confirmed Vim 9.2 and GNU Emacs 31.1 batch examples produce the shown output from freshly created scratch files.
