# Corpus 0.82 — Trustworthy paths

**Plan status:** In progress. Phases 0–2 delivered on 2026-11-13 at corpus
0.82.0, and the hub-to-step gap was closed at corpus 0.82.1; Phase 3
(independent audit) waits for Fred's word; Phases 4–5 follow the audit.
**Reviewed:** 2026-11-13
**Scope:** `tam` corpus lane, with one read-only audit report and possible command-lane fixes.
**Authority:** `tam/AGENTS.md`, `tam/docs/plan-maintenance.md`,
`tam/docs/corpus-audit.md`, `tam/docs/corpus-pedagogy.md` §Paths, and Fred's
direction of 2026-11-13.
**Next action:** On Fred's word, commission the Claude Code audit of corpus
0.82.1 (Phase 3). Phases 4–5 then follow its findings.

## Problem

The corpus is structurally green but not yet trustworthy enough to call
finished.

Measured on `develop` at corpus 0.81.0 / command 0.55.0:

| Measure | Value |
| --- | --- |
| Pages | 614 |
| Paths / steps | 23 / 164 |
| Skill cells | 45, no page or path gaps |
| Anatomy warnings | 0 |
| Components | 251: 198 covered / 53 exempt / 0 gaps |
| Pages with an inbound link | 614 (no orphans) |
| Pages changed since the last independent audit | 290, of which 35 are new |
| Corpus versions shipped since the last audit | 34 |

The last independent audit reviewed corpus 0.75.0 on 2026-11-05. It found
twelve factual defects, six of them High, in the newest pages. Those were
remediated in corpus 0.75.1. Since then the corpus has changed by 290 pages,
including a rewritten path metadata model and 19 new pages. The audit guide
calls for a new audit after about ten new or substantially changed pages.
That trigger is long past, and green coverage counts name mentions, not
correctness.

Separately, six path hubs do not meet the corpus's own path template. The
template in `docs/templates/path.html` and `docs/corpus-pedagogy.md` requires
three to six stages, one to four step links at stated levels, a checkpoint
per stage, a capstone with a folded solution, and next paths. The six hubs
below have fewer than three stages and no capstone:

| Path hub | Step pages it orders | Stages | Checkpoints | Capstone |
| --- | --- | --- | --- | --- |
| `learn/paths/desktop-basics.html` | 13 | 0 | 1 | no |
| `learn/paths/everyday-desktop.html` | 11 | 2 | 2 | no |
| `learn/paths/files-and-apps.html` | 12 | 2 | 2 | no |
| `learn/paths/people-and-security.html` | 9 | 2 | 2 | no |
| `learn/paths/terminal-stack.html` | 5 | 0 | 1 | no |
| `learn/paths/c11-systems.html` | 4 | 0 | 1 | no |

`learn/paths/first-steps.html` labels its stages "1.", "2." and so on rather
than "Stage 1:", so the new check must either accept that style or the page
must be normalized. The 0.81.0 roadmap entry also claims all topic paths kept
their quizzes and capstones; that is not true for the six hubs above.

## Agreed outcome

A reader can follow any of the 23 paths from its hub to a real capstone and
next paths, and every page changed since the last audit has been independently
checked and either fixed or carried with a recorded decision.

Structural conformance is not learner acceptance. This milestone does not
claim that a novice completes the tasks or that an installed candidate works.

## Scope

In scope:

1. Reconcile stale plan and contract records.
2. Rewrite the six path hubs to the path template and normalize
   `first-steps` headings.
3. Add a build-time path contract and tests so the template is enforced.
4. Commission an independent audit of the completed 0.82.0 corpus.
5. Remediate audit findings in severity order and harden the checker where
   the audit identifies a mechanical miss.
6. Close the milestone records.

Out of scope:

- An installer procedure; `tam install plan/apply/verify` still do not exist.
- Stateful `tam learn`, automated assessment, or new `tam` subcommands.
- A broad intermediate/expert content wave; that is the following milestone.
- Mass glossary expansion, localization, or new product promises.
- Learner review or packaged acceptance; both remain developer-directed.

## Phases

### Phase 0 — Reconcile records (no version)

Do this before the content work so the next plan starts from truth.

- Mark `docs/plans/corpus/absolute-beginner-coverage.md` **Completed**. All
  four slices are present at corpus 0.81.0: nine `use/desktop/` pages, three
  `use/files/`, five `use/apps/`, five `use/security/` and `use/people/`, and
  five `use/commands/` pages. Record the implementation evidence.
- Update the plan index row for absolute-beginner coverage, and mark
  `keyboard-modifiers`, `linux-fallback-rescue` and `snapper` as they already
  are.
- In the private backlog, mark the Omarchy-manual coverage work
  **Completed**: its last phase landed, zero component gaps remain, and
  `--check` is wired into `corpus-check`. Update the matching side-work
  entry.
- In `docs/plans/learning-levels.md`, state that source implementation is
  complete and only learner and packaged acceptance remain. Remove the
  obsolete "retire or redirect `learn/paths/*` hubs" follow-up: the 0.81.0
  multi-path engine makes the hubs the path entry points, not a legacy
  duplicate.
- Update the `docs/plans/development.md` header, which still says command
  0.44.0 / corpus 0.45.5, to the current versions.
- Keep the 0.81.0 claim in `ROADMAP.md` honest: either say the capstones were
  preserved except on the six paths completed here, or correct it in the
  0.82.0 row.

Acceptance: no stale status, and every claim names the version and evidence
that supports it.

### Phase 1 — Rewrite the six path hubs (corpus 0.82.0)

Use `docs/templates/path.html` and the Paths section of
`docs/corpus-pedagogy.md` as the contract. For each of the six hubs:

- Write a short opening: who the path helps, prerequisites, safety limits.
- Group the existing step pages into three to six stages. Each stage names
  the pages it covers with `?level=` links and adds one checkpoint quiz.
- Add one capstone: a realistic whole task in a scratch or reversible
  setting, with a folded worked solution and a way to check the result.
- Add next paths.
- Keep the `<meta name="paths">` records unchanged; the pager's step order is
  already correct.

Suggested stage groups, subject to the author's reading of the pages:

| Hub | Likely stages |
| --- | --- |
| `desktop-basics` | Meta key and hotkeys; windows and workspaces; bar and toggles; capture, clipboard and voice |
| `everyday-desktop` | Capture and clipboard; notices and sound; Bluetooth and displays; input, lock and night light |
| `files-and-apps` | Find and inspect files; USB drives and archives; launch and diagnose apps; documents, media and web apps |
| `people-and-security` | Identity and secrets; privileges and trusted updates; keys and hardware authentication; encryption and firewall |
| `terminal-stack` | Terminal capabilities; ncurses and full-screen programs; fonts and text rendering; colour and themes; Lua glue |
| `c11-systems` | C11 language and portability; object files and linking; sanitizers; language choices and trade-offs |

Rules for the prose:

- Summarize and link; do not copy a step page's worked example into the hub.
  The capstone is the only new whole task, and it must use the steps.
- Every Tamlinux claim traces to a product source, a real capture, or a
  public plan. Mark general Linux behavior as general.
- Capstones stay read-only, scratch, or reversible. A privileged capstone is
  not acceptable without Fred's explicit approval and a rollback point.
- Do not use `SUPER`; the desktop modifier is Meta.
- Bump `knowledge/version` to 0.82.0 and update the roadmap header, current
  state row, delivered history row, and the development contract decision
  record in the same change.

Acceptance: each hub matches the template; `make corpus-and-test` passes;
`tam show --pipe learn/paths/<hub>` reads as a complete lesson.

### Phase 2 — Enforce the path contract (tooling, no version bump)

Add structural validation in `tools/build-index.py`, where path metadata and
parent trees are already checked, with tests in `tests/test_build_index.py`.
A `kind=path` page other than `learn/paths/index.html` must have:

1. Three to six stage headings, labelled "Stage N:" or "N.".
2. In each stage, at least one and at most four links to existing pages with
   an explicit `?level=`.
3. In each stage, at least one `details.q` checkpoint with a summary and a
   nonempty answer.
4. A capstone heading containing "Capstone" with one `details.solution`.
5. A "Next paths" heading with at least one internal link.
6. Every step named in its `<meta name="paths">` record exists and is
   ordered; this check already exists and stays.

Use a fixture page in the tests for each failure. Run the check from
`make corpus` so `make corpus-and-test` and `make test` enforce it. Do not
add a corpus version bump for tooling alone.

Acceptance: the six rewritten hubs pass; a fixture with a missing capstone,
a missing stage quiz, or an unlevelled link fails with a named page and
reason; all existing paths pass.

### Phase 3 — Independent audit of corpus 0.82.0 (read-only, no version)

Fred has chosen **Claude Code** as the auditor. It wrote none of the six
rewritten hubs and none of the 0.79–0.81 pages. Four pages in the substantive
change set are its own work and are carved out of this cycle:

- `knowledge/maintain/boot-time.html`
- `knowledge/maintain/energy.html`
- `knowledge/ecosystem/tam-bundle.html`
- `knowledge/learn/glossary/bundle.html`

The report records them as **not audited this cycle** and carries them into the
0.83 audit. The SOP allows one auditor per cycle, so they are not handed to a
second auditor.

Scope: the 62 pages that changed substantively since the 0.75.0 audit, the 31
remaining added pages, all 23 path hubs, and a sample of the 228 pages that
changed only by the `<meta name="paths">` swap. The sample checks that the new
path records are correct on pages whose prose did not change.

Follow `docs/corpus-audit.md`:

- Record corpus 0.82.0, command 0.55.0, and the product commit used as
  authority.
- Run `make corpus-and-test` and record the result; do not repeat its checks.
- Re-check the previous report's A01–A12 findings marked fixed.
- Read every added page except the four carved out, the six rewritten hubs,
  and every path hub at least once. Check commands, bindings, paths, risk
  labels and planned-as-current claims against "Where facts live".
- Pick three recent session records and confirm their `Checked against:`
  sources support the pages they cover.
- Render a sample of pages at each level with `tam show --pipe` and read them
  as a user.
- Report findings as High / Medium / Low with `file:line`, authority, and a
  fix direction. Write only the report under `docs/ai/`.

Acceptance: one committed report, no content fixes in the same commit, a short
re-check table for A01–A12, and the four carved-out pages named as unaudited.

### Phase 4 — Remediate and harden (corpus 0.82.x patches)

Fix findings High to Low in separate slices, each with its own session
record and version bump:

- Verify every corrected instruction against the product source or a real
  capture. Add the environment and date when output varies.
- Correct quizzes in the same change as the page text they test.
- If a finding is a real product defect, record it in `bugs/` and fix it in
  the command lane; do not paper over it in the corpus.
- Use only the recognized risk labels: `read-only`, `scratch`, `user`,
  `system`, `boot`.
- Harden `tools/lint-corpus.py` where the audit shows a mechanical miss:
  reject an unknown `data-risk` value even when no privileged text is
  detected, and recognize package-writing wrappers such as `tam-install-*`
  and `tam-pkg-*` when requiring a system or boot label. Add tests.
- Re-render corrected pages and re-check the named source behavior rather
  than accepting name coverage.

Acceptance: no open High findings; every Medium and Low is fixed or carried
with Fred's ruling; the new linter tests pass.

### Phase 5 — Close the milestone

- Record the final command and corpus versions.
- Add the 0.82.0 row and current-state entry to `ROADMAP.md`.
- Update the plan index with this plan's status and evidence.
- Add the session record and `AI_PROVENANCE.md` row.
- Rerun `make corpus` after the page commits and commit the index follow-up.
- State plainly that learner review and packaged acceptance remain pending.

## Delivered

Phases 0–2 landed on 2026-11-13.

- **Phase 0** in `7216cdb`: absolute-beginner coverage marked Completed with
  evidence; plan-index and learning-levels rows updated; the development
  contract header corrected to command 0.55.0 / corpus 0.81.0; the 0.81.0
  roadmap row corrected to say the six new topic paths were quizzes only at
  that version.
- **Phase 1**: the six hubs rewritten to `docs/templates/path.html` with
  three to five stages, a checkpoint per stage, one reversible capstone with
  a folded solution, and next paths. Step lenses follow each path's starting
  level: novice paths use `?level=1`, `terminal-stack` uses `?level=2` and
  `c11-systems` uses `?level=3`. Command captures in the capstones were run
  on an Arch Linux workstation on 2026-11-13; the `desktop-basics` capstone
  names bindings checked against the packaged session definitions, and it
  says so.
- **Normalization** beyond the six: `first-steps` keeps its "N." headings
  (the check accepts them) but gains a levelled shell step in stage 1;
  `diagnose-from-principles`, `rescue`, `make-it-yours`,
  `session-and-desktop` and `settle-in` gained levelled stage links, and the
  last two split an over-full stage into a new one with its own checkpoint.
  No `<meta name="paths">` record changed.
- **Phase 2**: `validate_path_pages` in `tools/build-index.py` enforces the
  contract from corpus 0.82.0 onward (older fixture corpora are not
  re-checked), and `validate_global_entries` now rejects non-contiguous path
  steps. Stage links must name an existing page with `?level=0-3`; glossary,
  bibliography, `learn/paths/` and `index.html` links are not counted as
  steps. Fixture tests cover each failure mode.
- **Hub-to-step completion (corpus 0.82.1):** the contract passed at 0.82.0
  while 11 hubs still did not link every step in their own
  `index.paths.tsv` record — 32 steps in total. All 12 remaining hubs now
  link every step as a levelled stage link. Missing steps joined an existing
  stage where it had room (at most four links); otherwise a new stage with
  its own checkpoint was added (still at most six stages).
  `understand-design` and `command-line-fluency` were restructured to six and
  five stages so their full step lists are covered. No
  `<meta name="paths">` record changed. Verified by a script that parses each
  hub's stage headings and counts only levelled links to existing pages:
  **0 hubs with unlinked steps**, 164 of 164 steps linked.
- **Still open:** the private backlog reconciliation (plan 26) needs its own
  repository and claim. Phase 3 awaits Fred's word. Learner review and
  packaged acceptance remain pending and are not implied by this milestone.

## Done when

- 23 of 23 path hubs pass the contract; the six named hubs each teach stages,
  checkpoints, a real capstone and next paths.
- `make corpus-and-test` and `make test` pass; 0 skill gaps, 0 anatomy
  warnings, 0 component gaps.
- The independent audit report has no open High findings.
- The path contract fails a deliberately broken fixture and passes the real
  corpus.
- No plan or roadmap claims a capstone, audit result, or learner acceptance
  that does not exist.
- Versions and records agree in `knowledge/version`, `src/version.h`,
  `ROADMAP.md`, the development contract, and the plan index.

## Verification commands

```
make corpus-and-test
make test
python3 tools/report-coverage.py --warnings
python3 tools/report-component-coverage.py --check --quiet
python3 tools/check-repo.py
tests/guide.sh build/tam
```

The audit itself records its own `make corpus-and-test` result; remediation
slices rerun the focused checks and the full suite before their commits.

## Risks and boundaries

- **Audit independence.** The auditor must not have written the 0.81 pages or
  the rewritten hubs. Keep the audit commit report-only.
- **Hub duplication.** Stage prose summarizes; step pages keep the worked
  examples. The capstone is the one new whole task.
- **Capstone safety.** Scratch or reversible only. A privileged capstone
  needs Fred's approval and a recorded rollback point.
- **Lane discipline.** Content and tooling are corpus-lane work; a product
  defect moves to the command lane and its own record.
- **No acceptance by structure.** A green check does not mean a novice
  learned; say so in every record.
- **Index lag.** After page commits, rerun `make corpus` and commit the index
  follow-up; `make install` refuses while either is uncommitted.

## Handoff

Phase 0 and Phase 3 need Fred's word before they start; Phase 0 has landed and
Phase 3 is the next gate. The auditor is chosen: Claude Code, with the
four-page carve-out above.
