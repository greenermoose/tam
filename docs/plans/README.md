# tam plan index

Reviewed 2026-10-18 under the [plan maintenance procedure](../plan-maintenance.md).
This is the complete inventory of plans and corpus notes in this directory.
Read status/disposition before a historical body. A closed plan is not a task
queue. `ROADMAP.md` gives priority; `development.md` gives the current contract.

## Current work

| Plan | Status | Disposition / next action |
| --- | --- | --- |
| [Knowledge system, corpus 0.31.0](knowledge-system-0.31.0.md) | Ready | **Next implementation.** Five-part spine, learning routes, shared Guide/Man/Info navigation, in-guide manual and search discovery. Start TCK1; companion command target 0.30.0. |
| [Development contract](development.md) | Active | Living ownership, behavior and integration contract; incorporate each overhaul slice without rewriting historical decisions. |
| [Pager display portability](pager-display-portability.md) | In progress | Source implementation is command 0.29.0 (`581822a`); real Linux-console/Foot acceptance of an exact packaged candidate remains pending on the developer's instruction. No missing-source assignment. |

## Closed plans retained at stable addresses

| Plan | Status | Evidence / replacement |
| --- | --- | --- |
| [Offline guide](offline-guide.md) | Completed | Command 0.1.0, `51f43f6`; native renderer survives. New organization/navigation follows the overhaul. |
| [Orientation and explanation](orientation-and-explanation.md) | Completed | Command 0.4.0, `b01faf9`; current explain semantics are in the development contract. |
| [Installation inspection](installation-inspection.md) | Completed | Command 0.5.0, `dd2793a`; installer execution remains separately deferred. |
| [Explain topic scaling](explain-topic-scaling.md) | Superseded | Replace obsolete phases/domain proposals with the corpus 0.31.0 plan. Preserve implemented index/search, retire unimplemented flags and thresholds as assignments. |
| [Grounded ask](grounded-ask.md) | Completed | Command 0.17.0, `e929ee5`; existing opt-in/corpus-grounding contract survives. |
| [User Guide and man page](user-guide-and-man-page.md) | Superseded | Delivered originally in command/corpus 0.20.0, `a1a08ab`; replace seven parts and three-track organization with the corpus 0.31.0 plan. |
| [Help message redesign](help-message-redesign.md) | Completed | `9464b0f`; final developer decisions in the linked verification record override draft examples. |
| [Info subsystem](info-subsystem.md) | Completed | Command 0.22.0, `a5a3b4f`; parser/URI behavior survives; new common navigation follows the overhaul. |
| [Topic guides and tam page](topic-guides-and-tam-page.md) | Completed | Command 0.23.0, `a32f983`; show/explain separation survives; track labels and user guidance follow the overhaul. |
| [Independent versions and corpus alignment](decoupled-versioning-and-corpus-alignment.md) | Completed | Command/corpus 0.24.0, `a8cc06a`; independent versions survive. |
| [TTYs and virtual consoles](corpus-ttys-and-vts.md) | Completed | Corpus 0.27.0, `8458674`, corrected in `91d2c47`; page exists, new home is Learning. |
| [Boot fallback and rescue](corpus/linux-fallback-rescue.md) | Completed | Corpus 0.29.0, `b9c17ed`, index `9de0735`; remains Maintaining. |
| [Snapper](corpus/snapper.md) | Completed | Corpus 0.30.0, `d13c1f6`; remains Maintaining. The previously open note is closed. |

Each closed plan's preamble links its verification/evidence and defines exactly
what survives. The overhaul owns all new organizational work; none of these
historical plans creates an additional corpus release assignment.

## Templates

| File | Status | Use |
| --- | --- | --- |
| [Feature plan template](TEMPLATE.md) | Template | A new proposal with explicit scope, evidence and disposition. |
| [Corpus note template](corpus/TEMPLATE.md) | Template | A small missing-content request; same maintenance rules. |

The index itself is the inventory, not a feature plan. Future ideas such as
installer execution, stateful progression and syntax dissection are deferred in
the roadmap; their presence in an old design is not implementation approval.
