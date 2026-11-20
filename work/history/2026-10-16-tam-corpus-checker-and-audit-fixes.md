# Corpus checker, renderer goldens and audit fixes (corpus 0.27.1)

- **Date:** 2026-10-16
- **Time:** 13:09 to 13:20 -04:00
- **Stage:** Develop. Corpus 0.27.0 to 0.27.1; command unchanged at 0.25.2.
- **CLI Tool:** Claude Code (`claude`) 2.1.301
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `b788c77`, `44c9183`, `91d2c47`, and the commit that adds this record.
- **Authorship:** Fred Horch, the Tamlinux developer, had approved three
  delegated tasks, then asked Claude Code to do them directly. Claude Code
  wrote the checker, the fixtures and the page fixes.
- **Checked against:** `src/render.c`, `src/tam.c`, `src/install.c`,
  `Makefile`, `tam-work --help`, `build/tam --version`; `tamlinux`
  repository `desktop/session/hypr/tamlinux/bindings/tiling.lua`,
  `utilities.lua`, `applications.lua`; `pacman -Si wlopm`. The llama.cpp
  cache options could not be checked locally (llama.cpp is not installed).

## Guiding prompts

> This is taking too long. Do the work yourself instead of delegating to opencode or agy. I'll have them do other things later. We need to get this done ASAP.

> Take the claim from opencode. I shut it donw.

## Key decisions

- The checker reads the entity list and subcommands from the C sources, so
  it cannot drift from the renderer and parser. In `tam` command matching
  it ignores paths (`~/src/tam test`) and version numbers (`tam 0.25.0`),
  which were the only false positives on the live corpus.
- The checker runs in `make test-corpus` and `make test` only once the
  corpus is clean, in the same commit as the fixes.
- The renderer goldens pin current output, including four quirks: blank
  padded lines around `<pre>`, unindented `<blockquote>`, trailing spaces
  in table cells, and shallow indentation of nested lists.

## Verification

`make test`: command tests, 6 renderer goldens, the checker (clean), its
29 unit tests and the guide tests pass; the live `tam ask` checks are
skipped without an Ollama server. Two checker mutants were each caught.
The development build is installed and reports command 0.25.2 and corpus
0.27.1.

Found, not fixed: the renderer joins a lone `.` to the previous word even
inside `<code>`, so `git add .` in `improve/toolchains.html` renders as
`git add.`.
