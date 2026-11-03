# D221 page level labels: novice / learner / expert (corpus 0.62.0)

- **Date:** 2026-11-02
- **Time:** ~22:20 -05:00
- **Stage:** Develop. Corpus 0.61.0 to 0.62.0; the command version did not change in this commit.
- **CLI Tool:** Cursor Composer
- **Model:** Composer (Auto)
- **Transcript:** Private Cursor session; claim `cursor-9a07279d4220`.
- **Commits:** `88bd391` on `develop` (corpus 0.62.0).
- **Authorship:** Fred directed Cursor to take over opencode's stalled Slice 1 work and, while in the corpus, rename page difficulty levels to novice / learner / expert (D221). Cursor finished Slice 1 as `1f43879`, coordinated with agy (D221 command/pilot owner), renamed knowledge HTML labels 1:1 (`beginner`→`novice`, `intermediate`→`learner`) except agy's three pilots and `knowledge/index.html`, and paused `enforce_tracks` so new novice/learner anatomy hard-errors do not block the shared checkout until agy gates them to the pilots. Skill cell IDs (`*.beginner` / `*.intermediate`) are unchanged. Full Novice/Learner content anatomy and embedded paths remain agy's pilot scope. Command-lane UI label work remains agy's.

## Guiding prompts

> Take over opencode's tam knowledge corpus gaps work. Fix all pages for novice, learner, and expert instead of beginner, intermediate, and expert. Coordinate with agy.

## Key decisions

- Page `<meta name="level">` and `data-level` use `novice` / `learner` / `expert` via 1:1 rename.
- agy keeps command UI, `build-index.py`, pedagogy docs, `paths.json`, and the three pilot pages.
- `enforce_tracks` is emptied in this change so anatomy regressions become warnings until agy restores enforcement scoped to the pilots (or completes migration).

## Verification

Index generation and corpus unit tests pass against a snapshot that keeps the
three committed pilot pages unchanged (agy's working-tree pilot edits currently
break live `make corpus` / `guide.sh` with a dangling `ownership.html` link and
an unknown bibliography id). Live `make corpus-and-test` in the shared checkout
is blocked on those pilot edits, not on the rename. Pilots and `index.html`
were left for agy.
