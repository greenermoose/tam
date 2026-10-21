# tam plan index

Reviewed 2026-10-21 (learning levels plan and banner-ranking follow-up) under the [plan maintenance procedure](../plan-maintenance.md).
This is the complete inventory of plans and corpus notes in this directory.
Read status/disposition before a historical body. A closed plan is not a task
queue. `ROADMAP.md` gives priority; `development.md` gives the current contract.

## Current work

| Plan | Status | Disposition / next action |
| --- | --- | --- |
| [Learning levels, goal paths and self-quizzes](learning-levels.md) | In progress | Approved 2026-10-20/21 (Tamlinux decision D184; [planning record](../ai/2026-10-21-tam-learning-levels-plan.md)). TCK20 (contract, [`corpus-pedagogy.md`](../corpus-pedagogy.md), [templates](../templates/)) delivered with the plan; TCK21 (sticky level lens) delivered as command 0.37.0, with [banner ranking corrected in 0.37.1](../ai/2026-10-21-tam-banner-completion.md). TCK22 delivered as command 0.38.0 / corpus 0.37.1 ([record](../ai/2026-10-21-tam-folding.md)). Next: TCK23 (pilot pages and two goal paths); TCK24 scaffolding and TCK25 migration follow. |
| [Common pitfall → Best practice](common-pitfall-to-best-practice.md) | Completed | Corpus 0.35.2: 16 pages lead with the correct action (`ae1fa41`, index `ca25c8f`); [audit report](../ai/2026-10-20-common-pitfall-to-best-practice-audit.md) and [session record](../ai/2026-10-20-tam-common-pitfall-best-practice.md). Corpus only; package stages remain developer-directed. |
| [Pager quick keys, address display and status row](pager-quick-keys.md) | Completed | Approved and completed 2026-10-20 (Tamlinux decision D176; [planning record](../ai/2026-10-20-tam-pager-quick-keys-plan.md); [session record](../ai/2026-10-20-tam-pager-quick-keys.md)). TCK15–TCK19 delivered in command 0.34.0 / corpus 0.35.1. |
| [Developer loop: `make dev` and drift checks](dev-loop.md) | Completed | Approved and closed 2026-10-20 ([planning record](../ai/2026-10-20-tam-dev-loop-plan.md)). TCK11 (`make dev`, `ce4b617`), TCK12 (`make check-repo`, `docs/repo-map.md`), TCK13 (pager tests on `tests/fixtures/guide/`, `ee845af`) and TCK14 (`make dev` names the failing check, `e8da257`); developer acceptance by the corpus 0.34.1 run (`98222ff`, `4d41d84`). Non-interactive live-corpus assertions are on hold (roadmap section 4). Tooling and tests only; no component version change. Tooling only; no component version change. |
| [Home-row pager keys and attention column](pager-home-row-keys.md) | Completed | TCK10 (home-row pager keys), command 0.32.0 / corpus 0.33.5, decided in D175. Full suite passed; see the [session record](../ai/2026-10-20-tam-pager-home-row-keys.md). Interactive review by the developer and packaged acceptance remain developer-directed. |
| [Reliable corpus builds](corpus-build.md) | Completed | TCK9 (reliable corpus builds), corpus 0.32.2: pages/Git metadata rechecked; `corpus-and-test` replaces `test-corpus`. Focused, parallel and integrated checks passed; see the [session record](../ai/2026-10-19-tam-corpus-build.md). |
| [Version checks, source upgrade and stage switching](upgrade.md) | Completed | TCK6–TCK8, full source tests, isolated source build, live metadata check and developer-directed first activation completed. The development prefix was first activated at command 0.31.1 / corpus 0.32.0 and now holds later developer builds (`tam --version` reports them); package-stage acceptance remains developer-directed. See the [Luna handoff](../handoffs/tam-upgrade-luna.md) for the historical implementation record. |
| [Knowledge system, corpus 0.31.0](knowledge-system-0.31.0.md) | Completed | Command 0.30.0 / corpus 0.31.0; TCK1–TCK5 completed in `e0215fd`, `0b3f008`, `52e71f7`, `597cb71`, `f4fa539`. Automated acceptance passed; package and physical review remain developer-directed. |
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
