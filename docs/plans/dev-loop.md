# Developer loop: `make dev` and drift checks

**Plan status:** Completed
**Reviewed:** 2026-10-20
**Scope:** Repository tooling only: `Makefile`, two new scripts under `tools/`,
their tests and one new map document. No command or corpus version changes
for this work (command 0.32.0 and corpus 0.33.6 at approval).
**Authority:** The developer approved this design on 2026-10-20; see
[the planning record](../ai/2026-10-20-tam-dev-loop-plan.md).
**Next action:** None; closed 2026-10-20 (see Disposition). On hold: about
200 non-interactive live-corpus assertions in `tests/run.sh` (roadmap section 4).

Follow the [plan maintenance procedure](../plan-maintenance.md).

## Problem and agreed outcome

The developer writes the knowledge corpus (`knowledge/`) in VS Code and
sometimes changes the command (`src/`). Turning saved edits into a tested,
installed development build takes a sequence of steps that is easy to get
wrong, so he has been bringing in an AI agent just to run it. He wants to
spend his time on what the pages say and then run one deterministic command
that sees his changes through Git, asks a few questions, and commits, builds,
tests and installs them.

Agreed outcome:

- **`make dev`** is the developer's loop. He types it, answers prompts, and
  gets a commit, a passing test run and a development install. Plain `make`,
  `make all` and `make install` stay non-interactive. Package recipes
  (`tamlinux-packages` calls `make install DESTDIR=… PREFIX=/usr LIBTAM_PREFIX=…`)
  and agents are unaffected.
- **What it commits:** every changed or untracked path, except generated
  outputs and paths under another session's active `tam-work` claim in this
  repository. It lists the excluded paths.
- **Versions:** when `knowledge/` or `src/` changed, it proposes the next version
  for that lane and edits every file that carries the number.
- **Push:** it asks before pushing `develop`, every time.
- **Drift:** the script never rewrites the developer's prose. Facts that are
  copied between files (versions, file lists, plan index, links) are checked
  by a deterministic checker that is part of `make test`. A short repository
  map says what each top-level entry is and whether to edit it.

Out of scope: any change to `make`, `make all`, `make install` or packaging;
Test, Run or Release, tags and `tam-deploy`; changing how
`tools/build-index.py` computes page revisions.

## Baseline and dependencies

Verified 2026-10-20 at `d45a49a` on `develop`: command 0.32.0
(`src/version.h`), corpus 0.33.6 (`knowledge/version`).

Steps `make dev` takes over, with their current sources:

1. **The two index commits.** `tools/build-index.py` derives each page's
   `rev` and `updated` from `git rev-list --count HEAD -- knowledge/<page>`
   and the latest commit date. So pages are committed first, then
   `make corpus` runs, the changed index files are committed separately
   (as in `d022860` then `beda4e8`), and `make corpus` runs once more to
   refresh the ignored `knowledge/.build_id`.
2. **Version copies.** Corpus: `knowledge/version`, the `ROADMAP.md` header,
   its `develop` row in section 3 and a delivered-history row in section 2.
   Command: `TAM_COMMAND_VERSION` in `src/version.h`, the `--version`
   assertion in `tests/run.sh`, the same `ROADMAP.md` places and a
   decision-record entry in [development.md](development.md).
3. **Path-limited commits.** `AGENTS.md` requires `git commit -- <paths>`,
   never `-a`, because agents share this clone.
4. **Development install.** `make install PREFIX=~/.local/dev`, which finds
   libtam through `pkg-config` (the developer's shell environment puts
   `~/.local/dev/lib/pkgconfig` on `PKG_CONFIG_PATH`); the `libtam-check`
   target already stops with instructions when it cannot.

Dependencies: Python 3 standard library, Git, `tam-work` on `PATH`
(`tam-work status --json --repo tam` lists active claims; scopes carry a
`tam/` prefix).

## Implementation and handoff

Claim the paths named in each slice with `tam-work begin`, commit only those
paths, and follow `AI_PROVENANCE.md`. Put the TCK identifier in the subject of
the commit that completes each slice.

### TCK11 (`make dev` developer loop)

Files: `Makefile` (add `dev` to `.PHONY`, and
`dev: ; @python3 tools/dev.py $(if $(DRY_RUN),--dry-run)`; also run the new
test from `test`), `tools/dev.py` (new), `tests/test_dev.py` (new),
`README.md` (a short "Editing and installing a development build" section),
`AGENTS.md` (one line: `make dev` is the developer's loop; agents do not run it).

`tools/dev.py` reads top to bottom with one function per step, uses the
standard library only, and takes `--root` for test fixtures, like
`tools/build-index.py`. Each step prints a short heading. Any failure stops
with a plain message saying what was and was not committed.

1. **Preflight.** Refuse unless stdin and stdout are terminals. Require branch
   `develop` with no merge, rebase or cherry-pick in progress. Run
   `make libtam-check`. Require `tam-work` on `PATH`.
2. **Claim.** Set `TAM_WORK_OWNER=fred` and
   `TAM_WORK_SESSION=make-dev-<UTC timestamp>`, run `tam-work begin` with a
   scope for each path to be committed, and run `tam-work finish` from a
   `finally` block and on SIGINT/SIGTERM.
3. **Collect changes.** Parse `git status --porcelain=v1 -z --untracked-files=all`.
   Drop generated outputs (`knowledge/index.tsv`, `knowledge/index.json`,
   `knowledge/index.parents.tsv`, `knowledge/.build_id`, `build/`,
   `__pycache__/`). Drop paths at or under another session's claimed scope.
   Group the rest as corpus (`knowledge/`), command (`src/`) and other, print
   them with `git diff --stat`, and list the excluded paths with the claim
   that excluded each. After TCK12, label each path with its repository-map
   category and warn about edits to *generated* or *history* entries. Prompt
   `[c]ontinue, [d]iff, [q]uit`, where `d` pipes `git diff` for the listed
   paths through `$PAGER`. With nothing to commit, say so and exit 0.
4. **Versions.** For each of corpus and command with changes: if its version
   file already differs from `HEAD`, print "already bumped to X" and skip the
   prompt, so a rerun after a failure does not bump twice. Otherwise propose
   the next patch version and offer `[p]atch` (default) or `[m]inor`. The
   developer ruled on 2026-10-18 that changed code is never delivered under a
   reused version, so there is no "none" for a lane whose content changed.
   Prompt once for a one-line summary, which is also the default commit
   subject. Apply with exact-match replacements that fail loudly unless the
   pattern occurs exactly once:
   - corpus: `knowledge/version`; the `ROADMAP.md` header and `develop` row;
     a new last row in the delivered-history table,
     `| **X corpus** | <summary> | Done (YYYY-MM-DD) | <summary> |`;
   - command: `src/version.h`; the `tests/run.sh` assertion; the same
     `ROADMAP.md` places (`X command`); a decision-record entry
     `- **YYYY-MM-DD:** Command X (patch|minor): <summary>` in `development.md`.
   The edited files join the commit set.
5. **Build and test first.** Run `make test`. On failure print the last 40
   lines, then the `FAIL` lines and the check that failed, and stop. Nothing is
   committed; the version edits stay, and step 4 finds them next time. When only
   pages changed and the failing check does not read pages (TCK14), say so and
   tell the developer to ask an agent to fix that test.
6. **Commit.** Prompt for the subject (default: `<summary>; corpus X` and/or
   `command Y`) and an optional body. `git add --` the untracked paths, then
   `git commit -m … -- <paths>`. This is the developer's own commit: no AI
   trailers.
7. **Index follow-up.** Run `make corpus`. If any `knowledge/index.*` changed,
   commit them alone as `Refresh corpus index revisions after <short sha>`.
   Run `make corpus` again.
8. **Install.** Run `make command` so the binary embeds the new commit. Prompt
   `Install prefix [~/.local/dev]:`; any other prefix needs typed `yes`, and
   `/usr/local` is refused. Run `make install PREFIX=<prefix>`, then report
   `type -a tam` (the first entry must be `<prefix>/bin/tam`), `tam --version`,
   and a reminder to run `hash -r`. If other sessions' command paths are dirty,
   say that the build id will read `-dirty` and why.
9. **Push.** `git fetch`, then check `git log HEAD..@{u}`. If upstream has
   commits, say so and do not push; the shared checkout is never pulled,
   rebased or reset by this script. Otherwise prompt `Push develop? [y/N]` and
   run `git push origin develop`. Never touch `test`, `main` or tags.
10. **Finish.** Release the claim and print a summary: commits, versions,
    install prefix and push state.

`make dev DRY_RUN=1` runs steps 1, 3 and 4 as a preview: it prints the change
list and the versions it would propose, and writes, builds and commits nothing.

### TCK12 (drift checks and repository map)

Files: `tools/check-repo.py` (new), `tests/test_check_repo.py` (new),
`docs/repo-map.md` (new), `Makefile` (a `check-repo` target, also run from
`test`), and this plan's index row. Each failure names the file, the line, the
expected value and the fix.

| Check | Source of truth | Copies checked |
| --- | --- | --- |
| Command version | `src/version.h` | `tests/run.sh` assertion; `ROADMAP.md` header and `develop` row; a delivered-history row and a decision-record entry naming it |
| Corpus version | `knowledge/version` | `ROADMAP.md` header and `develop` row; a delivered-history row naming it |
| Build inputs | `src/*.c` and `src/*.h` on disk | `SRC` and `DEPS` in `Makefile` |
| Plan index | Markdown files under `docs/plans/` | exactly one row each in `docs/plans/README.md`, and no row for a missing file |
| Repository map | top-level entries and `docs/` subdirectories | one line each in `docs/repo-map.md`, and no line for a missing entry |
| Relative links | every tracked `*.md` file | each relative link target exists (fragment ignored) |
| Lane table | path lists in the `AGENTS.md` lane table | each named path exists |
| Pager fixtures (TCK13) | `tests/fixtures/` | interactive tests in `tests/run.sh` do not read `knowledge/` unless marked `# live-corpus: <reason>` |

`docs/repo-map.md` has one line per entry: what it is for and how it changes,
using four categories: *you edit* (pages, README, plans), *script edits*
(version files and the `ROADMAP.md` version places), *generated, never edit*
(`knowledge/index.*`, `build/`) and *history, read-only* (`docs/ai/`,
`docs/handoffs/`, closed plans). `tools/dev.py` reads these categories for
step 3. Keep the map short enough to read in two minutes.

The first run will find existing drift (for example, the `development.md`
preamble lagged the version files until this plan was added). Fix each finding
in its own commit; ask the developer about any finding whose correct value is
unclear.

### Finding from the first developer run (2026-10-20)

The developer reordered the guide index links and ran `make dev`. It stopped in
`make test`, as designed, with nothing committed. The cause was not his page:
the pager Tab/Shift-Tab test asserted the live index's link order. This plan
and the `AGENTS.md` lane rule ("a corpus-only release stays inside the corpus
lane") had both assumed that a failing `make test` on a page edit meant a page
problem, but about eight interactive tests in `tests/run.sh` read live page
titles, link order and breadcrumbs. The failing test moved to a fixture in
`fb6df35`; the rerun committed corpus 0.34.1 (`98222ff`, `4d41d84`), reported
"already bumped", installed and pushed. TCK13 and TCK14 close the gap.

### TCK13 (pager tests on fixtures)

Files: `tests/fixtures/guide/` (new: a small guide with an index, Welcome,
Learning, Installing, Desktop and a pipelines page with depth sections, plus a
hand-written `index.tsv`), `tests/run.sh`, `tests/guide.sh`,
`tools/check-repo.py` (check 8), `tests/test_check_repo.py`, `AGENTS.md`.
Every interactive test in `tests/run.sh` reads `tests/fixtures/`; `guide.sh`
reads the index heading from the page instead of fixed wording. Out of scope:
about 200 non-interactive `tam explain`/`tam search` routing assertions in
`tests/run.sh` still read the live corpus; they stay until the developer
decides whether they are corpus contracts (move to `guide.sh`) or command
tests (move to fixtures).

### TCK14 (`make dev` names the failing check)

Files: `tools/dev.py` (`explain_test_failure`), `tests/test_dev.py`. Step 5
names the last check `make` ran. Page checks are `tools/build-index.py`,
`tools/lint-corpus.py`, the corpus unit tests and `tests/guide.sh`; for those it
says to fix the page. When only pages (plus `ROADMAP.md` version edits)
changed and another check failed, it says the pages are probably fine and an
agent should fix the named test.

## Acceptance and limits

- `make test` passes, including `tests/test_dev.py` and
  `tests/test_check_repo.py`. Fixture repositories cover lane grouping,
  claimed-path exclusion, generated-file filtering, each version edit applying
  exactly once, "already bumped" detection, and refusal without a terminal, off
  `develop` or during a rebase. Each drift check fails on a fixture with one
  planted error and passes on the clean fixture.
- `make dev DRY_RUN=1` with one edited page lists only that page as corpus,
  proposes the next corpus patch, and leaves `git status` unchanged.
- With another session holding a claim, `make dev DRY_RUN=1` lists that
  session's paths as excluded.
- `make check-repo` passes on the tree after the drift fixes.
- TCK13: with the live index's links reversed and the index and Welcome
  retitled, `tests/run.sh` and `tests/guide.sh` still pass; `check-repo` flags
  each interactive test that reads `knowledge/` without a marker.
- TCK14: `tests/test_dev.py` covers the page-check, command-test, build and
  command-change messages.
- **Developer acceptance:** the developer edits a page, runs `make dev` and
  accepts the defaults. Expected: a page-and-version commit, then an index
  commit; `type -a tam` lists `~/.local/dev/bin/tam` first; `tam --version`
  shows the new commit without `-dirty` (unless another session's command
  paths are dirty); the new wording appears in `tam`. A page that fails
  linting stops `make dev` before any commit, and the rerun after the fix
  reports "already bumped".

Limits: a development install is not packaged acceptance; Test, Run and
Release stay on the developer's word. The live `tam ask` checks still skip
without a local Ollama server.

## Disposition

Completed 2026-10-20; the developer closed the plan after his corpus 0.34.1 run.
Evidence for each acceptance criterion:

- `make test` passes with `tests/test_dev.py` (9 tests) and
  `tests/test_check_repo.py` (10 tests), whose fixtures cover lane grouping,
  claimed-path exclusion, generated-file filtering, single-occurrence version
  edits, "already bumped" detection, the refusals, and each drift check
  ([TCK11](../ai/2026-10-20-tam-dev-loop-tck11.md),
  [TCK12](../ai/2026-10-20-tam-dev-loop-tck12.md),
  [TCK13 and TCK14](../ai/2026-10-20-tam-dev-loop-tck13-tck14.md)).
- `make dev DRY_RUN=1` with one edited page listed only that page and left
  `git status` unchanged (TCK11 record). Exclusion of another session's claimed
  paths is covered by the fixture test, not by a live dry run.
- `make check-repo` passes.
- Developer acceptance: the corpus 0.34.1 run made the page-and-version commit
  `98222ff` and the index commit `4d41d84`, installed to `~/.local/dev` (first
  in `type -a tam`; `tam --version` reads `dev 4d41d84`, not dirty) and pushed.
  The stop-before-commit and "already bumped" rerun were exercised by a failing
  test rather than by a lint failure.

Routed elsewhere: the non-interactive `tam explain`/`tam search` assertions that
still read the live corpus are on hold by the developer's decision (roadmap
section 4). Limits above still apply.
