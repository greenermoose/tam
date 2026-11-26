# Fedora docs

## What it is

- **Format:** docs-as-code using AsciiDoc in Git repositories assembled by Antora into one site ([Contribute to Fedora Documentation, search-result snippet, accessed 2026-11-26](https://docs.fedoraproject.org/ur/fedora-docs/contributing-docs/)).
- **Scope and licence:** user docs, Quick Docs, contributor guides and project start pages spread across roughly 60 Git repositories; a 4-eyes policy requires a second reviewer before merge ([Fedora Docs contributing, search-result snippet, accessed 2026-11-26](https://docs.fedoraproject.org/be/fedora-docs/contributing/); [Fedora Docs contributing, search-result snippet, accessed 2026-11-26](https://discussion.fedoraproject.org/t/discourse-bots-docs-team-meta-discussion-process-governance-outreach/97401/17)). No licence statement was checked in this study.
- **Evidence:** direct fetches were blocked by bot protection, so most page details are search-result snippets or search-engine extraction and are marked as such; verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Getting Started factors near-identical preparation into shared pages and branches into per-variant installation steps only where Anaconda details differ ([Getting Started, search extraction, accessed 2026-11-26](https://docs.fedoraproject.org/en-US/fedora-user-docs/getting-started/)).
  Tam application: share preparation checkpoints across paths and branch at the first genuinely different command.
- **adopt**: The Start hub separates contributor guides from user guides and says explicitly that "Common Issues are not guides" ([Fedora Start, search extraction, accessed 2026-11-26](https://fedoraproject.org/start)).
  Tam application: classify pages by learner intent before topic, and keep troubleshooting out of the path list.
- **adopt**: Local editing with an exact preview is the normal route; web editing is recommended only for small edits because it does not compile images and cross-references correctly ([Contribute to Fedora Documentation, search-result snippet, accessed 2026-11-26](https://docs.fedoraproject.org/ur/fedora-docs/contributing-docs/)).
  Tam application: make a local render or preview step part of the authoring contract for anything with links or generated indexes.
- **adopt**: A documented QA checklist covers broken links, procedure correctness, licensing, spelling and whether potential problems carry admonitions ([Docs QA Review Guidelines, full page read, accessed 2026-11-26](https://fedoraproject.org/wiki/Docs_QA_Review_Guidelines)).
  Tam application: turn those checks into a repeatable corpus review checklist before a path is accepted.
- **consider**: Reusable attributes in a partials file carry release names, URL prefixes and dates, imported with an include directive ([Reusable attributes, search-result snippet, accessed 2026-11-26](https://docs.fedoraproject.org/en-US/fedora-docs/contributing/content/asciidoc-reusable-attributes/)).
  Tam application: one small shared metadata file beats repeating version strings in every page.
- **consider**: A second reviewer is required for each contribution under the 4-eyes principle ([Fedora Discussion, 2023, accessed 2026-11-26](https://discussion.fedoraproject.org/t/discourse-bots-docs-team-meta-discussion-process-governance-outreach/97401/17)).
  Tam application: apply it to shipping behaviour, versions, paths and destructive commands rather than to every typo fix.
- **consider**: A page-level edit button links readers to the source repository ([Fedora Docs contributing, search extraction, accessed 2026-11-26](https://docs.fedoraproject.org/en-US/fedora-docs/contributing)).
  Tam application: a "propose a correction" pointer could live in records without putting source metadata on the page.
- **reject**: The full Antora playbook with many source repositories and version schemes ([Create a new documentation module, search-result snippet, accessed 2026-11-26](https://docs.fedoraproject.org/szl/fedora-docs/contributing-docs/contrib-new-documentation/)).
  Tam application: take reusable attributes and `nav`-style manifests, not multi-repo orchestration.
- **reject**: Treating the current Fedora workflow as stable and fully documented: 2026 meeting notes describe active simplification work and an intimidating contribution machine ([Docs Team Meeting 2026-02-24, full page read, accessed 2026-11-26](https://discussion.fedoraproject.org/t/docs-team-meeting-2026-02-24-simplifying-contributions-and-supporting-new-fedora-users/181834)).
  Tam application: borrow individual practices, not a claimed single settled process.

## Things to avoid

- Do not quote Fedora page wording as exact: the pages fetched here came through search extraction because Anubis blocked direct fetches.
- Do not confuse documentation "modules" with the retired Fedora Modularity feature; the docs explicitly warn that Modularity ended ([Modularity docs, search extraction, accessed 2026-11-26](https://docs.fedoraproject.org/en-US/modularity/building-modules/fedora/defining-modules/)).
- Do not treat a Quick Docs orientation page as a learning path with checkpoints ([Getting started guide, search extraction, accessed 2026-11-26](https://docs.fedoraproject.org/en-US/quick-docs/getting-started-guide/)).
- Do not adopt the historical role ladder of document managers, editors and writers found in old CVS-era pages ([Docs Project CVS usage, full page read, accessed 2026-11-26](https://www.fedoraproject.org/wiki/Docs_Project_CVS_usage)).

## Where to look

- All URLs below were accessed 2026-11-26; snippet and extraction labels show evidence depth.
- [Search extraction] [Getting Started](https://docs.fedoraproject.org/en-US/fedora-user-docs/getting-started/) — F42 document, shared preparation and variant branches.
- [Search extraction] [Getting started guide](https://docs.fedoraproject.org/en-US/quick-docs/getting-started-guide/) and [Fedora Start](https://fedoraproject.org/start) — orientation and intent split.
- [Search-result snippet] [Contribute to Fedora Documentation](https://docs.fedoraproject.org/ur/fedora-docs/contributing-docs/) and [contributing](https://docs.fedoraproject.org/be/fedora-docs/contributing/) — AsciiDoc, Antora and 4-eyes policy.
- [Search-result snippet] [AsciiDoc markup](https://docs.fedoraproject.org/en-US/fedora-docs/contributing-docs/asciidoc-markup/), [Create a new documentation module](https://docs.fedoraproject.org/szl/fedora-docs/contributing-docs/contrib-new-documentation/) and [Reusable attributes](https://docs.fedoraproject.org/en-US/fedora-docs/contributing/content/asciidoc-reusable-attributes/).
- [Search-result snippet] [Edit Documentation Locally](https://docs.fedoraproject.org/en-US/fedora-docs/contributing/content/tools-edit-local-clone/) — containerised local preview.
- [Full page read] [Docs QA Review Guidelines](https://fedoraproject.org/wiki/Docs_QA_Review_Guidelines) and [Docs QA Procedure](https://fedoraproject.org/wiki/Docs_QA_Procedure) — checks and verification flow.
- [Full page read] [2026-02-24 docs team meeting](https://discussion.fedoraproject.org/t/docs-team-meeting-2026-02-24-simplifying-contributions-and-supporting-new-fedora-users/181834) — current process churn.
- [Historical] [The New Fedora Documentation Website](https://communityblog.fedoraproject.org/fedora-docs-has-been-overhauled-the-rest-of-the-story), published 2018-08-02 — DocBook-to-AsciiDoc migration.
