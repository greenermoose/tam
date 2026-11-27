# P1: Offline knowledge foundation and pager workflow

- **Status:** done (command 0.1.0–0.34.0 / corpus 0.1.0–0.35.1)
- **Versions:** command 0.1.0–0.34.0, corpus 0.1.0–0.35.1

## Goal

Establish `tam` as the native C offline knowledge browser with its
five-part structure, reliable corpus build pipeline, developer loop,
source-upgrade tooling, and ergonomic pager navigation.

## Versions

- **0.1.0–0.24.0 (done):** Initial unified command/corpus offline reader,
  man/info/help/topic browsing, and clean pipe output.
- **Command 0.30.0–0.34.0 / Corpus 0.31.0–0.35.1 (done):** Decoupled
  command and corpus versions, five-part knowledge system
  (`docs/plans/knowledge-system-0.31.0.md`), source-upgrade and
  stage-switching (`docs/plans/upgrade.md`), reliable corpus builds
  (`docs/plans/corpus-build.md`), home-row and quick pager keys
  (`docs/plans/pager-home-row-keys.md`, `docs/plans/pager-quick-keys.md`),
  and developer loop (`docs/plans/dev-loop.md`).

## Acceptance

`make command`, `make corpus`, `make check-repo`, and PTY pager tests
pass.

## Dependencies

`libtam` (static) and POSIX libc; Python 3 for corpus build tooling.

## Risks

None (completed; detailed feature plans remain in `docs/plans/`).

## Open points

None.
