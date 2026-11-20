# tam command and corpus workstream lanes (build split; tam 0.15.0)

- **Date:** 2026-10-14
- **Stage:** Develop — the Makefile now builds the command and the corpus in
  independent lanes; the coordination and delegation docs name the lanes. No
  version change (tam stays 0.15.0).
- **CLI Tool:** `opencode` 1.19.0
- **Model:** `big-pickle`
- **Transcript:** Retained privately by the author.
- **Commit:** `2fb9a4a`
- **Authorship:** Fred directed the work in a direct session and approved the
  proposal: "Change the target for the knowledge corpus from "make index" to
  "make corpus". Otherwise, the proposed plan looks good. agy is done. Please
  close the review window and implement so that we can make command or make
  corpus to build the command or the knowledge corpus, respectively, and
  agents can claim one or the other or both (if they are working on something
  that tightly integrates the command and the corpus)." opencode investigated
  the build and test coupling, drafted the proposal, implemented the Makefile
  split and the development-doc section, ran the checks, and made the
  coordination/delegation doc pass.

## Guiding prompts

> I've got agy working on the tam command. Can you work on the knowledge corpus at the same time as agy is doing what it is doing now, or will that cause conflicts?
> Let's wait for agy to finish. In the meantime, help me figure out how we can adjust our skills, guides, runbooks and memories so that I can have two agents working in the tam repo at the same time. I'd like to be able to assign one agent work to the tam command itself, editing the C library and C code for the tam command, and another agent to be working on the knowledge corpus. I believe there should be a way for that to happen: the compiled command and the knowledge corpus are intentionally separated so it should be possible to be working on them both at the same time. Do you agree? What needs to change? Don't actually make the changes, just investigate and propose a plan for the changes to our work claiming procedure that would be necessary to allow this parallel develop effort to happen.
> Could we define more make targets so you could make just the parts of the project or the whole thing if you want? I can imagine sometimes you're just working on the command, sometimes you're just working on the knowledge corpus, and sometimes you're working on both together in a tightly coupled and integrated way (such as when we added support for definition lists in the parser).
> 1 keep plain make = whole project
> 2 Scope --dirty to the command's own paths
> 3 accept occasional cross-lane test noise as a signal that coordination is necessary
> Yes, draft those as a reviewable proposal.
> Change the target for the knowledge corpus from "make index" to "make corpus". Otherwise, the proposed plan looks good. agy is done. Please close the review window and implement so that we can make command or make corpus to build the command or the knowledge corpus, respectively, and agents can claim one or the other or both (if they are working on something that tightly integrates the command and the corpus).
> Do that doc pass now
> Yes, close it now. Commit all your work. Are you done?

## Decisions and scope

- **Lane partition.** The command lane is `src/`, `bin/`, `completions/`; the
  corpus lane is `knowledge/`, `tools/`, `tests/snapshots/`. Shared
  integration files (`Makefile`, `tests/run.sh`, `tests/guide.sh`,
  `AI_PROVENANCE.md`, `README.md`, `ROADMAP.md`, `docs/`) move by handoff. A
  change that tightly couples the lanes (parser support the corpus then uses)
  claims both.
- **Makefile targets.** `make` / `make all` build the whole project (command
  and corpus index); `make command` builds the C binary without regenerating
  the index; `make corpus` generates and validates
  `knowledge/index.{tsv,json}`. `make test-command` / `make test-corpus` run
  one lane's checks; `make test` / `make check` run the integrated suite.
  `install` still depends on `all`.
- **Scoped build id.** The binary no longer depends on `$(INDEX)`, and the
  `-dirty` suffix on `TAM_BUILD_ID` is computed from the command's own paths
  (`src bin completions Makefile`) instead of `git describe --dirty` over the
  whole tree, so a corpus-only edit neither re-stamps nor recompiles the
  binary.
- **Tests stay integrated by design.** `tests/guide.sh` renders corpus pages
  through the built binary, and `tests/run.sh` asserts corpus content through
  `search`/`explain`, which read the generated index. A cross-lane red is a
  coordination signal, not a broken split.
- **Documentation.** The lane convention was added to the coordination
  protocol (`COORDINATION.md`), the workspace `AGENTS.md`, the delegation
  guide's "Concurrent orchestrators", the session-start runbook, the
  delegation SOP, the `delegate` skill, and the two coordination/delegation
  memories. These are private-workstation docs, not part of this repository.
- **Plan 25.** The proposal was tracked as a private plan and closed on
  Fred's acceptance.

## Verification

- `make command` builds `build/tam` and does not run `tools/build-index.py`.
- `make corpus` regenerates `knowledge/index.tsv` and `knowledge/index.json`,
  byte-identical to the committed files.
- `make test` runs `tests/run.sh` and `tests/guide.sh` green (exit 0),
  including the golden text snapshots.
- The scoped dirtiness check (`git status --porcelain -- src bin completions
  Makefile`) ignores a scratch file created under `knowledge/`, confirming a
  corpus-only edit does not mark the command dirty.
- The split landed as commit `2fb9a4a` on `develop`; no version bump.
