# Algolia DocSearch

## What it is

- **Format:** a hosted search service made of an Algolia crawler that extracts documentation into an index and frontend packages that query it, built on Algolia Autocomplete ([DocSearch overview, full page read, accessed 2026-11-26](https://docsearch.algolia.com/docs/what-is-docsearch)).
- **Licence and access:** free for eligible documentation sites subject to an application, with a requirement to keep the "Search by Algolia" attribution visible ([DocSearch overview](https://docsearch.algolia.com/docs/what-is-docsearch)).
- **Evidence:** the overview and Docusaurus search page were read directly; no hosted search service was deployed or tested. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Separating indexing from the query frontend is a clean architecture, even when both happen locally ([DocSearch overview](https://docsearch.algolia.com/docs/what-is-docsearch)).
  Tam application: keep index generation in the corpus build and keep lookup in the reader.
- **consider**: Docusaurus adds language and version facets to queries so results match the current page ([Docusaurus search, full page read, accessed 2026-11-26](https://docusaurus.io/docs/search)).
  Tam application: the facet idea is transferable to a local index; the hosted crawler is not.
- **consider**: DocSearch's default theme is described as designed for accessibility with attention to colours and contrast ([Docusaurus search](https://docusaurus.io/docs/search)).
  Tam application: set a contrast and keyboard-usability baseline for any search UI in the pager.
- **consider**: A generated sitemap lets a crawler find pages without manual submission ([Docusaurus search](https://docusaurus.io/docs/search)).
  Tam application: a generated catalog serves the same discovery role offline.
- **reject**: Hosted crawled search for an offline-first command ([DocSearch overview](https://docsearch.algolia.com/docs/what-is-docsearch)).
  Tam application: it introduces network, account, crawler schedule and index-sync dependencies.
- **reject**: Eligibility-gated or attribution-bearing services as a product dependency ([DocSearch overview](https://docsearch.algolia.com/docs/what-is-docsearch)).
  Tam application: an offline tool should not need an external approval process to keep search working.
- **reject**: Ask AI as a front end for this corpus ([Docusaurus search](https://docusaurus.io/docs/search)).
  Tam application: it depends on a hosted model service and is not supported by the evidence here for local docs.

## Things to avoid

- Do not describe DocSearch as self-hosted: it is an Algolia crawler plus frontend packages ([DocSearch overview](https://docsearch.algolia.com/docs/what-is-docsearch)).
- Do not assume crawl freshness is instant: Docusaurus documents scheduled crawls with a default weekly cadence ([Docusaurus search](https://docusaurus.io/docs/search)).
- Do not treat the accessibility statement as a full audit ([Docusaurus search](https://docusaurus.io/docs/search)).
- Do not add a visible third-party attribution requirement to a local tool without a decision.

## Where to look

- All URLs below were accessed 2026-11-26.
- [Full page read] [What is DocSearch?](https://docsearch.algolia.com/docs/what-is-docsearch) — crawler/frontend split, eligibility and attribution.
- [Full page read] [Docusaurus search](https://docusaurus.io/docs/search) — sitemap, crawl schedule, contextual facets and accessibility statement.
