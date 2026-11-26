# elementary OS help

## What it is

- **Format:** web-first help organised by product and version, with Markdown sources in Git repositories reported for `elementary/help` and docs.elementary.io; the generator chain was not verified from a primary source ([elementary/help repository, search-result snippet, accessed 2026-11-26](https://github.com/elementary/help)).
- **Scope and licence:** per-app help and user guides under versioned and language-coded URL paths; no licence, article count or current landing-page text was verified.
- **Evidence:** this is the thinnest desktop-help note: the help site landing page returned no usable content in this environment, the URL shape comes from search results or cached references, and most help-menu wiring is a starter note. Labels below preserve `[N]`, `[G]` and snippet qualifiers; verdicts are research inferences, not approved work.

## Ideas worth taking

- **consider**: Version-in-path URLs make help links linkable and alignable with a release without an in-viewer version selector ([help.elementary.io URL shape, starter note from search results, accessed 2026-11-26](https://help.elementary.io/)).
  Tam application: pin generated deep links to a supported version label only if more than one version is actually shipped.
- **consider**: Markdown in Git with a generated site is a low-barrier authoring route that resembles a reviewed corpus lane ([elementary/help repository, search-result snippet, accessed 2026-11-26](https://github.com/elementary/help)).
  Tam application: keep sources in the repository and generate navigable output, but preserve local pages for offline use.
- **consider**: Language segments in URLs make translation state visible in the address rather than hidden in a viewer ([help.elementary.io URL shape, starter note, accessed 2026-11-26](https://help.elementary.io/)).
  Tam application: only relevant if localisation is later approved; a small language field in metadata would be enough.
- **consider**: Category navigation and site search are reported on the help site ([starter note from snippets, accessed 2026-11-26](https://help.elementary.io/)).
  Tam application: outcome-led category listings can sit above a generated search index.
- **reject**: A web-only help path when offline use is a requirement ([gap, accessed 2026-11-26](https://help.elementary.io/)).
  Tam application: keep local pages and a terminal reader as the primary surface.
- **reject**: Depending on a desktop-specific Help viewer that is not installed by default ([starter note, accessed 2026-11-26](https://docs.elementary.io/)).
  Tam application: a subcommand plus deep links must work without a desktop session.

## Things to avoid

- Do not assert a first-class offline help viewer exists for current Pantheon releases: no primary evidence was found ([gap, accessed 2026-11-26](https://help.elementary.io/)).
- Do not state the help site's landing text or build commands as verified: the fetch produced no usable content and the generator chain was not read ([starter note, accessed 2026-11-26](https://docs.elementary.io/)).
- Do not claim a specific app's Help menu opens a local page rather than a web URL: per-app code was not checked ([starter note, accessed 2026-11-26](https://github.com/elementary/help)).
- Do not claim a glossary or breadcrumb feature: none was source-read.

## Where to look

- URLs below were attempted or found on 2026-11-26; all elementary-specific details remain `[N]` or `[G]` until re-checked in a browser or repository.
- [Attempted, no usable content] [help.elementary.io](https://help.elementary.io/) — versioned and language-coded path shape from cached references.
- [Starter note] [docs.elementary.io](https://docs.elementary.io/) — contributor-facing learning and help documentation.
- [Search-result snippet] [elementary/help](https://github.com/elementary/help) — Markdown sources and site CNAME.
- [Gap] No primary source was found for a unified offline viewer, current landing text or per-app Help-menu wiring.
