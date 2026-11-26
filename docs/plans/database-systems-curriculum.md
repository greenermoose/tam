# Databases and data storage curriculum

**Plan status:** Completed
**Reviewed:** 2026-11-25
**Scope:** Corpus-only content, ending at corpus 0.130.0: a Programming >
Databases and data storage hub, ten three-level teaching pages, a five-stage
`databases` goal path, thirty-three glossary pages, twelve bibliography entries
and the `data.*` skill cells. The final slice adds SQLite, PostgreSQL and MySQL
glossary entries for pager `whatis` lookup. No command, install or stage change.
**Authority:** The developer asked for pages on SQLite and IndexedDB, then for a
series covering influential open source database programs such as SQLite,
PostgreSQL and MySQL, non-SQL databases, GraphQL and storage for very large data
sets. He approved the full-series shape with the instruction “Make it so.” He
then asked for SQLite, PostgreSQL and MySQL to be added to the glossary so those
names are discoverable through `whatis` in the tam pager.
**Next action:** None. Closed 2026-11-25 at corpus 0.130.0; learner review,
installation and packaged acceptance remain developer-directed.

## Problem and agreed outcome

Programming had no connected route into databases or data storage. A reader
could find a general `database` glossary entry and scattered references in web
development, but there was no path from keeping records in a file to choosing
between an embedded engine, a server database, a browser object store and a
distributed data system.

The agreed outcome is a canonical subject home under `programming/`, with
cross-links from the Programming hub and Files and storage. Each teaching page
offers a Novice idea, a Learner exercise and an Expert reference. A goal path
sequences the pages into five stages with checkpoints and one capstone.
Glossary, bibliography and skills bookkeeping follow the existing corpus
conventions.

## Baseline and dependencies

The source checkout was on `dev` at `e4b30e1`, with command 0.69.7 and corpus
0.128.4. The subject library, one-page three-level ladder, goal-path contract,
sandbox policy and `programming` track anatomy enforcement were already in
force. No command change was needed; corpus content versions independently.

## Implementation and handoff

1. **Hub and teaching pages (TCK33.1):** Create
   `knowledge/programming/databases/index.html` and ten pages: why databases,
   relational model and SQL, indexes/transactions/storage, SQLite, PostgreSQL,
   MySQL and InnoDB, non-SQL databases, IndexedDB, GraphQL and very large data
   sets. Each page declares `databases:<step>` for steps 1–10 and a prerequisite
   chain that can be walked backwards. Link the hub from Programming and Files
   and storage.
2. **Path and supporting corpus (TCK33.2):** Add `knowledge/paths/databases.html`
   with five numbered stages, a checkpoint at each, one capstone with a folded
   solution and one Next paths section. Add thirty glossary pages, twelve
   bibliography pages with manual index rows, and `data.novice`,
   `data.learner` and `data.expert` skills with reading links.
3. **Bookkeeping and verification (TCK33.3):** Bump `knowledge/version` to
   0.129.0, update `ROADMAP.md`, the plan index and development record, and
   write the session work record. Run `make corpus`, `make corpus-and-test`,
   `make check-repo` and reader spot checks; commit pages first and regenerated
   indexes after them.
4. **Whatis discoverability (TCK33.4):** Add `glossary/s/sqlite.html`,
   `glossary/p/postgresql.html` and `glossary/m/mysql.html` with exact titles,
   topics and keywords. Link them from the first use and New words sections of
   the matching teaching pages. Carry `knowledge/version` to 0.130.0 and verify
   that the pager's `whatis` prompt resolves each name to its glossary entry.

Examples are simulated teaching sandboxes. Scripted outputs are described in
page prose as samples, not readings from the developer's machine. No database
software is installed or started.

## Acceptance and limits

- `make corpus`, `make corpus-and-test` and `make check-repo` pass.
- All ten teaching pages declare three levels, a sandbox Learner section,
  a quiz in Learner and Expert, valid citations and working glossary links.
- `paths/databases.html` has five stages, ten levelled steps, five checkpoint
  quizzes, one capstone solution and one Next paths heading.
- The pager's `w` / `whatis:` prompt resolves `sqlite`, `postgresql` and `mysql`
  to their own glossary entries without ambiguity.
- Command version remains 0.69.7. No Test, Run, Release, tag or install action
  is part of this plan.
