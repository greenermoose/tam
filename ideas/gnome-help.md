# GNOME Help

## What it is

- **Format:** GNOME Help displays locally installed manuals in Yelp; Mallard is a topic-oriented XML format with standalone `<page>` files, `<info><title>`, `<desc>` and `<link type="guide" xref="...">`, while older help uses DocBook ([Mallard quickstart, full page read, accessed 2026-11-26](https://help.gnome.org/misc/tutorials/2013-12/mallard-quickstart.html); [Yelp README, source file read, accessed 2026-11-26](https://gitlab.gnome.org/GNOME/yelp/-/blob/main/README.md)).
- **Licence and scope:** GNOME Help pages state CC BY-SA 3.0; the help corpus covers desktop tasks, with app authors able to register in `org.gnome.Help` and a tile-based landing page ([GNOME Help landing page, full page read, accessed 2026-11-26](https://help.gnome.org/users/gnome-help/stable/)).
- **Evidence:** several GNOME help pages were read directly; the Mallard tutorial carries a deprecation banner and the wiki notes Mallard/Yelp are unmaintained. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Topic-oriented page IDs with explicit guide cross-links and a short description per page give hierarchy and searchable summaries without a heavy renderer ([Mallard quickstart, full page read, accessed 2026-11-26](https://help.gnome.org/misc/tutorials/2013-12/mallard-quickstart.html)).
  Tam application: give each corpus page a stable ID, a one-line description and typed outgoing links.
- **adopt**: Help documents install into language-coded directories and the help system falls back through locale variants ([GNOME help files, full page read, accessed 2026-11-26](https://developer.gnome.org/doc/gnome2.0/ghelp/ghelp-helpfiles.html)).
  Tam application: if localisation is approved, use a language directory layout with explicit fallback rather than duplicating the whole catalog.
- **adopt**: A tile hub gives each entry a one-line outcome, and an explicit "Get more help" tile handles failure routes ([GNOME Help landing page, full page read, accessed 2026-11-26](https://help.gnome.org/users/gnome-help/stable/)).
  Tam application: add outcome lines to path listings and a failure branch on risky checkpoints.
- **adopt**: A distinct visual overview page orients a newcomer before task pages begin ([GNOME Help landing page, full page read, accessed 2026-11-26](https://help.gnome.org/users/gnome-help/stable/)).
  Tam application: keep orientation as its own page type without checkpoint semantics.
- **consider**: Yelp profiles a SQLite search database built from installed help, and page descriptions are natural index inputs ([Mallard quickstart, full page read, accessed 2026-11-26](https://help.gnome.org/misc/tutorials/2013-12/mallard-quickstart.html)).
  Tam application: build a local index from page metadata and rendered sections rather than maintaining a second hand-written index.
- **consider**: Malcontent is a Mallard linter and validator, making validation part of the authoring toolchain ([GNOME help tools, full page read, accessed 2026-11-26](https://developer.gnome.org/doc/gnome2.0/ghelp/ghelp-tools.html)).
  Tam application: a small corpus linter for headings, IDs and links follows the same pattern.
- **consider**: Yelp's README names keyboard navigation and screen-reader support as goals ([Yelp README, source file read, accessed 2026-11-26](https://gitlab.gnome.org/GNOME/yelp/-/blob/main/README.md)).
  Tam application: treat keyboard and assistive-technology behaviour as pager requirements, but seek a dedicated source before quoting specific support.
- **reject**: Yelp or Mallard as a runtime dependency: the quickstart is deprecated and the project wiki says Mallard and Yelp are unmaintained ([Mallard quickstart, full page read, accessed 2026-11-26](https://help.gnome.org/misc/tutorials/2013-12/mallard-quickstart.html); [Mallard wiki page, full page read, accessed 2026-11-26](https://wiki.gnome.org/Projects/Mallard)).
  Tam application: copy the topic model only.
- **reject**: A breadcrumb UI as a first-class feature: no primary source read here describes one in Yelp or Mallard ([Gap, accessed 2026-11-26](https://help.gnome.org/users/gnome-help/stable/)).
  Tam application: typed guide links already express the hierarchy.

## Things to avoid

- Do not present Mallard guidance as current practice: the quickstart is dated 2013 and carries a deprecation banner, and Yelp's README points to newer documentation approaches ([Mallard quickstart](https://help.gnome.org/misc/tutorials/2013-12/mallard-quickstart.html); [Yelp README](https://gitlab.gnome.org/GNOME/yelp/-/blob/main/README.md)).
- Do not claim a specific GNOME Core app uses a local `help:` URI without checking its code; no primary source enumerating that wiring was found.
- Do not claim a glossary pane or breadcrumb component exists in Yelp; neither was source-read ([GNOME Help](https://help.gnome.org/users/gnome-help/stable/)).
- Do not rely on cabinet files or `.omf` registration: Yelp's README says cabinet files were removed and older registration docs are legacy ([Yelp README](https://gitlab.gnome.org/GNOME/yelp/-/blob/main/README.md)).
- Do not copy GNOME Help prose: its pages are CC BY-SA 3.0 ([GNOME Help](https://help.gnome.org/users/gnome-help/stable/)).

## Where to look

- All URLs below were accessed 2026-11-26; labels show evidence depth.
- [Full page read] [GNOME Help](https://help.gnome.org/users/gnome-help/stable/) — tile hub, flat contents list, edit link and licence.
- [Full page read] [Get help for an app](https://help.gnome.org/users/gnome-help/stable/get-help-app.html) — in-app Help menu pattern.
- [Full page read] [Mallard quickstart](https://help.gnome.org/misc/tutorials/2013-12/mallard-quickstart.html) — topic XML, cross-links, direct viewing and deprecation banner.
- [Source file read] [Yelp README](https://gitlab.gnome.org/GNOME/yelp/-/blob/main/README.md) — formats, URI handling, removed cabinet files and accessibility goals.
- [Full page read] [GNOME help files](https://developer.gnome.org/doc/gnome2.0/ghelp/ghelp-helpfiles.html) and [help tools](https://developer.gnome.org/doc/gnome2.0/ghelp/ghelp-tools.html) — install locators, locale fallback and validators.
- [Full page read] [Mallard wiki page](https://wiki.gnome.org/Projects/Mallard) — maintenance status.
