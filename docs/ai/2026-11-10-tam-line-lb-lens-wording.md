# Line Lb lens wording (command 0.51.1)

- **Date:** 2026-11-10
- **Time:** 09:58 to 10:15 -05:00
- **Stage:** Develop; command 0.51.1 changes line Lb to name the lens digit
  first: `0 Showing all text`, `1 Showing novice text`, `2 Showing learner
  text`, `3 Showing expert text`.
- **CLI Tool:** opencode v2.2.5
- **Model:** Big Pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `40bbf35` and the commit that rewords the corpus-style level
  line and adds this update
- **Authorship:** Fred Horch requested replacing the `Showing X: lens text`
  wording on line Lb with `X Showing lens text`. OpenCode rewrote the level
  line in `src/depth.c`, updated the tests, the golden renderer snapshot,
  the version files and this record.
- **Checked against:** `src/depth.c`, `src/depth.h`, `src/render.h`,
  `src/version.h`, `tests/run.sh`, `tests/render/pedagogy.txt`, `ROADMAP.md`,
  `docs/plans/development.md`, `docs/ai/2026-11-06-tam-hidden-level-notices.md`.

## Guiding prompts

> For the tam pager, change how line Lb displays reader lens. Instead of
>
> Showing X: lens text
>
> have it be
>
> X Showing lens text
>
> e.g.
>
> 0 Showing all text
> 1 Showing novice text
> 2 Showing learner text
> 3 Showing expert text

> Are all tests updated to correspond to the new format for showing reader lens?

> Reword to match the new display.

## Key decisions

The lens digit and the level name are two separate `lvl-N` spans, so both
carry the level color (previously only the combined `N: name` token was
colored); the words `Showing` and `text` remain dimmed and the line keeps
its right-aligned level-line style. The hiding notices below the heading and
the footer lens row are unchanged. On Fred's follow-up word, the visible-level
line in `docs/corpus-style.md` (locked by D225/D226) now quotes the Lb
display instead of the old `1: novice` tokens.

## Verification

- `make command`: clean build, no warnings.
- `make test-command`: all integration, upgrade, renderer and repository
  checks pass.
- `make test`: full suite passes (corpus linter, coverage, guide).
- Piped `tam show --level 2 welcome` and the PTY color assertions confirm
  the new wording and the ANSI colors for all four levels.
- The corpus-style reword is docs-only: `make check-repo` passes and no
  corpus or command version changes.
