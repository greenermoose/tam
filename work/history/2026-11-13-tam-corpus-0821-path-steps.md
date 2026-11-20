# Corpus 0.82.1: every indexed path step linked from its hub (corpus 0.82.1)

- **Date:** 2026-11-13
- **Time:** 20:20 to 20:45, -05:00
- **Stage:** Develop, corpus 0.82.1 (patch). No Test, Run or Release step.
- **CLI Tool:** opencode v2.2.5
- **Model:** Big Pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `19f3883`, `30e9061`, and the commit that adds this record
- **Authorship:** Fred asked what remained after the 0.82.0 milestone and
  chose to close the hub-to-step gap before the independent audit. The AI
  tool audited the 23 hubs against `index.paths.tsv`, edited the 12 hubs
  that did not link every step, bumped the corpus version, and updated the
  roadmap, plan, development contract, plan index, session record and
  provenance row. Fred did not approve a stage move.
- **Checked against:** `knowledge/index.paths.tsv` (the 164 `S` step
  records), `docs/templates/path.html`, `docs/corpus-pedagogy.md` §Paths,
  `tools/build-index.py` (`validate_path_pages`), and reader output from
  `build/tam show --pipe learn/paths/command-line-fluency`.

## Guiding prompts

> What else do you want to do?

> Yes, start on 1.

## Key decisions

- **Close the gap before the audit.** Corpus 0.82.0 made the path contract
  pass, but 11 hubs still did not link every step in their own record — 32
  steps, with `first-steps` adding one more that only appeared elsewhere.
  The pager still tracked those steps, so the path worked, but the hub a
  reader starts from did not show them. Fixing that is content work, not a
  new capability, so it is a patch release ahead of Phase 3.
- **Placement rule.** A missing step joins an existing stage when that stage
  has room (at most four step links). Otherwise a new stage with its own
  checkpoint is added, keeping the hub within six stages.
  `understand-design` and `command-line-fluency` were restructured to six
  and five stages because nearly all their steps were unlinked.
- **Records unchanged.** No `<meta name="paths">` record changed; only hub
  prose, stage structure and lenses. The step order in
  `index.paths.tsv` is still the contract.
- **No acceptance by structure.** The audit, learner review and packaged
  acceptance are not implied by a green check and remain pending.

## Verification

- A read-only script that parses each hub's `<h2>` stage headings and counts
  only links to existing pages with `?level=0-3`: **0 hubs with unlinked
  steps**, 164 of 164 steps linked.
- `make corpus-check`: 614 pages, checks passed, 0 component gaps.
- `make corpus-and-test`: corpus checks, component coverage, 85 corpus
  tooling unit tests and `check-repo.py` all pass.
- `make test`: both suites pass, including command integration, upgrade and
  renderer goldens.
- `python3 tools/report-coverage.py --warnings`: 45 skill cells, 0 anatomy
  warnings.
- `python3 tools/report-component-coverage.py --check --quiet`: exit 0.
- `tests/guide.sh build/tam`: all guide tests passed.
- `build/tam show --pipe learn/paths/command-line-fluency`: renders the five
  stages with their steps and checkpoints.
