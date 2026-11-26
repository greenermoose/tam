# openSUSE docs

## What it is

- **Format:** official manuals are DocBook 5 built with DAPS and SUSE XSLT stylesheets; the `doc.opensuse.org` repository itself is only a Jekyll navigation shell ([openSUSE/doc-o-o README, source file read, accessed 2026-11-26](https://raw.githubusercontent.com/openSUSE/doc-o-o/main/README.md); [SUSE/doc-sle repository, search-result snippet, accessed 2026-11-26](https://github.com/suse/doc-sle)).
- **Scope and licence:** manuals are shared with SUSE Linux Enterprise and assembled with release notes under one site; no article count or licence statement was verified in this study.
- **Evidence:** one repository README was read directly; published-site fetches failed with transport errors and several details rest on search-result snippets, marked below; verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: A small navigation site can aggregate several content repositories without rewriting each manual ([openSUSE/doc-o-o README, source file read, accessed 2026-11-26](https://raw.githubusercontent.com/openSUSE/doc-o-o/main/README.md)).
  Tam application: keep the generated index and navigation separate from page sources, but do not split them across repositories unless there is a real second product.
- **adopt**: Contributors validate with an explicit `daps -d DC-<YOUR_BOOK> validate` step plus HTML and PDF builds ([SUSE/doc-sle repository, search-result snippet, accessed 2026-11-26](https://github.com/suse/doc-sle)).
  Tam application: make an exact local validation command part of authoring, with render and lint as distinct checks.
- **adopt**: The site repository uses an ordinary `_config.yml`, `make build` and `make serve`, and a script that checks out a chosen branch before building manuals ([openSUSE/doc-o-o README, source file read, accessed 2026-11-26](https://raw.githubusercontent.com/openSUSE/doc-o-o/main/README.md)).
  Tam application: prefer plain build entry points over a bespoke publishing service.
- **consider**: Conditional text lets openSUSE and SLES manuals come from the same source ([openSUSE:Documentation Contribute, search-result snippet, accessed 2026-11-26](https://en.opensuse.org/openSUSE:Documentation_Contribute)).
  Tam application: only worthwhile if two genuinely close variants exist; otherwise label a single page.
- **consider**: DAPS supports DocBook 4.x and 5.x, several output formats and automatic profiling ([DAPS Conceptual Overview, search-result snippet, accessed 2026-11-26](https://opensuse.github.io/daps/doc/cha.daps.user.features.html)).
  Tam application: multi-output generation is the transferable idea, not DocBook itself.
- **reject**: Full DocBook/DAPS publishing for a single-product local corpus ([SUSE/doc-sle repository, search-result snippet, accessed 2026-11-26](https://github.com/suse/doc-sle)).
  Tam application: the validation and separation ideas are cheap; the toolchain is not.
- **reject**: Multi-product conditional authoring when the variants are not close enough to share almost all text ([openSUSE:Documentation Contribute, search-result snippet, accessed 2026-11-26](https://en.opensuse.org/openSUSE:Documentation_Contribute)).
  Tam application: use metadata labels and links instead of profiling attributes.

## Things to avoid

- Do not present snippet-only claims as full reads: the SUSE repository README path returned 404 on both `main` and `master`, so repository evidence came from search results.
- Do not rely on published-site details from the access date: both `documentation.opensuse.org` and its about page failed with transport errors.
- Do not adopt XSLT 1.0 stylesheet branding as a design goal: it exists to produce HTML, PDF and EPUB from DocBook ([SUSE Package Hub: suse-xsl-stylesheets, search-result snippet, accessed 2026-11-26](https://packagehub.suse.com/packages/suse-xsl-stylesheets)).
- Do not claim the Tumbleweed volunteer guide or the openSUSE style guide was reviewed; neither was read.

## Where to look

- All URLs below were accessed 2026-11-26; labels show evidence depth.
- [Source file read] [openSUSE/doc-o-o README](https://raw.githubusercontent.com/openSUSE/doc-o-o/main/README.md) — Jekyll shell, build commands and branch-aware script.
- [Search-result snippet] [SUSE/doc-sle](https://github.com/suse/doc-sle) — DocBook 5, DAPS validation and output builds.
- [Search-result snippet] [openSUSE:Documentation Contribute](https://en.opensuse.org/openSUSE:Documentation_Contribute) — shared source with conditional text.
- [Search-result snippet] [DAPS Conceptual Overview](https://opensuse.github.io/daps/doc/cha.daps.user.features.html) — formats and profiling.
- [Search-result snippet] [SUSE Package Hub: suse-xsl-stylesheets](https://packagehub.suse.com/packages/suse-xsl-stylesheets) — HTML, PDF and EPUB styling.
- [Gap] `https://documentation.opensuse.org/` and `https://documentation.opensuse.org/about/` failed to fetch.
