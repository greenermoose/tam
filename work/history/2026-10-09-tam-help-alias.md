# tam help alias (0.3.0)

- **Date:** 2026-10-09
- **Stage:** Develop — small command change on top of milestones 1–2.
- **CLI Tool:** opencode 1.18.35
- **Model:** `big-pickle`
- **Transcript:** Retained privately by the author.
- **Commit:** `70af728` ("Accept `help` as an alias of `--help`").
- **Authorship:** Fred set the behavior and the standing install rule; opencode
  wrote the dispatch change, test, docs and provenance records. No delegation.

## Guiding prompts

> update tam so tam help works the same as tam --help

> Have you installed the new version of tam? I always want you to do that when
> you update tam.

> Commit your work

## Decisions and scope

- `main` dispatches the bare word `help` through the same branch as `-h` and
  `--help`, so `tam help` and `tam --help` produce byte-identical output and
  both reject extra arguments with the existing usage error (exit 2). No new
  code path, no new exit status, no change to `browse`.
- Command version `0.2.0` → `0.3.0`: a new user-visible invocation takes the
  minor bump while the major is `0`, per the versioning convention in
  `docs/plans/development.md`. `tests/run.sh` asserts the new version string.
- `tests/run.sh` now checks that `tam help` prints `Usage:`, that its output
  equals `tam --help` exactly, and that `tam help extra` fails.
- README, the help text itself and the first-command contract table name the
  alias; the decision record entry is dated 2026-10-09.
- Fred's standing rule is recorded in agent memory: every `tam` source edit
  ends with `make && make test` and a `pkexec make -C <tam checkout>
  install`, and the install is reported as part of the change.

## Verification

- `make` builds with `-Wall -Wextra -Wpedantic -std=c11`, no warnings.
- `make test` passes: `tests/run.sh` (now including the `help` alias checks)
  and `tests/guide.sh` (nine guide checks, golden snapshots unchanged).
- Installed to `/usr/local/bin/tam` and confirmed `tam --version` reports
  `tam command 0.3.0`; `diff <(tam help) <(tam --help)` is empty;
  `tam help extra` exits 2 with the usage diagnostic.
