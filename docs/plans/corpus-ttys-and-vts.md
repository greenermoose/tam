# Plan & Note: TTY and Virtual Terminal (VT) Knowledge Corpus Pages

**Status:** Planned / Backlog (Note filed for future authoring)  
**Date:** 2026-10-16  
**Owner:** Fred  
**Target:** Knowledge Corpus (`knowledge/`)

---

## 1. Context & Motivation

While troubleshooting and working on the Tamlinux system, issues were encountered with TTYs and Virtual Terminals (VTs). Querying the offline guide yielded incomplete or missing coverage:

- `tam explain VT` returned no topic found (`tam: no topic found for 'VT'`).
- `tam explain TTY` returned only an emergency disaster recovery summary (`maintain/recovery.html`), which mentions dropping to a virtual console to kill hung processes, but provides no conceptual or architectural explanation of what a TTY is, what a Virtual Terminal is, or how they interact with Wayland compositors and the Linux kernel.

This document notes the requirement to author documentation for TTYs and VTs in the `tam` knowledge corpus, evaluates whether to combine them into a single page or split them into two separate pages, and outlines the planned content and depth ladder.

---

## 2. Recommendation: Combined Page vs. Separate Pages

### Recommendation: Combined Page (`knowledge/ecosystem/ttys-and-virtual-consoles.html`)

A single, comprehensive combined page is strongly recommended over two separate pages.

#### Rationale

1. **Inseparable Technical Relationship:** In Linux, a Virtual Terminal (VT) *is* an instance of a kernel-managed TTY device (`/dev/tty1` through `/dev/tty63`). Explaining what a VT is fundamentally requires explaining the TTY device model (character devices, line disciplines, controlling terminals, and session leaders). Conversely, explaining the TTY subsystem on Linux without mentioning VTs leaves out the primary way users interact with physical consoles.
2. **Untangling User Mental Model Confusion:** Users and administrators frequently conflate three distinct terminal concepts:
   - **The abstract TTY subsystem:** TeleTYpewriter heritage, character device stream, line discipline, canonical vs. raw mode, termios flags, and signal generation (`SIGINT` on `Ctrl+C`, `SIGTSTP` on `Ctrl+Z`).
   - **Virtual Terminals / Consoles (VTs / VCs):** The Linux kernel's multi-console facility on physical display hardware (`/dev/tty1`–`/dev/tty63`), hardware keyboard hotkeys (`Ctrl+Alt+F1` through `F7`), KMS/DRM graphics mode takeover, display manager allocation, and text-mode recovery consoles.
   - **Pseudo-terminals (PTYs):** Software-emulated terminal pairs (`/dev/ptmx` master and `/dev/pts/*` slave) used by Wayland/X11 terminal emulators (`foot`, `alacritty`), terminal multiplexers (`tmux`), and remote sessions (`ssh`).
   Presenting these three concepts together on one page establishes the clear conceptual contrast: **Physical TTY (Serial) $\rightarrow$ Kernel Virtual TTY (VT) $\rightarrow$ Software Emulated TTY (PTY)**.
3. **Unified Discovery & Search Routing:** With rich metadata keywords (`tty`, `ttys`, `vt`, `vts`, `virtual terminal`, `virtual console`, `pty`, `pts`, `line discipline`, `termios`, `chvt`, `fgconsole`), running either `tam explain tty` or `tam explain vt` (as well as `tam search vt`) will cleanly point to this single authoritative topic guide.
4. **Cohesive Troubleshooting Runbook:** Display manager and compositor issues in Tamlinux (e.g., SDDM greeters, Sway/Hyprland session launch, `seatd`/`logind` seat assignment, DRM KMS handover) directly span the boundary between VTs and TTYs. Keeping diagnostics, key shortcuts, and recovery commands together prevents users from having to navigate back and forth between fragmented pages.

#### Alternative Considered: Two Separate Pages
- **Page 1 (`knowledge/shell/tty.html`):** Pure Unix TTY abstraction, line disciplines, stty/termios, signals, and PTY architecture.
- **Page 2 (`knowledge/maintain/virtual-consoles.html`):** Kernel virtual terminals, switching hotkeys, KMS graphics takeover, and console recovery.
- **Why rejected as primary design:** Results in circular cross-references and duplicated explanations (e.g. both pages needing to define what `/dev/tty1` vs `/dev/tty` means). If the combined page ever grows excessively large, it can later be factored along this boundary, but starting with a cohesive combined page provides the most direct answer to the user's questions.

---

## 3. Proposed Knowledge Page Specification

- **File Path:** `knowledge/ecosystem/ttys-and-virtual-consoles.html` (or `knowledge/maintain/ttys-and-vts.html`)
- **Title:** TTYs, Virtual Consoles, and Terminal Architecture
- **Track:** `ecosystem` (with cross-links in `maintain/` and `shell/`)
- **Topic Slug:** `ttys-and-vts`
- **Metadata Keywords:** `tty, ttys, vt, vts, virtual terminal, virtual console, pty, pts, terminal, console, termios, line discipline, chvt, fgconsole, seatd, logind, agetty`
- **Metadata Summary:** Terminal subsystem architecture, Linux virtual consoles (VTs), pseudo-terminals (PTYs), switching hotkeys, and display manager seat handover.

---

## 4. Planned Content Outline & Depth Ladder

### A. Beginner Depth: Virtual Consoles & Everyday Switching
- **Concept:** What is a Virtual Console (VT)? The multiple independent, full-screen screens provided directly by the Linux kernel behind the graphical desktop.
- **Practical Action:**
  - Switching to a text console: `Ctrl + Alt + F2` (or `F3`–`F6`).
  - Understanding where the graphical desktop lives (e.g. VT1 or VT2 in modern systemd/SDDM systems, VT7 historically in X11).
  - Returning to the graphical session: `Ctrl + Alt + F1` (or the active desktop VT).
- **Escape Hatch:** When a Wayland compositor or fullscreen application freezes, using `Ctrl+Alt+F2` to drop to a text prompt, log in, find the runaway PID with `pgrep`/`top`, and recover without hard power-cycling the machine.

### B. Intermediate Depth: The TTY Subsystem, PTYs, and Terminal Control
- **The TTY Abstraction:**
  - History from electromechanical teletypes to kernel character devices.
  - The Line Discipline: Canonical (cooked) mode vs. raw mode; how the kernel handles backspace, line buffering, and character echoing before the application sees input.
  - Signal Generation: How the TTY driver intercepts `Ctrl+C` (`SIGINT`), `Ctrl+\` (`SIGQUIT`), and `Ctrl+Z` (`SIGTSTP`).
  - Controlling Terminals (`/dev/tty`) and session leaders.
- **Pseudo-Terminals (PTYs):**
  - Why graphical terminal emulators (`foot`, `alacritty`) cannot use raw `/dev/ttyN`.
  - The multiplexer master/slave pair (`/dev/ptmx` and `/dev/pts/N`).
- **Practical Commands & Inspection:**
  - `tty` — print current terminal name.
  - `stty -a` — inspect baud rate, terminal line settings, and control characters.
  - `fgconsole` — display the active virtual console number.
  - `chvt N` — switch virtual consoles from scripts or the command line.
  - `w` and `who` — list active logged-in users across VTs and PTS sessions.

### C. Expert Depth: Kernel KMS Handover, Seat Management, and Diagnostics
- **Kernel Console Architecture:**
  - `drivers/tty/vt/` in the Linux kernel.
  - Framebuffer console (`fbcon`) and DRM/KMS (Direct Rendering Manager / Kernel Mode Setting).
  - Terminal modes: `KD_TEXT` (text mode with VGA/fbcon font rendering) vs. `KD_GRAPHICS` (compositor has exclusive access to video memory).
- **Seat & Session Management:**
  - How display managers (SDDM) and seat arbiters (`systemd-logind`, `seatd`) allocate and switch VTs.
  - The DRM master lease/drop sequence: when switching away from a Wayland session, the compositor drops DRM master (`DRM_IOCTL_DROP_MASTER`), allowing the kernel or another VT to claim the display.
  - Unprivileged Wayland execution without SUID root via logind/seatd file descriptor passing.
- **Troubleshooting Common TTY/VT Issues:**
  - *Black screen on VT switch:* DRM driver failure to release or reacquire master, or missing `fbcon` driver in minimal kernels.
  - *Console text resolution & font:* Setting readable console fonts using `setfont` (`vconsole.conf`).
  - *Hung VT switch:* Diagnosing logind/seatd state via `loginctl session-status` and `loginctl list-seats`.
  - *Permissions:* Understanding ownership of `/dev/ttyN` (owned by root:tty or logged-in user).

---

## 5. Authoring & Release Steps (For Future Implementation)

When ready to author and publish this page:
1. Claim the corpus lane paths with `tam-work begin`:
   ```bash
   tam-work begin \
     --scope knowledge/ecosystem/ttys-and-virtual-consoles.html \
     --scope knowledge/ecosystem/index.html \
     --scope knowledge/index.html \
     --scope knowledge/version \
     --note 'Add TTYs and Virtual Terminals guide'
   ```
2. Authored page adhering to Tamlinux semantic HTML guidelines (Beginner, Intermediate, Expert `<section data-level="...">` blocks).
3. Bump `knowledge/version` (minor bump for new knowledge page).
4. Regenerate corpus index: `python3 tools/build-index.py`.
5. Update golden text snapshots: `UPDATE=1 tests/guide.sh build/tam`.
6. Verify test suites: `make test-corpus` and `make test-command`.
7. Verify topic resolution: `tam explain VT` and `tam explain TTY`.
8. Commit and release claim with `tam-work finish`.
