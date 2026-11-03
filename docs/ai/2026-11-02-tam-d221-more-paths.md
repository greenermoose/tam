# D221 more-path migration (corpus 0.65.0)

- **Date:** 2026-11-02
- **Time:** ~23:00 -05:00
- **Stage:** Develop. Corpus 0.64.0 to 0.65.0; command version unchanged at 0.48.2.
- **CLI Tool:** Cursor Composer
- **Model:** Composer (Auto)
- **Transcript:** Private Cursor session.
- **Commits:** On `develop` (this change).
- **Authorship:** Fred said "Keep migrating along more paths." Cursor migrated Desktop basics and Healthy diagnostics pages, registered them in `paths.json`, expanded hard-enforcement, bumped the corpus version and synced roadmap records.

## Guiding prompts

> Keep migrating along more paths.

## Key decisions

Two new embedded paths:

- Desktop basics: `desktop/index.html` → `desktop/meta-key.html` → `desktop/workspaces.html`
- Healthy diagnostics: `hardware/networking.html` → `maintain/logs.html` → `learn/documentation.html`

Each page gained Novice conceptual text (no Goal/Try it/quiz), Learner practice
with Start/Back/Next links, and Expert reference where a ladder already existed.
`desktop/meta-key.html` changed from `kind=reference` to `tutorial` so anatomy
enforcement applies. Anatomy hard-errors now cover the earlier pilots, path
neighbors, and these six pages.

## Verification

`make corpus-and-test` passes; learning warnings fall from 118 to 111.
`build-index.py` reports 242 entries. Guide verification and corpus unit tests
pass.
