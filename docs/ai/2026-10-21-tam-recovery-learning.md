# Maintaining recovery lessons (corpus 0.41.0)

- **Date:** 2026-10-21
- **Time:** 13:53 to 14:06, -04:00
- **Stage:** Develop; TCK25 Maintaining recovery slice
- **CLI Tool:** Codex CLI 0.163.0
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`)
- **Transcript:** Retained privately by the author.
- **Commits:** `447b60d` and the index-history follow-up updating this record.
- **Authorship:** The developer requested continuation; Codex planned, wrote and checked these lessons.
- **Checked against:** `476b499`, `ROADMAP.md`, `docs/plans/learning-levels.md`, `docs/corpus-pedagogy.md`, `docs/corpus-authoring.md`, `src/install.c`, `src/version.h`, `knowledge/version`, `tools/build-index.py`, `knowledge/coverage.json`; product `commands/tam-deploy/tam-deploy`; `tamlinux-packages/docs/deployment.md`; native `findmnt(8)`, `chroot(1)` and `tty(1)`; Btrfs and kernel primary documentation linked below.

## Guiding prompt

> We have been working in the tam repo. See what the last session accomplished and continue with the next chunk of work.

## Decisions and limits

The previous session delivered TCK23 pilot, TCK24 scaffolding and TCK25
Using migration at command 0.38.0 / corpus 0.40.0 (`476b499`). Its handoff
prioritized Maintaining recovery. This bounded slice follows the existing
teaching contract, with its execution plan written before the page changes.
Three new glossary pages take corpus minor 0.41.0; command is unchanged.

The lessons replace assumed repair targets, arbitrary generation activation,
uncaptured sample output and overbroad recovery promises with scratch or
read-only practice. They distinguish recorded product return from mutable
data and native boot/login recovery. SysRq is a reference and policy read,
never a command or keyboard exercise. A fixed pause is not proof of sync.
Boot, boot-fallback, Snapper, logs and power still require migration before
Maintaining enforcement; the Rescue path is not yet added. Pilot/learner,
physical-console and packaged acceptance remain unverified. No installation
or stage change is part of this work.

Coverage warnings fall from 38 to 34 and unmapped skill cells from 19 to 15;
21 cells still lack paths. These are source-metadata counts, not mastery claims.

## Primary references

- [Linux SysRq](https://docs.kernel.org/admin-guide/sysrq.html): policy bits, keyboard limitations, destructive operations and completion semantics.
- [Btrfs subvolumes](https://btrfs.readthedocs.io/en/latest/btrfs-subvolume.html): snapshots, nesting and explicit mount selection.
- [Deployment contract](https://github.com/greenermoose/tamlinux-packages/blob/develop/docs/deployment.md): exact retained selection, mutable state and native login boundary.

## Verification

- `make test`: passed; command/PTY tests, upgrade checks, seven renderer
  goldens, 32 lint tests, 20 builder tests, developer/install guards,
  repository consistency and corpus rendering/link checks. Live Ollama
  checks skipped because no server was available.
- `make corpus-and-test`: passed with all 107 guide pages and internal links.
- All seven displayed sessions rerun from the authored HTML with exact
  stdout matches and no stderr. Scratch setup, restore and cleanup also pass.
- All 24 changed/new page-and-level combinations render without control
  sequences; lesson answers remain available in pipe output. Both migrated
  ladders have no anatomy warnings; Maintaining remains warning-only.
- Six real PTY lesson/level views verify hidden answers, reveal/collapse,
  reset on return and the sticky lens. The goal path opens rollback at
  Intermediate, preserving its intended level.
- Relative planning/provenance links, public-boundary scan and
  `git diff --check`: clean. Index histories are rebuilt after the page
  commit and recorded separately. Source checks do not establish learner
  effectiveness, emergency-key behavior or successful system recovery.
