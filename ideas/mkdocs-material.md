# MkDocs Material

## What it is

- **Format:** Markdown pages built by MkDocs with Python-Markdown; MkDocs core defines file-to-URL mapping, an optional `nav:` manifest, hidden pages and relative `.md` link rewriting ([MkDocs writing docs, full page read, accessed 2026-11-26](https://www.mkdocs.org/user-guide/writing-your-docs/)).
- **Scope and licence:** a static documentation site with navigation, tags, versioning, offline distribution and plugins; the search plugin builds a client-side index from rendered HTML sections. No licence statement was verified, and the fetched pages announce a successor project whose implications were not researched ([Material search plugin, full page read, accessed 2026-11-26](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
- **Evidence:** core and search plugin pages were read directly; some setup details are search-result snippets or mirrored pages and are marked. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: An explicit `nav:` gives global order while auto-discovery handles the rest, and pages absent from `nav:` are still built but hidden from global navigation and previous/next links ([MkDocs writing docs](https://www.mkdocs.org/user-guide/writing-your-docs/)).
  Tam application: a nav manifest plus hidden pages supports both curated paths and deep-reference pages.
- **adopt**: The search index is extracted from generated HTML section titles and contents, stripped of formatting to stay small, and shipped with the site for client-side Lunr indexing ([Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
  Tam application: derive one compact index from rendered pages so the reader and any HTML output share a source of truth.
- **adopt**: Tokenisation is configurable: the documented separator splits case changes, keeps version strings like `1.2.3` searchable and handles encoded HTML tags ([Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
  Tam application: tune token rules for hyphenated command names, dotted plugin IDs and version strings.
- **adopt**: Front matter `search.boost` and `search.exclude` promote canonical pages and remove scaffolding from results without a ranking service ([Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
  Tam application: authors can mark a canonical page or exclude a hub page with two small metadata fields.
- **adopt**: Header IDs are generated from lowercased heading text with disallowed characters turned into dashes and repeats collapsed ([MkDocs writing docs](https://www.mkdocs.org/user-guide/writing-your-docs/)).
  Tam application: a deterministic anchor rule lets the build validate internal links.
- **adopt**: Front matter is stripped before rendering and passed to templates, with a defined title priority from nav label through metadata to first heading and filename ([MkDocs writing docs](https://www.mkdocs.org/user-guide/writing-your-docs/)).
  Tam application: one metadata block can drive title, navigation label and search behaviour.
- **consider**: Search setup documents suggestion, highlight and share-query options, plus a `data-search-exclude` pragma for per-block exclusion ([Material setup page, search-result snippet, accessed 2026-11-26](https://squidfunk.github.io/mkdocs-material/setup/setting-up-site-search)).
  Tam application: only the shareable query idea is useful offline; verify the rest against current docs first.
- **consider**: An offline plugin can distribute the built site as a ZIP, and Material's search works with it ([Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
  Tam application: packaged documentation should need no network at read time.
- **reject**: Hosted or server-side search when the corpus is read offline ([Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
  Tam application: build and ship the index locally.
- **reject**: Multilingual stemmer and segmentation payloads without a demonstrated need: the docs cite about 20 KB base JS plus 15–30 KB per language ([Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/)).
  Tam application: a single-language corpus should not carry that machinery.
- **reject**: Raw HTML links as the only way to reach a page: MkDocs cannot validate or convert links inside raw HTML ([MkDocs writing docs](https://www.mkdocs.org/user-guide/writing-your-docs/)).
  Tam application: author links in the supported Markdown form so the build can check them.

## Things to avoid

- Do not validate only HTTP links: derived relative links and generated anchors are where breakage usually starts ([MkDocs writing docs](https://www.mkdocs.org/user-guide/writing-your-docs/)).
- Do not assume client-side search works over a plain local file URL: a mirrored setup page says workers and requests are blocked there ([mirrored setup page, search-result snippet, accessed 2026-11-26](https://henrywhitaker3.github.io/mkdocs-material-dark-theme/plugins/search)).
- Do not claim synonym maps or explicit fuzzy configuration exist: none was found in the pages read.
- Do not cite mirrored query-convention pages as canonical Material documentation ([mirrored setup page, search-result snippet, accessed 2026-11-26](https://t.pxeger.com/mkdocs-material/setup/setting-up-site-search)).

## Where to look

- All URLs below were accessed 2026-11-26; snippet-only sources are marked.
- [Full page read] [MkDocs: Writing your docs](https://www.mkdocs.org/user-guide/writing-your-docs/) — Markdown layout, nav, hidden pages, links, anchors and front matter.
- [Full page read] [Material search plugin](https://squidfunk.github.io/mkdocs-material/plugins/search/) — generated index, tokenisation, boost/exclude, languages and offline plugin.
- [Search-result snippet] [Material site search setup](https://squidfunk.github.io/mkdocs-material/setup/setting-up-site-search) — suggest, highlight, share and exclusion pragma.
- [Search-result snippet] [Offline-worker caveat](https://henrywhitaker3.github.io/mkdocs-material-dark-theme/plugins/search) and [query convention mirror](https://t.pxeger.com/mkdocs-material/setup/setting-up-site-search).
- [Gap] The pages fetched announce a successor project; its implications were not researched.
