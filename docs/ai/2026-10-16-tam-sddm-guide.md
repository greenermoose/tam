# SDDM Display Manager Guide and Corpus Bump (tam 0.26.0)

- **Date:** 2026-10-16
- **Stage:** Develop — Knowledge Corpus Expansion (`ecosystem/sddm.html`) and Knowledge Corpus Version Bump to 0.26.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.4.0
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested adding a page to `tam` for SDDM after noting that `tam explain SDDM` found nothing, specifying that this should be exclusively work in the knowledge corpus and should bump the version number of the corpus, and requesting to get the most recent version of `tam` and the corpus into Tamlinux 0.4.4c. Antigravity claimed the affected corpus and documentation paths under `tam-work`, authored `knowledge/ecosystem/sddm.html` across beginner, intermediate, and expert depths, bumped the corpus version to 0.26.0 in `knowledge/version`, updated `knowledge/ecosystem/index.html` and `knowledge/index.html`, regenerated the index (`tools/build-index.py`), updated golden snapshots, verified command and guide test suites, and coordinated with Codex for packaging integration into Tamlinux 0.4.4c.

## Guiding prompt

> Add a page to tam for SDDM. I tried tam explain SDDM and it found nothing. I believe this should be exclusively work in the knowledge corpus and should bump the version number of the corpus. Ask if you have any questions. Get the most recent version of tam and the corpus into Tamlinux 0.4.4c, which codex is working on now.

## Decisions and scope

- **New Knowledge Corpus Page (`knowledge/ecosystem/sddm.html`):**
  - Explains the architectural role of a display manager (login manager) in the Tamlinux boot and session startup sequence.
  - Documents why Tamlinux selects SDDM (Simple Desktop Display Manager): Wayland-native design, declarative Qt 6 / QML themes matching Quickshell and KDE application styling, low resource footprint, and zero mandatory resident background daemons.
  - Compares SDDM against alternatives evaluated for Tamlinux: GDM (rejected due to heavy GNOME daemons and dependencies), LightDM (rejected as X11-first architecture), greetd (viable minimalist alternative evaluated for future lightweight runit/terminal profiles), and ly (curses TUI greeter suited for text profiles).
  - Documents the Tamlinux login flow:
    - *Autologin:* Fast, frictionless startup on single-user workstations (`User=fred`, `Session=tamlinux.desktop`, `Relogin=false`).
    - *Manual logout:* Returning to the graphical greeter rather than re-triggering autologin.
    - *Session selection:* Curated session menu presenting primary desktop vs maintenance/recovery entries.
  - Details configuration drop-in precedence: `/usr/lib/sddm/sddm.conf.d/` vendor defaults overridden by `/etc/sddm.conf.d/` administrator drop-ins.
  - Details Wayland session discovery in `/usr/share/wayland-sessions/` and `/etc/sddm/wayland-sessions/`, and execution of `/usr/bin/tam-session`.
  - Documents runtime state tracking in `/var/lib/sddm/state.conf` and theme testing via `sddm-greeter-qt6 --test-mode`.
  - Documents PAM authentication integration and seat management via `systemd-logind` / `seatd`.
  - Details emergency recovery via text virtual consoles (`Ctrl+Alt+F2` through `F6`), log inspection (`journalctl -u sddm`), and service restarts.
  - Provides a 3-depth ladder (`beginner`, `intermediate`, `expert`).
- **Corpus Version Bump (0.26.0):**
  - Updated `knowledge/version` from `0.25.1` to `0.26.0`.
  - Preserved `TAM_COMMAND_VERSION` at `0.25.1` in `src/version.h` (exclusively knowledge corpus work).
- **Navigation, Index, and Tests:**
  - Added links to `ecosystem/sddm.html` in `knowledge/ecosystem/index.html` and `knowledge/index.html`.
  - Rebuilt index with `tools/build-index.py` (46 entries).
  - Validated topic resolution: `tam explain SDDM`, `tam explain sddm`, `tam explain login`, `tam explain "display manager"`.
  - Regenerated golden text snapshots (`tests/snapshots/ecosystem-sddm.txt`, `ecosystem-index.txt`, `index.txt`).
  - Ran `make test-corpus` (`tests/guide.sh`) and `make test-command` (`tests/run.sh`).

## Verification

- `python3 tools/build-index.py`: Validated metadata and breadcrumbs; generated 46 entries.
- `make test-corpus`: All 10 guide tests passed cleanly.
- `tests/run.sh build/tam`: All command tests passed cleanly (including decoupled corpus version checks).
- Interactive verification: `tam explain --pipe SDDM` resolves to `Display Managers: SDDM` topic guide.
