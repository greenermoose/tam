# The Linux input subsystem and evdev (corpus 0.59.0)

- **Date:** 2026-11-02
- **Time:** 11:03 to 13:50 -05:00
- **Stage:** Develop, corpus 0.59.0
- **CLI Tool:** Antigravity CLI 1.4.12
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `4b9634a`, and the follow-up index commit adding this record
- **Authorship:** Fred Horch directed: "Add a page for evdev to the tam knowledge corpus and include evdev in the glossary." Antigravity claimed paths with `tam-work`, authored `knowledge/ecosystem/evdev.html`, created companion glossary entry `knowledge/learn/glossary/evdev.html`, linked both in `knowledge/ecosystem/index.html` and `knowledge/learn/glossary/index.html`, bumped `knowledge/version` to 0.59.0, updated `ROADMAP.md` and `AI_PROVENANCE.md`, and regenerated corpus indexes.
- **Checked against:** `/proc/bus/input/devices`, `/dev/input/by-id/`, `/usr/include/linux/input.h`, `/usr/include/linux/input-event-codes.h`, `/usr/include/linux/uinput.h`, `/usr/include/libevdev-1.0/libevdev/libevdev.h`, `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `tools/lint-corpus.py`, `tools/build-index.py`, and `tests/guide.sh`.

## Guiding prompts

> Add a page for evdev to the tam knowledge corpus and include evdev in the glossary.

## Key decisions

- Authored `knowledge/ecosystem/evdev.html` under track `learn` with parent `ecosystem/index.html` (topic: `evdev`), presenting a comprehensive three-tier content-depth ladder:
  - **Beginner:** Locating input devices in `/proc/bus/input/devices` and persistent device symlinks in `/dev/input/by-id/`, understanding character device node naming (`event*`), and explaining device node permissions (`0660 root:input`).
  - **Intermediate:** Detailed `struct input_event` binary layout (time, type, code, value), core event types (`EV_SYN`, `EV_KEY`, `EV_REL`, `EV_ABS`, `EV_MSC`, `EV_SW`, `EV_LED`), roles of `libevdev` and `libinput`, component choice table, troubleshooting table, and completion/independent tasks.
  - **Expert:** Low-level `ioctl` primitives (`EVIOCGVERSION`, `EVIOCGID`, `EVIOCGBIT`, `EVIOCGRAB`, `EVIOCREVOKE`), Multi-Touch Type A versus Type B (Slotted) protocols with `ABS_MT_SLOT` and `ABS_MT_TRACKING_ID`, virtual device synthesis via `/dev/uinput`, unprivileged Wayland compositor seat handover (`seatd` / `systemd-logind` fd passing via `SCM_RIGHTS`), and `SYN_DROPPED` ring buffer overflow resynchronization.
- Authored companion glossary definition at `knowledge/learn/glossary/evdev.html` (topic: `glossary-evdev`) across Beginner, Intermediate, and Expert levels, and updated `knowledge/learn/glossary/index.html` in alphabetical order.
- Linked `ecosystem/evdev.html` in `knowledge/ecosystem/index.html`.
- Bumped corpus version in `knowledge/version` from 0.58.1 to 0.59.0 and updated `ROADMAP.md`.

## Verification

- `python3 tools/lint-corpus.py`: passes with 0 findings.
- `python3 tools/build-index.py`: generated `index.tsv` and companion files with 190 entries (2 new pages).
- `make corpus-and-test`: all linter checks, 32 linter unit tests, 21 index unit tests, and 12 guide verification tests pass cleanly.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/evdev`: renders clean formatted plain text across all levels with working links and folds.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe learn/glossary/evdev`: renders glossary entry across all three levels.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam explain evdev --pipe`: resolves both the ecosystem guide and glossary entry.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam search evdev --pipe`: lists both entries cleanly.
