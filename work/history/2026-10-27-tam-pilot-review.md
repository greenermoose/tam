# Pilot lesson source review and beginner guide clarity (corpus 0.45.4–0.45.5)

- **Date:** 2026-10-27
- **Time:** 20:38 to 20:46 -04:00
- **Stage:** Develop, source review and corrections
- **CLI Tool:** Codex CLI `0.165.0`
- **Model:** `gpt-6.1-terra`
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record and subsequent index refresh.
- **Authorship:** Fred requested a next priority and directed the work; Codex reviewed and wrote the corrections.
- **Checked against:** `ROADMAP.md`, `docs/plans/learning-levels.md`, `docs/corpus-pedagogy.md`, the First steps path and pilot pages, rendered guide output, `src/discovery.c`, `src/pager_input.c`, `src/pager_draw.c`, Tamlinux application and clipboard bindings, and commands run in an isolated temporary directory.

## Guiding prompts

> take a look at the plans and ROADMAP in the tam repo. What's the most important thing to do next?

> Make it so. Proceed.

> Proceed

> I don't have time to do this. What else can you be working on?

> Proceed on making hte guide easier for a new reader to understand.

## Key decisions

Reviewed the First steps path and the three pilot lessons in source, including all level sections. Place each observed result immediately after its command in permissions and updates examples. Give readers an explicit stop on scratch setup failure. Replace the First steps process-count comparison with a checkpoint about the example actually taught. These corrections do not establish developer, learner, physical-terminal, or packaged acceptance. The review route in `2026-10-27-tam-learning-review.md` remains open.

## Verification

- The permissions, shared-directory, file replacement and red-line count examples ran in an isolated temporary directory and produced the shown results.
- `make corpus-and-test`: corpus lint, 32 lint tests, 20 index tests and guide checks passed.

## Beginner guide follow-up

Clarified how to navigate answers and links, leave and reopen a lesson in the same shell for scratch exercises, and understand the first commands. Simplified shell streams and filesystem descriptions; corrected the search walkthrough against `src/discovery.c`. These source improvements are not a learner trial or packaged acceptance.

Verification: `make corpus-and-test` passed again for corpus 0.45.5 (32 lint tests, 20 index tests and guide checks); the updated documentation and filesystem pages were inspected as rendered text.
