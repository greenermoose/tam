# <Topic> (<command or corpus version, if one changed>)

<!--
Copy to docs/ai/<YYYY-MM-DD>-tam-<slug>.md and fill in. Size the record to
the work: a few lines for a note or a small fix, about twenty to thirty for
a page or a feature. Do not restate the diff; a reader can open the commits.
Never include conversation or session IDs, transcript or store paths, home
paths, or private repository names. Delete this comment.
-->

- **Date:** YYYY-MM-DD
- **Time:** <start> to <end>, local time with UTC offset, e.g. 09:12 to 10:40 -04:00
- **Stage:** Develop | Test | Run | Release, and what changed
- **CLI Tool:** <tool> <version from the running binary, e.g. `agy --version`>
- **Model:** <model name> (`<model id>`)
- **Transcript:** Retained privately by the author.
- **Commits:** `<sha>`, ... and the commit that adds this record
- **Authorship:** who asked for what, and what the AI tool wrote
- **Checked against:** the sources you opened to confirm facts, e.g.
  `src/install.c`, `tam --help`, `tamlinux` repository
  `desktop/session/hypr/tamlinux/bindings/tiling.lua`

## Guiding prompts

> <each of the developer's prompts that directed the work, verbatim>

## Key decisions

<Only what the diff cannot show: why this approach, what was rejected, and
any ruling the developer made. Write "None beyond the prompts." if there
were none.>

## Verification

<The commands you ran and their results, e.g. `make test`: both suites
pass.>
