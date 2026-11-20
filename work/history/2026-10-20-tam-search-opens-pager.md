# Search results open in the pager (command 0.36.0)

- **Date:** 2026-10-20
- **Time:** 22:05 to 22:30 -04:00
- **Stage:** Develop; command 0.35.0 to 0.36.0
- **CLI Tool:** Claude Code 2.1.305
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** Fred asked for `tam search` to leave the reader in the pager;
  Claude Code wrote the `cmd_search` change, the pty tests, and the help, man,
  README, roadmap and decision-record updates.
- **Checked against:** `src/tam.c` (`cmd_search`, `cmd_explain`,
  `view_document`), `src/discovery.c` (`tam_guide_search_document`),
  `tests/run.sh`, `build/tam search --help`, `man -l man/tam.1`

## Guiding prompts

> Let's change how tam search works. Right now it just leaves you in the
> terminal. That isn't helpful because you can't click to follow any of the
> hyperlinks you've found. Make it so you end up in the pager and can start
> using tam and reading about the knowledge you've found.

## Key decisions

- Reuse the existing Guide Search page (`tam:search?query=...`, the same page
  the pager's `\` prompt opens) rather than a new layout, so the command and
  the in-pager search show identical results and history/address behave alike.
- Follow `tam explain`: interactive only when stdout is a terminal and a
  terminal is readable; `--pipe` (previously a no-op) forces the old text
  listing, and `--json` is unchanged.
- A miss keeps the terminal suggestions and exit status 1 instead of opening an
  empty results page, matching `tam explain` misses and keeping the fuzzy
  "Did you mean" hint.

## Verification

- `make test`: both suites pass, including new pty cases: `tam search pipe`
  opens the results page with address `tam:search?query=pipe`, `]` and Enter
  follow the result to its guide page, and a miss exits 1 with the banner.
