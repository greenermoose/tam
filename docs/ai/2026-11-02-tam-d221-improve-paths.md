# D221 improve-track paths (corpus 0.67.0)

- **Date:** 2026-11-02
- **Time:** ~23:20 -05:00
- **Stage:** Develop. Corpus 0.66.0 to 0.67.0; command version unchanged at 0.48.2.
- **CLI Tool:** Cursor Composer
- **Model:** Composer (Auto)
- **Transcript:** Private Cursor session.
- **Commits:** `3ee4333` (pages) and `a22eff5` (indexes) on `develop`.
- **Authorship:** Fred said to proceed with the next logical grouping. Cursor migrated the Improving track plus `hardware/resources`, registered Run lean and Improve your tools in `paths.json`, expanded hard-enforcement, bumped the corpus version and synced roadmap records.

## Guiding prompts

> Proceed with the next logical grouping.

## Key decisions

After use/maintain/desktop paths, the next coherent batch is the Improving track
and its resource prerequisite (matching the existing Run lean goal path):

- Run lean: `hardware/resources.html` → `improve/efficiency.html` → `improve/zram.html`
- Improve your tools: `improve/terminal.html` → `scripting` → `toolchains` → `ai-performance`

Remaining warnings are mostly deep `ecosystem/` pages plus a few learn/man leftovers.

## Verification

`make corpus-and-test` passes; learning warnings fall from 88 to 74.
`build-index.py` reports 242 entries. Guide verification and corpus unit tests pass.
