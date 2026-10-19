# Reliable corpus builds

**Plan status:** Completed
**Reviewed:** 2026-10-19
**Scope:** Corpus tooling correction, corpus 0.32.2; command 0.31.1 unchanged.
**Authority:** Developer-approved plan and implementation instruction recorded
in [the session record](../ai/2026-10-19-tam-corpus-build.md).
**Next action:** None for source implementation. Package stages remain separate.

## Problem and agreed outcome

The old Make dependencies miss Git-only metadata changes, removed pages and
deeply nested HTML. The developer wants one reliable rebuild command and chose
to always recheck instead of maintaining a cache. `make corpus` regenerates and
validates indexes; `make corpus-and-test` rebuilds and runs the corpus checks.
The old `test-corpus` target is removed without an alias.

## Implementation and handoff

**TCK9 (reliable corpus builds) — completed:** Invoke the generator once per Make execution
through the phony corpus target, with direct index targets depending on it.
Rebuild before corpus tests. Preserve unchanged output bytes and mtimes and
derive the build marker after writing indexes. Keep validation before writes,
the index schemas, Git metadata rules and archive behavior. Update active
authoring/reference instructions, including rebuilding after page commits and
committing changed indexes. Versions remain manual.

## Acceptance and limits

Temporary fixtures exercise edits with preserved timestamps, additions,
deletions, renames and deep pages; page and index commits; build-marker
clean/dirty transitions; missing outputs; unchanged mtimes; parallel/direct
targets; invalid content; archives without Git; and rebuild-before-test order.
The 10 index/Make tests, 29 linter tests, 15 upgrade tests, command/PTY checks,
six renderer goldens and all 55 guide-page checks passed with the installed
development library. The explicit parallel `corpus corpus-and-test` invocation
passed and generated the indexes once. Live Ollama checks were skipped because
no local server was running. See the linked session record for commands.

The work stays on develop. It does not install software, assemble a package,
promote branches, tag or release. Command compilation still needs installed
libtam; the corpus-only target does not.
