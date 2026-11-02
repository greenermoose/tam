# Tam corpus gap remediation (in progress toward 0.60.0)

- **Date:** 2026-11-02
- **Time:** 17:30 to 18:10 -05:00
- **Stage:** Develop — corpus content under the approved gap remediation plan
- **CLI Tool:** Cursor Agent
- **Model:** Composer
- **Transcript:** Retained privately by the author.
- **Commits:** `613496b` (content); index/version bump still pending on freed claims
- **Authorship:** Developer approved the remediation plan; the agent authored
  the audit record and corpus page edits.
- **Checked against:** `bindings/utilities.lua`, `bindings/voxtype.lua`,
  `tam --version` (KBN WB 0.6.1 / command 0.48.1), 
  `tamlinux-packages/docs/console-startup.md`, `docs/corpus-authoring.md`,
  `docs/corpus-pedagogy.md`, prior audit 2026-10-16

## Guiding prompts

> Implement the plan as specified, it is attached for your reference.

## Key decisions

- Install track left as an honest stub.
- Quantum depth limited to solid-state semiconductor literacy
  (`matter-and-energy` + glossary terms).
- Index/`knowledge/version` bump deferred while a neighbouring session holds
  those paths; content commits proceed without them.

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

## Verification

- Spot-checked bindings against product Lua.
- `make corpus` / `make corpus-and-test` pending claim on `knowledge/version`
  and primary index files held by another session.

## Follow-up

- Claim version + primary indexes, bump corpus to 0.60.0, run
  `make corpus-and-test`, commit sidecars and parents, add AI_PROVENANCE and
  ROADMAP rows when `agy-e86705208bac` releases those paths.
- Validated offline: `build-index.py` and `lint-corpus.py` against a temp
  tree with 231 entries and no page/path gaps for the new `signals` cells.
- Prepared index outputs live under `/tmp/tam-index-handoff/` for that follow-up.
