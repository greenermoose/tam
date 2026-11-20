# Terminal input section and glossary entries (corpus 0.83.0)

- **Date:** 2026-11-13
- **Time:** 21:17 to 21:35 -05:00
- **Stage:** Develop — corpus 0.83.0
- **CLI Tool:** opencode `v2.2.5` (authoring), Antigravity CLI `1.5.8` (handoff, verification, provenance, and commit)
- **Model:** Big Pickle (`big-pickle`) and Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** `1bce26c`, `a58049f`, and the commit that adds this record
- **Authorship:** Fred Horch requested a comprehensive, three-level (`novice`, `learner`, `expert`) section on how key presses translate through hardware, Linux kernel virtual consoles, `foot`, and the terminal line discipline into bytes, escape sequences, and signals for programs like `tam-edit`. `opencode` (`big-pickle`) authored the `ecosystem/terminal-input/` hub and eight pages, the four glossary entries, and the `ROADMAP.md` / `knowledge/version` updates before running out of tokens. Antigravity CLI (`gemini-4-hydrogen-medium`) took over the `tam-work` claim, verified the corpus checks, unit tests, repo check, rendering, public boundary, and `libtam` key-decoding source, committed the section and updated indexes, and wrote this provenance record.
- **Checked against:** `libtam/src/tam_key.c`, `knowledge/ecosystem/terminal-input/*.html`, `knowledge/learn/glossary/*.html`, `ROADMAP.md`, `knowledge/version`, `make corpus-and-test`, `make check-repo`

## Guiding prompts

> I want to set ergonomic and mnemonic keybindings for tam-edit. When started investigating Ctrl + H for help, I ran into some issues about how that is actually 0x08 in some contexts. Write up a section for the tam knowledge corpus that explains how CTRL + A, Esc, and all other key presses translate to ASCII and other codes in all terminal contexts.
>
> I'm especially interested to know how things work in a standard Linux kernel console virtual terminal, such as you get pressing Ctrl + Alt + F2. And I need to know how key presses working and are seen by programs running in foot terminals.
>
> This should be a very comprehensive and technical set of pages. In each page, include novice, learner, and expert text so the reader can choose the lens. Every page should introduce the page topic at a novice level, assuming zero prior knowledge. The expert level text sections should contain very detailed explanations and examples. The learner text should have quizzes and other interactive features that help you digest and master the material.
>
> Ask if you have questions. When I'm done reading these pages, I should have a clear understanding of what happens when I press a key on my keyboard and exactly how signals reach programs like tam-edit when they run in different types of terminals.

> Opencode has run out of tokens. Take over its work claims and finish up for it.

## Key decisions

None beyond the prompts.

## Verification

- `make corpus-and-test`: 627 pages parsed and rendered, component coverage check passed, 85 unit tests passed, repository consistency check passed.
- `make check-repo`: passed.
- `python3 tools/report-coverage.py --warnings`: no warnings on any `terminal-input` or new glossary page.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/terminal-input/choosing-keys`: rendered cleanly with Novice, Learner, and Expert sections.
