# Knowledge authoring page (corpus 0.77.0)

- **Date:** 2026-11-09
- **Time:** 18:44 to 19:00 -05:00
- **Stage:** Develop
- **CLI Tool:** Antigravity CLI `1.5.3`
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred asked for an explanation of how the corpus hides and shows sections (retrieval practice, reader level lenses, quiz folds, and session blocks) and directed that the explanation of how to write knowledge pages become a page in the corpus. Antigravity authored `knowledge/improve/authoring.html`, linked it from `knowledge/improve/index.html`, bumped the corpus version, regenerated the corpus indexes, and verified the changes.
- **Checked against:** `src/fold.c`, `src/depth.c`, `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/templates/ladder.html`, `tools/lint-corpus.py`, `tools/build-index.py`, and `tests/guide.sh`.

## Guiding prompts

> The tam knowledge corpus hides some sections, giving the user a chance to think about it before answering. I think there is a way to hide or show things depending on whether you are new to this.
>
> Look at the HTML of the corpus pages and explain how this works.
>
> I see <section> and <details> tags, with class="quiz", "q" and "session" attributes.
>
> I would like to see a complete explanation of how a knowledge page can be written. This itself should become a page in the knowledge corpus.

> Proceed

## Key decisions

- Authored `knowledge/improve/authoring.html` as a multi-level ladder page following `docs/templates/ladder.html`, classified as a tutorial under the `improve` track with parent `improve/index.html`.
- Novice section provides conceptual orientation on the unified single-page ladder, retrieval practice, and progressive disclosure without command exercises or quiz pressure.
- Learner section provides hands-on authoring instructions, metadata conventions, `pre.session` risk annotations, fold usage, and self-quiz folds.
- Expert section details the two-stage display pipeline in C (`src/fold.c` numbering followed by `src/depth.c` level filtering), fold/aside class semantics, index inspection, and public boundary enforcement.
- Linked from the Improving part hub (`knowledge/improve/index.html`) under Improvement guides.

## Verification

- `make corpus`: generated index, companion sidecars, and component coverage map.
- `python3 tools/lint-corpus.py`: clean pass with zero errors.
- `python3 tools/check-repo.py`: clean pass with zero errors.
- `git diff --check`: clean pass with zero whitespace errors.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe improve/authoring`: verified clean rendering, risk badges, level headers, and deferred answer collection.
- `tests/guide.sh`: all 12 guide test suites passed.
