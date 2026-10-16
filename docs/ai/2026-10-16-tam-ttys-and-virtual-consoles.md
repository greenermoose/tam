# TTYs, Virtual Consoles, and Terminal Architecture Guide and Corpus Bump (tam corpus 0.27.0)

- **Date:** 2026-10-16
- **Stage:** Develop — Knowledge Corpus Expansion (`ecosystem/ttys-and-virtual-consoles.html`) and Knowledge Corpus Version Bump to 0.27.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.4.0
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred requested authoring the VT page for the `tam` knowledge corpus, bumping the corpus version, and installing the updated version for testing. Antigravity claimed the affected corpus and documentation paths under `tam-work`, authored `knowledge/ecosystem/ttys-and-virtual-consoles.html` with beginner, intermediate, and expert depth ladders adhering to the planned specification, bumped `knowledge/version` from 0.26.1 to 0.27.0, integrated cross-links across `knowledge/ecosystem/index.html`, `knowledge/index.html`, `knowledge/maintain/recovery.html`, and `knowledge/ecosystem/sddm.html`, regenerated the index (`tools/build-index.py`), updated golden snapshots, passed the command and guide test suites, updated `ROADMAP.md` and `docs/plans/development.md`, and installed the updated build under `~/.local/dev`.

## Guiding prompt

> Write the VT page for the tam knowledge corpus. Bump the corpus version. Install the updated version so I can test it when you have it ready.

## Decisions and scope

- **New Knowledge Corpus Page (`knowledge/ecosystem/ttys-and-virtual-consoles.html`):**
  - Clarifies the conceptual distinctions and interactions across the three terminal layers: physical serial teleprinters (TTYs), kernel virtual terminals / consoles (VTs, `/dev/tty1`–`/dev/tty63`), and software pseudo-terminals (PTYs, `/dev/ptmx` and `/dev/pts/N`).
  - *Beginner Depth:*
    - Explains what virtual consoles are as full-screen character displays managed directly by the Linux kernel beneath any graphical desktop.
    - Documents console switching hotkeys (`Ctrl+Alt+F2` through `F6` to switch to text consoles, `Ctrl+Alt+F1` to return to the Wayland desktop, and `Fn` key modifier notes on laptops).
    - Explains the emergency escape hatch: using virtual consoles to safely inspect and terminate frozen applications without risk of filesystem corruption or unwritten cache loss from hard power cycling.
  - *Intermediate Depth:*
    - Details the TTY line discipline (`N_TTY`): canonical (cooked) mode vs. raw mode, line buffering, and signal generation (`Ctrl+C` for `SIGINT`, `Ctrl+\` for `SIGQUIT`, `Ctrl+Z` for `SIGTSTP`, `Ctrl+D` for EOF).
    - Details pseudo-terminal architecture: PTY master/slave multiplexer pairs for graphical terminal emulators (`foot`), terminal multiplexers (`tmux`), and remote SSH connections.
    - Documents controlling terminals (`/dev/tty`), process sessions, and session leaders (`setsid`).
    - Provides inspection commands: `tty`, `stty -a`, `fgconsole`, `chvt`, and `w`.
  - *Expert Depth:*
    - Explores the kernel virtual terminal subsystem in `drivers/tty/vt/`, terminal operating modes (`KD_TEXT` with `fbcon` vs `KD_GRAPHICS` for direct GPU scanout), and VT switching ioctl controls (`VT_SETMODE`, `VT_ACTIVATE`).
    - Explains seat management and unprivileged hardware access: `seat0`, mediation by `systemd-logind` or `seatd`, and device file descriptor passing via `SCM_RIGHTS`.
    - Details the DRM KMS master handover sequence: compositor dropping DRM master (`DRM_IOCTL_DROP_MASTER`) when switching to a text console, kernel switching modes to `KD_TEXT`, and restoring pipelines with `DRM_IOCTL_SET_MASTER` on return.
    - Documents console troubleshooting: black screens on VT switches, high-resolution console fonts via `setfont` and `/etc/vconsole.conf` on HiDPI displays, and seat inspection commands.
- **Corpus Version Bump (0.27.0):**
  - Minor version bump in `knowledge/version` from `0.26.1` to `0.27.0` for adding a new knowledge page.
  - Updated corpus version in `ROADMAP.md` header to `0.27.0`.
  - Recorded version bump and summary in `docs/plans/development.md`.
  - Marked `docs/plans/corpus-ttys-and-vts.md` status as Done.
- **Navigation, Index, and Snapshots:**
  - Added links to `ecosystem/ttys-and-virtual-consoles.html` in `knowledge/ecosystem/index.html` and `knowledge/index.html` (Part VI).
  - Cross-referenced from `knowledge/maintain/recovery.html` and `knowledge/ecosystem/sddm.html`.
  - Regenerated corpus index (`knowledge/index.tsv` and `knowledge/index.json`) via `tools/build-index.py` (47 entries).
  - Regenerated golden snapshots via `UPDATE=1 tests/guide.sh build/tam`.
- **Installation:**
  - Installed development build to `~/.local/dev` via `make install PREFIX=$HOME/.local/dev`.

## Verification

- `python3 tools/build-index.py`: Validated metadata, tracks, levels, prerequisites, and breadcrumb trails; generated 47 entries.
- `make test-corpus`: All 10 guide tests passed cleanly.
- `make test-command`: All command tests in `tests/run.sh` passed cleanly.
- Topic resolution:
  - `tam show vt --pipe` and `tam show vts --pipe` resolve to the new guide.
  - `tam explain vt --pipe` resolves to topic guide linking `ecosystem/ttys-and-virtual-consoles.html`.
  - `tam explain tty --pipe` resolves to topic guide linking both emergency recovery and the new architecture guide.
  - `tam search vt` returns `ttys-and-vts`.
