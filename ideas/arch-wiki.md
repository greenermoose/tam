# Arch Wiki

## What it is

- **Format and licence:** MediaWiki articles; page footers state content is available under the GNU Free Documentation License 1.3 or later unless otherwise noted ([Help:Style, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Help:Style)).
- **Scope and size:** a broad community Linux reference wiki plus a deliberately linear Installation guide; a whole-wiki Table of contents index and an official offline package named `arch-wiki-docs` were source-read ([ArchWiki:About, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/ArchWiki:About); [Table of contents, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Table_of_contents)). No reliable current count of articles, editors or translations was collected.
- **Evidence:** full page reads for ArchWiki:Contributing, Help:Style, Help:I18n, Installation guide, General recommendations and the translation-team page; verdicts below are research inferences, not approved work.

## Ideas worth taking

- **adopt**: A fixed article skeleton with `Installation`, `Known issues`, `Tips and tricks`, `Troubleshooting` and `See also` makes both reader expectations and editing decisions predictable ([Help:Style, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Help:Style)).
  Tam application: give each corpus page a small set of named sections so lookups and additions have an obvious home.
- **adopt**: Status templates carry a reason field, sit at the top of an affected article or immediately above a section, and feed maintenance categories and statistics ([Template:Out of date, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Template:Out_of_date); [Help:Template, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Help:Template)).
  Tam application: expose a reasoned `stale`, `version-specific` or `needs re-check` state in a generated index rather than as untracked prose.
- **adopt**: General recommendations states its prerequisites exactly, naming the earlier guide and concepts a reader must already have followed ([General recommendations, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/General_recommendations)).
  Tam application: put an explicit "start here if you have already done X" line on every learning path landing page.
- **adopt**: The Installation guide reads pre-installation → installation → configuration → reboot → post-installation and links out to detail pages instead of embedding every variant ([Installation guide, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Installation_guide)).
  Tam application: keep one canonical route in a path and link alternatives from a separate reference page.
- **adopt**: An annotated index holds orientation, tips and links while the deep articles hold the detail, instead of duplicating both ([General recommendations, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/General_recommendations)).
  Tam application: make hub pages thin and link to canonical topic pages from them.
- **adopt**: Known issues and temporary workarounds should link to a bug report, and prose should avoid dated assertions and "currently" ([Help:Style, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Help:Style)).
  Tam application: keep durable evidence links in records and avoid baking access dates into evergreen pages.
- **consider**: Watchlists are described as the most important maintenance task, because staff cannot review everything ([ArchWiki:Contributing, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/ArchWiki:Contributing)).
  Tam application: assign a named watcher per subject area rather than introducing a wiki-wide review bureaucracy.
- **consider**: Translation state is explicit: flagged pages should not be translated, and `TranslationStatus` tracks the source title, date and revision ([ArchWiki:Translation Team, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/ArchWiki:Translation_Team)).
  Tam application: only useful if localisation is later approved; a source-revision field would be the first step.
- **reject**: Open wiki editing plus routine reversion of incomprehensible edits depends on a large watcher community ([ArchWiki:Contributing, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/ArchWiki:Contributing)).
  Tam application: a reviewed corpus with disjoint path claims already solves the trust problem more cheaply.
- **reject**: Full MediaWiki category trees and interlanguage redirect structures derive their value from a large wiki ecosystem ([ArchWiki:Contributing, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/ArchWiki:Contributing); [Help:I18n, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Help:I18n)).
  Tam application: a generated catalog and explicit aliases are enough for a compact offline corpus.

## Things to avoid

- Do not leave a status flag without a reason and a visibility path: Arch requires an explanatory argument and tracks flagged pages in maintenance categories ([Help:Style](https://wiki.archlinux.org/title/Help:Style); [Help:Template](https://wiki.archlinux.org/title/Help:Template)).
- Do not put dates, version numbers or "at the time of writing" into troubleshooting prose: the style guide says such claims rot silently ([Help:Style](https://wiki.archlinux.org/title/Help:Style)).
- Do not recommend one option over another in a reference page: Arch asks for objective advantages and disadvantages and moves personal comment to discussion pages ([Help:Style](https://wiki.archlinux.org/title/Help:Style)).
- Do not translate a page flagged `Accuracy`, `Style`, `Archive`, `Remove` or `Out of date`: the translation team rule exists to avoid propagating bad source text ([ArchWiki:Translation Team](https://wiki.archlinux.org/title/ArchWiki:Translation_Team)).
- Do not copy ArchWiki prose into a corpus: its content licence is GFDL 1.3-or-later, so ideas and structure are safe but expressive text carries conditions ([Help:Style](https://wiki.archlinux.org/title/Help:Style)).

## Where to look

- All URLs below were accessed 2026-11-26; labels reflect how the research notes obtained each page.
- [Full page read] [ArchWiki:Contributing](https://wiki.archlinux.org/title/ArchWiki:Contributing) — edit summaries, atomic edits, watchlists and maintenance culture.
- [Full page read] [Help:Style](https://wiki.archlinux.org/title/Help:Style) — section grammar, status templates, callouts, evidence and dating rules.
- [Full page read] [Help:I18n](https://wiki.archlinux.org/title/Help:I18n) — title conventions, interlanguage links and localized redirects.
- [Full page read] [Installation guide](https://wiki.archlinux.org/title/Installation_guide) — the linear novice path with detail links.
- [Full page read] [General recommendations](https://wiki.archlinux.org/title/General_recommendations) — annotated index, exact prerequisites and role cues.
- [Full page read] [ArchWiki:Translation Team](https://wiki.archlinux.org/title/ArchWiki:Translation_Team) and [Template:Out of date](https://wiki.archlinux.org/title/Template:Out_of_date) — translation and freshness states.
- [Full page read] [ArchWiki:About](https://wiki.archlinux.org/title/ArchWiki:About) and [Table of contents](https://wiki.archlinux.org/title/Table_of_contents) — offline package and browsable index.
- [Gap] `ArchWiki:Translations` returned 404; use Help:I18n and the translation-team page instead.
