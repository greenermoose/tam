# Terminal-input keybinding tradeoffs, Linux VT, SSH & F1–F12 sequences (corpus 0.88.1)

- **Date:** 2026-11-14
- **Time:** 07:07 to 07:12 -05:00
- **Stage:** Develop; expanded four existing `ecosystem/terminal-input/` pages and bumped `knowledge/version` from `0.88.0` to `0.88.1`
- **CLI Tool:** Antigravity CLI `1.5.9` (`agy --version`)
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** recorded in `git log` for corpus `0.88.1`
- **Authorship:** Fred directed the topic list and structure; Antigravity CLI surveyed `knowledge/ecosystem/terminal-input/`, verified terminal/keymap data, authored the expansions, and ran corpus checks.
- **Checked against:** `knowledge/ecosystem/terminal-input/{index,choosing-keys,linux-console,other-contexts,escape-sequences}.html`, `infocmp -1 linux`, `infocmp -1 xterm-256color`, `infocmp -1 foot`, `infocmp -1 rxvt`, `/usr/share/kbd/keymaps/i386/include/linux-keys-bare.inc`, `whatis setmetamode`, `libtam/src/tam_key.c`

## Guiding prompts

> Goal: Document TUI keybinding behavior, terminal escape sequences, and function-key encoding across environments in the tam knowledge corpus. (Full structured prompt covering 1. TUI Keybinding Tradeoffs, 2. Linux Kernel Console Virtual Terminal, 3. SSH & Slow/Unreliable Connections, 4. How Function Keys Are Sent Across Environments with Hex Table.)

## Key decisions

Expanded the four matching existing pages in `knowledge/ecosystem/terminal-input/` (`choosing-keys.html`, `linux-console.html`, `other-contexts.html`, `escape-sequences.html`) rather than duplicating them into new standalone pages, preserving the existing nine-page section hierarchy.

## Verification

`make corpus-and-test`: Corpus checks (667 pages, 667 parsed, 667 rendered), component coverage check, 85 unit tests, and `check-repo.py` all passed. Previewed rendered output with `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/terminal-input/escape-sequences`.
