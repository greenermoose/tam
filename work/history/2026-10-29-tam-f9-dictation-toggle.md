# F9 voice dictation toggle update (corpus 0.48.1)

- **Date:** 2026-10-29
- **Time:** 19:55 to 20:05 -04:00
- **Stage:** Develop; corpus 0.48.1
- **CLI Tool:** Antigravity CLI 1.4.10
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `336e9d3`, and the commit that adds this record and refreshed index revisions
- **Authorship:** Fred Horch directed updating the tam corpus to reflect how F9 works now (tap to start, tap to stop dictation instead of hold to speak). Antigravity updated the voice dictation, desktop orientation, and shell voice commands knowledge pages, bumped the corpus version to 0.48.1, and refreshed the corpus indexes.
- **Checked against:** `docs/corpus-authoring.md`, `ROADMAP.md`, `knowledge/desktop/voice.html`, `knowledge/desktop/index.html`, `knowledge/desktop/voice-commands.html`

## Guiding prompts

> Update the tam corpus to reflect how F9 works now. It is no longer hold to speak. It is now tap to start, tap to stop dictation.

## Key decisions

- Updated `knowledge/desktop/voice.html` to describe F9 as a toggle key (tap to start, tap to stop and transcribe) across the summary, introductory bullets, first-time aside, worked example, explanation, try-it exercise, and key reference table.
- Updated `knowledge/desktop/index.html` key reference table to show F9 as `Voice dictation (tap start/stop)`.
- Updated `knowledge/desktop/voice-commands.html` comparison table and cross-reference to align with F9 toggle behavior.
- Bumped corpus version to 0.48.1 in `knowledge/version` and `ROADMAP.md`.

## Verification

- `make corpus` built index and companion files without errors (132 entries).
- `tools/lint-corpus.py` and `tests/test_lint_corpus.py` passed with 0 findings.
- Rendered output checked via `build/tam show --pipe` with `TAM_KNOWLEDGE_DIR` for `desktop/voice`, `desktop/index`, and `desktop/voice-commands`.
