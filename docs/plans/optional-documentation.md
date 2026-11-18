# Optional host documentation and native identity

- **Plan status:** Source complete — command and corpus checks pass; packaged acceptance pending.
- **Reviewed:** 2026-11-16
- **Scope:** Command 0.61.0 and corpus 0.95.0.
- **Authority:** Developer approved the design and requested implementation.
- **Next action:** Select committed sources during explicit candidate preparation.

Host documentation is independent of the guide: Info under `/usr/share/doc/info`,
curated tldr under `/usr/share/doc/tldr`, and Arch Wiki under
`/usr/share/doc/arch-wiki/html/en`. Explicit `INFOPATH`, `TAM_TLDR_DIR` and
`TAM_ARCHWIKI_DIR` selections remain supported; `TAM_DOC_DIR` selects an isolated
documentation root for tests and previews. Empty collections are unavailable.

Corpus links use `data-doc-source="info|tldr|archwiki"`; a missing source removes
only the anchor, retaining its text. Spans with `data-doc-missing` use the same
identifiers and appear only while absent. Labels, descriptions and fallback
wording belong to HTML. The corpus loader evaluates this before folding and
level selection; installed HTML remains unchanged. The authoring checker rejects
unknown identifiers and attributes on inappropriate elements.

`BUNDLE_ROOT` explicitly selects native identity at build time. It reads the
format-2 BUNDLE and packaged cultivar; stale user/environment identity cannot
replace invalid native metadata. With an empty setting, a relocatable
bundle uses its own manifest. A development build otherwise reads the
active bundle, then uses legacy per-user metadata when no valid bundle
manifest is available.

Tests cover all eight availability combinations, empty/index-only directories,
reopening after collection changes, and native identity with stale or invalid
metadata. Physical package acceptance is separate.
