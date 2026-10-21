# Banner links related pages by parent (command 0.37.1)

- **Date:** 2026-10-21
- **Time:** 06:15 to 06:23 -04:00 (last transcript event)
- **Stage:** Develop. Command 0.37.1 (patch); corpus unchanged.
- **CLI Tool:** Antigravity CLI 1.4.4
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer asked the AI tool to pick up the learning
  levels plan from the previous session's handoff, which stopped at the
  banner-ranking question. He accepted the recommendation recorded in the
  plan. The AI tool implemented it, added a fixture parent projection and
  tests, and updated the man page, roadmap, decision record and plan. In the
  `tamlinux` repository it also updated the pending `welcome_seen` path.
- **Checked against:** `docs/plans/learning-levels.md` (TCK21 delivery
  note), `src/tam.c` (`related_at_level()`), `src/index.h` and `src/index.c`
  (`load_parent_projection()`), `knowledge/index.parents.tsv`,
  `tests/fixtures/guide/index.tsv`, `tests/run.sh`, `man/tam.1`

## Guiding prompts

> Find the handoff about the tam repo project to improve learning levels and pick up from where the last claude session left off.

> I accept the recommendation.

## Key decisions

- Relatives rank children, then siblings, then the parent. Within each
  group the old order applies (shared prerequisites, then title), with at
  most three links. Track no longer matters.
- The fixture guide gained `index.parents.tsv` so the command tests cover
  a child, a sibling, and the level-path fallback.
- On the real corpus, the welcome page at Intermediate now links its
  sibling "Intermediate path" instead of unrelated reference pages.

## Verification

The session's final `make test` failed in the new interactive banner assertion:
the PTY helper left a character-set escape's `B` in the captured link title.
The implementation and documents remained uncommitted when the session ended.
Completion and fresh verification are recorded in the
[continuation record](2026-10-21-tam-banner-completion.md).
The session also spot-checked the checkout corpus: `welcome` and
`about` at Intermediate and `learn/index` at Expert link the matching level
path page as a relative; `use/index` at Expert links its child pages.
