# TCK25 Improving track completion and enforcement (corpus 0.43.8)

- **Date:** 2026-10-25
- **Time:** 21:15 to 23:25 -04:00
- **Stage:** Develop; final Improving lesson migrated, track-wide enforcement enabled, command unchanged at 0.43.2.
- **CLI Tool:** Antigravity CLI 1.4.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** Implementation commit and the generated-index follow-up containing this line.
- **Authorship:** The developer directed continuing TCK25 Improving track migration with the final Improving lesson: `tam.html`, claiming scopes first under `AGENTS.md`, and enabling Improving track build enforcement in `knowledge/learn/skills/skills.json`. Antigravity CLI claimed the scopes via `tam-work begin`, migrated `knowledge/tam.html` to the full ladder anatomy with captured read-only examples, mapped skills (`contributing.beginner`, `contributing.intermediate`, `contributing.expert`), enabled `"improve"` in `enforce_tracks`, reduced corpus-wide anatomy warnings from 6 to 4, and updated plan and roadmap records.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, `tools/build-index.py`, `tools/lint-corpus.py`, `knowledge/learn/skills/skills.json`; live captured `./build/tam --version`, `make command`, `make test-command`, `make -n command`, `./build/tam upgrade`, `ldd build/tam`, and `time ./build/tam --version` outputs; [make(1)](https://man7.org/linux/man-pages/man1/make.1.html), [gcc(1)](https://gcc.gnu.org/onlinedocs/), [ldd(1)](https://man7.org/linux/man-pages/man1/ldd.1.html).

## Guiding prompts

> In the tam repo, we are completing checklist item TCK25. Improving track slices 1–5 landed in corpus 0.43.3, 0.43.4, 0.43.5, and 0.43.6. Continue TCK25 Improving track migration with the final Improving lesson: tam.html. Follow tam/AGENTS.md and claim scopes before editing. Once tam.html is migrated, enable Improving track build enforcement in knowledge/learn/skills/skills.json.

## Key decisions

- Use a corpus patch bump (0.43.8) because this slice restructures and migrates the existing `tam.html` lesson and enables track enforcement without introducing new guide pages or command changes.
- `knowledge/tam.html` was migrated to the full ladder anatomy with captured read-only `./build/tam --version` and `make command` compilation output, single-lane test verification (`make test-command`), local user development prefix installation (`PREFIX=$HOME/.local/dev`), dry-run inspection, `tam upgrade` stage comparison, dynamic library inspection via `ldd`, transactional development generations (`~/.local/dev/opt/tam/generations/`), C11 architectural principles, decoupled versioning discovery, and four-phase lifecycle discipline (`develop -> test -> run -> release`). Mapped to `contributing.beginner`, `contributing.intermediate`, and `contributing.expert`.
- `"improve"` was added to `"enforce_tracks"` in `knowledge/learn/skills/skills.json`, promoting anatomy and quiz warnings across Using, Maintaining, and Improving to fatal build errors. The Improving track has zero anatomy warnings and zero gaps.
- Whole-corpus anatomy warnings dropped from 6 to 4 (all 4 remaining warnings are in unmigrated lessons in the Learning track: `ecosystem/sddm.html` and `ecosystem/ttys-and-virtual-consoles.html`). Unclassified pages dropped from 37 to 36; unmapped pages dropped from 77 to 76.

## Verification

- `make corpus`: cleanly generated index and sidecars with 110 entries.
- `python3 tools/lint-corpus.py`: zero findings or errors.
- `python3 -m unittest tests/test_lint_corpus.py tests/test_build_index.py`: 52 tests passed (OK).
- `sh tests/guide.sh`: all 12 guide test assertions passed cleanly.
- `make test-command`: all CLI and pager tests in `tests/run.sh` passed cleanly.
- `make check-repo`: passes cleanly with updated version headers in `ROADMAP.md`.
- `coverage.json`: confirmed `enforced_tracks: ["use", "maintain", "improve"]`, warnings dropped from 6 to 4, unclassified pages fell from 37 to 36, and unmapped pages fell from 77 to 76.
- All captured outputs verified against live host command outputs.

Pilot/learner review and physical Linux-console/Foot acceptance of an exact packaged candidate remain unverified. No package deployment or stage promotion was performed.
