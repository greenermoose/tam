# D221 final anatomy batches (corpus 0.70.0)

- **Date:** 2026-11-02
- **Time:** ~23:45 -05:00
- **Stage:** Develop. Corpus 0.69.0 to 0.70.0; command version unchanged at 0.48.2.
- **CLI Tool:** Cursor Composer
- **Model:** Composer (Auto)
- **Transcript:** Private Cursor session.
- **Commits:** Pending on `develop` at record write time.
- **Authorship:** Fred directed four remaining batches in order. Cursor migrated twenty-one pages, registered four paths, expanded hard-enforcement, bumped the corpus version and synced roadmap records.

## Guiding prompts

> Do session and desktop, then understand the design, then terminal stack, and then the small leftovers.

## Key decisions

Four embedded paths covering the last anatomy warnings:

- Session and desktop: ttys → sddm → evdev
- Understand the design: packaging → nix → distros → patches → virtualization → toolkits
- Terminal stack: terminfo → ncurses → fonts → themes → lua
- Guide leftovers: philosophy → sustainability → ai → learn/tam → practice → man → tam.html

## Verification

`make corpus-and-test` passed (HTML linter, linter unit tests, index unit tests including the non-pilot fixture moved off `ecosystem/ai.html`, index generation with zero learning warnings, guide.sh).
