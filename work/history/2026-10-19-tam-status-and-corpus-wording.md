# Status refresh and current-versus-planned corpus wording (corpus 0.33.2)

- **Date:** 2026-10-19
- **Time:** 22:10 to 22:40 -04:00
- **Stage:** Develop; roadmap, plan index, `AGENTS.md` and twelve corpus pages
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `839f459` (roadmap and plan index; committed without the
  `AI-Tool`/`AI-Model` trailers, which this record supplies) and the commit
  that adds this record
- **Authorship:** The developer asked for a review of out-of-date status files
  and a corpus pass separating current state from future plans. Claude Code
  found the stale statements and wrote the edits.
- **Checked against:** `src/version.h`, `knowledge/version`, `git show
  <branch>:knowledge/version` for `develop`, `test` and `main`, `git tag`,
  `Makefile`, `src/tam.c`, `docs/plans/upgrade.md`, `docs/plans/development.md`,
  packaged and development `tam --version`; `tamlinux` repository
  `docs/botanical-system-architecture.md`, `docs/plans/0.4.4-boot-recovery.md`,
  `commands/tam-update/tam-update`, `VERSIONING.md`, `CHANGELOG.md`, `README.md`

## Guiding prompts

> Please carefully review the current status and the ROADMAP for the Tamlinux
> project. Are there any files that are out of date and need to be updated? I'd
> like to do a pass on that, and then go through the tam knowledge corpus to
> clarify some wording so that it is more clear what is the current status of
> the project and what are future plans that have not yet been done.

## Key decisions

- The roadmap's section 3 now states what each branch carries, as a dated
  snapshot that names its authoritative sources, instead of narrating plans
  that have since completed.
- Corpus fixes are corrections only (patch bump). The terminology page named
  systemd-boot as the pot, which the public architecture document rejects;
  Limine replaces it. Void, other root-stocks and the Sway steps are marked
  planned; prepared Sway adapters are acknowledged without calling any step
  delivered.
- Invented example output (`tam --version`, a compile line) is replaced with
  real output from the 0.4.4-k Test package and the development build.
- For the developer to confirm: the welcome page said he uses Tamlinux on his
  "workstations"; only one workstation runs it, so it now says "workstation".

## Verification

`make corpus` regenerated 67 index entries; `make corpus-and-test`: lint, unit
and guide tests pass (staged-install test skipped as usual). Edited pages read
correctly with `build/tam show --pipe`.
