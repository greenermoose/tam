# Five-part Tamlinux knowledge system — corpus 0.31.0

**Plan status:** In progress
**Reviewed:** 2026-10-18
**Scope:** Corpus 0.31.0 and companion command 0.30.0; organization, learning,
navigation and integrated manual discovery. TCK1 is implemented; TCK2 is next.
**Authority:** The developer approved the five-part design and in-guide manual
lookup, then requested this authoritative work plan; see the
[approval record](../ai/2026-10-18-tam-knowledge-overhaul-planning.md).
**Next action:** Implement TCK2 (shared reader navigation) in the existing
shared clone, claiming its exact command paths before editing.

This is the single implementation authority for this overhaul. The
[roadmap](../../ROADMAP.md) sets priority; the [development contract](development.md)
sets invariants. Plan status changes follow [plan maintenance](../plan-maintenance.md).
The handoff is part of this plan, not a separate competing specification.

## Outcome and boundaries

The guide has exactly five primary parts, in this order: **Installing, Learning,
Using, Maintaining, Improving**. A reader can start a task, follow a learning
route, look up an unfamiliar word, open any installed man page, or explore Info
manuals without losing the way back to the Tamlinux User Guide.

Learning provides guided routes through shared knowledge. Beginner, Intermediate
and Expert describe increasing independence within a subject, not separate
collections of unrelated subjects. Search, Topics, Man pages, Info manuals and
navigation Help are common utilities, not extra numbered parts.

This milestone reorganizes and reviews all existing pages, adds the missing
hubs/learning/navigation material, and implements the command support required
by those pages. It does not fill every historical wishlist chapter. It does
not implement installer plan/apply/verify, stateful `tam learn`, shell syntax
dissection, a new rationale/depth command, network documentation retrieval or
AI search over system manuals. No Test, Run, Release, installation or deployment
is authorized by this plan alone.

## Verified baseline and problems

Inspected `develop` at `d13c1f6` (2026-10-18): command **0.29.3** in
`src/version.h`, corpus **0.30.0** in `knowledge/version`, 50 HTML pages.
Tracks are currently `use` (13 pages), `maintain` (12), `improve` (25).

- `knowledge/index.html` has seven parts and many unimplemented chapter notices.
  Installation is filed under maintenance; the paths and architecture sit
  alongside the three-track model. There is no Learning or Using hub.
- Beginner/intermediate/expert reading lists mostly select different chapters;
  they do not consistently select a page's matching depth or teach outcomes.
  Prerequisites exist but are sparse and do not form coherent guided routes.
- `tools/build-index.py` accepts only three tracks and derives breadcrumb parents
  from physical directories. Logical reorganization therefore needs index and
  breadcrumb changes even if existing URLs stay unchanged.
- `src/info.c` generates no Guide breadcrumb for Info directories/nodes.
  `src/pager.c` substitutes Info-only footer hints and omits the Guide key.
  Backtick still opens the Guide. Left Arrow is history Back; Info `u` is Up.
- `src/man.c` supports named pages but bare `tam man` is help/usage, not a
  manual discovery screen. `knowledge/man/index.html` offers a few links, not
  arbitrary in-guide lookup, and misnumbers manual sections after section 3.
- `src/tam.c`, `src/man.c` and `src/info.c` have separate loader contexts.
  Every entry path must support the same generated utilities and return routes.

These are observations of source and authored pages, not packaged acceptance
claims or a complete factual audit. Recheck the baseline before implementation.

## Information architecture

| Part and hub | Reader's goal | Contents and boundary |
| --- | --- | --- |
| Installing — `install/index.html` | Get a working system. | Requirements, supported routes, preparation, inspection, first login and verification. Explain the available route from current product sources; label stub installer commands planned. Link maintenance/recovery rather than duplicate them. |
| Learning — `learn/index.html` | Understand and build skill. | Orientation, reading the guide/manuals, foundations, architecture, terminology, and ordered beginner/intermediate/expert routes through shared pages. No compulsory course before doing a task. |
| Using — `use/index.html` | Accomplish a task. | Desktop, files, shell workflows, applications, workspaces, displays, ordinary settings and shortcuts. Link reference near the relevant example. |
| Maintaining — `maintain/index.html` | Keep the system dependable or recover. | Updates, package operations, health, logs, snapshots, boot, recovery and troubleshooting. Explain prerequisites and consequences before any changing command. |
| Improving — `improve/index.html` | Change or extend the system. | Scripting, development, plugins, packaging builds, performance engineering, contributions and AI-assisted development. Everyday preferences belong in Using. |

The root shows a brief welcome, the five choices with useful entry links, and
common utilities near the top. At 80x24 the first screen exposes all five choices
and the utility entry points. Detailed chapter listings move to hubs. Every
existing page is reachable from a hub, path or utility; no user-facing outline
of nonexistent chapters. Keep missing-content work in planning notes.

Each page has one primary home and may be linked from many routes. Preserve
existing page paths, topic slugs, aliases, anchors and query-depth syntax where
possible. Use logical parent metadata instead of moving all files into new
directories. Breadcrumbs describe the canonical home, not the path the reader
happened to follow; history Back preserves that actual journey.

### Page disposition: all 50 existing pages

Paths below are relative to `knowledge/`. A listed directory group contains only
the named files, not arbitrary future additions. This table accounts for the
baseline; update it if another change lands before implementation.

| Existing pages | Primary home / disposition |
| --- | --- |
| `index.html` | Guide root; shared navigation, not a sixth track. |
| `welcome.html`, `beginner.html`, `intermediate.html`, `expert.html` | Learning; rewrite orientation and paths; preserve these addresses. |
| `install/index.html` | Installing hub; separate installation from ongoing maintenance. |
| `shell/index.html`, `filesystem/index.html` | Learning foundations; retain topic-directory addresses. |
| `shell/pipelines.html`, `shell/jobs.html`, `filesystem/permissions.html` | Using; learning routes link to appropriate depths. |
| `desktop/index.html`, `desktop/workspaces.html` | Using; desktop overview and practical workflows. |
| `hardware/resources.html` | Maintaining; cross-link from Using and learning routes. |
| `man/index.html` | Learning to read manuals; link to the dynamic Man and Info entrances. |
| `ecosystem/index.html` | Learning architecture hub; retain its existing address. |
| `ecosystem/ai.html`, `compositors.html`, `desktop-apps.html`, `distros.html`, `init.html`, `libc.html`, `philosophy.html`, `sustainability.html`, `toolkits.html`, `terminology.html`, `sddm.html`, `ttys-and-virtual-consoles.html` (all under `ecosystem/`) | Learning, beneath the architecture hub. Contextual links lead to operational or development procedures. |
| `ecosystem/packaging.html` | Maintaining; teach ownership/layers and package operations, linking build/pin work under Improving. |
| `ecosystem/patches.html`, `ecosystem/package-versions.html`, `ecosystem/lua.html`, `ecosystem/virtualization.html` | Improving; retain conceptual introductions and link from Learning. |
| `maintain/index.html`, `updates.html`, `rollback.html`, `logs.html`, `power.html`, `recovery.html`, `boot.html`, `boot-recovery.html`, `snapper.html` (all under `maintain/`) | Maintaining. Retain each distinct topic; fix hub enumeration and cross-links. |
| `improve/index.html`, `efficiency.html`, `zram.html`, `terminal.html`, `scripting.html`, `toolchains.html`, `ai-performance.html` (all under `improve/`) | Improving. Retain technical material; link everyday use to Using. |
| `tam.html` | Improving: command architecture/build/lifecycle. Extract user-facing entry guidance into the new `learn/tam.html` without duplicating implementation detail. Preserve existing anchors or add compatibility anchors where text moves. |

New authored pages required: `learn/index.html`, `use/index.html`,
`learn/tam.html` (using the knowledge system), `learn/documentation.html`
(guide/man/Info/search and navigation), and `learn/practice.html` (safe exercises
and self-checks shared by the three paths). Existing pages supply subject
lessons; add further pages only when the agreed outcomes cannot be met by
revising or linking those pages, and record the reason in this plan.

### Metadata and compatibility contract

Retain the `track` field with values `install`, `learn`, `use`, `maintain`,
`improve`. Add optional `parent` metadata containing the canonical parent page
path (corpus-root-relative, including `.html`). Make it explicit on every
non-root authored page in corpus 0.31.0. The root has no parent and is allowed
an empty track; it is displayed as Guide, not assigned to one of its five parts.
Generated utilities have an explicit shared-navigation location, not a fake
corpus track. Templates, index docs and JSON must describe these rules.

For older corpora, absent parent retains the existing directory-based fallback;
an existing three-track corpus must still load. A newer index adds parent as an
additive JSON field and index projection while preserving the existing TSV
columns. Old readers must ignore added index data without corrupting their
projection (change the projection strategy if necessary). The index builder validates
parent existence, root reachability, no cycles, and breadcrumb agreement with
the declared hierarchy. Track inheritance/ownership must agree with the part
ancestor; shared root is the only authored exception.

The command renders all five track labels in topic guides, catalogs and search.
Level and prerequisite metadata remain separate from navigation parents.
Preserve existing search JSON keys and document additive fields. Prerequisites
name valid topics, form no cycles and appear as useful links rather than hidden
author-only facts. A learning route can visit a page from any part without
changing that page's primary home.

## Learning design and content review

Every route states an outcome, prerequisites, ordered steps, safe practice,
observable self-checks and next choices. No progress database or automatic host
modification is needed. Exercises use temporary user-owned files; cleanup and
expected output variation are explicit. Do not use live boot, package removal,
privilege or recovery operations as practice tasks.

| Route | Required progression and evidence of independence |
| --- | --- |
| Beginner | Navigate the desktop and guide; open terminal; find/read files; understand streams; run a small read-only pipeline; open a named man page and an Info manual; return to the guide. Complete examples explain each action. |
| Intermediate | Adapt pipelines and permissions in a scratch directory; inspect processes/resources; read logs; explain package ownership and snapshot/backup boundaries; use keyword manual lookup. Supply partial examples, diagnostic questions and hints. |
| Expert | Explain process/descriptor/TTY behavior and system boundaries; reason about boot/recovery dependencies; inspect architecture/tradeoffs; build or extend a small tool in a safe development context. Supply goals, constraints and authoritative references with less scaffolding. |

Paths link explicitly to `?level=beginner`, `intermediate` or `expert` when a
page offers that depth. Do not send a beginner link to an expert-only page or
silently show no content when a depth is missing. Preserve `0`/all and `1`/`2`/`3`
shortcuts. Single-depth references are allowed; do not manufacture three nearly
identical versions of every page. Use existing fading templates, improved with
outcomes, prerequisites, examples, common mistakes, self-checks and next links.

Review all 50 pages for placement, misleading examples, duplicated material,
current/planned distinctions and links. Verify Tamlinux facts against the
sources in `docs/corpus-authoring.md`, including keybindings, package ownership,
deployment, boot/login and developer workflow. Do not turn planned compositor,
base-system or installer choices into current facts. Preserve the sourcing and
public-boundary rules. Man pages can be full manuals; Info can contain reference
as well as tutorials. Do not equate documentation format with reading depth.

## A coherent Guide / Man / Info reader

### Common navigation

Use one shared location/navigation model across authored pages, generated topic
guides, manual discovery/results, roff man pages, Info directory and Info nodes.
In normal interactive views, keep a compact location row visible while content
scrolls. Its Guide link and ancestors are selectable with the normal link keys;
do not paint a decorative header that cannot be followed. Do not duplicate that
row as an additional scrollable breadcrumb. Plain text emits the location once.

Examples: `Guide > Maintaining > Snapper`, `Guide > Man pages > printf(3)`,
`Guide > Info manuals > Bash > Shell Syntax`. The latter is a location trail,
not a fabricated Info ancestry; retain the manual's genuine Up relationship.

| Action | Contract |
| --- | --- |
| Back | Left Arrow always restores browsing history, including scroll/cursor position. Label it Back. |
| Guide | Backtick always opens the Guide and preserves a route back through history. Provide the visible link as well. |
| Up | Info `u` retains manual-hierarchy Up. On ordinary pages its existing Back behavior remains. Label the context rather than equating it with history. |
| Info traversal | Retain `n`/`p`/`t`/`i`; show Next/Previous/Top/Directory links as applicable. Active search retains its existing `n`/`N` priority. |
| Help | Add `?` for navigation help, and a visible Help action. Preserve the current page, focus and history when closing help. |
| Quit | `q` exits the reader; Esc cancels an active prompt/search before its existing quit behavior. It does not mean Back to Guide. |

Show labeled Back, Guide, Help and Quit actions in available footer/header space
before secondary mode hints. Info must not hide the Guide action. At narrow
widths abbreviate labels and ancestors, preserve access to full help, and avoid
wrapping status into content. With very short screens prioritize readable
content and a compact action hint; for a one-row or unusable terminal retain
the existing plain-output fallback. Use the portable backend and cell-width
helpers. Header focus, content focus, search prompts, resize, selection and
history must work together without introducing polling or full-screen flicker.

If the corpus is unavailable, man/Info remain usable. Guide navigation must
give a truthful, recoverable missing-guide message rather than leave the
reader or fabricate a working breadcrumb target.

### Man entrance and arbitrary lookup inside the guide

The Guide's Man pages action opens a dynamic landing screen, also opened by
bare `tam man` in a TTY. Keep `tam man --help` as usage and retain named-page,
section and pipe forms. Bare noninteractive `tam man` prints the landing text
and examples without attempting to prompt. Use `tam:man` as the shared landing
URI, supported by every loader. It is distinct from `man/index.html`, the lesson.

The landing offers selectable actions **Open a page**, **Find by keyword**,
and **Browse sections**, plus curated starting pages. Selecting an action
opens an in-reader prompt; Enter runs the lookup and Esc cancels to the same
screen. The page-name action accepts a separate optional section. A result
list shows name, section and description; selecting a hit opens its `man:` URI.
Section browsing paginates/bounds results rather than loading the entire manual
tree on every keystroke. Keep `/` for searching within displayed text; label
manual lookup separately so the two searches cannot be confused.

Keyword lookup uses the installed man database (`apropos`/`man -k` via an argv
process interface, never a shell command assembled from input). Use the existing
safe-probe patterns with suitable documented lookup time/output bounds and
literal option-safe arguments. If an input/helper cannot be handled safely,
give a useful error. No automatic `mandb`, writes to system databases, network
fetches or installation. Missing tools/database, ambiguous sections, no hits,
missing pages and truncated results each get distinct recoverable messages.
Use deterministic fake helpers/man trees in tests; installed catalogs vary.

Describe sections accurately: 1 commands, 2 system calls, 3 library calls,
4 devices, 5 file formats, 6 games, 7 miscellaneous/conventions, 8 administration,
and optional 9 kernel routines; allow installed subsection names. Update the
manual-reading lesson, command help, `man/tam.1` and completions where affected.

### Info and discovery

Keep bare `tam info` opening the installed directory and retain its categories.
Add shared navigation and a concise explanation of how to choose a manual;
use the same location model for every node. Preserve real manual links,
node relationships, indirect files and existing `info:` URI syntax. Offer
curated starts only when installed, or label absence truthfully. Do not assume
that Bash, Coreutils or libc documentation ships on every installation.

Guide Search remains corpus-ranked and keeps its CLI/JSON contract. Provide an
in-guide search entry (`tam:search`) with a prompt and linked results so readers
need not exit to the shell. Present Guide results separately from explicit
Man and Info discovery actions. Man keyword results must work in this milestone;
Info discovery may use the installed directory and in-page search. Cross-manual
full-text Info search and an eager merged index of all installed documentation
are deferred. `tam explain` remains a topic guide, not a command executor or
an automatic redirect into a system manual. `tam ask` stays corpus-grounded.

Generated lookup/action URIs are typed, allowlisted requests with correctly
encoded input and bounded parsing; document their grammar alongside the loader
implementation. Centralize routing/navigation metadata where practical so direct
CLI entry and following links cannot produce different supported actions.

## Implementation slices and ownership

Use the existing clone and `tam-work` claims. These slices are sequential where
they share contracts; they are not instructions to dispatch other agents.
Claim only current paths, adding shared integration files briefly as needed.
Each slice is a checklist item with a permanent TCK identifier; see the
[checklist identifier rules](../../CHECKLISTS.md).

| Item | Work and likely paths | Completion evidence |
| --- | --- | --- |
| TCK1: Index/hierarchy contract | `tools/build-index.py`, `src/index.[ch]`, generated index schema, focused index tests, `docs/corpus-authoring.md`. Implement five tracks, optional logical parent with old-corpus fallback, root handling and hierarchy validation. Keep the current corpus readable. | Fixtures cover five tracks, old three-track input, missing/cyclic parents, track mismatch, additive hierarchy data and unchanged topic/level/prerequisite behavior. Record schema decisions here. **Completed 2026-10-18:** builder and C loader support five tracks and logical parents; old corpora retain directory fallback. `make corpus`, four builder fixtures, 29 corpus-linter tests, `tests/guide.sh`, and the command PTY suite pass. ASan found and verified the fix for uninitialized parent pointers after index-array growth. |
| TCK2: Shared reader navigation | `src/pager.[ch]`, `render.[ch]` as needed, loader contexts in `tam.c`, `man.c`, `info.c`, focused PTY fixtures/tests. Implement common location/actions and Help, preserving native Info traversal and history. Set command 0.30.0 and its matching test assertion in the change introducing the first new reader capability. | Guide, topics, man and Info all expose reachable Guide/Back/Help/Quit; scrolling/resize/narrow screens/search preserve them. Direct CLI entry works too. |
| TCK3: Manual and search discovery | `src/man.[ch]`, common routing and prompt support, `tam.c`, `info.c`, `completions/bash/tam`, `man/tam.1`, deterministic helper fixtures. Implement `tam:man`, open/keyword/section actions and `tam:search`; retain Info directory. | A guide reader opens an arbitrary installed manual and keyword result without leaving tam; missing docs and literal hostile-looking input fail safely; pipe mode never prompts. |
| TCK4: Corpus migration and learning | All 50 mapped pages, five new authored pages, `knowledge/index.{tsv,json}`, `knowledge/version`, `tools/lint-corpus.py` and related tests if rules change. Rebuild root/hubs, paths, metadata, navigation lesson and contextual manual links; review current facts. Set corpus 0.31.0 in the change introducing the reorganized/new content. | Every baseline page has a deliberate home, valid breadcrumbs and preserved public addresses; all three routes meet outcomes; complete reachability and query checks pass. |
| TCK5: Integrate and hand off candidate | Verify the version files/assertions already changed with their features; reconcile `README.md`, `ROADMAP.md`, development/authoring/audit docs and provenance. Run stable-tree integration and prepare package-consumption notes. | Command 0.30.0/corpus 0.31.0 pair passes acceptance below. Mark plan Completed only against evidence; keep real-terminal/package review limitations explicit. No deployment/stage change. |

Allocate command **0.30.0** to the first new reader/discovery capability and
corpus **0.31.0** to the reorganized content, in the changes delivering those
features. Do not bump either just for this plan. If another authorized feature
consumes a target first, stop and reconcile the version target with the developer;
do not silently renumber this agreed corpus milestone. Intermediate development
commits may be incomplete; do not mark the milestone complete after TCK1 or TCK4.

TCK1 schema decision: `knowledge/index.tsv` keeps its existing ten columns in
the same positions. An appended TSV column would be absorbed into `updated` by
older readers, so parent data is projected additively in `knowledge/index.json`
and `knowledge/index.parents.tsv`. The C reader loads the optional sidecar;
older command versions continue to read the unchanged TSV. The hierarchy
fallback is used only when a page omits parent metadata, while corpus 0.31.0
requires explicit parents and validates part-track ownership.

## Acceptance and verification

1. On an 80x24 terminal the root presents five named parts plus utilities on the
   first screen. Follow each part to real content. Every original page remains
   reachable; a checked manifest accounts for all 50 plus new pages.
2. Start each learning route and follow its prerequisites, explicit depths,
   practice and next choices. Check the outcomes in the route table. Returning
   from a canonical home does not strand the reader or lose their history.
3. Guide -> Info directory -> manual -> node -> Guide via both visible link and
   backtick; history Back restores node, position and focus. Repeat from direct
   `tam info`. Confirm Info Up differs visibly from Back.
4. Guide -> Man pages -> arbitrary name and section -> SEE ALSO -> Info where
   available -> Guide. Repeat with keyword results, section browsing and direct
   `tam man`. Cancel and failure paths remain inside the reader.
5. Search an unfamiliar concept from the guide, choose a Guide result or explicit
   manual discovery, and return. Guide ranking/JSON and `tam explain` semantics
   remain stable. Queries include `install`, `learning`, `workspaces`, `permissions`,
   `snapper`, `limine-snapper-sync`, `tty`, `vt`, `boot recovery`, `packaging`, `man`,
   and `info`. Record expected destinations; do not merely assert nonempty output.
6. Deterministic helper tests cover missing tools/catalogs/pages, multiple manual
   sections, bounded large results, timeouts, Unicode, long input and literal
   shell metacharacters. No helper runs submitted input as shell code or writes
   a database. Missing corpus still permits man/Info and truthful Guide failure.
7. Focused PTY/screen tests cover 80/40/narrow columns, short/long documents,
   very short screens, scrolling, active prompts/search, resize, repeated entry,
   selected navigation links and history across depths. Pipe/redirected/dumb
   output has no ANSI controls, prompts, terminal probes or network activity.
8. Run `make corpus`, meaningful focused tests, `make test-command` and
   `make test-corpus`; run `make test` with a short exclusive claim/stable tree.
   Bound foreground test time, use the verified `libtam` dependency, and review
   intended renderer golden diffs. Use sanitizer checks for new ownership,
   prompt/input parsing and header/history geometry. Record exact results.
9. Follow `docs/corpus-authoring.md` and the separate independent-audit procedure
   when the developer requests that audit. Automated lint is required; this
   plan does not assign an independent agent or claim an audit has happened.
10. Record the exact command/corpus commits and package inputs needed by the
    owning assembly. Real Linux-console and Foot review of the exact packaged
    candidate remains developer-directed; PTYs cannot certify visual acceptance.
    Follow package-only delivery and stage authority even where an older
    archived plan describes a direct development install.

Before closing, reconcile the plan index, roadmap and development contract using
the maintenance procedure. Do not mark deferred future ideas implemented.

## Supersession and next-agent handoff

This replaces the seven-part contents/three-track organization in
`user-guide-and-man-page.md`, the obsolete scaling phases in
`explain-topic-scaling.md`, and those organizational portions of older plans.
It extends, rather than removes, the implemented Info reader, topic-guide
semantics, independent versions and portable pager. Their source and verification
records remain useful; their old task lists are not new assignments.

Start with `AGENTS.md`, `docs/plans/README.md`, the current roadmap/development
sections and this plan. Read `docs/corpus-authoring.md` before editing content.
Refresh command/corpus versions, `git status --short`, HEAD and relevant claims;
preserve others' changes. Claim TCK1's exact paths with `tam-work begin` and check
the inbox. Inspect index readers and fixtures before adding the parent field.
Then work TCK1 -> TCK2 -> TCK3 -> TCK4 -> TCK5, recording checkpoints in this plan and
provenance. Keep the command runnable between slices. This document has no
unresolved design question that blocks TCK1. Report a newly discovered conflict
instead of reviving an older proposal.
