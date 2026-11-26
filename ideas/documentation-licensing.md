# Documentation licensing and attribution

## What it is

- **Format:** a cross-project policy topic rather than a single system, drawn from wiki footers, Debian policy and licence texts: ArchWiki content is GFDL 1.3-or-later, KDE TechBase content is CC BY-SA 4.0, and Debian manuals must use DFSG-compliant licences ([ArchWiki Help:Style, full page read, accessed 2026-11-26](https://wiki.archlinux.org/title/Help:Style); [KDE TechBase, full page read, accessed 2026-11-26](https://techbase.kde.org/Projects/Documentation); [DDP, full page read, accessed 2026-11-26](https://www.debian.org/doc/ddp.en.html)).
- **Scope and size:** licence terms, attribution conditions, adaptation obligations and the boundary between reusing an idea and copying expressive text; no page-count or local licence statement was verified.
- **Evidence:** the cited pages were read directly, except the GNU FDL FAQ which timed out; GFDL facts come from SPDX's licence text mirror and Debian's formal resolution. Verdicts are research inferences, not approved work, and this is not legal advice.

## Ideas worth taking

- **adopt**: Describe mechanisms in original words and cite the source URL, rather than importing chunks of upstream prose ([Creative Commons FAQ, full page read, accessed 2026-11-26](https://creativecommons.org/faq/)).
  Tam application: these idea files summarise patterns; no upstream sentences were copied into the corpus.
- **adopt**: Treat ideas, structures and workflows as distinct from expressive text when deciding whether attribution or share-alike applies ([CC FAQ](https://creativecommons.org/faq/)).
  Tam application: a status-template concept can be reimplemented without inheriting GFDL or CC BY-SA obligations.
- **adopt**: Check a source's licence before vendoring any documentation text, including translations and template wording ([SPDX GFDL-1.3, full page read, accessed 2026-11-26](https://spdx.org/licenses/GFDL-1.3); [CC FAQ](https://creativecommons.org/faq/)).
  Tam application: record the decision in notes rather than assuming "publicly visible" means reusable.
- **consider**: A small documented attribution policy would keep research notes and published pages separate ([CC FAQ](https://creativecommons.org/faq/)).
  Tam application: keep source links in research and session records while the public corpus keeps its existing sourcing boundary.
- **consider**: Debian's distinction between `GFDL` and `GFDL-NIV` is a useful checklist for evaluating a candidate source ([Debian copyright-format 1.0, full page read, accessed 2026-11-26](https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/)).
  Tam application: check for cover texts, invariant sections and DRM clauses before accepting GFDL material.
- **reject**: Importing GFDL invariant sections: Debian's 2006 resolution found works without them free under DFSG, with invariant sections among the objections ([Debian GR 2006 vote 001, full page read, accessed 2026-11-26](https://www.debian.org/vote/2006/vote_001)).
  Tam application: never carry unmodifiable text into a corpus.
- **reject**: Copying CC BY-SA or GFDL prose without attribution and adaptation notices ([CC FAQ](https://creativecommons.org/faq/); [SPDX GFDL-1.3](https://spdx.org/licenses/GFDL-1.3)).
  Tam application: original summaries are the safe default for this study.
- **reject**: Claiming this study changes upstream attribution files: no code or text was taken ([Gap, accessed 2026-11-26](https://wiki.archlinux.org/title/Help:Style)).
  Tam application: no upstream-file changes are part of this work.

## Things to avoid

- Do not imply endorsement when attributing: CC guidance warns against it ([CC FAQ](https://creativecommons.org/faq/)).
- Do not treat the CC FAQ as legal advice: CC itself says it is informational ([CC FAQ](https://creativecommons.org/faq/)).
- Do not assume the bare `GFDL-1.3` SPDX identifier is current: SPDX marks it deprecated since licence-list version 3.0 ([SPDX](https://spdx.org/licenses/GFDL-1.3)).
- Do not claim the local corpus licence was inspected; no claim is made here about it.
- Do not sign or credit individual page authors upstream where the project's style rules say page history is the credit ([ArchWiki Help:Style](https://wiki.archlinux.org/title/Help:Style)).

## Where to look

- All URLs below were accessed 2026-11-26; the GNU FDL FAQ fetch timed out.
- [Full page read] [ArchWiki Help:Style](https://wiki.archlinux.org/title/Help:Style) — GFDL footer and credit conventions.
- [Full page read] [KDE TechBase: Projects/Documentation](https://techbase.kde.org/Projects/Documentation) — CC BY-SA 4.0 footer.
- [Full page read] [Debian Documentation Project](https://www.debian.org/doc/ddp.en.html) — DFSG requirement.
- [Full page read] [Debian GR 2006 vote 001](https://www.debian.org/vote/2006/vote_001) — invariant sections and GFDL.
- [Full page read] [SPDX GFDL-1.3](https://spdx.org/licenses/GFDL-1.3) — modification, history, translation and invariant-section terms.
- [Full page read] [Creative Commons FAQ](https://creativecommons.org/faq/) — attribution, ShareAlike, version 4.0 and plagiarism distinction.
- [Full page read] [Debian copyright-format 1.0](https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/) — GFDL versus GFDL-NIV.
- [Gap] The GNU FDL FAQ page timed out; GFDL details here use SPDX and Debian sources.
