# Gentoo docs

## What it is

- **Format:** a collaborative MediaWiki plus a protected, architecture-specific Handbook; the old `gentoo.org/doc/en/handbook/` URL redirects to the wiki Handbook main page ([Gentoo Handbook, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Handbook:Main_Page)).
- **Scope and licence:** installation handbook, broad reference wiki, guidelines and translation help; no current article count was collected, and the licence was not checked in this study.
- **Evidence:** full page reads of the Handbook main page, Gentoo Wiki:Guidelines, Help:Translating and the contributor guide; verdicts below are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Typed templates such as `{{Warning}}`, `{{Cmd}}`, `{{RootCmd}}`, `{{Emerge}}`, `{{FileBox}}` and `{{KernelBox}}` make risk, privilege and configuration context scannable without depending on each author's prose style ([Gentoo Wiki:Guidelines, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Gentoo_Wiki:Guidelines)).
  Tam application: define a small callout and command-block set in the authoring format, including an explicit root-required marker.
- **adopt**: The Handbook is protected while the wider wiki stays editable, separating a stable install path from living reference material ([Handbook:Main_Page, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Handbook:Main_Page)).
  Tam application: keep high-risk path pages under a tighter review convention than ordinary topic pages.
- **adopt**: The Handbook recommends section-by-section reading for installation work and a single-page view for printing or searching ([Handbook:Main_Page, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Handbook:Main_Page)).
  Tam application: let the same page source render as paged steps and as one continuous view without duplicating text.
- **adopt**: Semantic MediaWiki properties such as `[[Article description::...]]` plus `{{See also}}` give article roles and relationships explicit machine-visible form ([Gentoo Wiki:Guidelines, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Gentoo_Wiki:Guidelines)).
  Tam application: a one-line description field per page can feed both listings and a search index.
- **consider**: Translation uses the MediaWiki Translate extension, `<!--T:15-->` markers, a translation-aware `{{Link}}` template and ISO 8601 dates ([Help:Translating, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Help:Translating)).
  Tam application: if localisation is ever approved, track source units and link targets rather than treating translated pages as independent copies.
- **consider**: A separate contributor guide gives new wiki authors an onboarding route distinct from the style rules ([Gentoo Wiki:Contributor's guide, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Gentoo_Wiki:Contributor%27s_guide)).
  Tam application: keep corpus-authoring instructions separate from a short "how to propose a change" page.
- **reject**: The full Semantic MediaWiki and template stack is not proportionate to a compact offline corpus ([Gentoo Wiki:Guidelines, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Gentoo_Wiki:Guidelines)).
  Tam application: take the typed-fields idea, not the wiki extension machinery.
- **reject**: Architecture-split handbook variants should not be copied without a real multi-architecture need ([Handbook:Main_Page, full page read, accessed 2026-11-26](https://wiki.gentoo.org/wiki/Handbook:Main_Page)).
  Tam application: label version- or environment-specific material instead of forking whole page trees.

## Things to avoid

- Do not make root commands visually identical to ordinary examples: the `{{RootCmd}}` convention exists because the privilege difference matters ([Gentoo Wiki:Guidelines](https://wiki.gentoo.org/wiki/Gentoo_Wiki:Guidelines)).
- Do not depend on translated link templates or unit markers unless the whole translation pipeline is adopted ([Help:Translating](https://wiki.gentoo.org/wiki/Help:Translating)).
- Do not cite `Gentoo Wiki:Style_guide` as evidence: it returned 404 on the access date, so style evidence came from Gentoo Wiki:Guidelines.
- Do not claim per-architecture content drift or translation-queue state was reviewed; neither was checked in this study.

## Where to look

- All URLs below were accessed 2026-11-26.
- [Full page read] [Handbook:Main_Page](https://wiki.gentoo.org/wiki/Handbook:Main_Page) — protected architecture-specific handbook and reading/view modes.
- [Full page read] [Gentoo Wiki:Guidelines](https://wiki.gentoo.org/wiki/Gentoo_Wiki:Guidelines) — semantic properties, article roles and typed templates.
- [Full page read] [Help:Translating](https://wiki.gentoo.org/wiki/Help:Translating) — translate extension, unit markers and link template.
- [Full page read] [Gentoo Wiki:Contributor's guide](https://wiki.gentoo.org/wiki/Gentoo_Wiki:Contributor%27s_guide) — contributor onboarding.
- [Gap] `https://wiki.gentoo.org/wiki/Gentoo_Wiki:Style_guide` returned 404; use Guidelines instead.
