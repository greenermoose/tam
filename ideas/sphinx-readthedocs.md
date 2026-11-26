# Sphinx and Read the Docs

## What it is

- **Format:** Sphinx uses reStructuredText with roles and directives; reST uses blank-line-separated paragraphs, significant indentation and inline emphasis, strong and literal markup that cannot nest ([Sphinx reST basics, full page read, accessed 2026-11-26](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)).
- **Scope and licence:** a documentation builder plus a hosting service that defines `latest` from the default branch and `stable` from the highest tag; no licence statement was collected ([Read the Docs versions, search-result snippet, accessed 2026-11-26](https://readthedocs-lst.readthedocs.org/en/latest/versions.html)).
- **Evidence:** the Sphinx basics page was read directly; Read the Docs details are search-result snippets, and linked Sphinx sections on toctrees, builders and the generated search index were not fetched. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Reuse mechanisms are explicit: an `include` directive, substitutions defined with `replace::` or `image::`, and site-wide values in prolog or epilog files ([Sphinx reST basics](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)).
  Tam application: resolve shared glossary definitions during the corpus build rather than at read time.
- **adopt**: A reused file can carry a non-page extension so the builder does not publish it as a standalone page ([Sphinx reST basics](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)).
  Tam application: keep snippets and partials visibly distinct from real pages in the source tree.
- **adopt**: `.. meta::` keywords are added to the search index, with language-qualified variants for specific indices and unqualified keywords for all languages ([Sphinx reST basics](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)).
  Tam application: let authors supply a few search keywords in front matter without hand-editing an index.
- **adopt**: A local `.. contents::` directive and documented admonition, code-block, table and doctest directives give consistent page structure ([Sphinx reST basics](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)).
  Tam application: a small set of typed blocks is enough; a full directive system is not needed.
- **consider**: `latest` and `stable` are separate version concepts tied to branch and highest tag ([Read the Docs versions, search-result snippet, accessed 2026-11-26](https://readthedocs-lst.readthedocs.org/en/latest/versions.html)).
  Tam application: only meaningful if a stable snapshot is actually published alongside development pages.
- **consider**: User-defined redirects apply across all versions and translations of a documentation set ([Read the Docs redirects, search-result snippet, accessed 2026-11-26](https://docs.readthedocs.com/platform/stable/user-defined-redirects.html)).
  Tam application: persist old page IDs in an alias map so moved pages resolve in the pager.
- **consider**: Version URL slugs are generated from branch or tag names and can be customised with documented limitations ([Read the Docs slugs, search-result snippet, accessed 2026-11-26](https://about.readthedocs.com/blog/2025/05/customize-versions-urls)).
  Tam application: avoid exposing raw branch names in user-facing deep links.
- **reject**: Sphinx and Read the Docs as the corpus toolchain: the benefits here are authoring patterns, not hosting ([Sphinx reST basics](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)).
  Tam application: the existing Python build can own both pages and index.
- **reject**: Treating builder-specific details as facts: `toctree`, `searchindex.js`, the linkcheck builder and code-sample execution were not source-read ([Sphinx documentation index, full page read, accessed 2026-11-26](https://www.sphinx-doc.org/en/master/usage/index.html)).
  Tam application: re-check those pages before borrowing any specific mechanism.

## Things to avoid

- Do not copy reST nesting restrictions into a different Markdown-based format without need ([Sphinx reST basics](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)).
- Do not present Read the Docs search, validation or translation UI behaviour as researched; no source was read for them.
- Do not assume generated search index internals from hosting-page snippets.
- Do not create a second hand-maintained keyword file alongside the page source.

## Where to look

- All URLs below were accessed 2026-11-26; snippet-only sources are marked.
- [Full page read] [Sphinx reStructuredText basics](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html) — markup, roles, directives, include, substitutions, admonitions and meta keywords.
- [Full page read] [Sphinx documentation index](https://www.sphinx-doc.org/en/master/usage/index.html) — areas to read next; linked pages were not fetched.
- [Search-result snippet] [Read the Docs versions](https://readthedocs-lst.readthedocs.org/en/latest/versions.html) — `latest` and `stable`.
- [Search-result snippet] [Read the Docs user-defined redirects](https://docs.readthedocs.com/platform/stable/user-defined-redirects.html) — cross-version redirects.
- [Search-result snippet] [Customizing version URLs](https://about.readthedocs.com/blog/2025/05/customize-versions-urls) — slugs and limitations.
- [Gap] Sphinx `toctree`, `searchindex.js`, linkcheck and code-sample execution were not source-read.
