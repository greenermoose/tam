# tam checklist item identifiers

Checklist items in this repository use permanent, repository-wide `TCK`
identifiers ("tam command and corpus"). A plan's implementation slices are
checklist items. The identifier belongs to the outcome, not to a plan, version
or wording, so it can be cited alone: `TCK2`, not "slice 2 of the knowledge
plan". Keep item text in the plan that owns it; do not maintain a second item
registry here.

## Rules

- Mint top-level IDs in sequence (`TCK1`, `TCK2`, ...) across all plans; never
  restart per plan, reuse or renumber an ID. **Next free: TCK9.** Update this
  counter in the same change that adds a new top-level item.
- Mint IDs only while holding a `tam-work` claim on this file and the plan
  being changed. Two sessions minting at once then conflict on the counter
  line in Git instead of sharing a number.
- Keep each item to one verifiable outcome, with a short name and stated
  completion evidence. Put detailed procedure in the plan body.
- When an outcome is clarified without changing what counts as done, keep its
  ID. If its done condition changes after evidence exists, record
  `revised YYYY-MM-DD: <reason>` on the item; earlier evidence no longer
  counts.
- For a split, retain the parent as a rollup and give children dotted IDs
  (`TCK4.1`, `TCK4.2`). Children cover the parent's full scope without
  overlap, and the parent completes only when every child does. One further
  level is allowed; deeper breakdown belongs in the plan body.
- For a merge, the older ID survives; mark the other `merged into TCK<n>`.
  When a plan is superseded, its open items keep their IDs in the replacement
  plan. For a drop, retain and strike through the item, then mark it
  `dropped: <reason>`.
- Do not create other letter-number series (`K1`, `S1`, `M1`) in this
  repository's plans; such labels collide with identifiers used elsewhere in
  Tamlinux.

On first mention in a message, commit or record, give the name too:
`TCK2 (shared reader navigation)`; use `TCK2` after that. Put the ID in the
subject of a commit that completes an item, so `git log --grep 'TCK2\b'` gives
its history. Find an item with `rg -n 'TCK2\b' docs/plans ROADMAP.md`.
Dictated variants (`TCK 2`, `T.C.K. 2`, `tee see kay two`) mean `TCK2`.

The knowledge-system plan's items were labelled `K1`-`K5` in commit `cbb6d04`
and renamed to `TCK1`-`TCK5` on 2026-10-18; older records may use the old
labels.
