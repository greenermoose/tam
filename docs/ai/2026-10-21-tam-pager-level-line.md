# Pager level line (command 0.39.0)

- **Date:** 2026-10-21
- **Time:** 22:30 to 22:51 -04:00
- **Stage:** Develop; command 0.39.0 replaces the level bar and the "Written
  for" banner with a right-aligned level line above the page heading
- **CLI Tool:** Claude Code 2.1.307
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** Fred Horch wrote the layout note (the line's form, its
  position, alignment and examples) and decided the level bar's and related
  links' placement. Claude proposed the relatives line's wording, dimming
  and narrow-width wrapping, and wrote the code, tests and documentation.
- **Checked against:** `src/depth.c`, `src/render.c`, `src/pager.c`,
  `src/tam.c`, `tests/run.sh`, `tests/pager_screen.py`, `man/tam.1`,
  `infocmp -1 xterm-256color`, and pages in `knowledge/`

## Guiding prompts

> Read my pager layout for tam idea in the [private ideas repository] then
> implement in the tam repo.

> I don't like separating the related page links from the level bar. Let's
> make that a third line, above the page heading. So it would go
>
> Breadcrumb (aligned left)
>     Level bar (aligned right)
>     Related page links (optional, aligned right)
> Page Heading
>
> Does this seem like a good UI/UX design choice?
>
> Push back if you think there is a better way. I do not like the current
> page layout. The level stuff and the related pages are not done well. We
> need to improve that. Either my idea or something else that is better than
> what we have now.

> Build it with refinements 1 to 3. I'm not sure should about the clickable
> way back. What is your recommendation on that.

## Key decisions

- The developer's note gives the form
  `<Level> text [on <Level> path] [(no <Level> text)]`, right-aligned below
  the breadcrumb and above the heading, with the path linked. "All" names a
  page showing several levels.
- The developer chose to replace the level bar rather than keep it under the
  heading, and to put the related pages on a third right-aligned line rather
  than drop them or separate them from the level line.
- Refinements the developer accepted: the relatives line reads
  `<Level> text: A | B | C`; both lines are dimmed but keep link
  highlighting; a line too long for the terminal wraps left-aligned.
- Found while checking real pages: titles contain commas, so links are
  separated by bars, and the level path page is left out of the relatives
  because the line above already links it.
- Claude recommended against a clickable level chooser to replace the bar's
  links (keys `0`-`3` and the help overlay cover switching); none was built.
- The fixture `shell/pipelines.html` gains one `?level=beginner` link, so the
  test that a level-naming link sets the lens still has a link to follow.

## Verification

- `make test-command`: all command tests pass, including the PTY level-lens
  tests and the refreshed `pedagogy` render golden (`UPDATE=1
  tests/render.sh build/tam`, diff read).
- `make test`: both suites pass.
- PTY renders at 100 and 44 columns with the tests' screen model: right-aligned
  at 100 columns, left-aligned wrap at 44, and `dim` (`ESC [2m`) emitted
  off the attention line.
