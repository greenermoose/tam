# Add start (tam-start) page and link from welcome.html (corpus 0.93.0)

- **Date:** 2026-11-15
- **Time:** 10:03 to 10:08 -05:00
- **Stage:** Develop, corpus 0.93.0 (`knowledge/desktop/session/start.html`, `knowledge/desktop/session/index.html`, `knowledge/welcome.html`, `knowledge/version`, `ROADMAP.md`, and rebuilt corpus indexes)
- **CLI Tool:** Antigravity CLI `1.5.9` (`agy --version`: `1.5.9`)
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending at record creation; committed with corpus index rebuild
- **Authorship:** Fred directed adding a page for `start` (`tam-start`) and linking to it from `welcome.html`; Antigravity CLI drafted the page, updated the links and version metadata, and ran the corpus checks and repository consistency check.
- **Checked against:** `tamlinux` repository `commands/start/README`, `commands/start/menu.json`, `commands/start/src/main.c`, and `./tamlinux/commands/start/build/start --help`, `--list`, `--check` output.

## Guiding prompts

> Is there a page in the tam knowledge corpus documenting the start command?

> Add a page for start (tam-start). Link to it from the welcome.html page.

## Key decisions

Placed the new page at `knowledge/desktop/session/start.html` under `desktop/session/index.html` ("Login, input and sessions") because `tam-start` is the text-login workstation activities menu that launches the graphical desktop, Guide, or terminal tools. Linked it from both `desktop/session/index.html` and `welcome.html`.

## Verification

`make corpus-and-test && make check-repo` (and `make corpus` index rebuild).
