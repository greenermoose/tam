# Warmer welcome page (corpus 0.32.1)

- **Date:** 2026-10-18
- **Time:** 16:17 to 16:22 -04:00
- **Stage:** Develop; rewrote `knowledge/welcome.html` and bumped the corpus to 0.32.1
- **CLI Tool:** Antigravity CLI 1.4.2
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `86b9da5` and the commit that adds this record
- **Authorship:** Fred asked for a warmer, friendlier welcome page and supplied
  the project description. The AI tool wrote the page text, bumped the version,
  regenerated the index and wrote this record.
- **Checked against:** `tamlinux` repository `README.md` (description, name
  meanings, efficiency rationale, current base and planned Sway move, no
  installable image yet) and `desktop/session/hypr/tamlinux/bindings/applications.lua`,
  `utilities.lua`; `src/tam.c` (`welcome`, `hello`, `hi`); `src/pager.c`
  (`Tab`, `Enter`, Left Arrow, backtick, `/`, `?`, `q`); `tam --help`; earlier
  versions of the page; welcome screens of Linux Mint, KDE Plasma,
  elementary OS and the Omarchy manual for tone.

## Guiding prompts

> Please review and rewrite the tam welcome page to be warmer, friendlier, and better suited as the first thing a new user reads about Tamlinux. This
>   should be corpus-only edit. Bump the corpus version number. Ask if you have questions. Take a look at prior versions of the welcome page and look at
>   other welcome pages for other Linux distributions and graphical shell workstation environments for ideas. We want to strike the right balance between
>   being terse and verbose, professional and goofy. It should be fun and interesting to read about Tamlinux.
>
>   When describing Tamlinux use something along the lines of the following (doesn't have to be verbatim, but try to stray too far from this):
>
>   Tamlinux is a workstation environment for Linux developed by Fred Horch so he can use the computers he owns to do both personal and professional work
>   for many years. The project aims to deliver a well-appointed, ergonomic, and super-efficient system that runs well on laptop and desktop computers up
>   to twenty years old.
>
> Don't do anything besides rewrite one page in the corpus. I don't want you messing with the tam command. Ask if you have questions.

## Key decisions

Fred chose the patch bump (0.32.1) and approved committing the regenerated
index, the `ROADMAP.md` version line and this record with the page. Page
metadata is unchanged. The page restores the tamarack story and reading
paths from earlier versions. It also says plainly that no installable image
exists yet.

## Verification

`tools/lint-corpus.py`: clean. `tests/test_lint_corpus.py` and
`tests/test_build_index.py`: OK. `tests/guide.sh build/tam`: passed. Read the
page with `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe welcome`.
`make test-corpus` stopped while building the command (`src/discovery.c`: `tam.h`
not found). This is in the command lane and was not touched here, so its
corpus steps were run one by one.
