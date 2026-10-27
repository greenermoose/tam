# Learning review preparation (corpus 0.45.1)

- **Date:** 2026-10-27
- **Time:** 14:44 to 14:47 -04:00
- **Stage:** Develop, source corrections and review preparation
- **CLI Tool:** Codex CLI `0.164.0`
- **Model:** `gpt-6.1-terra`
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record and the subsequent index refresh.
- **Authorship:** Fred requested the review; Codex reviewed the path and wrote the corrections.
- **Checked against:** `docs/plans/learning-levels.md`, `docs/corpus-pedagogy.md`, `knowledge/coverage.json`, `src/tam.c`, the Tamlinux Hyprland application and clipboard bindings, and the captured capstone commands.

## Guiding prompts

> take a look at the tam repo. What is the most important project to tackle next?

> Make it so.

> Prepare the route for later

## Key decisions

The Welcome page had lost the path links required by the approved pilot and its capstone. Restore them, explain how to return to the shell, and put each captured output immediately after its command. Replace stale skills-map migration notices with mapped reading links; a mapping is not proof of mastery. Networking still has no mapped lessons. Source review does not substitute for a developer or learner review.

## Review route for later

1. Open Welcome and follow First steps. Walk through every stage and checkpoint, including the scratch capstone and the return to Welcome. Note where instructions or terms are unclear and where the actual result differs.
2. Read Pipelines, File permissions, and Updates at each lens (0, 1, 2 and 3). Check navigation, level filtering, examples, folded answers, and whether the level teaches what it promises. Record any changes to the teaching anatomy or templates.
3. Invite a novice to try First steps and an experienced reader to try the pilot pages, thinking aloud without coaching. Record where each gets stuck. Use that evidence to decide whether the three levels need reconsideration.
4. On a separately selected and authorized packaged candidate, repeat representative navigation on a Linux console and Foot; check external-link confirmation, cancellation, browser handoff and return in desktop and console. Record the exact candidate and results separately.

Developer review, learner review, physical desktop/console review and packaged acceptance are pending.

## Verification

- Capstone commands run in a temporary directory: the count is 2 and the mode is 600.
- `make corpus-and-test`: corpus lint, index tests and guide checks passed.
