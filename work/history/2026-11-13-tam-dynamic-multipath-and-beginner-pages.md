# Dynamic multi-path engine, beginner backlog, and tools coverage gate (command 0.55.0 / corpus 0.81.0)

- **Date:** 2026-11-13
- **Time:** 05:25 -05:00 (multi-turn session concluding 2026-11-13)
- **Stage:** Develop; command `0.55.0` and corpus `0.81.0` add dynamic `<meta name="paths">` navigation, retire `beginner`/`intermediate`/`expert` hubs while preserving quizzes/capstones, add 17 remaining beginner pages plus `tam-journal` and `tam-file-select` guides, close skill/anatomy gaps, and gate `tamlinux-tools` coverage in `corpus-check`.
- **CLI Tool:** Antigravity CLI 1.5.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Primary implementation + follow-up corpus index refresh on `develop`
- **Authorship:** Fred Horch directed replacing `paths.json` and the `beginner`/`intermediate`/`expert` hubs with dynamic `<meta name="paths">` multi-path navigation in the C pager, then approved executing the full corpus overhaul and beginner backlog plan. Antigravity CLI implemented the schema-2 `index.paths.tsv` pipeline, C pager multi-path engine (`tam:paths`, `tam:path?id=`, sticky active path, course switching, detour recovery), migrated all paths and quizzes, authored the 17 beginner pages plus `tam-journal` and `tam-file-select` guides, eliminated remaining anatomy and skill gaps, integrated `report-component-coverage.py --check` for `tamlinux` and `tamlinux-tools` into `corpus-check`, updated authoring docs and roadmap/version records, and added unit, guide, and PTY multi-path tests.
- **Checked against:** `src/version.h`, `src/corpus.c`, `src/views.c`, `src/pager_nav.c`, `docs/plans/corpus/absolute-beginner-coverage.md`, `tamlinux-tools/src/tam-journal.c`, `tamlinux-tools/src/tam-file-select.c`, `tamlinux/commands/*`, `tamlinux/share/nftables/tamlinux-host.conf.template`, `tamlinux/desktop/install-product.py`, `knowledge/learn/skills/skills.json`, `tools/check-repo.py`, `build/tam --version`.

## Guiding prompts

> remove the three (beginner, intermediate, and expert). Put content where it makes the most sense after we define paths through the corpus that allow a reader to learn about a topic.
> The pager should dynamically construct possible paths. A user should pick the path they want to be on. When they get to a path that is on multiple paths, they should automatically stay on the path they are on. But they should have a way to change course and follow a new path if they want. And if they are not on any path, when they land on a page they should have a way to pick a path and then follow it.
> I accept the plan. Close the review window. Start working on it.
> Commit

## Key decisions

- Chose `<meta name="paths" content="<id>:<step>, ...">` (schema 2 `index.paths.tsv` sidecar and `P`/`S` rows) as the single authority for path ordering, keeping shared pages on multiple paths while preventing topic-slug collisions across 23 topic hubs under `learn/paths/`.
- Preserved active path per history frame (`active_path`) without mutating canonical page URLs in history, surfacing `Switch course` links at multi-path intersections, `Active path` + `Resume path` banners on off-path detours, and `Follow a path from here` pickers when unattached.
- Deleted `knowledge/beginner.html`, `knowledge/intermediate.html`, `knowledge/expert.html`, and `knowledge/paths.json` after migrating their unique quizzes and scratch capstones onto topic paths and pages (`learn/paths/first-steps`, `learn/paths/keep-it-healthy`, `shell/pipelines`, `desktop/workspaces`, `shell/jobs`, `maintain/updates`).
- Authored all 17 remaining beginner backlog pages across security, accounts, files, apps, and command-line guides, plus `use/tools/tam-journal.html` and `use/tools/tam-file-select.html`, closing every remaining anatomy warning and skill cell gap.
- Expanded `tools/report-component-coverage.py` to scan sibling `tamlinux-tools/src/` utilities as `kind="tool"` and gated `make corpus-check` with `--check --quiet`.

## Verification

- `python3 -B -m unittest discover -s tests -p 'test_*.py' -q`: Ran 147 tests OK.
- `make corpus-and-test`: `Corpus: 614 pages, 614 parsed, 614 rendered; checks passed.`, component coverage check passed (`254` components: `201` covered, `53` exempt, `0` gaps), 81 corpus tooling tests OK, and `tools/check-repo.py` passed.
- `python3 tools/report-coverage.py --warnings`: `anatomy warnings: 0`, `Gaps: none` across all skill cells.
- `tests/guide.sh` and `tests/render.sh`: All checks passed (including multi-path views, query order independence, and 7 render goldens).
- `tests/run.sh`: Passed 34 integration and PTY tests including `multi-path PTY navigation: course picker, switch course, path continuity, and detour recovery`, stopping inside the sandbox at `FAIL: --check storage_btrfs missing status pass`.
