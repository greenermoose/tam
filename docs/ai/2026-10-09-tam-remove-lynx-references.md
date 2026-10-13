# Remove Lynx references from tam plans and documentation

- **Date:** 2026-10-09
- **Stage:** Develop — documentation and acknowledgements refinement.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.2
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred asked whether Lynx was still needed given that `tam` parses and displays HTML on its own, whether any code from Lynx was used, and directed removing talk of needing Lynx for `tam` across all related files. Antigravity confirmed that `tam` renders HTML natively with zero external dependencies and uses no code from Lynx, updated all plans, README, and acknowledgements across `tam`, `tamlinux`, and private planning notes, and verified all tests pass.

## Guiding prompts

> Take a look at the README and the plan for the tam command. I don't think we need lynx any more. Is that true? If so, edit all files related to tam so that we're not talking about needing lynx for it. I believe it can parse and display HTML on its own. Perhaps we used some code from lynx?

## Decisions and scope

- **Lynx status:**
  - `tam` does not need Lynx. It parses semantic HTML in C natively (`src/render.c`) and displays it in its own interactive terminal pager (`src/pager.c`), with clean plain text emitted when piped (`--pipe` or redirected stdout).
  - No code from Lynx was ever used or imported. Lynx was only mentioned in early proposals as a possible external program to shell out to.
- **Documentation and plan updates:**
  - `tam/README.md`: Updated layout table to describe `tests/` as checking offline guide rendering rather than browser integration.
  - `tam/ACKNOWLEDGEMENTS.md`: Removed Lynx credit section, Lynx community note, and the 8 individuals whose credits were solely tied to the optional Lynx proposal (Charles Rezac, Craig Lavender, Garrett Blythe, Lou Montulli, Michael Grobe, Ravikumar Kolli, Thomas E. Dickey, Tim Berners-Lee).
  - `tam/docs/plans/development.md`: Updated command table, implementation section, milestone 2 description, acceptance criteria, and decision record to remove all Lynx requirements and highlight self-contained native rendering.
  - `tam/docs/plans/offline-guide.md`: Updated decision, section 1 heading and rationale, and work item 7 to reflect that the external browser accelerator idea was dropped in favor of self-contained native rendering.
  - `tam/docs/plans/explain-topic-scaling.md`: Removed mention of Lynx under guiding invariants.
  - `tamlinux/docs/plans/tamlinux-command.md`: Removed Lynx fallback/dependency text, updated knowledge format, first slice, implementation order, and review record.
  - `tamlinux/docs/botanical-system-architecture.md`: Updated offline documentation description from Lynx rendering to native `tam` rendering.
  - `tamlinux/ACKNOWLEDGEMENTS.md`: Removed Lynx entry and historical Lynx lineage contributors.
  - Private planning notes: added implementation status notes recording that native C parsing/paging replaced Lynx.

## Verification

- Ran `make test` in `tam/`: all command and offline guide tests passed cleanly.
- Verified that `tam/` contains no remaining references to Lynx (outside historical AI transcripts).
