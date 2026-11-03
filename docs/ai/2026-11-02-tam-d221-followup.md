# D221 follow-up: pilot anatomy gate and corpus 0.63.0

- **Date:** 2026-11-02
- **Time:** ~22:40 -05:00
- **Stage:** Develop. Command already 0.48.2 from agy; corpus 0.62.0 to 0.63.0.
- **CLI Tool:** Cursor Composer
- **Model:** Composer (Auto)
- **Transcript:** Private Cursor session; claim `cursor-77a594e52b00`.
- **Commits:** Agy D221 command/pilot `c4dd503`; Cursor corpus 0.63.0 follow-up on `develop` (see `git log -1`).
- **Authorship:** Fred directed Cursor to finish after agy released the `tam` checkout. Agy delivered command UI labels, pedagogy/templates, the three pilot pages and `paths.json` (`c4dd503`). Cursor gated novice/learner anatomy hard-errors to those pilots, restored `enforce_tracks`, set the guide root to `novice`, bumped corpus to 0.63.0, synced ROADMAP/development/provenance, and regenerated indexes.

## Guiding prompts

> agy is done working in the tam repo. It's all you now.

## Key decisions

- New novice/learner anatomy errors apply only to
  `shell/pipelines.html`, `filesystem/permissions.html` and
  `maintain/updates.html`; other pages keep warnings until migrated.
- Legacy beginner/intermediate anatomy again follows `enforce_tracks`.
- Guide root `knowledge/index.html` uses `novice`.

## Verification

`make corpus-and-test` and `python3 -m unittest tests.test_build_index`.
Command integration tests need a real PTY; run outside the agent sandbox when
confirming 0.48.2 interactively.
