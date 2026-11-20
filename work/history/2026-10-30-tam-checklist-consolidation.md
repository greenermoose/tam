# Consolidated TCK checklists in CHECKLISTS.md

- **Date:** 2026-10-30
- **Time:** 11:59 to 12:08 -04:00
- **Stage:** Develop; documentation only, no version change
- **CLI Tool:** Antigravity CLI 1.4.11
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** Fred Horch directed adding all checklists for tam to `CHECKLISTS.md` so there is one place to see them all. Antigravity CLI surveyed all plans in `docs/plans/`, compiled the inventory of TCK1 through TCK27, and updated `CHECKLISTS.md`.
- **Checked against:** `CHECKLISTS.md`, `ROADMAP.md`, `docs/plans/README.md`, `docs/plans/knowledge-system-0.31.0.md`, `docs/plans/upgrade.md`, `docs/plans/corpus-build.md`, `docs/plans/pager-home-row-keys.md`, `docs/plans/dev-loop.md`, `docs/plans/pager-quick-keys.md`, `docs/plans/learning-levels.md`, `docs/plans/external-links.md`

## Guiding prompts

> Is there a file with the list of TCK for tam?

> Add all the checklists for tam to tam's CHECKLISTS.md file so there is one place to look to see them all.

## Key decisions

- Added an overview table and plan-grouped checklist sections (`TCK1`–`TCK27`) to `CHECKLISTS.md`, providing a single consolidated inventory of all checklist items, their status, owning plans, and completion evidence.
- Preserved existing minting rules, the monotonic counter (`Next free: TCK28.`), and relative links to owning plans.
- Updated the preamble in `CHECKLISTS.md` to reflect that this file now maintains the consolidated checklist of all repository TCK items while detailed procedural and design documentation resides in the respective plans.

## Verification

- `python3 tools/check-repo.py` verified relative links and repository invariants.
- Confirmed all 27 TCK identifiers map directly to their source plans.
