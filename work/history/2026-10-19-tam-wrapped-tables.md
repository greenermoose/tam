# Wrapped tables and keyword corrections (command 0.31.2, corpus 0.32.4)

- **Date:** 2026-10-19
- **Time:** 17:25 to 17:45, -04:00
- **Stage:** Develop — renderer table layout, topic list headings, corpus keywords
- **CLI Tool:** Claude Code 2.1.303
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit adding this record and the commit regenerating the indexes
- **Authorship:** Fred reported the stacked topics list and directed the fix,
  versions, keyword pruning and headings; Claude Code wrote the renderer change,
  heading text, keyword edits, tests and documentation.
- **Checked against:** `src/render.c`, `src/tam.c`, `src/index.c`,
  `tests/run.sh`, `tests/render.sh`, every corpus page with a table rendered
  with `tam show --pipe` before and after, and every page's keywords compared
  with the text of all corpus pages.

## Guiding prompts

> This list of topics in tam is not formatted very well. Please make this into a table rather than repeating the words Topics Keyword, Pages, and Summary / Coverage. I think a table with a heading would be a better format. Look for other places in the corpus where this happens and fix it. Report if there are any problems. Bump the corpus version to 0.33 and install the dev version so I can play with it. Ask if you have questions.

> Bump the command version to 0.31.2, as a fix, and leave the corpus version as 0.32.3. Fix the ROADMAP and development.md files. Do you think piped output should ever be wrapped? Check the weird entries in the topic keywords list, prune any that are wrong, and make the headings more clever. Ask if you have questions.

## Key decisions

The topics page was already an HTML table; the renderer stacked any table
wider than the terminal. Fixing the renderer repaired the topics and keyword
pages and 16 of the 21 corpus pages with tables at 80 columns, so no page
layout changed. Columns narrow widest-first, never below their longest word
or 12 cells; stacking remains for tables that still cannot fit.

A keyword was pruned only if it misdirects (its page never mentions it while
another page covers it) or means nothing as a lookup: `2006`, `v11`, `zram`
(sustainability), `energy` (philosophy), `distro` (terminology), `tam`
(scripting), `top` and `htop` (jobs), `hyprland` (desktop), `hyprctl eval`
(Lua), `nautilus` (desktop applications). `top` moved to the resources page,
which covers it. Shell-operator keywords such as `2>&1` stay: they are lookups.

Because the keyword edits change corpus content, Fred chose a corpus patch
bump to 0.32.4 rather than keeping 0.32.3.

## Verification

`make test`: both suites pass; the live `tam ask` checks skip without Ollama.
The table render golden covers a wrapped table and a stacked fallback.
