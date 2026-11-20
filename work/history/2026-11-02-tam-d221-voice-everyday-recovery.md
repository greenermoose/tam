# D221 voice, everyday desktop, and recovery/boot paths (corpus 0.66.0)

- **Date:** 2026-11-02
- **Time:** ~23:15 -05:00
- **Stage:** Develop. Corpus 0.65.0 to 0.66.0; command version unchanged at 0.48.2.
- **CLI Tool:** Cursor Composer
- **Model:** Composer (Auto)
- **Transcript:** Private Cursor session.
- **Commits:** `5938852` (pages) and `d9fbf2e` (indexes) on `develop`.
- **Authorship:** Fred directed three batches in order: desktop voice, everyday desktop tasks, then recovery/boot. Cursor migrated the pages, registered three paths in `paths.json`, expanded hard-enforcement, bumped the corpus version and synced roadmap records.

## Guiding prompts

> Do desktop voice, then everyday desktop tasks, and then recovery / boot.

## Key decisions

Three new embedded paths:

- Desktop voice: `desktop/voice.html` → `desktop/voice-commands.html`
- Everyday desktop tasks: nine `use/desktop/*` pages in the Using hub order
- Recovery and boot: `maintain/recovery` → `power` → `boot` → `boot-recovery` → `snapper` → `checkpoint`

Each page gained Novice conceptual text and Learner practice with Start/Back/Next
links. Anatomy hard-errors now cover these pages in addition to earlier D221
migrations.

## Verification

`make corpus-and-test` passes; learning warnings fall from 111 to 88.
`build-index.py` reports 242 entries. Guide verification and corpus unit tests
pass (non-pilot fixture updated from `desktop/voice.html` to `ecosystem/ai.html`).
