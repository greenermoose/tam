# Learning levels, goal paths and self-quizzes plan (TCK20)

- **Date:** 2026-10-20 to 2026-10-21
- **Time:** began 2026-10-20 (start time not recorded); ended 2026-10-21 05:55 -04:00
- **Stage:** Develop. An in-progress plan with TCK20 (the contract) delivered;
  no code or component version change.
- **CLI Tool:** Claude Code 2.1.305
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `1de4945` (the plan, the pedagogy guide and the templates) and
  the commit that moves `tam`'s state into `tamlinux/tam/` in the plan
- **Authorship:** The developer asked for an expansive proposal and posed the
  questions quoted below. He approved the first draft's six recommendations,
  then asked for a decision on four levels, for self-quizzes and for
  goal-based paths, and accepted the four further recommendations. The AI tool
  surveyed the source and corpus, wrote both drafts, the plan,
  `docs/corpus-pedagogy.md`, the four templates and the entry-point updates.
- **Checked against:** `src/depth.c` (`depth_line()`,
  `tam_depth_apply_rev()`), `src/pager.c` (`switch_depth()`, `pager_t`,
  `follow()`, status row, key cases), `src/tam.c` (`read_corpus_page()`,
  `record_welcome_seen()`, "Depth" strings), `src/render.c` and
  `src/render.h` (handled tags), `tools/build-index.py`, `tests/run.sh`
  (depth case), `tests/guide.sh`, `tests/fixtures/guide/`, `knowledge/`
  level metadata counts, `knowledge/beginner.html`,
  `knowledge/intermediate.html`, `knowledge/expert.html`,
  `knowledge/learn/practice.html`, `knowledge/shell/pipelines.html`,
  `docs/corpus-authoring.md`, `docs/plan-maintenance.md`, `CHECKLISTS.md`,
  `ROADMAP.md`, `docs/plans/development.md`, `src/version.h`,
  `knowledge/version`, `Makefile` (`DATADIR`), the `tamlinux` repository's
  `docs/plans/tamlinux-command.md` and state paths in its `commands/` and
  `desktop/`, the XDG Base Directory Specification 0.8, the `platformdirs`
  platform table and the `directories` crate's `ProjectDirs` documentation

## Guiding prompts

> Ponder and propose an overhaul to the tam knowledge corpus that delivers on the idea of three levels of detail for our pages (beginner, intermediate, and expert). Understand best practices in training and documentation systems so that we really do help users learn and achieve mastery over their system. A few questions I have are
>
> 1) Should every page have a way to toggle between the four states (all, beginner only, intermediate only, expert only)
> 2) How should the pager interface show current level and level toggle
>
> We already have 0, 1, 2 and 3 as keybindings associated with levels.
>
> What kind of worked examples, questions and answers, glossaries, bibliographies and further reading sections, etc. would be helpful?
>
> Be expansive in your thinking. Now is the time to plan the world's best knowledge corpus for a desktop environment that runs on Linux!

> Decide if we should expand our level system from three (1=beginner, 2=intermediate, 3=expert) to four (1=novice, 2=learner, 3=intermediate, 4=expert). Also, should we be able to have self-quiz sections in the pager: questions showing with answers hidden but revealable. And perhaps we should have several defined paths that are about what a reader wants to gain rather than only about level of competence. I approve your recommendations. Please revise the plan with the above feedback in mind and let me review again.

> I accept all recommendations a,b,c,d

> Should tam be writing to $XDG_STATE_HOME/tamlinux or $XDG_STATE_HOME/tam? I wonder if welcome_seen is an error, and all state for the tam command should be stored in $XDG_STATE_HOME/tam rather than tamlinux. Ponder the pros and cons and make a recommendation support by evidence of industry best practice.

> I agree. Make those changes.

## Key decisions

- **Every page keeps the 0-3 control, not three versions.** The keys set a
  sticky lens. Pages follow one of three patterns (ladder, layered, single),
  and a page without the lens level opens with a banner instead of an
  error. Forcing three sections everywhere would pad pages and triple every
  fact.
- **Three levels, not four.** A fourth level adds a section, and another
  copy of each fact, to every laddered page. Readers cannot reliably place
  themselves between "learner" and "intermediate". `?level=2` has meant
  intermediate since command 0.26.0. The novice gap the developer saw is
  met by the First steps path, folded "New to this?" asides and glossary
  links. The developer reconsiders only with learner-test evidence.
- **Quizzes are a renderer feature.** `r` and `*` are free keys; digits stay
  reserved for levels, so multiple choice never uses them. Answers hide
  again on leaving a page so a second visit is real retrieval.
- **`tam`'s state lives in `$XDG_STATE_HOME/tamlinux/tam/`.** The lens file
  is `tamlinux/tam/level`, and TCK21 moves `welcome_seen` there from the top
  of `tamlinux/`. The XDG specification puts restart-surviving view state
  under `$XDG_STATE_HOME` but says nothing about naming the subdirectory.
  `platformdirs` and `directories` name it after the application alone on
  Linux, which argued for `$XDG_STATE_HOME/tam`. The local evidence weighed
  more. `tam` already uses the `tamlinux` namespace for its installed corpus
  (`share/tamlinux/knowledge`) and for the version it reads
  (`~/.config/tamlinux/`). Most writers in `~/.local/state/tamlinux/`
  already use a subdirectory. "tam" is a short name more likely to collide.
  A top-level `welcome_seen` did not say which component owned it. It was
  not a mistake: the command plan kept the `tamlinux` data namespace when
  the command was renamed. Nothing reads it, so the move needs no migration.
  `tam-plugin`'s own `$XDG_STATE_HOME/tam-plugin` is a known exception, left
  unchanged.
- **Found while surveying:** the beginner example in
  `knowledge/shell/pipelines.html` shows output that was never captured, and
  its `grep` matches its own process. TCK23 replaces it.

## Verification

Documentation only. Checked that every relative link in the changed planning
documents resolves, `git diff --check` is clean, and the diff contains no
private identifiers. No build or test was needed.
