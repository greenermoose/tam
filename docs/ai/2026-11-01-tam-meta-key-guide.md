# The Meta key and desktop modifiers (corpus 0.56.0)

- **Date:** 2026-11-01
- **Time:** 10:00 to 10:55 -05:00
- **Stage:** Develop, corpus 0.56.0
- **CLI Tool:** Antigravity CLI 1.4.12 (from `agy --version`)
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** This commit adds the Meta key guide, glossary entry, aligned desktop bindings, plan note, provenance rows, and refreshed indexes.
- **Authorship:** Fred directed an audit of the knowledge corpus against current Tamlinux configuration, followed by the platform rule: "Remember that we are NOT calling it SUPER any more. It is called the Meta key on Tamlinux. Who calls it SUPER and why? Do some research into the history of the Meta / Windows / Command / SUPER key. Make a plan to add that topic to the tam knowledge corpus." Upon research into keyboard modifier history and developer approval of placing the guide in the `desktop/` track, the AI tool claimed paths with `tam-work`, authored `knowledge/desktop/meta-key.html`, authored `knowledge/learn/glossary/meta.html`, updated `knowledge/learn/glossary/index.html`, corrected outdated keybindings in `knowledge/desktop/index.html` and `knowledge/desktop/workspaces.html`, bumped `knowledge/version` to 0.56.0, updated `ROADMAP.md` and `docs/plans/README.md`, rebuilt index sidecars, and recorded this provenance entry.
- **Checked against:** `tamlinux` repository `desktop/session/hypr/tamlinux/bindings/tiling.lua`, `utilities.lua`, `applications.lua`; `/usr/include/linux/input-event-codes.h`; `docs/corpus-authoring.md`; `docs/corpus-pedagogy.md`; `knowledge/desktop/index.html`; live command output from `python3 tools/lint-corpus.py`, `python3 tools/build-index.py`, `make corpus-and-test`, and `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe desktop/meta-key`.

## Guiding prompts

> Do an audit of the tam knowledge corpus. Compare what it says with the current state of Tamlinux. Are there any errors that need correcting? Improvements that you recommend?

> Remember that we are NOT calling it SUPER any more. It is called the Meta key on Tamlinux. Who calls it SUPER and why? Do some research into the history of the Meta / Windows / Command / SUPER key. Make a plan to add that topic to the tam knowledge corpus.

> Proceed

## Key decisions

- Placed the primary reference guide at `knowledge/desktop/meta-key.html` (topic: `meta-key`) under track `use` with parent `desktop/index.html`. While modifier origins are historical, the Meta key is the core physical interface and interaction pillar of the Tamlinux desktop, making `desktop/` its natural home.
- Added companion glossary entry `knowledge/learn/glossary/meta.html` (topic: `glossary-meta`) and updated `knowledge/learn/glossary/index.html`.
- Structured `knowledge/desktop/meta-key.html` as a three-tier reference guide:
  - **Beginner:** Physical keyboard identification (Windows logo key, Apple Command `⌘` key, or unbranded icon), the core mental model (`Ctrl` for inner program operations, `Alt` for secondary options, `Meta` for desktop shell operations), and essential desktop chords (`Meta + Return` for terminal, `Meta + Space` for runner, `Meta + K` for keybindings, `Meta + Q` / `Meta + Backspace` to close, `Meta + Shift + F` to toggle fullscreen).
  - **Intermediate:** 50 years of modifier history detailing the 1970 SAIL bucky bits (Stanford AI Lab), 1975 MIT Knight keyboard, 1978 MIT Space-Cadet keyboard (introducing Meta, Super, Hyper), 1984 Susan Kare Apple Command key (`⌘`), 1994 Microsoft Natural Keyboard (introducing the Windows logo key), and XFree86/X11's mapping to `Mod4`/`XK_Super_L`. Clarified why tiling window managers picked up `SUPER` and why Tamlinux reclaims `Meta` for semantic precision, vendor independence, and hacker heritage.
  - **Expert:** Low-level Linux input subsystem plumbing: evdev scancodes (`KEY_LEFTMETA = 125`, `KEY_RIGHTMETA = 126`), `xkbcommon` keysym mapping (`XKB_KEY_Meta_L` / `XKB_KEY_Super_L`), Wayland compositor dispatchers (`hyprland.conf` `$mainMod = SUPER`), and universal middle-click / keyboard clipboard mechanics.
- Aligned existing desktop documentation to remove outdated references:
  - Replaced all `SUPER` occurrences in `knowledge/desktop/workspaces.html` with `Meta`.
  - Corrected desktop binding drift in `knowledge/desktop/index.html` and `knowledge/desktop/workspaces.html`: close window (`Meta + Q` and `Meta + Backspace`), file manager (`Meta + F`), fullscreen toggle (`Meta + Shift + F`), silent workspace move (`Ctrl + Alt + <key>`), monitor move (`Alt + Shift + Arrow`), and scratchpad move (`Ctrl + Shift + S`).
- Corpus version moves to 0.56.0.

## Verification

- `python3 tools/lint-corpus.py`: passes with 0 findings.
- `python3 tools/build-index.py`: builds 148 entries with 6 index and metadata sidecars.
- `make corpus-and-test`: all 32 corpus linter unit tests, 21 index unit tests, and 12 guide verification tests pass cleanly.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe desktop/meta-key`: renders cleanly in plain text with clear section headers and tables.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam explain meta-key --pipe`: resolves topic cleanly.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam search --pipe "super key"`: resolves `meta-key` and `glossary-meta`.
