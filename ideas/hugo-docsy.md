# Hugo Docsy

## What it is

- **Format:** Docsy is a preconfigured Hugo documentation theme that builds HTML from Markdown or HTML source plus theme files ([Docsy documentation, search-result snippet, accessed 2026-11-26](https://www.docsy.dev/docs)).
- **Scope and licence:** documentation theming with a version selector, archived-version banners and several search backends; licence and size were not checked.
- **Evidence:** this note is thin and snippet-only: no Docsy page was fetched in full, and search behaviour was described through search-result excerpts. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **consider**: A version selector is configured from a small list of URL and name entries, with an archived-version flag that displays a banner ([Docsy versioning, search-result snippet, accessed 2026-11-26](https://www.docsy.dev/docs/content/versioning)).
  Tam application: a single metadata field and a generated version list are enough if more than one version is shipped.
- **consider**: Local Lunr search can be enabled with an offline option that generates a JSON search index ([Docsy search, search-result snippet, accessed 2026-11-26](https://www.docsy.dev/docs/content/search)).
  Tam application: confirms the wider pattern of shipping a derived index with static pages.
- **consider**: No search box appears until a search backend is explicitly configured ([Docsy search, search-result snippet, accessed 2026-11-26](https://www.docsy.dev/docs/content/search)).
  Tam application: avoid advertising a search UI unless the index actually exists and is built.
- **reject**: Google Custom Search or hosted Algolia search for an offline-first reader ([Docsy search, search-result snippet, accessed 2026-11-26](https://www.docsy.dev/docs/content/search)).
  Tam application: local index generation is the only compatible option here.
- **reject**: Adopting Hugo or Docsy from these snippets alone ([Docsy documentation, search-result snippet, accessed 2026-11-26](https://www.docsy.dev/docs)).
  Tam application: verify any platform detail in primary docs before treating it as settled.

## Things to avoid

- Do not name exact configuration keys beyond those quoted in the snippets without re-reading the Docsy source.
- Do not claim the offline index format or size: only that a JSON index is generated.
- Do not treat Docsy's version list as evidence that a small corpus should be versioned.
- Do not assume the site theme's accessibility behaviour from these sources.

## Where to look

- URLs below were found on 2026-11-26; every item is a search-result snippet, not a full page read.
- [Search-result snippet] [Docsy documentation](https://www.docsy.dev/docs) — theme purpose and Hugo build model.
- [Search-result snippet] [Docsy versioning](https://www.docsy.dev/docs/content/versioning) — version selector and archived banner.
- [Search-result snippet] [Docsy search](https://www.docsy.dev/docs/content/search) — hosted and local search options.
