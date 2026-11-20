# Install only a clean checkout

- **Date:** 2026-10-20
- **Time:** 17:20 to 17:50 -04:00; guide.sh follow-up 18:20 to 18:30 -04:00
- **Stage:** Develop: `make install` refuses a dirty checkout; tooling and tests only, no version change
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `ca25c8f`, `7914ae5`, and the commit that adds the guide.sh follow-up
- **Authorship:** Fred asked for the root cause of a dirty development install and a fix that holds for every agent; Claude Code wrote the Makefile guard, its tests, the `AGENTS.md` and decision-record text, and committed the missing index follow-up.
- **Checked against:** `Makefile`, `tools/build-index.py`, `tools/dev.py`, `tests/guide.sh`, `AGENTS.md`, `git log -- knowledge/index.tsv`, the `tamlinux-packages` tam recipe

## Guiding prompts

> I asked opencode to update some pages in the tam knowledge corpus and it left my tam in this state: [`tam --version` output showing `knowledge corpus 0.35.1 (dev ae1fa41-dirty)`]
>
> This indicates there is a problem in the harness or makefiles or build scripts. Agents should know that I don't want to build from a dirty repo.
>
> Find the root cause of this agent failure and fix so all agents behave correctly when building in the tam repo.
>
> Do item 2

(Item 2 was the staged-install check in `tests/guide.sh`, which always skipped.)

## Key decisions

- Root cause: the committed index records each page's commit count and date, so
  a page commit always leaves it one commit behind. `make dev` makes the
  follow-up index commit; other agents had only a sentence in `AGENTS.md`, and
  `make install` installed whatever the tree held. Fourteen earlier "corpus
  index" commits show how often the follow-up is needed.
- Keep the index committed: the Nix package builds without `.git` and reads
  revisions from the committed index.
- Guard the install, not the build: agents must build and test edits before
  committing. Staged `DESTDIR` installs (packaging, `tests/guide.sh`) and
  sources without their own `.git` are exempt.
- No override switch, per the developer's instruction not to build from a dirty
  repository. `make dev` drops its "another session" dirty note, now unreachable.
- `tests/guide.sh` named `LIBTAM_PREFIX=/usr/local` for its staged install,
  a default from before the Makefile found libtam with pkg-config, and treated
  any failure as "make unavailable". It now lets the Makefile find libtam,
  skips only when `make` is missing, and fails with the build log otherwise.
- Tooling only, so no component version change, following the TCK13/TCK14
  precedent.

## Verification

`python3 -m unittest -q tests/test_install_guard.py`: 7 tests pass (clean,
edited page, untracked source, index-only hint, outside paths, `DESTDIR`, no
`.git`). `make test` passes through `test_install_guard.py`, then stops at
`check-repo` on a plan index row missing since `56cc928`, which this work
does not touch; `test_check_repo.py` and `tests/guide.sh` pass when run
directly. After `ca25c8f`, `make corpus` leaves the index unchanged.

Follow-up: `make test` passes, and `tests/guide.sh` now reports "ok - staged
install relocates and finds the corpus". With `LIBTAM_PREFIX=/nonexistent` it
prints the build log and fails instead of skipping.
