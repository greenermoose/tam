# TCK25 Improving track migration slice 2 (corpus 0.43.4)

- **Date:** 2026-10-25
- **Time:** 08:37 to 09:12 -04:00
- **Stage:** Develop; two corpus pages migrated, desktop.expert page gap resolved, command unchanged at 0.43.2.
- **CLI Tool:** Antigravity CLI 1.4.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** Implementation commit and the generated-index follow-up containing this line.
- **Authorship:** The developer directed continuing TCK25 Improving track migration with the next slice: `improve/terminal.html` and `improve/efficiency.html`, claiming scopes first under `AGENTS.md`. Antigravity CLI claimed the scopes, migrated `improve/terminal.html` and `improve/efficiency.html` to the full ladder anatomy with captured read-only examples, mapped skills (`customizing.beginner`, `customizing.intermediate`, `desktop.expert`, `hardware.beginner`, `hardware.intermediate`, `hardware.expert`), resolved the `desktop.expert` page gap, reduced corpus-wide anatomy warnings from 18 to 14, and updated plan and roadmap records.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, `tools/build-index.py`, `tools/lint-corpus.py`, `knowledge/learn/skills/skills.json`; live captured Micro scratch editing, `stat(2)` live disk reload configuration (`reload auto`), `foot 1.28.0` Wayland terminal client/compositor boundaries (`$XDG_RUNTIME_DIR/wayland-*`, `wl_surface.damage_buffer`, `WAYLAND_DEBUG=1`), sub-10ms shell startup latency benchmarking (`time bash --norc -i -c exit`), `vmstat 1 3` processor context switch and interrupt rates, `inotifywait -e modify` kernel notifications, and $O(1)$ `epoll(7)` sleep semantics; [micro(1)](https://man7.org/linux/man-pages/man1/micro.1.html), [foot(1)](https://man7.org/linux/man-pages/man1/foot.1.html), [foot.ini(5)](https://man7.org/linux/man-pages/man5/foot.ini.5.html), [inotifywait(1)](https://man7.org/linux/man-pages/man1/inotifywait.1.html), [epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html), [epoll_wait(2)](https://man7.org/linux/man-pages/man2/epoll_wait.2.html), [vmstat(8)](https://man7.org/linux/man-pages/man8/vmstat.8.html).

## Guiding prompts

> In the tam repo, we are completing checklist item TCK25. The Using and Maintaining tracks are fully migrated and enforced, and the Improving track hub, scripting, and lua lessons landed in corpus 0.43.3. Continue TCK25 Improving track migration with the next slice: improve/terminal.html and improve/efficiency.html. Follow tam/AGENTS.md and claim scopes before editing.

## Key decisions

- Use a corpus patch bump (0.43.4) because this slice restructures and migrates existing lessons without introducing new guide pages or command changes.
- `improve/terminal.html` was migrated to the full ladder anatomy with captured read-only Micro scratch editing examples, live disk reload configuration (`reload auto`), Wayland client/compositor boundaries (`wl_shm`, `wl_surface.damage_buffer`, `WAYLAND_DEBUG=1`), sub-10ms shell startup latency benchmarking (`time bash --norc -i -c exit`), and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to `customizing.beginner`, `customizing.intermediate`, and `desktop.expert`.
- `improve/efficiency.html` was migrated to the full ladder anatomy with captured read-only `vmstat 1 3` processor wakeup and context switch measurements, event-driven kernel filesystem monitoring with `inotifywait -e modify`, $O(1)$ `epoll(7)` multiplexing, Wayland socket event dispatching, and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to `hardware.beginner`, `hardware.intermediate`, and `hardware.expert`.
- Whole-corpus anatomy warnings dropped from 18 to 14 (all 14 remaining warnings are in unmigrated lessons). Unclassified pages dropped from 45 to 43; unmapped pages dropped from 87 to 83. The page gap for `desktop.expert` was resolved.

## Verification

- `make corpus`: cleanly generated index and sidecars with 110 entries.
- `python3 tools/lint-corpus.py`: zero findings or errors.
- `python3 -m unittest tests/test_lint_corpus.py tests/test_build_index.py`: 52 tests passed (OK).
- `sh tests/guide.sh`: all 12 guide test assertions passed cleanly.
- `make check-repo`: passes cleanly with updated version header in `ROADMAP.md`.
- `coverage.json`: confirmed warnings dropped from 18 to 14, unclassified pages fell from 45 to 43, unmapped pages fell from 87 to 83, and `desktop.expert` shows `page_gap: false`.
- All captured outputs verified against live host command outputs.

Pilot/learner review and physical Linux-console/Foot acceptance of an exact packaged candidate remain unverified. No package deployment or stage promotion was performed.
