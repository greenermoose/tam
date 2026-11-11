# Command and corpus defect tracker

- **Date:** 2026-11-11.
- **Time:** 2026-11-11 07:55:10 -0500 to 2026-11-11 17:38:10 -0500.
- **Stage:** Develop; no installation or stage transition.
- **Checked against:** The changed source files, repository defect template, and recorded verification commands.
- **CLI Tool:** Codex CLI 0.167.1 (runtime checked).
- **Model:** gpt-6.1-sol.
- **Transcript:** Retained privately by the author.
- **Authorship:** AI-assisted source/documentation under the developer's approved plan.

## Guiding prompts

> Make a plan to implement the good ideas. Suggest a better home for the daily journal feature. I do want to add the ability to jot down quick notes and keep a plain text journal. I don't want to add any more icons to my bar so we'll need to carefully consider placement and decide what goes and stays in each slot.

> Implement the plan.

> continue

## Changes and boundaries

Add repository-local defect template/index, repository-map registration and navigation. No command or corpus version change.

Repository-map/link checks; command/corpus implementation unchanged.

No installation, Test, Run or Release. User data and private host evidence
remain outside the source records.

## Key decisions

Use Markdown records beside their source; keep host evidence outside this
tracker. No command/corpus version change for documentation-only tracking.

## Verification

`python3 tools/check-repo.py` passed after registering `bugs/` in the map.
The developer’s concurrent code/corpus edits were preserved.
