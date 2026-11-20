# About page and welcome page orientation (corpus 0.36.0)

- **Date:** 2026-10-20
- **Time:** 19:15 to 19:46 -04:00
- **Stage:** Develop, corpus 0.36.0
- **CLI Tool:** Antigravity CLI 1.4.3
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `38fb18d`, and the follow-up index commit
- **Authorship:** Fred Horch requested paring down the welcome page and moving background sections into a dedicated about page; Antigravity drafted the plan, displayed it in Micro for review, claimed scopes, created `knowledge/about.html`, updated `knowledge/welcome.html`, cross-linked the new page, bumped corpus version, and regenerated indexes.
- **Checked against:** `knowledge/welcome.html`, `knowledge/ecosystem/philosophy.html`, `knowledge/ecosystem/sustainability.html`, `knowledge/tam.html`, `docs/corpus-authoring.md`, `ROADMAP.md`

## Guiding prompts

> /plan Pare down the welcome page of the tam corpus to include the two welcome paragraphs, the navigating sections, and a Where to next section. Move the Why "Tam" and Where things stand now to an about page. On the about page mention that Fred in the Tamlinux Guide usually means Fred Horch, the developer of Tamlinux. Include whatever else about Tamlinux and the Tam project you think is relevant and users would like to know. Figure out where to put the new about page and how to link it from other pages.

> /plan Show me the plan in a micro review window. I'm ready to see it now.

> I approve and accept the plan. Claim the work and proceed.

## Key decisions

- Placed the new about page at `knowledge/about.html` directly at the corpus root to parallel `knowledge/welcome.html`, `knowledge/beginner.html`, and `knowledge/tam.html`. This allows direct invocation via `tam show about` without a path prefix, while declaring track `learn` with parent `learn/index.html` to fit the five-part hierarchy.
- Linked `about.html` from `knowledge/welcome.html` (under Where to next), `knowledge/index.html` (Guide root list alongside Welcome), `knowledge/learn/index.html` (Foundations and references), `knowledge/ecosystem/index.html`, and `knowledge/ecosystem/philosophy.html`.
- On `knowledge/about.html`, explicitly noted that references to "Fred" refer to Fred Horch, the developer of Tamlinux, following the naming rules in `docs/corpus-authoring.md`. Covered the tamarack botanical metaphor, project status, sustainability and hardware longevity mission, capability-based profiles, C/libtam utility architecture, GPL-3.0-or-later licensing, and human-directed development with AI provenance.
- Bumped corpus version in `knowledge/version` and `ROADMAP.md` from 0.35.2 to 0.36.0 (minor bump for a new page).

## Verification

- `python3 tools/build-index.py`: generated 69 index entries cleanly across `index.tsv`, `index.json`, and `index.parents.tsv`.
- `python3 tools/lint-corpus.py`: zero findings.
- `python3 tools/check-repo.py`: clean repository check.
- `tests/guide.sh build/tam`: all guide tests passed, confirming rendering, stamps, link integrity, and extensionless name resolution (`tam show welcome` and `tam show about`).
- `python3 -m unittest discover tests`: all 80 unit tests passed.
- `tests/run.sh build/tam`: full test suite passed.
