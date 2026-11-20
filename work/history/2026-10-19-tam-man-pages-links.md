# Manual links resolve after installing man-pages (corpus 0.33.4)

- **Date:** 2026-10-19
- **Time:** 23:12 to 23:25 -04:00
- **Stage:** Develop; two corpus pages, roadmap header
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer asked for the package and the link fixes.
  Claude Code installed `man-pages` 6.19-1 from Arch `core` with the
  developer's approval, checked every `man:` link and wrote the edits.
- **Checked against:** `man -w` for each of the 29 `man:` links in
  `knowledge/`, the `.TH` line and PROLOG of `sh.1p` from `man-pages` 6.19-1,
  `tam man 'sh(1p)'` and `tam show --pipe man/index` from the checkout build,
  `make corpus-and-test`

## Guiding prompts

> For the tam knowledge corpus, install man-pages and fix the remaining links.

## Key decisions

- With `man-pages` installed, the section 2 links (`dup2`, `pipe`,
  `sigaction`, `stat`, `umask`, `wait`) resolve unchanged.
- `sh(1)` and `umask(1)` resolved only because man-db falls back from
  section 1 to `1p`; no section 1 page exists for either. They now link
  `sh(1p)` and `umask(1p)` directly rather than `bash(1)`, which is one long
  page with no anchor for `umask`.
- The man pages guide gains one sentence on what the `p` suffix means, quoting
  the pages' own "POSIX Programmer's Manual" label. A correction, so corpus
  takes the patch bump.

## Verification

All 29 links now resolve to a page in their exact section. `make
corpus-and-test` passed; the staged install check is skipped when make is
unavailable inside the test.
