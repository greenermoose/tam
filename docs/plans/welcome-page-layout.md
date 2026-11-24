# Welcome page and intro-first layout

**Plan status:** Completed
**Reviewed:** 2026-11-24
**Scope:** Corpus 0.124.0: rewrite `knowledge/welcome.html` as an intro-first
welcome page, and make that opening pattern standard across the corpus by
removing body headings that repeat the breadcrumb. Update templates, authoring
rules, the glossary hub generator and the guide test so the pattern holds. No
command, install or stage change.
**Authority:** The developer asked for a better welcome introduction with a
blank line after the breadcrumb, the exact opening text `Hello!`, the Tamlinux
performance sentence and `get started | get help`, with no duplicate `Welcome
to Tamlinux` heading. He asked that page headings repeating the breadcrumb be
removed corpus-wide, that intro paragraphs be well written, and that pages
follow an intro then novice, learner and expert sections, with the welcome page
as the single-section exception. He chose whole-corpus application when asked
about scope.
**Next action:** None. Closed 2026-11-24 at corpus 0.124.0; learner review,
installation and packaged acceptance remain developer-directed.

## Problem and agreed outcome

The welcome page opened with a Start Menu instruction and a `Welcome to
Tamlinux` heading that repeated its breadcrumb. The same pattern appeared on
almost every page: a body `<h1>` restating the breadcrumb title before the real
text. Readers met the page name twice before any content, and the first
paragraph became an afterthought.

Every non-root page now opens with the breadcrumb followed directly by a
written intro. The welcome page has the requested three-paragraph opening and
curated links that show what Tamlinux is and what the Guide contains. It is the
only page whose whole body is shared intro text with no level sections. Other
topic pages keep their novice, learner and expert sections below a shared
intro. The guide root keeps its `<h1>` because it has no breadcrumb.

## Corpus-wide decision

- A page's name belongs in `<title>` and the breadcrumb, not in a body heading.
  Remove a heading when it repeats the current breadcrumb crumb, including
  small capitalisation or HTML-entity differences.
- Keep an intro paragraph after the breadcrumb on every page. Do not use a
  label or bold text on it.
- A topic page has an intro, then its level sections according to its pattern.
  The welcome page is the exception: shared content only, visible at every
  lens.
- Generated glossary hubs must follow the same rule, so the pattern survives
  `make corpus`.

## Implementation and handoff

1. **Welcome page (TCK32.1):** Rewrite `knowledge/welcome.html` with the exact
   opening, an Open this Guide section, a What the Guide contains table of the
   eight subject hubs, Ways to read, Fred's message, return keys, Next page and
   Sources. Link to subject hubs, paths, the skills map, glossary, bibliography
   and help so a reader gets a full picture of the project and the Guide.
2. **Corpus headings (TCK32.2):** Remove the redundant `<h1>` from all 1,442
   non-root pages; leave `knowledge/index.html` untouched. Fix
   `tools/glossary_index.py` so regenerated glossary hubs have no `<h1>`.
3. **Standards and checks (TCK32.3):** Update the four templates,
   `docs/corpus-pedagogy.md`, `docs/corpus-authoring.md`, the `build-index.py`
   module docstring and `tests/guide.sh` title extraction. Add TCK32 and update
   the version, roadmap, plan index, development contract and work records.
4. **Verification (TCK32.4):** Run `make corpus`, `make corpus-and-test` and
   `make check-repo`. Spot-check the welcome render, an ordinary page, a
   glossary hub, `tam:paths` and `tam explain updates`.

## Verification

Completed 2026-11-24 at corpus 0.124.0. `make corpus`, `make corpus-and-test`
and `make check-repo` pass. Reader smoke checks confirm the requested welcome
opening, no duplicate heading on a regular page or glossary hub, and working
path and `explain updates` lookups. Command version and all stage promotion,
installation and packaged acceptance remain developer-directed.
