# Docusaurus

## What it is

- **Format:** Markdown/MDX docs in a `docs/` tree with generated routes; cutting a version copies the whole docs tree into a versioned directory, creates a versioned sidebar and appends the version to a versions file ([Docusaurus versioning, full page read, accessed 2026-11-26](https://docusaurus.io/docs/versioning)).
- **Scope and licence:** a static documentation site builder with version banners, whole-file translation, UI-string catalogues and pluggable search; no licence statement was checked.
- **Evidence:** versioning, i18n and search pages were read directly. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Relative `.md` links are rewritten to the correct version's route, which is why versioned copies remain navigable ([Docusaurus versioning](https://docusaurus.io/docs/versioning)).
  Tam application: author links against source identifiers and rewrite them at build time, not against rendered paths.
- **adopt**: Version banners and badges mark unreleased or unmaintained state on every page ([Docusaurus versioning](https://docusaurus.io/docs/versioning)).
  Tam application: a single front-matter "applies to version X" field is enough if a second supported version ever appears.
- **adopt**: Contextual search adds language and version facets so a reader sees results matching the page being read ([Docusaurus search, full page read, accessed 2026-11-26](https://docusaurus.io/docs/search)).
  Tam application: add facets to the local index only if multiple languages or versions are actually shipped.
- **consider**: Whole-file translation preserves context, while UI strings live in a separate catalogue with a message and a translator description ([Docusaurus i18n, full page read, accessed 2026-11-26](https://docusaurus.io/docs/i18n/introduction)).
  Tam application: if localisation is approved, keep page translations whole and centralise interface labels.
- **consider**: i18n goals include low runtime overhead, independently built locales, localisable assets, hreflang defaults, RTL support and no automatic locale detection ([Docusaurus i18n](https://docusaurus.io/docs/i18n/introduction)).
  Tam application: predictable locale selection matters more than automatic guessing in an offline tool.
- **consider**: The default DocSearch theme is described as designed for accessibility with attention to colours and contrast ([Docusaurus search](https://docusaurus.io/docs/search)).
  Tam application: use contrast-aware, keyboard-friendly search UI patterns even if the backend is local.
- **reject**: Docusaurus-style version trees: the docs advise versioning only when traffic or change rate justifies it and keeping the number of versions small ([Docusaurus versioning](https://docusaurus.io/docs/versioning)).
  Tam application: label pages rather than forking whole trees for a small stable corpus.
- **reject**: Hosted crawled search and Ask AI for this corpus ([Docusaurus search](https://docusaurus.io/docs/search)).
  Tam application: they require network services and a crawler queue, conflicting with offline reading.
- **reject**: Copying per-version asset trees when assets are really global ([Docusaurus versioning](https://docusaurus.io/docs/versioning)).
  Tam application: decide asset ownership once and keep it explicit.

## Things to avoid

- Do not version by default: Docusaurus itself warns that build time and contributor complexity grow with each version ([Docusaurus versioning](https://docusaurus.io/docs/versioning)).
- Do not assume search facets exist without putting facet fields into the index ([Docusaurus search](https://docusaurus.io/docs/search)).
- Do not treat the accessibility statement as a completed audit of every Docusaurus site ([Docusaurus search](https://docusaurus.io/docs/search)).
- Do not import MDX tooling to solve a problem the corpus's own build already solves.

## Where to look

- All URLs below were accessed 2026-11-26.
- [Full page read] [Docusaurus versioning](https://docusaurus.io/docs/versioning) — version trees, banners, badges and the under-10-versions rule of thumb.
- [Full page read] [Docusaurus i18n](https://docusaurus.io/docs/i18n/introduction) — whole-file translation, UI JSON and locale goals.
- [Full page read] [Docusaurus search](https://docusaurus.io/docs/search) — search options, crawler schedule, contextual facets and accessibility statement.
