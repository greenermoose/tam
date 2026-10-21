# Learning levels pilot (corpus 0.38.0)

- **Date:** 2026-10-21
- **Time:** 07:29 to 07:43, -04:00
- **Stage:** Develop; TCK23 source, with developer pilot review pending
- **CLI Tool:** Codex CLI 0.163.0
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`)
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record and its corpus-index follow-up.
- **Authorship:** The developer requested continuation and review; Codex wrote the pilot content and synchronized its status.
- **Checked against:** `docs/plans/learning-levels.md`, `docs/corpus-pedagogy.md`, templates, `src/fold.c`, `src/pager.c`, `src/tam.c`, `src/version.h`, `knowledge/version`; product `desktop/session/hypr/tamlinux/bindings/{applications,clipboard,utilities}.lua`, `commands/tam-update/tam-update`; `tamlinux-packages/docs/deployment.md`; native `pacman -Q` and `-Qo`; the primary references linked below.

## Guiding prompt

> Check to see if there is anything left to do on the learning levels project in the tam repo. If there is, work on that project. If there is not, report back on the status. Review the work done and let me know if you see room for improvement.

## Decisions and review

TCK20–TCK22 were delivered at the starting head `56a2866`. TCK23 source
was next; its explicit developer review gate precedes TCK24. Command remains
0.38.0, corpus takes the next minor, 0.38.0. No installation, stage move or
release is part of this work.

The pipeline's process-count example had uncaptured output and a self-matching
grep. It now uses fixed input. Permission examples now create their own files,
distinguish SetGID from sticky, avoid assuming a fixed-width mode type and
explain creation masks and directory access. Update examples distinguish
package ownership from repository origin, name replacement from truncation,
and software-generation recovery from mutable-data recovery. The maintenance
path's capstone prepares a recovery decision without changing the workstation.

Every displayed session was run in a temporary directory or as a read-only
native query, then rerun from the authored HTML and compared byte for byte.
The new pages remain offline; linked web sources are labeled online. No
unsupported case study or rationale was invented to fill a template block.

Remaining improvement priorities:

- Run the planned novice and experienced-reader think-aloud tests. Source
  checks establish rendering and navigation, not whether the lessons teach.
- Add the planned glossary, placement, skills map and coverage report after
  pilot review; make anatomy enforcement gradual during TCK25 migration.
- Prioritize older maintenance content in that migration. In particular,
  `knowledge/maintain/rollback.html` still describes direct generation
  activation, which is not the recorded Tamlinux product-return contract.
  Keep it healthy links its level-1 concepts only and explicitly does not
  use its recovery operations as exercises.
- Command checks still include older hard-coded live-corpus wording. The
  updated-page assertion exposed by this slice moves to corpus checks;
  level-filtering assertions now use the existing small pipeline fixture.
  Topic lookup checks retain operator aliases and verify the destination
  rather than summary prose. Broader separation remains follow-up.

## Primary references checked

- [GNU Bash pipelines](https://www.gnu.org/software/bash/manual/html_node/Pipelines): pipeline status and execution environment.
- [GNU Bash redirections](https://www.gnu.org/s/bash/manual/html_node/Redirections.html): left-to-right descriptor changes.
- [Linux pipe(7)](https://man7.org/linux/man-pages/man7/pipe.7.html): byte streams, capacity and EOF.
- [GNU mode structure](https://www.gnu.org/s/coreutils/manual/html_node/Mode-Structure.html) and [directory SetGID](https://www.gnu.org/software/coreutils/manual/html_node/Directory-Setuid-and-Setgid.html): special bits.
- [Linux umask(2)](https://man7.org/linux/man-pages/man2/umask.2.html) and [inode(7)](https://man7.org/linux/man-pages/man7/inode.7.html): access metadata and creation masks.
- [Linux unlink(2)](https://man7.org/linux/man-pages/man2/unlink.2.html): lifetime after name removal.

## Verification

- `make test`: passed, including command/PTY cases, upgrade checks, seven
  renderer goldens, lint, index generation, developer/install guards, repository
  consistency and the live-corpus suite. Live Ollama checks skipped because
  no local server was available.
- `make corpus-and-test`: passed; all 73 guide pages render and internal links
  resolve. Both goal paths are discoverable through search.
- All 17 displayed command sessions rerun from the authored HTML with exact
  stdout comparisons; no stderr or failed commands.
- All 24 pilot/path page-and-level combinations (0–3) render without terminal
  controls. Real PTY checks confirm beginner folds and quiz counts on all
  pilots and Welcome → goal hub → First steps → capstone solution reveal.
- Public-provenance scan, repository checks and `git diff --check`: clean.
- Indexes rebuilt after the page commit and committed separately to include
  the new page histories.

Developer pilot review, learner tests and packaged acceptance remain pending.
