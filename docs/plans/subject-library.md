# Subject library and discovery

Plan status: Completed
Reviewed: 2026-11-14
Scope: Corpus organization and the command views used to discover it.
Authority: The developer approved the subject-first plan and directed implementation.
Next action: None in Develop; learner review and packaged stage promotion remain separate.

## Approved outcome

The root offers Browse subjects, Learn step by step, Search, and Sources and
further reading. Readers can explore, follow prerequisites, or retrieve an
answer without choosing between overlapping catalogs. Keywords become search
metadata; the glossary and alphabetical page catalog remain secondary lookup
views. Definitions also link directly from articles.

Canonical subjects are hardware, systems, terminal, networking, security,
programming, desktop, and tamlinux. Shared facilities are paths, glossary,
bibliography, skills, and help. Each folder has a hub; each article has one
home. Three levels remain within a topic page. Task guides and learning paths
cross subject boundaries without copying articles.

The developer selected restructuring and targeted repair, with substantial
missing curricula recorded for later work. The developer also selected a clean
address break: moved pages have no redirects. Existing useful command shortcuts
remain. This is Develop work, not installation or packaged acceptance.

## Migration and editorial rules

The baseline is commit `04e4d10`, command 0.56.0, corpus 0.89.8 (1,081 HTML pages).
[The disposition table](../corpus-migration.tsv) accounts for every original
page. It records canonical destinations, merged identities, and rationale.

- Preserve useful content when moving, splitting, or consolidating pages.
- Merge repeated clipboard, notification, capture, editing, and synonymous
  glossary material; preserve distinct tasks and precise reference details.
- Separate security setup responsibilities and general programming decisions
  from project-specific policy. Make titles describe actual page contents.
- Provide Before this, Next, Related, and Sources where they have content.
- Keep prerequisite metadata and visible links consistent. Learning-path Next
  remains the active course's next step, distinct from suggested reading.
- Retain path outcomes and IDs, deduplicate merged stops, and renumber steps.
- Keep bibliography IDs stable and distinguish supporting citations from
  optional reading. Glossary pages link back to explanatory articles.

## Command and build contract

Subject tracks replace the old five-part hierarchy in this corpus. Index
columns and sidecar shapes stay stable. Older corpus metadata remains readable.
Page kinds classify search results and glossary entries, with legacy fallback.

`tam:hubs` browses the subject hubs; `tam:topics` lists every explainable
topic A to Z, matching `tam explain topics` in both the pager and plain text;
`tam:catalog` lists page titles alphabetically. The 2026-11-23 decision
separated the two lists so one address does not carry two meanings.
`tam:paths` is the single generated learning-path catalog; authored path pages
retain teaching and capstones. `tam:keywords` and bare `tam explain` open the
search entrance. Named topic explanations and the definition shortcut remain.
Search groups definitions, practical guidance, explanations/reference, paths,
subject guides, and sources. Ranking considers the entire index; additional
matches fold within their group instead of displacing other kinds of material.

## Acceptance

Every original page has a disposition; every current page is reachable. Parent
and prerequisite graphs are acyclic. Links, citations, paths, levels, skills,
and coverage validate without relaxing existing teaching checks. All pages
render from the checkout. Fixtures cover hierarchy, kinds, old corpora,
more-than-64 search matches, ambiguous queries, and missing results.

Representative journeys cover desktop, shell, hardware, cryptography,
programming, and recovery. Source checks establish structural consistency;
reader review is still required to establish teaching effectiveness. No claim
of complete PhD-level coverage follows from passing a checker.

Expansion work is recorded in [the curriculum gaps](../curriculum-gaps.md).
