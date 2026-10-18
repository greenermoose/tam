# Maintaining tam plans

Adopted by the developer on 2026-10-18. This procedure governs `ROADMAP.md`,
`docs/plans/development.md`, and every plan or corpus note in `docs/plans/`.
It changes documentation workflow; it grants no Test, Run or Release authority.

## Authority and entry points

The developer's current instructions decide scope. Repository documents then
have distinct jobs:

1. `ROADMAP.md` states current component versions, the next approved outcome,
   dependencies and deferred ideas. It does not duplicate detailed work items.
2. `docs/plans/development.md` is the living integration contract: ownership,
   current behavior, architectural invariants, versioning and decision history.
3. `docs/plans/README.md` inventories every plan, its status and its disposition.
4. One authoritative feature plan owns each approved outcome, including its
   acceptance criteria and implementation handoff. Other plans link to it.
5. `docs/ai/` records what happened and why; it is evidence, not a task queue.

Read a plan's disposition before its body. Older date-stamped decisions describe
their time; they do not overrule the current contract or a replacement plan.
Conflicts without a recorded developer decision go back to the developer.

## Status and required fields

Every plan starts with `Plan status`, `Reviewed` (ISO date), `Scope`,
`Authority`, and `Next action`. State the component for every version number.
Use [the template](plans/TEMPLATE.md); short corpus notes use their own template.

| Status | Meaning | Required disposition |
| --- | --- | --- |
| Proposed | A suggestion awaiting a scope/design decision. | Name the question and who decides; implementation is not assigned. |
| Ready | The developer approved the scope; implementation has not started. | Link the approval record; provide runnable first steps and acceptance criteria. |
| In progress | Approved work is underway. | State completed work, remaining work and the next bounded action. |
| Active | A living integration document, not a finite feature task. | Identify its authoritative feature plan and current integration action. |
| Completed | The defined implementation or documentation outcome is achieved. | Name the version/commit or resulting files, verification record and any outstanding acceptance limit. No implementation next action. |
| Superseded | A different plan now owns all or part of its design. | Link the replacement, identify exactly what it replaces and route any surviving work. |
| Retired | Work is intentionally withdrawn without a replacement. | Record the developer's reason and disposition of unfinished work. |
| Template | A form for creating a plan; no assigned work. | Link this procedure. |

Completed, Superseded and Retired are closed task states. Completion of source
work does not imply packaged acceptance, deployment or release. Keep those
limits visible; route genuine follow-up into an active plan or a clearly labeled
deferred item. Never silently discard unfinished requirements or call an
unverified implementation accepted.

## Change a plan in the same change as the decision

1. Read the plan index, roadmap and relevant development sections. Verify
   versions from `src/version.h` and `knowledge/version`, and inspect source,
   commits and verification records before declaring work completed.
2. Claim only the affected files. Include the roadmap, development contract,
   plan index, old/new plans and affected authoring/routing docs as necessary.
3. For a small change that preserves the outcome, revise the existing plan and
   record the changed decision. For a replacement outcome or architecture,
   create one new authoritative plan and make the old plan Superseded.
4. Put a disposition banner before the old body. Preserve historical design
   text at its existing address so links keep working. Say which requirements
   survive, which change and where unfinished work goes. Remove contradictory
   current-status claims from the old preamble; do not rewrite historical
   session records or Git history.
5. Update the plan index, roadmap and current development contract together.
   Replace active links to old instructions with links to their successor;
   links explicitly labeled historical may remain. Drop stale version
   reservations rather than reusing numbers already delivered.
6. On completion, check each acceptance criterion, cite the implementation
   and verification, close the plan and update the same entry points. If a
   criterion remains, leave the plan In progress or record an explicit
   developer-approved scope change and its destination.
7. Record provenance, review the full diff and commit only claimed paths.
   A documentation-only plan change does not bump either component version.

Supersession links point forward to an existing plan and must not form cycles.
Keep closed plans in place by default. Do not maintain a second archive copy;
Git preserves revisions. Reopening a closed plan requires an explicit scope
decision and synchronized entry points, not an old unchecked task list.

## Review and handoff checks

Perform these checks whenever a plan changes, when a milestone finishes, and
before handing work to another agent:

- Enumerate `rg --files docs/plans -g '*.md'`. Every plan/note has exactly one
  index row; only `README.md` is the inventory itself. Templates are listed.
- Check the required fields, matching index status, replacement/evidence links,
  and that every relative link in changed planning documents resolves.
- Compare roadmap/development current versions with the version files. Clearly
  label future targets; do not mark a version shipped merely by planning it.
- Search active documents for stale terminology, already-completed "next" work,
  conflicting future version reservations and references to retired designs.
  Historical sections must be visibly labeled as such.
- Read the handoff as a new agent: one starting plan, current baseline, exact
  scope, dependencies, ordered slices, verification, limits and next action.
- Run `git diff --check` and inspect the public diff for private identifiers.
  Run code tests only when code changes warrant them; link checking cannot
  establish product behavior.

The 2026-10-18 reconciliation is recorded in the [plan index](plans/README.md).
