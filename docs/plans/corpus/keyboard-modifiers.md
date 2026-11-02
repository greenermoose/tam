# Corpus note: The Meta Key and Desktop Modifiers

**Plan status:** Completed
**Reviewed:** 2026-11-02
**Scope:** Missing-content plan for the Meta key, modifier history, and desktop navigation flow.
**Authority:** [Plan maintenance procedure](../../plan-maintenance.md) and developer direction.
**Next action:** None — pages shipped; no further implementation.

- **Date:** 2026-11-01
- **Asked for:** "Remember that we are NOT calling it SUPER any more. It is called the Meta key on Tamlinux. Who calls it SUPER and why? Do some research into the history of the Meta / Windows / Command / SUPER key. Make a plan to add that topic to the tam knowledge corpus."
- **Failed queries:**
  - `tam explain meta` -> `tam: no topic found for 'meta'`
  - `tam explain super` -> `tam: no topic found for 'super'`
  - `tam search "super key"` -> matches only `desktop` and `help`
  - `tam search "meta key"` -> matches only `desktop` and `help`
- **Proposed page:** `knowledge/desktop/meta-key.html`, parent `desktop/index.html`, track `use`. Belongs under `desktop/` because the Meta key is Tamlinux's primary desktop interface and interaction model, not a third-party ecosystem choice.
- **Scope:**
  - Beginner: Physical keyboard identification (Windows logo, Apple ⌘, or unbranded key), essential desktop chords (`Meta + Return`, `Meta + Space`, `Meta + K`, `Meta + Q`), and the mental model (`Ctrl` for programs, `Alt` for options, `Meta` for the desktop).
  - Intermediate: 50 years of modifier history (1970 SAIL bucky bits, 1975 MIT Knight, 1978 Space-Cadet, 1984 Susan Kare's Apple Command key, 1994 Microsoft Natural Keyboard, XFree86 / X11 `SUPER` mapping), and why Tamlinux reclaims the name **Meta**.
  - Expert: Evdev scancodes (`KEY_LEFTMETA = 125`), `xkbcommon` translation, and compositor dispatchers.
- **Sources to check:**
  - `tamlinux` repository `desktop/session/hypr/tamlinux/bindings/`
  - `/usr/include/linux/input-event-codes.h` (`KEY_LEFTMETA`, `KEY_RIGHTMETA`)
  - `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `knowledge/desktop/index.html`
- **Open questions:** None; developer approved placement in `desktop/`.
- **Completion / replacement:** Delivered in corpus 0.56.0 as
  `knowledge/desktop/meta-key.html` and `knowledge/learn/glossary/meta.html`.
  Closed 2026-11-02 during the corpus gap remediation audit (content already
  present; Ready status was stale).
