# Xfce docs

## What it is

- **Format:** docs.xfce.org is a DokuWiki-style site holding per-application manuals; component source trees may also contain local DocBook or HTML help, but that per-component detail was not verified ([xfce4-terminal manual page, page-structure read, accessed 2026-11-26](https://docs.xfce.org/xfce/xfce4-terminal/start)).
- **Scope and licence:** application manuals, FAQs and community support routes, with version and language segments in URLs; no licence or page count was checked.
- **Evidence:** the wiki page layout and versioned URL shape were observed directly; the existence of an offline `xfce4-help` package and the exact "Getting help" page path are gaps or starter notes. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Version and language segments in documentation URLs let a Help-menu link target the matching installed major release without a viewer-side selector ([xfce4-terminal 4.20 manual, page read, accessed 2026-11-26](https://docs.xfce.org/xfce/xfce4-terminal/4.20/start)).
  Tam application: emit deep links that carry the supported version label, and resolve them to the installed corpus.
- **adopt**: Site-wide search plus per-application navigation keeps a multi-component corpus discoverable ([docs.xfce.org page layout, accessed 2026-11-26](https://docs.xfce.org/)).
  Tam application: one generated index over all pages plus component-level listings is enough.
- **consider**: Wiki-style category navigation helps casual browsing, but only in a lightweight form ([docs.xfce.org page layout, accessed 2026-11-26](https://docs.xfce.org/)).
  Tam application: subject hubs can provide that lateral route without importing a wiki.
- **consider**: Distinct support routes — documentation, forums and IRC — are advertised as separate destinations ([Xfce getting help, starter note about exact path, accessed 2026-11-26](https://docs.xfce.org/faq)).
  Tam application: keep a "where to ask" page separate from the reference corpus.
- **reject**: Web-only help when offline use is a requirement ([gap, accessed 2026-11-26](https://docs.xfce.org/)).
  Tam application: local pages must remain authoritative even if an HTML site exists.
- **reject**: A unified offline help browser as an assumed dependency: no primary source was found for one ([gap, accessed 2026-11-26](https://docs.xfce.org/)).
  Tam application: CLI discovery must work with no desktop help service.

## Things to avoid

- Do not assert that an `xfce4-help` binary or a version-aware Help menu exists without reading component code ([gap, accessed 2026-11-26](https://docs.xfce.org/)).
- Do not quote `xfce_dialog_show_help()` behaviour as fact: it is a per-component starter note, not primary evidence ([starter note, accessed 2026-11-26](https://docs.xfce.org/)).
- Do not claim a dedicated glossary or breadcrumb component: none was found beyond wiki path breadcrumbs.
- Do not cite the exact "Getting help" page path as verified; only its generic purpose was noted.

## Where to look

- All URLs below were accessed 2026-11-26; qualifiers reflect evidence depth.
- [Page-structure read] [Xfce documentation home](https://docs.xfce.org/) — site search and per-app navigation.
- [Page-structure read] [xfce4-terminal manual](https://docs.xfce.org/xfce/xfce4-terminal/start) — DokuWiki-style manual source.
- [Page read] [xfce4-terminal 4.20 manual](https://docs.xfce.org/xfce/xfce4-terminal/4.20/start) — versioned URL pattern.
- [Starter note] [Xfce FAQ](https://docs.xfce.org/faq) — documentation, forum and IRC support routes.
- [Gap] No primary source was found for a bundled offline help reader or version-aware Help-menu wiring.
