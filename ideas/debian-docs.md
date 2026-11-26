# Debian docs

## What it is

- **Format:** the Debian Documentation Project maintains manuals in reStructuredText or DocBook XML, with DocBook described as deprecated and sources hosted in Git; the community wiki uses MoinMoin ([Debian Documentation Project, full page read, accessed 2026-11-26](https://www.debian.org/doc/ddp); [DebianWiki, search-result snippet, accessed 2026-11-26](https://wiki.debian.org/DebianWiki)).
- **Licence and scope:** manual licences must comply with the Debian Free Software Guidelines, and every document "should be actively maintained"; the portal sorts manuals, HOWTOs, FAQs, short documents and links to manpages, info files and README files ([DDP, full page read, accessed 2026-11-26](https://www.debian.org/doc/ddp); [Documentation portal, full page read, accessed 2026-11-26](https://www.debian.org/doc/index.en.html)).
- **Evidence:** several wiki-page fetches were challenged by an "I Challenge Thee" page, so wiki-specific claims below are marked search-result snippet; all verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Debian separates curated release-quality manuals from a freely editable operational wiki, matching two different jobs rather than forcing one format on both ([Debian Documentation Project, full page read, accessed 2026-11-26](https://www.debian.org/doc/ddp); [DebianWiki, search-result snippet, accessed 2026-11-26](https://wiki.debian.org/DebianWiki)).
  Tam application: keep the reviewed corpus as the product surface and treat community contributions as a separate intake, not a second wiki.
- **adopt**: Requiring every document to be actively maintained makes "exists" and "is current" separate states ([Debian Documentation Project, full page read, accessed 2026-11-26](https://www.debian.org/doc/ddp)).
  Tam application: add a per-page maintenance state to the generated index with a reason, instead of assuming presence implies freshness.
- **adopt**: DDP sources are rebuilt from Git roughly every four hours with public build logs ([DDP VCS, full page read, accessed 2026-11-26](https://www.debian.org/doc/vcs)).
  Tam application: keep generated artefacts reproducible from source and let build output be inspectable.
- **consider**: Debian release notes are explicitly version-bound and document upgrades only from the previous release ([Release notes, full page read, accessed 2026-11-26](https://www.debian.org/releases/stable/release-notes/about.en.html)).
  Tam application: isolate compatibility facts in a labelled page rather than mixing release numbers into evergreen instructions.
- **consider**: `doc-base` registers non-manual online documentation at install and removal time so tools can find it ([Debian wiki: doc-base, search-result snippet, accessed 2026-11-26](https://wiki.debian.org/doc-base)).
  Tam application: a single generated catalog is a better fit than directory scans for skills and paths shipped beside pages.
- **consider**: Write access follows useful patches and team approval while sources stay readable and cloneable by everyone ([DDP VCS, full page read, accessed 2026-11-26](https://www.debian.org/doc/vcs)).
  Tam application: a low-friction patch route with a small committer group is a workable review model if contributor numbers stay small.
- **reject**: MoinMoin-style free editing as the primary corpus depends on wiki-specific categories and syntax ([DebianWiki, search-result snippet, accessed 2026-11-26](https://wiki.debian.org/DebianWiki); [EditorQuickStart, search-result snippet, accessed 2026-11-26](https://wiki.debian.org/DebianWiki/EditorQuickStart)).
  Tam application: keep review in Git rather than adopting a forum-era wiki engine.
- **reject**: DocBook for new manual sources is explicitly deprecated by DDP ([Debian Documentation Project, full page read, accessed 2026-11-26](https://www.debian.org/doc/ddp)).
  Tam application: do not introduce a DocBook toolchain for a Markdown-style corpus.
- **reject**: GFDL works with invariant sections are incompatible with the project's documentation judgement ([Debian GR 2006 vote 001, full page read, accessed 2026-11-26](https://www.debian.org/vote/2006/vote_001)).
  Tam application: ideas can be reused in original wording, but invariant text must never be imported.

## Things to avoid

- Do not treat wiki details as settled facts: account creation, MoinMoin syntax, language namespaces and the `#language` instruction were read only as search-result snippets ([EditorQuickStart](https://wiki.debian.org/DebianWiki/EditorQuickStart); [TranslationCoordination](https://wiki.debian.org/DebianWiki/TranslationCoordination); [HelpOnProcessingInstructions](https://wiki.debian.org/HelpOnProcessingInstructions)).
- Do not assume a current wiki category, page or translation count exists; none was collected.
- Do not let a manual exist without a maintenance owner: DDP's rule exists because unmaintained documentation misleads readers ([DDP](https://www.debian.org/doc/ddp)).
- Do not quote release-note wording without noting that only the previous-release upgrade path is documented ([Release notes](https://www.debian.org/releases/stable/release-notes/about.en.html)).

## Where to look

- All URLs below were accessed 2026-11-26; snippet-only sources are marked.
- [Full page read] [Debian Documentation portal](https://www.debian.org/doc/index.en.html) — document-type taxonomy and links to local manuals.
- [Full page read] [Debian Documentation Project](https://www.debian.org/doc/ddp) — accepted formats, Git sources, licence and maintenance rule.
- [Full page read] [DDP VCS](https://www.debian.org/doc/vcs) — access model, rebuild cadence and public logs.
- [Full page read] [Debian release notes](https://www.debian.org/releases/stable/release-notes/about.en.html) — version-bound documentation and bug reports.
- [Full page read] [Debian GR 2006 vote 001](https://www.debian.org/vote/2006/vote_001) — invariant sections and DFSG.
- [Search-result snippet] [DebianWiki](https://wiki.debian.org/DebianWiki), [EditorQuickStart](https://wiki.debian.org/DebianWiki/EditorQuickStart), [TranslationCoordination](https://wiki.debian.org/DebianWiki/TranslationCoordination) and [HelpOnProcessingInstructions](https://wiki.debian.org/HelpOnProcessingInstructions).
- [Search-result snippet] [doc-base](https://wiki.debian.org/doc-base) — registration of non-manual documentation.
