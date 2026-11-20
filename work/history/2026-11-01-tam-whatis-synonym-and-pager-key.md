# tam whatis synonym and in-pager w glossary lookup

- **Date:** 2026-11-01
- **Time:** 14:30 to 15:52 -05:00
- **Stage:** Develop; `tam whatis <term>` is a CLI synonym for `tam explain <term>`, and in the tam pager `w` opens a `whatis:` prompt for glossary lookup with closest-match resolution.
- **CLI Tool:** Antigravity CLI 1.4.12
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `c885aff`
- **Authorship:** Fred Horch requested `tam whatis <term>` as a synonym for `tam explain <term>` and an in-pager `w` key that prompts for a string and looks it up in the glossary to display the closest match. Antigravity implemented the command alias, pager prompt action, index glossary matching algorithm, test fixtures, integration tests, completion rules, and documentation.
- **Checked against:** `src/tam.c`, `src/index.h`, `src/index.c`, `src/pager_input.c`, `src/pager_search.c`, `src/pager_nav.c`, `man/tam.1`, `completions/bash/tam`, `tests/run.sh`

## Guiding prompts

> Is there a way to look up a term in tam? Something like whatis? I.e., is there a key press you can do on any page, type in a term and displays its definition from the glossary?

> Make tam whatis <term> a synonym for tam explain <term>. In the tam pager, when a user presses w they should get a whatis: prompt that allows them to type a string and that string looked up in the glossary to display the closest match.

## Key decisions

- `tam whatis` is handled as an alias for `tam explain` in `src/tam.c`, supporting all flags (`--pipe`, `--level`, etc.) and subcommands (`topics`).
- Pager key `'w'` invokes `tam:prompt:whatis`, presenting a `whatis: ` prompt.
- `tam_index_closest_glossary_match` searches only glossary entries (identified by `learn/glossary/` path or `glossary-` topic prefix, excluding the table of contents).
- Closest-match resolution evaluates candidates across exact matches, prefix matches, word/sub-word matches, substring matches, and fuzzy edit distance (Levenshtein) against term slugs, titles, and keywords.
- When a match is found, the pager navigates to that glossary page and adds the previous view to history, allowing immediate return via `u` or Left Arrow.
- On miss or cancellation, the pager stays on the current page with a clear diagnostic message in the status line (`no glossary match for '...'`).

## Verification

- `make command`: clean compilation with zero warnings or errors.
- `make test-command`: all integration tests in `tests/run.sh` pass, upgrade tests pass (15/15), and renderer golden checks pass.
- PTY testing confirms interactive `w` keypress, `whatis: ` prompt input, exact glossary page navigation, fuzzy matching for typos, miss diagnostic reporting, and escape cancellation.
