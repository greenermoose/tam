# Lunr

## What it is

- **Format:** a small, dependency-free full-text search engine usable in a browser or Node.js, designed to avoid a server-side search service and extensible through language processors ([Lunr homepage, full page read, accessed 2026-11-26](https://lunrjs.com/)).
- **Licence and scope:** client-side indexing of a shipped document index; no licence statement or size measurement was collected in this study.
- **Evidence:** this is a thin note: only the Lunr homepage was read directly, and no page was obtained for fuzzy operators or synonym support. Material and Docsy details below are marked. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Shipping an index with the documents and building it client-side keeps search in step with the built docs and needs no server ([Material search plugin, full page read, accessed 2026-11-26](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
  Tam application: generate one index alongside rendered pages and read it from the local corpus.
- **adopt**: A dependency-free engine avoids adding a runtime service to a documentation tool ([Lunr homepage](https://lunrjs.com/)).
  Tam application: if a JavaScript HTML reader exists, a local index is compatible; the terminal reader can use the same data.
- **consider**: Material configures stemming, stop words, token trimming and case-change splitting around Lunr rather than replacing the engine ([Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
  Tam application: search quality comes mostly from token rules and metadata, not engine size.
- **consider**: Docsy's offline option generates a local JSON index ([Docsy search, search-result snippet, accessed 2026-11-26](https://www.docsy.dev/docs/content/search)).
  Tam application: a derived JSON index shared by renderers is a reasonable interchange format.
- **reject**: Assuming fuzzy operator or synonym support from the homepage alone ([gap, accessed 2026-11-26](https://lunrjs.com/)).
  Tam application: do not promise fuzzy matching until a source confirms it.
- **reject**: A server-side or hosted search backend for an offline corpus ([Lunr homepage](https://lunrjs.com/)).
  Tam application: local indexing is the compatible model.

## Things to avoid

- Do not claim Lunr's fuzzy syntax or synonym behaviour is documented here; it is not.
- Do not assume browser workers work over a plain local file URL: a mirrored Material page says they are blocked there ([mirrored page, search-result snippet, accessed 2026-11-26](https://henrywhitaker3.github.io/mkdocs-material-dark-theme/plugins/search)).
- Do not treat Docsy's index format as source-read.
- Do not add language stemmer payloads without a demonstrated need ([Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/)).

## Where to look

- URLs below were accessed 2026-11-26; labels show evidence depth.
- [Full page read] [Lunr](https://lunrjs.com/) — purpose, dependency-free design and language processors.
- [Full page read] [Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/) — shipped index, tokenisation and language costs.
- [Search-result snippet] [Docsy search](https://www.docsy.dev/docs/content/search) — generated offline JSON index.
- [Gap] No source-read page was obtained for fuzzy operators or synonyms.
