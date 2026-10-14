# tam User Guide TOC, Botanical Analogy, and tam(1) Manual Page (tam 0.20.0)

- **Date:** 2026-10-14
- **Stage:** Develop — Milestone 12: Comprehensive User Guide Table of Contents (`knowledge/index.html`), Botanical Analogy (`knowledge/ecosystem/terminology.html`), Section 1 UNIX manual page (`man/tam.1`), `src/man.c` development man page resolution, `src/tam.c` `tam show topics` and `tam:topics` routing, and version bump to 0.20.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.4
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred directed the creation of an overarching Table of Contents for the Tamlinux offline guide, retitling the home page to "The Tamlinux User Guide", authoring the missing `tam(1)` manual page, integrating the full scope of user-facing chapters gleaned from the Omarchy manual (Plan 26), structuring the guide around Fred's core architectural triad (Using, Maintaining, Improving your workstation), and adding a dedicated chapter covering Fred's botanical conceptual framework analogy. Antigravity prepared the design plan in `docs/plans/user-guide-and-man-page.md`, reviewed it with Fred using `micro-review`, claimed the repository paths using `tam-work`, authored the 7-part User Guide index, authored `knowledge/ecosystem/terminology.html` with Beginner/Intermediate/Expert fading templates, authored the standard roff manual page `man/tam.1`, updated `Makefile` to install manual pages, enhanced `src/man.c` and `src/tam.c`, regenerated corpus index and golden text snapshots, updated test suites (`tests/run.sh` and `tests/guide.sh`), and verified full test suite passes (`make test`).

## Guiding prompt

> Establish an overarching Table of Contents / Index for the Tamlinux offline guide, retitle the home page from "Welcome to the Tamlinux guide" to "The Tamlinux User Guide", provide seamless discoverability for explanation topics (`tam explain topics`), author the missing `tam(1)` manual page, integrate the full scope of user-facing chapters gleaned from the Omarchy manual (Plan 26), and add a dedicated chapter covering Fred's botanical conceptual framework analogy.

## Decisions and scope

- **Tripartite Architectural Framework:**
  - Organized the User Guide around Fred's three distinct workstation aspects:
    1. *Using your workstation*: installing it, learning what it can do, and using the features it ships with.
    2. *Maintaining your workstation*: fixing bugs, upgrading packages, investigating and troubleshooting issues.
    3. *Improving your workstation*: adding new packages, writing new packages, coding, hacking, and specifically using AI agents to code on Tamlinux.
- **7-Part Overarching User Guide Structure (`knowledge/index.html`):**
  - Retitled index to "The Tamlinux User Guide" (`<title>` and `<h1>`).
  - Structured into seven comprehensive parts:
    - *Part I: Getting Started* (Orientation, The Three Paths, Installing).
    - *Part II: Guided Reading Paths* (Beginner, Intermediate, Expert paths).
    - *Part III: Using Your Workstation* (Foundations and command center, Filesystem and storage, Desktop environment and navigation, Desktop utilities and customization, Hardware, input, and power).
    - *Part IV: Maintaining Your Workstation* (Maintenance overview, Non-disruptive updates, Snapshots and rollbacks, Logs and diagnostics, Power ladder, Recovery, Troubleshooting runbook, FAQ).
    - *Part V: Improving Your Workstation: Coding, Hacking, and AI Agents* (Extending Tamlinux, Coding with AI agents, Writing tools, Desktop plugins, Packaging, Native toolchains).
    - *Part VI: Architecture and Ecosystem (Why Tamlinux Is So)* (Ecosystem overview, Botanical analogy, Philosophy, Distros, Compositors, Packaging, Init, Toolkits, Desktop apps, Libc, Lua, Patches, Virtualization, AI, Sustainability).
    - *Part VII: Reference, Command Manuals, and Indexes* (Explanation topics catalog, tam(1) manual page, Reading man pages, Guide search and inquiries).
  - Incorporated existing pages as active hyperlinks; planned chapters from Plan 26 are clearly marked as plain text labeled `(planned)`, preserving link integrity tests.
- **Botanical Conceptual Framework Analogy (`knowledge/ecosystem/terminology.html`):**
  - Documented Fred's five-layer botanical analogy:
    1. *Greenhouse*: The hardware platform providing physical resources (CPU, RAM, storage, displays, power).
    2. *Pot*: The container environment—either the system firmware/bootloader (systemd-boot, GRUB) or a virtual machine container (QEMU/KVM).
    3. *Nut*: The minimal bootable system image (a unified kernel image / UKI running Linux and a minimal shell) containing essential bootstrapping commands.
    4. *Root-stock*: The foundational Linux distribution (Arch Linux today, Void Linux target, antiX fallback) providing core system libraries, init, and package manager.
    5. *Cultivar*: The tailored Tamlinux workstation release, comprising the Wayland tiling desktop, Quickshell panels, and application suite.
  - Provided Beginner, Intermediate, and Expert fading template sections with concrete CLI verification tasks (`lscpu`, `bootctl`, `lsinitrd`, `ldd`, `/etc/os-release`).
- **Standard Section 1 UNIX Manual Page (`man/tam.1`):**
  - Authored comprehensive roff manual page covering: NAME, SYNOPSIS, DESCRIPTION, SUBCOMMANDS (`tam`, `welcome`, `show`, `search`, `explain`, `explain topics`, `man`, `install inspect`, `install check`, `ask`, `--help`, `--version`), PAGER KEYBOARD CONTROLS, OUTPUT MODIFIERS (`--pipe`, `--terse`, `--verbose`, `--json`, `--level`), ENVIRONMENT variables (`TAM_KNOWLEDGE_DIR`, `TAM_ASK_*`, XDG paths), FILES, and SEE ALSO.
  - Updated `Makefile` with `MANDIR ?= $(PREFIX)/share/man` and installation rule to `$(DESTDIR)$(MANDIR)/man1/tam.1`.
- **Command Enhancements in `src/man.c` and `src/tam.c`:**
  - `src/man.c`: Added fallback in `locate_man_file()` to search `../man/` and in-tree `man/` directories relative to the knowledge corpus root, allowing `tam man tam` to resolve cleanly in development and uninstalled test builds.
  - `src/tam.c`: Updated `cmd_show()` to intercept `topics`, `tam:topics`, and `man:*` arguments and dispatch them directly through the interactive pager and `--pipe` renderers.
- **Test Suite and Snapshot Updates:**
  - `tests/guide.sh`: Added `ecosystem/terminology` to page roster, updated title check to `Tamlinux User Guide`, and exempted `tam:*` links from filesystem resolution.
  - `tests/run.sh`: Updated command version assertion to `0.20.0`, updated index title assertions to `The Tamlinux User Guide`, updated pager Shift-Tab backward cycling targets for the new index link order, and added assertions for `tam show topics`, `tam show tam:topics`, and `tam man tam`.
  - Regenerated golden snapshots for all modified and new pages (`UPDATE=1 tests/guide.sh build/tam`).

## Verification

- `make all` built cleanly with zero warnings (`gcc -Wall -Wextra -Wpedantic -O2`).
- `tests/guide.sh` passed 100% across all 36 guide pages.
- `tests/run.sh` passed 100% across all command behavioral tests.
- `make test` executed completely green.
- Installed to user development prefix (`make install PREFIX=$HOME/.local/dev`) and verified `tam --version`, `tam show topics --pipe`, and `tam man tam --pipe`.
