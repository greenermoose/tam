# tam plan revision for help and self-inspection (milestones 6–9)

- **Date:** 2026-10-12
- **Stage:** Develop — plan revision; no code change and no version bump.
- **CLI Tool:** opencode 1.19.0
- **Model:** `big-pickle`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred asked for the `tam` plan to be revised after reading the 2026-10-12 help and self-inspection research and requirements. OpenCode revised `docs/plans/development.md` (corrected milestone status, added milestones 6–9, the offline/`ask` boundary, excluded work and acceptance evidence), marked the superseded parts of `docs/plans/explain-topic-scaling.md`, and refreshed the README stage line. No source, tests or version were changed.

## Guiding prompts

> Please revise the tam plan as you see fit to do so.

## Decisions and scope

- **Stale status corrected.** The stage header and milestone 5 now record that
  milestones 1–5 are implemented (0.9.1), matching the decision record; milestone
  5 is no longer labelled "planned".
- **Milestones 6–9 added**, from the 2026-10-12 help and self-inspection
  requirements:
  6. corpus index replacing the hand-written keyword table, plus `tam search
     [--json] TERM...` as the single search verb, with the alias → field-weighted
     score → substring retrieval and the shown (never auto-applied) fuzzy miss;
  7. the content-depth ladder with Beginner/Intermediate/Expert fading templates;
  8. a read-only self-inspection findings layer and versioned check contract;
  9. an opt-in, grounded `ask` surface.
- **Offline identity amended explicitly.** A new section records that `tam`
  stays offline by default and that `ask` is the one deliberate, opt-in
  exception: grounded and cited, talks only to an already-running local server,
  never starts or bundles a daemon or model, and fetches only under interactive
  consent with a verified hash.
- **Supersession reconciled.** `tam search` supersedes the `tam explain search`
  spellings in the scaling plan, and the scaling plan's Phase 5 (corpus-driven
  metadata) is promoted to milestone 6; the scaling plan carries a revision
  banner.
- **Excluded work recorded.** Installer `plan`/`apply`/`verify`, the stateful
  progression UI, the `why`/depth axes and computer-science track, fuzzy
  auto-redirect, compound shell parsing, `tam ai`, writable settings and
  localisation are explicitly excluded or deferred.
- **No version bump.** The command version changes only when behaviour changes
  (`src/version.h`); this is a plan-only change.

## Verification

- Markdown only; no build, test or version change to run.
- Cross-checked against `docs/plans/development.md` decision record, the
  supersession note, README stage line, and `AI_PROVENANCE.md`.
- The underlying design decisions are `proposed` in the private research until
  Fred accepts them; this revision is a public projection for his review.
