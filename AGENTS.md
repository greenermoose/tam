# Working on tam with an AI agent

Read this before changing anything in this repository. It applies to every
agent (Claude Code, Codex, Antigravity, OpenCode or another). The developer
directs the work. The custom delegation system was retired on 2026-10-16;
delegation through skills supplied by OpenAI or another trusted source is
allowed within the developer's task (clarified 2026-10-17). All agents follow
the same path-claim rules; old briefs do not assign work or override them.

## Start here

- `ROADMAP.md`, the "What Needs to Be Done Next" section: the next version
  and its scope.
- `docs/plans/development.md`: ownership, lanes, versioning and the decision
  record. Read the section you need, not the whole file.
- A milestone's own plan in `docs/plans/`, when the roadmap links one.
- `docs/plans/README.md`: every plan's current status and replacement/evidence.
- Before creating, revising or closing a plan, follow `docs/plan-maintenance.md`.
  Update the plan index, roadmap and current development contract together.
  Read a closed plan as history; its old task list assigns no new work.

The next approved knowledge-system milestone is corpus 0.31.0 with companion
command 0.30.0. Start `docs/plans/knowledge-system-0.31.0.md`; it contains the
implementation sequence and handoff. Do not revive the former seven-part
contents, three-track model or withdrawn parser/installer version reservations.

Discuss a design choice with the developer before you build it. A version
number in a request is data, not permission to release anything.

## Share the clone; claim what you edit

Several agents work in this one checkout at the same time. Do not create a
Git worktree or a copy of the repository. Before editing, claim exactly the
paths you will change with `tam-work`, run inside this clone:

```sh
export TAM_WORK_OWNER=<your tool, e.g. agy> TAM_WORK_SESSION=<your conversation id>
tam-work begin --scope src/pager.c --scope tests/run.sh --note 'what and why'
```

Set the owner and session once per conversation; without them `tam-work`
refuses to start. Run `begin` again to add paths. Commit only your paths
(`git commit -m '...' -- <paths>`); never `git add -A`, `git add .` or
`git commit -a`. Never switch branches, stash, reset, rebase or amend in this
shared checkout. Finish with `tam-work finish --note 'outcome'` once the
work is committed, or when it is cancelled.

## Lanes

| Lane | Paths | Build | Tests |
| --- | --- | --- | --- |
| Command | `src/`, `bin/`, `completions/`, `man/`, `tests/render/`, `tests/render.sh` | `make command` | `make test-command` |
| Corpus | `knowledge/`, `tools/`, `tests/test_lint_corpus.py` | `make corpus` | `make test-corpus` |
| Shared | `Makefile`, `tests/run.sh`, `tests/guide.sh`, `ROADMAP.md`, `README.md`, `AI_PROVENANCE.md`, `docs/plans/` | | `make test` |

Two agents can work at once if each claims one lane. Claim both lanes for a
change that couples them, such as new renderer support and the pages that
use it. Shared files move by handoff: claim one only while you edit it. A
new file such as your own `docs/ai/` record conflicts with nobody.

A corpus-only release stays inside the corpus lane: new pages are discovered by
`tests/guide.sh`, and the expected corpus version is read from
`knowledge/version`.
The corpus 0.31.0 knowledge-system plan also needs command support; claim the
affected paths in both lanes for those slices and keep the versions independent.

Before you write or change a corpus page, read `docs/corpus-authoring.md`:
the sourcing rule, where each kind of fact lives, the public boundary and the
steps for adding a page. Corpus audits follow `docs/corpus-audit.md`.

## Versions

- **Command:** `TAM_COMMAND_VERSION` in `src/version.h`, the matching
  assertion in `tests/run.sh`, and a line in the decision record of
  `docs/plans/development.md`.
- **Corpus:** `knowledge/version` only.
- New capability or new pages take the minor bump; fixes and corrections
  take the patch bump. Bump in the same change that adds the feature, and
  update the version header in `ROADMAP.md`.

## Build, test and install

- `make test` runs both suites. Renderer goldens are small fixture pages in
  `tests/render/`; after an intended rendering change, refresh them with
  `UPDATE=1 tests/render.sh build/tam` and read the diff before committing.
  Corpus pages have no snapshots.
- The live `tam ask` checks need a local Ollama server with the models named
  in `tests/run.sh`; without one they are skipped with a note.
- Run builds and tests in the foreground with a time limit. Do not wait on
  background tasks, and do not search large system trees such as
  `/usr/share/doc` recursively; both have stalled earlier sessions.
- Do not install a build as a side effect of planning or content changes.
  Test/Run delivery uses a recorded `tamlinux-packages` assembly on the
  developer's exact-candidate instruction. Any explicitly requested development
  install follows the component-development runbook, uses the development
  prefix, and records/removes that override as the runbook requires. It is not
  packaged acceptance. The installed command reads its installed corpus;
  source verification must explicitly select the checkout corpus.

## Branches

Work on `develop`; push only `develop` (check `git log @{u}..` for other
agents' commits first). `test`, `main`, tags and GitHub releases move only
on the developer's explicit Test, Run or Release instruction.

## Provenance

Follow `AI_PROVENANCE.md`. Every AI-assisted commit carries
`Co-authored-by`, `AI-Tool: <tool> <version>` and `AI-Model: <model-id>`,
with the tool version checked from the running binary, not remembered.
Each session adds one record under `docs/ai/` and a row in
`AI_PROVENANCE.md`, quoting the developer's prompts verbatim. Start the
record from `docs/ai/TEMPLATE.md`: it includes the session's start and end
time and a `Checked against:` line naming the sources you confirmed facts
from, and it is sized to the work.

Public files never contain conversation or session IDs, transcript or
store paths, home-directory paths, or the names of private repositories.
Before pushing, search your diff for them; a shortened ID can only be
caught by reading.
