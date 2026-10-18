# Source upgrades and stage switching: planning

- **Date:** 2026-10-18
- **Time:** 12:14 to 12:31, -04:00.
- **Stage:** Planning before Develop; no implementation or installation.
- **CLI Tool:** Codex CLI 0.163.0, checked with the runtime detector.
- **Model:** `gpt-6.1-sol`, checked with the runtime detector.
- **Transcript:** Retained privately by the author.
- **Commits:** The commit containing this accepted plan and implementation handoff.
- **Authorship:** The developer specified the outcome and stage/version rules;
  Codex inspected source and wrote the proposed design and planning entry points.
- **Checked against:** `src/tam.c`, `src/version.h`, `knowledge/version`,
  `Makefile`, `src/ask.c`, `completions/bash/tam`, libtam build/execution source,
  package assembly definitions, repository plan procedures, GitHub REST and curl
  documentation linked in the plan. Remote component API responses unavailable.

## Guiding prompts

> Create a work plan to improve the tam command so I can run a subcommand such as tam upgrade to see if there is a more recent version of tam available than the one running. Think about what this subcommand should be and how it should work. Ask if you have any questions. I will hand off the implementation of your plan to an agent such as Luna.

> I want to be able to install a newer version from source.

> We should also be able to switch between development, test, run and release versions. Development versions are ones where the feature list is still being worked on. Test ones are those where we believe the feature list has been done and we are looking for bugs. Run are those we believe are reliable for daily use. Release are ones that are pegged to a specific release. We don't want to have too many releases: we just tag a run version with a release tag when we think it's especially noteworthy and worth upgrading to. Ask if you have any questions about how version numbers track these stages or have questions about these stages of the software as we develop it.

Clarification answers, verbatim:

> Both released and development versions

> No, the version number follows changes in the code. Everytime we build a version for Fred, bump the version number. command and corpus have their own version numbers.

> Use the development prefix (recommended)

> Yes, preserve versions when installing existing source (recommended)

> Yes, tag the version pair (recommended)

The final answer approves paired tags such as `tam-0.31.0-corpus-0.32.1`,
as proposed in the clarification question. The development-prefix answer
selects `~/.local/dev` for source installations. These answers decide those
parts of the design; they do not approve implementation of the entire draft.

## Key decisions and verification

Propose bare `upgrade` for discovery, explicit application for forward updates,
and `switch` for stage/version selection with confirmation and recovery. Release
tags establish paired version identity; installation never promotes a stage.
Build private dependencies/generations and preserve source versions on reinstall.
Existing older versions need audited compatibility records and a retained
updater/recovery entry point, rather than becoming impossible to switch away from.

TCK6–TCK8 partition discovery, source installation/recovery and CLI/documentation.
The feature plan is Ready following the approval below. `git diff --check`, relative-link/whitespace
checks, complete plan-index inventory and new-file public-boundary checks passed.
No implementation tests, source build, installation or deployment in this session.

## Approval and handoff

The developer reviewed the plan in Micro and accepted it on 2026-10-18:

> I accept this work plan. Close the review session. Commit your work and release your work claims. Prepare a handoff so I can have a Luna session implement this work plan.

Micro closed successfully; the saved plan was reread before finalization.
The plan/index/roadmap/development contract now record Ready, and `AGENTS.md`
routes new sessions to it. The [Luna handoff](../handoffs/tam-upgrade-luna.md)
gives the starting point, ordered TCK6–TCK8 work, accepted decisions, checks and
bootstrap/activation boundaries. This session implements no feature code.
