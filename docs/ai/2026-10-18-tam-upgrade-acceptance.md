# TCK7 acceptance follow-up: isolated source build and approval boundaries

- **Date:** 2026-10-18
- **Time:** 13:33 to 14:13 -04:00
- **Stage:** Develop; TCK7 source-build acceptance follow-up
- **CLI Tool:** Codex CLI 0.163.0
- **Model:** `gpt-6.1-sol`
- **Transcript:** Retained privately by the author.
- **Commits:** The commit containing this acceptance follow-up and its code,
  test and planning changes.
- **Authorship:** Fred authorized the isolated source build, corrected the
  GitHub 404 history, and granted standing approval for disposable builds.
  Codex improved the build preview, repaired and isolated the staged corpus
  check, and updated the plan, handoff and approval boundary.
- **Checked against:** `bin/tam-upgrade`, `tests/test_upgrade.py`,
  `tests/guide.sh`, `Makefile`, `src/tam.c`, `docs/plans/upgrade.md`,
  `docs/handoffs/tam-upgrade-luna.md`, live development/test metadata and the
  exact pinned libtam and tam source commits.

## Guiding prompts

> Yes, push these commits and run the isolated source build and live metadata check.

> Claude found the cause of your 404: 5a84294 changed the repository root request to "/", and GitHub returns 404 for /repos/greenermoose/tam/ with a trailing slash. 627c126 fixes it and adds a URL regression test, and 4afdea5 corrects the blocker notes in the plan, handoff, ROADMAP and AI record. Both are pushed to origin/develop, so no push is pending. GitHub access was never broken: your SSH and gh failures came from the sandbox, not lost credentials, so request escalation for any future push. The live check now resolves development (0.31.0) and test. Run still errors because main (0.9.1) has no knowledge/version file, which is expected. Read docs/handoffs/tam-upgrade-luna.md, then continue TCK7: show me the preview for the isolated source build and wait for my confirmation. First activation in ~/.local/dev still needs my instruction.

> I can't see what you're talking about. Where can I confirm yes?

> yes, proceed with the isolated build. Can we revisit what needs my approval and what doesn't? I don't want to have to approve your isolated builds. I want you to make those without bothering me about them. I think we have put in place some policies that are too restrictive and forcing me to get involved before I need to be.

## Key decisions

Fred's standing authorization covers fetching pinned sources, running their
checks and activating only within a disposable temporary home/prefix for
acceptance. Agents should not ask for per-build confirmation. The command
continues to preview and confirm normal switches into the configured user
prefix. First activation in `~/.local/dev` remains explicitly developer-
directed; stage promotions, deployments and privileged system changes keep
their existing approval boundaries.

## Verification

- The live metadata check previously resolved development and test; `main`
  lacks `knowledge/version`, as expected for its older `0.9.1` source.
- A first managed source build fetched and compiled the exact pinned tam and
  libtam commits. Its staged-install check exposed stale expected corpus text
  and dependence on caller `XDG_DATA_HOME`.
- `tests/guide.sh` now supplies a temporary home/data path, verifies the
  bundled XDG data directory, and matches the current welcome text.
- Full `make test` with the exact pinned libtam passed: 15 upgrade tests, the
  command suite, six render goldens, 29 corpus tests, four index tests, and all
  guide checks including staged installation.
- The managed isolated build against the updated development head remains to
  be rerun after this follow-up is pushed. No real `~/.local/dev` activation
  has been performed.
