# TCK25 Maintaining completion and enforcement (corpus 0.43.2)

- **Date:** 2026-10-25
- **Time:** 06:02 to 06:42 -04:00
- **Stage:** Develop; three corpus pages migrated/classified, track-wide enforcement enabled, command unchanged at 0.43.2.
- **CLI Tool:** Antigravity CLI 1.4.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Authorship:** The developer directed finishing the remaining Maintaining pages under TCK25. Antigravity CLI migrated `hardware/resources.html` and `ecosystem/packaging.html`, classified `maintain/index.html` as a hub, enabled track-wide build enforcement in `knowledge/learn/skills/skills.json`, and updated plan and roadmap records.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, `tools/build-index.py`, `tools/lint-corpus.py`, `knowledge/learn/skills/skills.json`; live captured `free -h`, `df -h /`, `nproc`, `uptime`, `pacman -Q bash`, `pacman -Qo /usr/bin/git`, and `home-manager generations` outputs; [free(1)](https://man7.org/linux/man-pages/man1/free.1.html), [ps(1)](https://man7.org/linux/man-pages/man1/ps.1.html), [uptime(1)](https://man7.org/linux/man-pages/man1/uptime.1.html), [pacman(8)](https://man.archlinux.org/man/pacman.8), [procps-ng](https://gitlab.com/procps-ng/procps), Linux kernel `/proc/meminfo` and `/proc/pressure/memory` documentation.

## Guiding prompts

> Examine the tam repo. Are there any open checklist items? What is the next slice to be done according to any open plans or the ROADMAP?

> Finish the remaining Maintaining pages under TCK25.

## Key decisions

- Use a corpus patch bump (0.43.2) because this slice restructures and migrates existing pages and introduces no new guide pages or command changes.
- `hardware/resources.html` was migrated to the full ladder anatomy with captured read-only examples for `free -h`, `df -h /`, `nproc`, and `uptime`. It distinguishes available memory from unallocated cache, explains load averages against logical core counts, details ZRAM swap in RAM and Pressure Stall Information, and maps to `hardware.beginner`, `hardware.intermediate`, and `hardware.expert`.
- `ecosystem/packaging.html` was migrated to the full ladder anatomy with captured read-only examples for `pacman -Q`, `pacman -Qo`, and Home Manager generation inspection. It teaches the distinction between base OS packages, declarative Tamlinux workstation assemblies, and user runtimes, and maps to `software.beginner`, `software.intermediate`, and `software.expert`.
- `maintain/index.html` was classified as `kind="hub"` with links to all 11 Maintain track guides and the Keep it healthy and Rescue goal paths.
- `"maintain"` was added to `"enforce_tracks"` in `knowledge/learn/skills/skills.json`, promoting anatomy and quiz warnings across both Using and Maintaining to fatal build errors.
- Anatomy warnings dropped from 24 to 22 across the whole corpus (the 22 remaining warnings belong exclusively to unmigrated tracks: Learning, Improving, and root index). Maintaining has zero warnings and zero build errors.

## Verification

- `make corpus`: cleanly generated index and sidecars with 110 entries.
- `python3 tools/lint-corpus.py`: zero errors or warnings.
- `python3 -m unittest tests/test_lint_corpus.py tests/test_build_index.py`: 52 tests passed (OK).
- `sh tests/guide.sh`: all 12 guide test assertions passed cleanly.
- `make corpus-and-test`: clean compilation and all checks passed.
- `report-coverage.py`: confirmed `enforced_parts: use, maintain`, unclassified pages fell from 51 to 48, unmapped pages fell from 89 to 87, and whole-corpus anatomy warnings fell to 22.
- `make check-repo`: passed.
- All captured outputs verified against live host command outputs.

Pilot/learner review and physical Linux-console/Foot acceptance of an exact packaged candidate remain unverified. No package deployment or stage promotion was performed.
