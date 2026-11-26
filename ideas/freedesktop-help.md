# freedesktop.org help conventions

## What it is

- **Format:** two related specifications rather than one product: AppStream component metadata with a `<url type="help">` web link, and a draft `help:` URI scheme with a document/page/anchor grammar ([AppStream metadata spec, full page read, accessed 2026-11-26](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html); [help URI draft, full page read, accessed 2026-11-26](https://www.freedesktop.org/wiki/Specifications/help-uri/)).
- **Scope and licence:** cross-desktop discovery metadata and help-document resolution; the help-uri page is explicitly a draft or proposal, not a mandatory standard. Licence was not checked in this study.
- **Evidence:** both specifications were read directly; a search for an `org.freedesktop.help` bus service found no primary source. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: The `help:<document>/<page>?<options>#<anchor>` grammar gives short, deep-linkable identifiers for a document, a page and a section ([help URI draft, full page read, accessed 2026-11-26](https://www.freedesktop.org/wiki/Specifications/help-uri/)).
  Tam application: model topic deep links as `topic`, optional subsection and anchor, resolved by the local catalog.
- **adopt**: Help documents are laid out in XDG data directories by language and document name, with locale fallback rules ([help URI draft, full page read, accessed 2026-11-26](https://www.freedesktop.org/wiki/Specifications/help-uri/)).
  Tam application: if localisation is approved, follow a language-coded directory layout rather than inventing a naming scheme.
- **adopt**: AppStream declares a distinct URL type for user documentation, separate from homepage, bugtracker, donate and translate links ([AppStream metadata spec, full page read, accessed 2026-11-26](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html)).
  Tam application: keep "documentation" metadata distinct from project and support links in any generated manifest.
- **consider**: AppStream `<provides>` records typed capabilities such as DBus name, binary, library and mediatype ([AppStream metadata spec, full page read, accessed 2026-11-26](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html)).
  Tam application: a small typed capability record could let external front-ends locate command help without embedding prose.
- **consider**: The help-uri draft names Yelp and KHelpCenter as example handlers, showing that one URI grammar can serve more than one viewer ([help URI draft, full page read, accessed 2026-11-26](https://www.freedesktop.org/wiki/Specifications/help-uri/)).
  Tam application: keep deep links viewer-neutral so both terminal and HTML output can resolve them.
- **reject**: Inventing an `org.freedesktop.help` D-Bus service as if it were an existing standard ([gap, accessed 2026-11-26](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html)).
  Tam application: if a service is ever needed, design it explicitly and do not claim freedesktop endorsement.

## Things to avoid

- Do not describe the help URI scheme as widely mandatory: the source calls it a draft or proposal ([help URI draft](https://www.freedesktop.org/wiki/Specifications/help-uri/)).
- Do not treat AppStream's help URL as a local document ID: it is defined as a web URL ([AppStream metadata spec](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html)).
- Do not assert that an `org.freedesktop.help` bus name or registry exists; no primary source was found ([gap](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html)).
- Do not copy the full XDG resolution algorithm into a small corpus until a multi-locale need is real.

## Where to look

- All URLs below were accessed 2026-11-26.
- [Full page read] [AppStream metadata specification](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html) — URL types and `<provides>` capability metadata.
- [Full page read] [freedesktop help URI draft](https://www.freedesktop.org/wiki/Specifications/help-uri/) — URI grammar, XDG document layout, locale fallback and example handlers.
- [Gap] Searches of the specifications index, AppStream spec and help-uri page found no `org.freedesktop.help` service.
