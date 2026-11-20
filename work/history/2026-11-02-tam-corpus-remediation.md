# Tam corpus gap remediation (corpus 0.60.0)

- **Date:** 2026-11-02
- **Time:** 17:30 to 20:25 -05:00
- **Stage:** Develop — corpus 0.60.0 content and indexes
- **CLI Tool:** Cursor Agent
- **Model:** Composer
- **Transcript:** Retained privately by the author.
- **Commits:** `613496b` (content), `a8a220a` (pending-note), plus this
  index/version commit
- **Authorship:** Developer approved the remediation plan; the agent authored
  the audit record, corpus page edits, and the 0.60.0 index bump.
- **Checked against:** `bindings/utilities.lua`, `bindings/voxtype.lua`,
  `tam --version` (KBN WB 0.6.1 / command 0.48.1),
  `tamlinux-packages/docs/console-startup.md`, `docs/corpus-authoring.md`,
  `docs/corpus-pedagogy.md`, prior audit 2026-10-16

## Guiding prompts

> Implement the plan as specified, it is attached for your reference.
>
> Is agy playing nice with you?
>
> How about now?

## Key decisions

- Install track left as an honest stub.
- Quantum depth limited to solid-state semiconductor literacy
  (`matter-and-energy` + glossary terms).
- Index/`knowledge/version` bump waited until neighbouring claim
  `agy-e86705208bac` released shared index paths.

## What landed

- Audit: `docs/ai/2026-11-02-tam-corpus-audit.md`; closed Meta corpus note.
- Phase 1: Apps menu Ctrl+F12; F9 push-to-talk; console-first login centering;
  cultivar examples refreshed; Omarchy independence wording; first-steps
  navigation/files stage; path skill links; rootstock boilerplate shortened.
- Phase 2: P0 glossary terms; `signals` skill domain; pedagogy catalog updated.
- Phase 3: `matter-and-energy`; prereq order fixed; atoms/gates/display paths.
- Phase 4: `kernel-and-processes`, `filesystems`; init/compositor ladders;
  how-software-runs and session-and-desktop paths; understand-design extended.
- Phase 5: `use/recipes/*`, `maintain/symptoms/*`.
- Phase 6: diagnose-from-principles; use/maintain FAQs; thickened kernel/shell/
  package/process glossary; settle-in and make-it-yours enrichment.
- Corpus version 0.60.0 with regenerated indexes (231 entries).

## Verification

- `make corpus`: Generated index.tsv and five companion outputs with 231 entries.
- `make corpus-and-test`: passed after the 0.60.0 bump.
