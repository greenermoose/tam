# D221 path-neighbor migration (corpus 0.64.0)

- **Date:** 2026-11-02
- **Time:** ~22:50 -05:00
- **Stage:** Develop. Corpus 0.63.0 to 0.64.0; command version unchanged at 0.48.2.
- **CLI Tool:** Cursor Composer
- **Model:** Composer (Auto)
- **Transcript:** Private Cursor session.
- **Commits:** Corpus 0.64.0 on `develop` (hash filled after commit).
- **Authorship:** Fred accepted the three D221 pilots and said "Proceed." Cursor migrated the path-neighbor pages on the three embedded paths, expanded hard-enforcement to those pages, bumped the corpus version and synced roadmap records.

## Guiding prompts

> The pilot pages look great. Proceed.

## Key decisions

Migrate the pages already linked from the accepted pilots and `paths.json`:

- Mastering the Command Line: `shell/index.html`, `shell/jobs.html`
- Securing Files and Ownership: `filesystem/index.html`
- Maintaining Tamlinux: `maintain/index.html`, `maintain/rollback.html`

Each gained Novice conceptual text (no Goal/Try it), Learner practice with
embedded Start/Back/Next links, and Expert reference where a ladder already
existed. Anatomy hard-errors now cover the pilots plus these neighbors.

## Verification

`make corpus-and-test` passes; learning warnings fall from 124 to 118.
