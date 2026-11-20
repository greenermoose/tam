# Pager quick keys plan: `]` `[` `=` `\`, status row, Guide index

- **Date:** 2026-10-20
- **Time:** 13:55 to 14:30 -04:00
- **Stage:** Develop; a Ready plan (TCK15–TCK19), no code or component version change
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record and the plan
- **Authorship:** The developer wrote three idea notes (pager key bindings,
  Guide search, status row) and asked for a review. He decided the design
  questions below and asked for a plan that another agent could build. The AI
  tool reviewed the notes against the source, recommended changes, and wrote
  the plan and its entry-point updates.
- **Checked against:** `src/pager.c` (`handle_key_inner`, `handle_key`,
  status row, help overlay, `pager_search_prompt`, `pager_prompt_action`,
  `follow`, `switch_depth`), `src/tam.c` (`cmd_show`, `corpus_load`,
  `resolve_page_path`), `src/man.c`, `src/info.c`, `knowledge/index.html`,
  `knowledge/welcome.html`, `README.md`, `man/tam.1`, `tests/run.sh`,
  `tests/pager_screen.py`, `tests/guide.sh`, `tests/fixtures/guide/`,
  `tools/lint-corpus.py`, `src/version.h`, `knowledge/version`, `AGENTS.md`,
  `CHECKLISTS.md`, `docs/plan-maintenance.md`, `build/tam` at `52b38fe`

## Guiding prompts

> Take a look at the ideas for changing the tam command, including new keybindings, in the ideas/fred folder of the [redacted: private repository] repo. What do you think? Which ideas are good and which are bad?

> I accept your recommendations about the bottom row and the Tab keep. Keep Tab and Shift-Tab as aliases for ] and [.

> 1 okay, add a page: prefix
> 2 Keep tldr in Appendices and More
> 3 Limit N to mean only the previous search hit
> 4 yes, add a tam:help
>
> Any other questions?

> I want to have agy build from your plan. How should I prompt it?

> Write the plan now. I want to be able to close this session, open an agy session, and assign that new session the task of building from the plan.

The developer also chose two options when asked: both search rows in the
Quick keys table are links, and the table keeps his four rows.

## Key decisions

- **Tab and Shift-Tab stay link keys.** The developer's note proposed them as
  jumps to `tam:topics` and `tam:tldr`. The review objected: Tab means "next
  link" everywhere, and terminals send Shift-Tab in at least five ways
  (`src/pager.c` already handles five), some of them plain Tab, so a press
  meant for tldr could open topics. He kept them as aliases for `]` and `[`.
- **Status row `L65 3-78/79`** instead of the note's `L65 3-78 79`. The slash
  reads as "of", the way `less -M` and vim show it.
- **`=` shows an address that `tam show` reopens.** That needs a new `page:`
  scheme for guide pages, plus `tam:explain` and `tam:keywords` in `tam show`,
  which today rejects them. Man addresses keep their parentheses, and the docs
  tell readers to quote them.
- **tldr stays in the Guide index.** The note predated tldr reaching the index.
- **`=` clears on a page change** as well as on Esc. This is the planner's
  default; the developer may change it.
- **`tests/guide.sh` must change in TCK19.** It requires the index to link
  `tam:search`, and the new index does not.
- **Entry-point fixes made while planning:** the checklist counter said "Next
  free: TCK13" although TCK13 and TCK14 already existed. The contract header
  still named command 0.32.0 / corpus 0.34.1.

## Verification

Documentation only. Read the plan back as a new agent would: it names one
starting slice, the baseline, the claims, the checks and the stop points.
Links resolve, and `git diff --check` is clean.
