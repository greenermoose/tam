# GNU Info subsystem plan (milestone 14)

- **Date:** 2026-10-14
- **Stage:** Develop — research and plan; no code.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5, from the agy process log that served this session. The commit trailer says 1.3.4.
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commit:** `dfc0058`
- **Record:** Written on 2026-10-15 by Claude Code from the session transcript; the session wrote no record of its own.
- **Authorship:** Fred asked for the research and the plan. Antigravity surveyed GNU Info and the other documentation installed on the system and wrote `docs/plans/info-subsystem.md`, coordinating with a concurrent Antigravity session working on another milestone.

## Guiding prompts

Session-management questions are omitted.

> For tam, include info in addition to man as a source of information about the system. Start by doing a deep dive into what info is, how it differs from man, and examine the info available on this system. Then update the plan for tam. There is another agy session working on a milestone for tam so you'll need to coordinate with it. Are there any other system guide or documentation packages installed on this system besides the Omarchy manual, man pages, and the info system?

> You look stuck. See if you can get yourself unstuck.

> It seems you've gotten stuck again. Have you? What have you been doing or waiting for?

> Explain what tam info will do

## Decisions and scope

- `tam info` reads installed Info manuals natively and shows them in the
  hyperlinked pager, without an external reader. Milestone 14 implemented
  it as tam 0.22.0.

## Process note

The session made no visible progress for about three hours and needed two
prompts to resume; it attributed the delay to a recursive `find` over
`/usr/share/doc` that stalled. `AGENTS.md` now asks
agents to avoid recursive searches of large system trees and to run
commands in the foreground with a time limit.

## Verification

Plan text only; no build or test was needed.
