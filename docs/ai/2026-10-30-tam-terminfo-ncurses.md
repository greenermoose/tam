# Terminfo / termcap and ncurses corpus pages (corpus 0.50.0)

- **Date:** 2026-10-30
- **Time:** 06:44 to 06:55 -04:00
- **Stage:** Develop, corpus 0.50.0
- **CLI Tool:** Cursor `3.27.22` (from `cursor --version`)
- **Model:** Composer (`composer`). This session identified itself as Auto, powered by Composer; no finer model id was logged by the CLI.
- **Transcript:** Retained privately by the author.
- **Commits:** (pending Fred's `make dev`)
- **Authorship:** Fred directed two ecosystem tutorials (terminfo/termcap display stack, plus a separate ncurses page with program examples and how `tam` uses ncurses). Cursor authored the pages, hub cross-links, corpus bump, and this record. Indexes left for Fred's `make dev` (no `make` in this session).
- **Checked against:** `docs/corpus-authoring.md`, `docs/templates/ladder.html`, `ecosystem/ttys-and-virtual-consoles.html`, `improve/terminal.html`, `tam.html`, `src/terminal.c`, Makefile `ncursesw` flags; `ldd` on `/usr/bin/clear`, `/usr/bin/tput`, `nano`, `build/tam`; `pacman -Q ncurses`; `tput`/`infocmp` with `TERM=xterm-256color` on 2026-10-30

## Guiding prompts

> Create a topic in the knowledge corpus for terminfo and termcap and how displaying text in terminals works. Do not commit your changes because I will run make dev after this set of changes is made.

> Include an explanation of ncurses in this set of updates to the corpus.

> Edit the plan to include a separate ncurses-only page.

> Give examples of programs that use ncurses.

> Explain whether tam uses ncurses if so or if not, explain why.

> Implement the plan as specified, it is attached for your reference. Do NOT edit the plan file itself.

> Don't run make. Release the bio claim. I'm going to run make dev myself. Leave the repo with no work claims for cursor and ready for me to make dev.

## Key decisions

- Two learn-track ecosystem tutorials: `terminfo-and-termcap.html` owns the display/capability stack; `ncurses.html` owns the library, verified example programs, and the `tam` terminfo-helper vs high-level API distinction.
- Named only programs verified with `ldd` on the writing host (`clear`, `tput`, `nano`, `tam`); noted `nmtui` uses newt/slang, not ncurses.
- Did not run `make` / `make corpus`; Fred regenerates indexes via `make dev`.
- Released abandoned bio claim `cursor-fa5546755063` on Fred's instruction so paths were free; finished this session's claim with no cursor claims left.

## Verification

- Page meta and beginner/intermediate/expert sections present on both new HTML files.
- `ldd` confirmed `libncursesw` for `clear`, `tput`, `nano`, and `build/tam`.
- No `make corpus` or `make corpus-and-test` in this session (Fred's instruction).
