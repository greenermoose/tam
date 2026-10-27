# Safe practice cleanup (corpus 0.44.1)

- **Date:** 2026-10-27
- **Time:** 09:50 to 09:56 -04:00
- **Stage:** Develop; corpus source
- **CLI Tool:** Codex CLI 0.164.0
- **Model:** GPT-6.1 Terra (`gpt-6.1-terra`)
- **Transcript:** Retained privately by the author.
- **Commits:** The commit that adds this record and the index follow-up.
- **Authorship:** Fred requested the highest-priority project; Codex selected and made this learning-plan slice.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, local `mktemp(1)`, `rm(1)` and `rmdir(1)`.

## Guiding prompt

> Take a look at the work plans in the tam repo. Take the highest priority project and do it.

## Key decisions

Use the learning plan's identified practice correction. Guard against temporary-directory creation failure, remove the known file, and remove the directory only when empty. Explain the terms and cleanup boundary in context.

## Verification

Ran the scratch example and inspected the rendered page. `make corpus-and-test` and `make test` passed. Live Ollama checks were skipped because no local server was available. The first full run exposed stale roadmap version references, corrected before the passing rerun.
