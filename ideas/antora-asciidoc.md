# Antora and AsciiDoc

## What it is

- **Format:** a static site generator for AsciiDoc docs-as-code that separates content, configuration and presentation, with content in version-controlled repositories ([Antora documentation, full page read, accessed 2026-11-26](https://docs.antora.org/antora/latest/)).
- **Scope and licence:** components and component versions map to branches or tags, with modules and families for pages, partials, examples, images and attachments; no licence statement was checked.
- **Evidence:** the Antora documentation and partials page were read directly. Fedora's use of Antora, `nav.adoc` and a playbook was seen in snippet-only sources. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Navigation is composed from lists registered per component version rather than inferred from directory layout ([Antora documentation](https://docs.antora.org/antora/latest/)).
  Tam application: an explicit navigation manifest gives predictable path order while a catalog still reaches hidden pages.
- **adopt**: Partials are single-source reusable snippets that are not published as pages and must be included by a page or another partial ([Antora partials, full page read, accessed 2026-11-26](https://docs.antora.org/antora/latest/page/partials/)).
  Tam application: repeated glossary definitions or boilerplate should live in non-page source files.
- **adopt**: An include can select only tagged regions or line ranges of a partial ([Antora partials](https://docs.antora.org/antora/latest/page/partials/)).
  Tam application: region selection avoids duplicating a whole file just to reuse one definition or example.
- **adopt**: A partial is converted after insertion, so the including page's context and attributes apply ([Antora partials](https://docs.antora.org/antora/latest/page/partials/)).
  Tam application: resolve inclusions during the corpus build so output matches the host page.
- **consider**: `page-aliases` preserve old URLs after a page moves ([Antora documentation](https://docs.antora.org/antora/latest/)).
  Tam application: an alias table can print "moved to X" in the pager instead of failing on a stale link.
- **consider**: Separating content families makes it clear which files are pages, examples and assets ([Antora documentation](https://docs.antora.org/antora/latest/)).
  Tam application: a modest directory convention can keep examples and glossary snippets out of the page catalog.
- **consider**: Reusable attributes centralise release names, URL prefixes and dates in Fedora's AsciiDoc practice ([Reusable attributes, search-result snippet, accessed 2026-11-26](https://docs.fedoraproject.org/en-US/fedora-docs/contributing/content/asciidoc-reusable-attributes/)).
  Tam application: one shared metadata file is enough; no AsciiDoc toolchain required.
- **reject**: A full Antora component and repository playbook for a single offline corpus ([Antora documentation](https://docs.antora.org/antora/latest/)).
  Tam application: take partials, nav lists and aliases, not multi-repo orchestration.
- **reject**: Cross-component version facets and sorting until multiple supported versions exist ([Antora documentation](https://docs.antora.org/antora/latest/)).
  Tam application: metadata labels are cheaper than version trees.
- **reject**: Letting a partial set attributes that override page attributes from the inclusion point onward ([Antora partials](https://docs.antora.org/antora/latest/page/partials/)).
  Tam application: keep included snippets data-only so they cannot change host-page semantics.

## Things to avoid

- Do not allow element IDs from included snippets to collide with the host page: Antora warns about this explicitly ([Antora partials](https://docs.antora.org/antora/latest/page/partials/)).
- Do not reuse a partial without checking heading levels; Antora notes that `leveloffset` may be needed ([Antora partials](https://docs.antora.org/antora/latest/page/partials/)).
- Do not claim Fedora's Antora workflow was read in full: its pages were blocked by bot protection.
- Do not adopt a UI bundle or sitemap machinery for a local reader.

## Where to look

- All URLs below were accessed 2026-11-26; snippet-only sources are marked.
- [Full page read] [Antora documentation](https://docs.antora.org/antora/latest/) — components, modules, families, nav, aliases and redirects.
- [Full page read] [Antora partials](https://docs.antora.org/antora/latest/page/partials/) — reuse, region selection, inclusion context and warnings.
- [Search-result snippet] [Fedora reusable attributes](https://docs.fedoraproject.org/en-US/fedora-docs/contributing/content/asciidoc-reusable-attributes/) — shared values in a real AsciiDoc corpus.
