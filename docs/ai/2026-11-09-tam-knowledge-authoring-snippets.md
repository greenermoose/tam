# Knowledge authoring snippets (corpus 0.77.1)

- **Date:** 2026-11-09
- **Time:** 19:50 to 20:45 -05:00
- **Stage:** Develop
- **CLI Tool:** Antigravity CLI `1.5.3`
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred Horch requested actual HTML snippets on `improve/authoring.html` for each of a question and answer, a hint, a first time, on Tamlinux, and a peek under the hood, showing the full set of HTML and how it renders. Antigravity added the markup patterns, HTML snippets, rendering explanations, and live instances to `knowledge/improve/authoring.html`, bumped `knowledge/version` to 0.77.1, updated roadmap and provenance, regenerated corpus indexes, and verified rendering and test suites.
- **Checked against:** `src/fold.c`, `src/render.c`, `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `tools/lint-corpus.py`, and `tests/guide.sh`.

## Guiding prompts

> On the improve/authoring.html page of the tam knowledge corpus, show actual HTML snippets of how to do each of a question and answer, a hint, a first time, on Tamlinux, and a peek under the hood. I need to see the whole thing: the full set of HTML and how it renders.

## Key decisions

- Added a dedicated subsection `<h3>Markup patterns: folds and asides</h3>` under `Reference: Fold and aside classes` in the expert section of `knowledge/improve/authoring.html`.
- Included complete HTML snippets in `<pre class="session" data-risk="read-only">` blocks for:
  1. `details.q` / `details.q.open` (self-quiz questions and model answers)
  2. `details.hint` (progressive exercise clues)
  3. `aside.first-time` (novice terminal orientation fold)
  4. `aside.tamlinux` (static callout with bold "On Tamlinux:" header)
  5. `aside.peek` (static callout with bold "Peek under the hood:" header)
- Provided clear descriptions of console pager rendering (labels, keyboard shortcuts Enter/`r`/`*`, fold expansion) versus piped output (`--pipe`, inline questions and deferred answers under `<h3>Answers</h3>`).
- Placed live instances of `aside.first-time` and `aside.peek` in the novice section, and `details.hint` in the learner section, enabling readers to interact directly with all five constructs on the live page.
- Bumped corpus patch version from 0.77.0 to 0.77.1.

## Verification

- `python3 tools/lint-corpus.py`: clean pass with zero errors.
- `python3 tools/check-repo.py`: clean pass with zero errors.
- `git diff --check`: zero whitespace errors.
- `tests/guide.sh`: all 12 guide test suites passed.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe improve/authoring`: verified rendering across all levels.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --level novice --pipe improve/authoring`: verified novice filtering, first-time fold, and peek callout.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --level learner --pipe improve/authoring`: verified learner filtering, hint fold, tamlinux callout, and quiz folds.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --level expert --pipe improve/authoring`: verified expert filtering, code blocks, and model answer folds.
