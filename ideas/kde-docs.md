# KDE docs

## What it is

- **Format:** KDE handbooks are DocBook XML compiled with `kdoctools` and CMake macros into HTML for KHelpCenter, while UserBase is a community wiki; the developer documentation still describes this as a living system ([Documentation / Handbook, full page read, accessed 2026-11-26](https://develop.kde.org/docs/developer-guide/documentation/)).
- **Scope and licence:** application handbooks, FAQs, API docs, tutorials and translation teams; KDE TechBase footers state CC BY-SA 4.0, but the handbook pages' licence was not separately checked ([KDE TechBase: Projects/Documentation, full page read, accessed 2026-11-26](https://techbase.kde.org/Projects/Documentation)).
- **Evidence:** handbooks and developer docs were read directly; some KHelpCenter "legacy" claims and per-app help wiring are starter notes and are marked as such. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: KHelpCenter offers a contents tree and search over locally installed documentation, reached from an application Help menu or the desktop menu ([KHelpCenter handbook, full page read, accessed 2026-11-26](https://docs.kde.org/stable5/en/kde-workspace/khelpcenter/index.html)).
  Tam application: keep a searchable local catalog plus a single `help`-style entry point rather than requiring a browser.
- **adopt**: Handbooks install under language-coded documentation directories that the help center scans to build its listing ([Documentation / Handbook, full page read, accessed 2026-11-26](https://develop.kde.org/docs/developer-guide/documentation/)).
  Tam application: discoverability should come from a generated catalog of installed pages, not from a hardcoded page list.
- **adopt**: DocBook carries glossary and index semantics through `<glossary>`, `<glossterm>` and `<glossdef>` ([Documentation / Handbook, full page read, accessed 2026-11-26](https://develop.kde.org/docs/developer-guide/documentation/)).
  Tam application: a small front-matter term list can provide term lookup without adopting DocBook.
- **adopt**: UserBase marks obsolete tutorials visibly on the index instead of leaving stale routes silent ([Tutorials, full page read, accessed 2026-11-26](https://userbase.kde.org/Tutorials)).
  Tam application: stale paths need a visible reason and a maintenance list.
- **consider**: A numbered subtutorial series gives a lightweight graded route without a new data model ([Tutorials, full page read, accessed 2026-11-26](https://userbase.kde.org/Tutorials)).
  Tam application: use explicit ordinal links between checkpoints instead of a separate platform.
- **consider**: The Getting Started index places a "Jargon File" glossary bridge at the start of a route ([Tutorials, full page read, accessed 2026-11-26](https://userbase.kde.org/Tutorials)).
  Tam application: link unfamiliar terms to the glossary at first use while keeping the step itself action-only.
- **consider**: Contributor guidance sorts work by skill, from spelling and readability through Git and XML authorship to mentoring ([KDE TechBase: Projects/Documentation, full page read, accessed 2026-11-26](https://techbase.kde.org/Projects/Documentation)).
  Tam application: offer a small contribution ladder rather than one undifferentiated intake.
- **reject**: DocBook and `kdoctools` as a dependency for a command-line corpus ([Documentation / Handbook, full page read, accessed 2026-11-26](https://develop.kde.org/docs/developer-guide/documentation/)).
  Tam application: take the section hierarchy and glossary semantics, not the XML toolchain.
- **reject**: An "Advanced" catch-all bucket as a levelling scheme ([Tutorials, full page read, accessed 2026-11-26](https://userbase.kde.org/Tutorials)).
  Tam application: require stated prerequisites for any advanced route.
- **reject**: Weblate as a translation platform: a 2023 KDE discussion answered "no" to plans to move to it ([KDE Discuss, full page read, accessed 2026-11-26](https://discuss.kde.org/t/any-plans-to-move-to-weblate-for-translation/3415)).
  Tam application: if translation is ever approved, borrow state metadata rather than the platform.

## Things to avoid

- Do not call KHelpCenter deprecated: the pages read document it as part of current developer guidance, and the "legacy" impression is an unverified starter note ([Getting Help](https://userbase.kde.org/Getting_Help); [starter note, accessed 2026-11-26](https://docs.kde.org/stable5/en/kde-workspace/khelpcenter/index.html)).
- Do not claim a dedicated breadcrumb control in KHelpCenter: none was source-read; DocBook's nested sections are the pattern.
- Do not assert specific screen-reader behaviour: KDE accessibility notes mention keyboard navigation generally, and specific claims need a primary source ([Documentation / Handbook](https://develop.kde.org/docs/developer-guide/documentation/)).
- Do not quote individual tutorial wording from UserBase: only the index was read, not the linked tutorials.
- Do not cite `community.kde.org/Documentation`: it returned 404 on the access date.

## Where to look

- All URLs below were accessed 2026-11-26; labels show evidence depth.
- [Full page read] [KHelpCenter handbook](https://docs.kde.org/stable5/en/kde-workspace/khelpcenter/index.html) — contents tree, search and Help-menu entry.
- [Full page read] [KDE Documentation / Handbook](https://develop.kde.org/docs/developer-guide/documentation/) — DocBook structure, build steps, install paths and translations.
- [Full page read] [UserBase Getting Help](https://userbase.kde.org/Getting_Help) — in-app, wiki and official-doc entry points.
- [Full page read] [UserBase Tutorials](https://userbase.kde.org/Tutorials) — category grouping, jargon bridge, numbered series and obsolete flag.
- [Full page read] [KDE TechBase: Projects/Documentation](https://techbase.kde.org/Projects/Documentation) — contributor skill tiers and licence footer.
- [Full page read] [Get Involved/translation](https://community.kde.org/Get_Involved/translation) and [Translate for KDE](https://community.kde.org/Translate_for_KDE) — language teams and bug reporting.
- [Full page read] [KDE Discuss: Weblate](https://discuss.kde.org/t/any-plans-to-move-to-weblate-for-translation/3415) — 2023 answer against adoption.
