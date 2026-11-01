# Themes and fonts guide pages (corpus 0.54.0)

- **Date:** 2026-11-01
- **Time:** 05:50 to 06:10 -05:00
- **Stage:** Develop, corpus 0.54.0
- **CLI Tool:** Antigravity CLI 1.4.12 (from `agy --version`)
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `02c0b0a` adds the fonts and themes pages, ecosystem hub links, corpus version bump and roadmap update; this commit adds the record, provenance rows and refreshed indexes.
- **Authorship:** Fred asked for an in-depth guide to themes and fonts on Unix-like operating systems, focusing on Arch Linux and Void Linux, a deep dive into Tamlinux&apos;s current theme and font mechanics, suggestions for future work to make code smaller and more efficient, and research on the historical development of fonts, displays, and colors in human-computer interfaces. The guide was requested as pages in the tam knowledge corpus with a progression from novice to expert developer. The AI tool gathered sources, confirmed code and historical facts, authored `knowledge/ecosystem/fonts.html` and `knowledge/ecosystem/themes.html`, updated the ecosystem hub and roadmap, and wrote this record and provenance rows.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `knowledge/ecosystem/index.html`, `knowledge/ecosystem/nix.html`, `knowledge/ecosystem/ttys-and-virtual-consoles.html`, `knowledge/ecosystem/terminfo-and-termcap.html`, `knowledge/ecosystem/toolkits.html`, `knowledge/ecosystem/distros.html`; `tamlinux` repository `desktop/theme/engine.py`, `desktop/theme/rename-font.py`, `commands/tam-theme-set/tam-theme-set`, `commands/tam-font-set/tam-font-set`, `commands/tam-install-font/tam-install-font`, `commands/tam-font-current/tam-font-current`, `commands/tam-theme-current/tam-theme-current`, `desktop/assets/defaults/fonts.conf`, `desktop/assets/tamarack.toml`; live command output from `fc-match`, `fc-list`, `readlink`, `python3 tools/lint-corpus.py`, and `build/tam show --pipe`.

## Guiding prompts

> Prepare an in-depth guide to themes and fonts on Unix-like operating systems, with specific focus on Arch Linux and Void Linux. Include a deep dive into how themese and fonts currently work in Tamlinux. Also include suggestions for future work to improve how themes and fonts can work to make the code smaller and more efficient. Do research on the historical development of fonts and displays and colors for human computer interfaces. The goal of this topic is to help a complete novice become an expert. There should be a progression of information from very basic to extremely detailed that would allow a person to become a competent developer for the theme and font systems for Tamlinux.

> This guide should be done as pages in the tam knowledge corpus.

## Key decisions

- The guide is organized into two dedicated, deep-dive companion pages under the `learn` track with parent `ecosystem/index.html`: `ecosystem/fonts.html` (topic: `fonts`) and `ecosystem/themes.html` (topic: `themes`). Both link to each other and are linked from the ecosystem hub.
- Both pages implement the three-tier pedagogical ladder:
  - Beginner: Read-only inspection commands (`fc-match`, `tam-font-current`, `tam-theme-current`, `tam-theme-color`), core definitions (glyphs, monospace, proportional, palettes, tokens, contrast), and self-quizzes.
  - Intermediate: Historical HCI evolution (character generator ROMs, Xerox Alto, PostScript Type 1, TrueType hinting VM, OpenType GSUB/GPOS, variable fonts; monochrome phosphors P1/P4/P3, ECMA-48/ANSI SGR codes, CGA/EGA/VGA CLUT, 24-bit TrueColor, sRGB CIE 1931 color science, gamma 2.2 and WCAG 2.1 relative luminance), Unix architecture (FreeType, HarfBuzz, Fontconfig; terminal OSC 4/10/11 escapes, GTK CSS, Qt QPalette, XDG desktop portal), comparison tables between Arch Linux (systemd) and Void Linux (runit/turnstile/seatd), and troubleshooting.
  - Expert: Full architectural dissection of `tamlinux/desktop/theme/engine.py` (atomic directory exchange via Linux `renameat2(RENAME_EXCHANGE)`, transactional recovery via `Journal`, mathematical contrast calculation, GTK 4 adaptive `@media (prefers-color-scheme)` stylesheet generation, dependency-free raw PNG synthesis via `struct` and `zlib`), OpenType table rewriting via `rename-font.py` (preserving `cmap`, `glyf`, `hmtx`), and concrete suggestions for future work to reduce code size and increase efficiency (consolidating subprocess adapters, minimal C engine/font tools, pruning unused font tables, daemonless inotify reload, and decoupling from dconf for Void Linux).
- Corpus version moves to 0.54.0.

## Verification

- `python3 tools/lint-corpus.py`: passes with 0 errors.
- `make corpus`: builds 142 entries with 6 index and metadata sidecars.
- `make corpus-and-test`: all 32 corpus-linter unit tests, 21 index unit tests, and 12 guide verification tests pass cleanly.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/fonts` and `build/tam show --pipe ecosystem/themes`: render cleanly in terminal without control sequences.
