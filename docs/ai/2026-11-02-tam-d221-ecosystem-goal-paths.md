# D221 ecosystem goal paths (corpus 0.68.0)

- **Date:** 2026-11-02
- **Time:** ~23:25 -05:00
- **Stage:** Develop. Corpus 0.67.0 to 0.68.0; command version unchanged at 0.48.2.
- **CLI Tool:** Cursor Composer
- **Model:** Composer (Auto)
- **Transcript:** Private Cursor session.
- **Commits:** Pending on `develop` at record write time.
- **Authorship:** Fred directed five ecosystem goal-path batches in order. Cursor migrated twenty pages, registered the paths in `paths.json`, expanded hard-enforcement, bumped the corpus version and synced roadmap records.

## Guiding prompts

> Do ecosystem goal paths, then gates/machines, cables, display/GPU and then how software runs.

## Key decisions

Five embedded paths matching existing learn/paths curricula:

- From atoms to bits: matter-and-energy → circuits-and-power → digital-logic
- From gates to machine code: computer-architecture → instruction-cycle → assembly
- Cables and protocols: serial-and-parallel → cables-and-signaling → communication-protocols → error-detection-and-correction → hardware-diagnostics (circuits remains on the atoms path)
- Display and GPU: computer-graphics → gpu-architecture → display-cables → display-history → compositors
- How software runs: kernel-and-processes → filesystems → init → package-versions

## Verification

`make corpus-and-test` passes; learning warnings fall from 74 to 34.
`build-index.py` reports 242 entries. Guide verification and corpus unit tests pass.
