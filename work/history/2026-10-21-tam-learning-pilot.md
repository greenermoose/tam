# Learning levels pilot, scaffolding and Using migration (corpus 0.38.0–0.40.0)

- **Date:** 2026-10-21
- **Time:** 07:29 to 11:58, with a developer continuation; -04:00
- **Stage:** Develop; TCK23, TCK24 and TCK25 Using source, with developer pilot review pending
- **CLI Tool:** Codex CLI 0.163.0
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`)
- **Transcript:** Retained privately by the author.
- **Commits:** TCK23 `80bfa7c`, index `8859559`; TCK24 `d43fe8e`, index `91b46d0`; the TCK25 Using commit updating this record and its index follow-up.
- **Authorship:** The developer requested continuation and review; Codex wrote the pilot content and synchronized its status.
- **Checked against:** `docs/plans/learning-levels.md`, `docs/corpus-pedagogy.md`, templates, `src/fold.c`, `src/pager.c`, `src/tam.c`, `src/version.h`, `knowledge/version`; product `desktop/session/hypr/tamlinux/bindings/{applications,clipboard,utilities}.lua`, `commands/tam-update/tam-update`; `tamlinux-packages/docs/deployment.md`; native `pacman -Q` and `-Qo`; the primary references linked below.

## Guiding prompt

> Check to see if there is anything left to do on the learning levels project in the tam repo. If there is, work on that project. If there is not, report back on the status. Review the work done and let me know if you see room for improvement.

> Continue

## Decisions and review

TCK20–TCK22 were delivered at the starting head `56a2866`. TCK23 source
was next; its explicit developer review gate precedes TCK24. Command remains
0.38.0, corpus takes the next minor, 0.38.0. No installation, stage move or
release is part of this work. After the agent reported the pilot-review gate,
the developer directed "Continue". TCK24 proceeds on that direction while
pilot and learner review remain unverified. Its baseline is `8859559`;
command stays 0.38.0 and corpus takes minor 0.39.0.

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
- TCK24 now provides glossary, placement, skills map and coverage. TCK25
  still needs gradual content migration and anatomy enforcement.
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

The first verification list records the TCK23 source checks. TCK24 checks are
recorded below it.

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

## TCK24 continuation

Optional metadata permits migration without relabeling old pages by guesswork.
The established TSV remains ten columns; JSON adds fields and new TSV
sidecars project kind/skills. The generated report separates pages from paths
and includes every registered cell, so absence is visible. Anatomy/quiz gaps
warn rather than breaking legacy content. No learner scores or history are
stored.

The 12 glossary terms absorb the botanical analogy while distinguishing
concepts from delivered architecture. The old terminology address is retained,
with unsupported claims of an installed signed immutable nut removed. Public
design evidence is `docs/plans/user-guide-and-man-page.md` and product
`docs/plans/base-operating-system.md`; actual package boundaries are checked
against the deployment contract. Installed GNU Info copying notices confirm
the Bash/Coreutils documentation license; repository LICENSE confirms the
deployment documentation's repository license.

The initial report has 36 cells, 24 page gaps, 30 path gaps, and 40 legacy
sections with anatomy warnings. The three pilot pages have no anatomy gaps.
Those counts are a migration baseline, not learner-test results.

Real link checks exposed that the current loader cannot resolve a cross-page
fragment such as `index.html#bash`. Bibliography entries therefore get their
own pages, with stable reference IDs inside them. Citations link those full
pages; the builder rejects cross-page glossary fragments. This preserves
compatibility without changing the command or claiming working fragment
navigation. The existing source checks caught the unusable links before commit.

- `make test` and `make corpus-and-test`: passed with all 96 guide pages,
  32 lint tests, 17 builder tests, developer-loop and installation guards,
  command/PTY tests, renderer goldens, repository consistency and live-corpus
  link resolution. Optional live Ollama checks remain skipped without a server.
- Metadata fixtures check unknown kinds/IDs, wrong-level/duplicate skills,
  malformed registries, dangling teaching links, warning-only anatomy gaps,
  page/path coverage distinctions and atomic preservation of previous outputs.
  All six generated outputs participate in unchanged-output/build tests.
- All 92 new scaffolding page/level combinations render without control
  sequences. Real-pager checks confirm placement sets Expert, and bibliography
  and glossary links open from a pilot while preserving Intermediate.
- All 17 pilot sessions and the 24 pilot/path level combinations still pass
  their exact-output/rendering checks after vocabulary/citation conversion.
- `git diff --check`, repository checks and public-provenance scan: clean.

TCK25 migration and learner acceptance remain open. Completing these source
checks changes no installed package and promotes no lifecycle stage.

## TCK25 Using migration

Using was offered as the recommended first migration part, with Maintaining
as the alternative. No different priority was supplied before work proceeded.
The execution detail was written in the authoritative plan before implementation.
Baseline `91b46d0`; command 0.38.0 unchanged, corpus minor 0.40.0 adds the
Command-line fluency path. All six logical Using pages are classified, with
instructional skill mappings. The validated registry policy now enforces
anatomy/quiz checks in Using while other parts retain warnings.

The jobs page no longer teaches a name-wide process kill. Its captured
examples start/wait/signal only their own disposable child. They distinguish
signal delivery from cleanup, existence from identity and a shell job from
a process. Desktop and workspace practice has explicit starting/undo steps,
empty-window scope, choices and folded checkpoints; it is sourced against
product `bindings/tiling.lua`, `applications.lua` and `utilities.lua` and is
not a live GUI acceptance claim. Seven glossary terms support the new pages.

All three level paths now use stages, checkpoints and a folded capstone.
A goal path's skills can name its visited levels even when its page metadata
advertises the starting level. Ordinary page mappings still reject unoffered
levels. The first coverage report after this migration has 19 page gaps, 21
path gaps and 38 warning sections elsewhere. Catalog assertions drain grep
output to avoid irrelevant broken-pipe diagnostics as the catalog grows.

Additional primary checks: [Bash signals](https://www.gnu.org/s/bash/manual/html_node/Signals.html),
[signal(7)](https://man7.org/linux/man-pages/man7/signal.7.html) and
[kill(2)](https://man7.org/linux/man-pages/man2/kill.2.html). Further checked
against `tools/build-index.py`, `tools/report-coverage.py`, `tools/dev.py`,
Makefile and their fixtures.

Verification before the Using commit:

- `make test` and `make corpus-and-test`: passed, including all 104 guide
  pages, 20 builder fixtures, developer/install guards, command/PTY behavior,
  renderer goldens, repository checks and complete corpus link resolution.
  Live Ollama checks skipped without a local server.
- Four new displayed sessions reproduce exact output; 28 new teaching/path
  page-and-level combinations and 28 additional glossary views render.
  Real-pager quiz toggles pass for the newly migrated teaching pages and
  level paths; the goal hub opens the Command-line fluency capstone.
- Enforcement fixtures prove completed-part failures, warning-only behavior
  elsewhere, invalid-policy rejection and multi-level goal-path mappings.
  Optional metadata sidecars omit unsupplied values; this avoids meaningless
  empty-field rows while JSON/coverage retain the absence explicitly.
- The roadmap now distinguishes cached remote source refs from stale local
  stage branches and installed selections: `origin/test` is 0.36.0 / 0.37.0
  at `c9b6c5d`; `origin/main` is 0.31.1 / 0.32.1 at `230594d`. No refs moved.
- Repository/provenance scans and `git diff --check`: clean. Index histories
  are regenerated in the separate follow-up commit.

Maintaining, Learning, Installing and Improving migration remain. Maintaining's
older recovery examples deserve the next bounded plan; learner and packaged
acceptance remain unverified.
