# Typography curriculum from marks to research

**Plan status:** Completed
**Reviewed:** 2026-11-28
**Scope:** Corpus-only content ending at corpus 0.140.0: a Desktop >
Typography hub, fifteen three-level teaching pages, three linked goal paths,
glossary coverage for the requested typography terms and the practical full
set a developer meets, bibliography sources, `typography.*` skill cells, and
contextual links from the existing Tamlinux fonts page. Teaching examples use
labelled sandboxes with sample files and output; no font is installed, no
system setting is changed and no release stage moves. The command version is
unchanged.
**Authority:** The developer asked: "Create a series in the tam knowledge
corpus about fonts and typography on Tamlinux. Add glossry pages for TrueType,
FreeType, OpenType, kerning, ligatures, and every other typography term a
developer might need to know. Make sure the series takes a reader who has no
background at all in typography all the way to world-leading expertise. Fully
describe all the font formats. Explain which fonts are built into the Linux
kernel. List font foundries and include a history of the printing press going
back to Gutenberg and before with scribes. Start with the beginning of writing
and cover all the forms of writing that have been invented by various
civilizations around the world. Explain the differenc between alphabet,
syllabaries, etc." He then said: "Make it so." When asked about design
choices, he chose Desktop > Typography as the canonical home, all major
families plus a worldwide reference catalogue for writing systems, and three
linked paths.
**Next action:** None for implementation. Learner review and any packaged
acceptance remain developer-directed.

Follow the [plan maintenance procedure](../plan-maintenance.md).

## Problem and agreed outcome

A reader can learn typography from the first marks on clay and stone to
current research without leaving the offline guide. The series distinguishes
writing systems, explains the history of scribes and printing, gives a complete
map of font formats, opens TrueType and OpenType files, names every font built
into a cited Linux kernel source, follows characters through shaping and
rasterization, places Fontconfig and friends in one pipeline, lists foundries,
and ends with accessibility, diagnosis, tooling and open questions. The
existing `desktop/appearance/fonts.html` remains the Tamlinux-specific
workflow page.

The agreed scope limit is that "all forms of writing" means deep teaching on
the major families plus a worldwide reference catalogue, not a separate thin
page for every known script.

## Baseline and dependencies

- Baseline at the start of work: command 0.72.0, corpus 0.139.1, branch `dev`.
- `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/corpus-style.md`
  and the templates in `docs/templates/` define the teaching contract.
- The subject library places the series under `Desktop and applications >
Typography`; task and learning paths cross-link without duplicating lessons.
- Kernel font statements are pinned to Linux stable v6.12 source and its
  versioned documentation. Tamlinux-specific statements use product source:
  the Fontconfig config, `tam-font-list`, `tam-font-current` and the menu
  integration.
- General history and specifications are cited from bibliography entries.
  Sections disagreeing with each other are reported in the work record, not
  silently resolved on a page.

## Implementation and handoff

| Checklist | Slice | Paths and evidence |
| --- | --- | --- |
| TCK37.1 | Hub and foundations | `knowledge/desktop/typography/index.html`, the desktop hub link, `marks-to-meaning`, `writing-systems`, `scribes-and-print`, `anatomy-of-type`, `font-formats`, `foundries`, `typography-foundations` |
| TCK37.2 | Internals | `opentype-internals`, `kernel-fonts`, `text-shaping`, `rasterization`, `linux-text-stack`, `workbench`, `font-internals` |
| TCK37.3 | Practice and support | `accessibility`, `troubleshooting`, `research-frontiers`, the revised `appearance/fonts.html`, `typography-on-tamlinux`, glossary, bibliography and skills |
| TCK37.4 | Bookkeeping and verification | Corpus 0.140.0 records, corpus build, repository check, coverage report, reader spot checks and `whatis` checks |

Each teaching page carries all three levels, a Learner sandbox with labelled
sample output, a Learner quiz and expert open prompts. Capstones give
observable expert practice. Structural checks prove that the pages exist and
link correctly; they do not prove learner mastery or human acceptance.

## Acceptance and limits

- `make corpus`, `make corpus-and-test` and `make check-repo` pass.
- `python3 tools/report-coverage.py --warnings` reports no new anatomy, skill,
  path or broken-link errors attributable to this series.
- Reader spot checks with `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe`
  cover the hub, a ladder page, a path, a glossary entry and a bibliography
  entry.
- Live `whatis` checks resolve TrueType, FreeType, OpenType, kerning and
  ligature.
- Commit trailers, `work/W158-typography` and `work/WHATNOW` record the work
  without session IDs, transcript paths, home paths or private repository
  names.
- No Test, Run, Release, install, tag, marketplace action, stage push or
  `main`/`test` push is part of this plan. Learner review and packaged
  acceptance remain developer-directed.

## Disposition

Completed on 2026-11-28 at corpus 0.140.0. The Desktop > Typography hub,
fifteen three-level teaching pages, three paths, 107 new and 18 refined
glossary entries, 27 bibliography pages and the three `typography.*` skill
cells are in `knowledge/`. The command version stays 0.72.0.

Verification: `make corpus` built 1943 pages; `make corpus-and-test`,
`make check-repo` and `python3 tools/report-coverage.py --warnings` all exit 0,
with the three typography skill cells reporting no gaps. All 511 `<cite>`
references resolve and every internal link in the new pages resolves; the one
remaining link-check report is a pre-existing escaped `<link>` example in
`programming/web-development/html-and-css.html`. Reader spot checks with
`TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe` covered the hub, a ladder
page, a path, a glossary entry and a bibliography entry, and live `whatis`
resolved TrueType, FreeType, OpenType, kerning and ligature.

Outstanding limit: structural and reader checks do not prove learner mastery
or human acceptance. No Test, Run, Release, install, tag, marketplace action,
stage push or `main`/`test` push was part of this work.
