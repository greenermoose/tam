# Pilot lesson source review (corpus 0.45.4)

- **Date:** 2026-10-27
- **Time:** 20:38 to 20:41 -04:00
- **Stage:** Develop, source review and corrections
- **CLI Tool:** Codex CLI `0.165.0`
- **Model:** `gpt-6.1-terra`
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record and subsequent index refresh.
- **Authorship:** Fred requested a next priority and directed the work; Codex reviewed and wrote the corrections.
- **Checked against:** `ROADMAP.md`, `docs/plans/learning-levels.md`, `docs/corpus-pedagogy.md`, the First steps path and pilot pages, rendered guide output, and commands run in an isolated temporary directory.

## Guiding prompts

> take a look at the plans and ROADMAP in the tam repo. What's the most important thing to do next?

> Make it so. Proceed.

## Key decisions

Reviewed the First steps path and the three pilot lessons in source, including all level sections. Place each observed result immediately after its command in permissions and updates examples. Give readers an explicit stop on scratch setup failure. Replace the First steps process-count comparison with a checkpoint about the example actually taught. These corrections do not establish developer, learner, physical-terminal, or packaged acceptance. The review route in `2026-10-27-tam-learning-review.md` remains open.

## Verification

- The permissions, shared-directory, file replacement and red-line count examples ran in an isolated temporary directory and produced the shown results.
- `make corpus-and-test`: corpus lint, 32 lint tests, 20 index tests and guide checks passed.
