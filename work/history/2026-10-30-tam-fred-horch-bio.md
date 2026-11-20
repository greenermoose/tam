# Fred Horch bio page (corpus 0.49.0)

- **Date:** 2026-10-30
- **Time:** 06:11 to 06:17 -04:00
- **Stage:** Develop, corpus 0.49.0
- **CLI Tool:** Cursor `3.27.22` (from `cursor --version`)
- **Model:** Composer (`composer`). This session identified itself as Auto, powered by Composer; no finer model id was logged by the CLI.
- **Transcript:** Retained privately by the author.
- **Commits:** (pending Fred's commit instruction)
- **Authorship:** Fred Horch supplied the bio text and asked for `learn/about/fred.html`; Cursor planned then implemented the page, About cross-link, corpus bump, and index rebuild.
- **Checked against:** Fred's supplied bio text in the session request; `docs/corpus-authoring.md` (developer naming); `knowledge/about.html`; `knowledge/learn/glossary/rootstock.html` (existing link to the new page); `docs/templates/single.html`

## Guiding prompts

> Add a page, tam/knowledge/learn/about/fred.html, with a short bio for Fred Horch. He is the developer of Tamlinux. He earned a degree in computer science from Swarthmore College where he was a sysadmin for the department's Sun workstation lab from 1989 to 1991. After graduation, he worked in the San Francisco Bay Area as a network admin for Sybex, then attended UC Berkeley Law School and worked as general counsel for TriNet Services, an Internet consulting firm in the reserch triangle Raleigh-Durham area of North Carolina. He is developing Tam so he can work more effectively with AI agents to get work done on the computers he and his family own.

> Implement the plan as specified, it is attached for your reference. Do NOT edit the plan file itself.

## Key decisions

- Placed the page at `knowledge/learn/about/fred.html` as requested, with parent `learn/index.html` and kind `explanation` (Single / beginner pattern). No `learn/about/index.html` hub.
- Spelled Research Triangle correctly in the published bio.
- Linked from `knowledge/about.html` “The developer” without duplicating the full bio there.
- Left the existing rootstock glossary link unchanged; it resolves now that the page exists.
- Bumped corpus version from 0.48.1 to 0.49.0 (minor bump for a new page).

## Verification

- `make corpus`: 133 index entries generated cleanly.
- `make corpus-and-test`: lint, unit tests, `check-repo.py`, and `tests/guide.sh` all passed.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe learn/about/fred`: renders the bio.
- Confirmed About and rootstock pages expose the `learn/about/fred.html` link.
