# LXQt docs

## What it is

- **Format:** a GitHub Pages documentation site with per-component manuals; Markdown sources in the repository are reported, but the static generator name was not read from a primary source ([LXQt documentation site, site read, accessed 2026-11-26](https://lxqt.github.io/lxqt-documentation/); [generator detail, starter note, accessed 2026-11-26](https://github.com/lxqt/lxqt-documentation)).
- **Scope and licence:** lightweight, web-first and decentralised help for Qt components, with man pages and local package documentation as the offline fallback; no licence or page count was checked.
- **Evidence:** this note is thin and heavily qualified: Help-menu wiring, generator choice, versioning and search details are `[N]` or `[G]`. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **consider**: Markdown sources in Git with a generated site lower the barrier to authoring and suit a corpus lane ([starter note, accessed 2026-11-26](https://github.com/lxqt/lxqt-documentation)).
  Tam application: keep reviewed Markdown sources and generate indexes and navigation from them.
- **consider**: Per-component documentation ownership maps naturally to separate lanes or subject areas ([site structure read, accessed 2026-11-26](https://lxqt.github.io/lxqt-documentation/)).
  Tam application: let subject owners work on disjoint paths without a whole-tree build.
- **consider**: Man pages and installed package documentation are the local fallback where no help center exists ([gap, accessed 2026-11-26](https://lxqt.github.io/lxqt-documentation/)).
  Tam application: treat local pages plus a terminal reader as the base layer, with any web site derived from them.
- **reject**: Assuming a desktop help daemon or central help center exists ([gap, accessed 2026-11-26](https://lxqt.github.io/lxqt-documentation/)).
  Tam application: command-line discovery must work without a session service.
- **reject**: Naming a specific generator as fact when the repository config was not fetched ([starter note, accessed 2026-11-26](https://github.com/lxqt/lxqt-documentation)).
  Tam application: reuse the pattern, not an unverified tool choice.

## Things to avoid

- Do not claim Qt Assistant `.qch` files are a central LXQt help mechanism: no primary evidence was found ([gap, accessed 2026-11-26](https://lxqt.github.io/lxqt-documentation/)).
- Do not assert help-menu wiring or per-component `QDesktopServices::openUrl()` behaviour without reading component code ([starter note, accessed 2026-11-26](https://lxqt.github.io/lxqt-documentation/)).
- Do not claim versioning by branch or tag as verified: that is a starter note only ([starter note, accessed 2026-11-26](https://github.com/lxqt/lxqt-documentation)).
- Do not treat site-search behaviour as source-read: it depends on an unverified generator.

## Where to look

- All URLs below were accessed 2026-11-26; qualifiers mark what was and was not verified.
- [Site read] [LXQt documentation](https://lxqt.github.io/lxqt-documentation/) — per-component manual layout.
- [Starter note] [lxqt-documentation repository](https://github.com/lxqt/lxqt-documentation) — Markdown sources and generator claim to verify.
- [Gap] No primary evidence was found for a unified offline viewer, central help service or versioning scheme.
