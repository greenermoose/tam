# Corpus 0.82: trustworthy paths (corpus 0.82.0)

- **Date:** 2026-11-13
- **Time:** 06:11 to 06:52, then 20:14 for the record commit, -05:00
- **Stage:** Develop, corpus 0.82.0 (minor) plus the path-contract tooling and
  reconciled records. No Test, Run or Release step.
- **CLI Tool:** opencode v2.2.5
- **Model:** Big Pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `7216cdb`, `af18dcf`, `5a503a3`, and the commit that adds this
  record
- **Authorship:** Fred asked for the next corpus milestone, chose Claude Code
  as the independent auditor, and chose a one-page sample of the
  metadata-only changes. The AI tool wrote the six rewritten path hubs, the
  hub normalization, the path-contract validator and its tests, and the
  record updates; Fred set the audit scope and gated Phase 3.
- **Checked against:** `docs/templates/path.html`, `docs/corpus-pedagogy.md`
  §Paths, `docs/corpus-audit.md`, `knowledge/index.paths.tsv`,
  `tools/build-index.py`, `tools/corpus_pipeline.py`, the `tamlinux`
  repository's `desktop/session/hypr/tamlinux/bindings/` (`tiling.lua`,
  `utilities.lua`, `clipboard.lua`, `applications.lua`), and command output
  from this Arch Linux workstation captured 2026-11-13.

## Guiding prompts

> Plan the next milestone for the tam knowledge corpus.

> Explain what choices I have for independent auditor

> You seem to be re-doing a lot of work to answer a simple question. Are you going down rabbit holes or are you really just answering my question?

> I choose claude code as the independent auditor.

> 1 sample

> go ahead

## Key decisions

- **Milestone.** Corpus 0.82.0 is "Trustworthy paths": reconcile records,
  rewrite the six thin topic paths, enforce the path contract in the build,
  then commission an independent audit. The audit needs its own word and is
  not part of this Develop step.
- **Auditor.** Claude Code was chosen because it has the smallest overlap
  with the changed pages: four of the 62 substantively changed pages are its
  work, so those four are carved out and carried to the next audit instead
  of handing the cycle to a second auditor.
- **Contract scope.** The check counts only stage links that name an existing
  page with `?level=0-3`; glossary, bibliography, `learn/paths/` and
  `index.html` links are not steps. A stage link without a lens is an error.
  Both "Stage N:" and "N." headings are accepted, so `first-steps` keeps its
  numbering.
- **Version gate.** `validate_path_pages` runs from corpus 0.82.0 onward so
  legacy fixture corpora are not re-checked. Step contiguity is enforced for
  every corpus because it is metadata, not page markup.
- **No new step records.** The six rewrites keep each page's existing
  `<meta name="paths">` step order; only the hub prose and lenses changed.
- **Audit sample.** Fred chose one page for the metadata-only sample rather
  than a percentage.
- **Not acceptance.** Green structural checks do not mean a novice learned
  the paths or that an installed candidate works. Learner review and
  packaged acceptance remain pending.

## Verification

- `make corpus-check`: 614 pages, 614 parsed, 614 rendered; checks passed;
  0 component gaps.
- `make corpus-and-test`: corpus checks, component coverage, 85 corpus
  tooling unit tests, and `check-repo.py` all pass.
- `make test`: both suites pass, including the command integration,
  upgrade and renderer goldens.
- `python3 tools/report-coverage.py --warnings`: 45 skill cells, 0 anatomy
  warnings, 0 skill gaps.
- `python3 tools/report-component-coverage.py --check --quiet`: exit 0.
- `tests/guide.sh build/tam`: all guide tests passed.
- All 23 path hubs pass `validate_path_pages`; four fixture tests cover a
  conforming page, the pre-0.82 skip, thirteen broken shapes, and
  non-contiguous steps.
