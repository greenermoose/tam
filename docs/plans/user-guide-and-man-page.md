# tam User Guide Table of Contents and Manual Page Plan

**Stage:** Develop — Proposed. Milestone for `tam` command and knowledge base.

---

## 1. Motivation and Problem Statement

The `tam` command and its bundled offline knowledge corpus have grown significantly (from 5 pages originally to 29 curated chapters across shell, filesystem, desktop, hardware, and ecosystem architecture), with more content planned (Maintain, Improve, Computer Science).

However, key discoverability and documentation gaps exist:

1. **Guide Home Page Identity:**
   `knowledge/index.html` is currently titled "Welcome to the Tamlinux guide". Because `welcome.html` already serves as the dedicated onboarding orientation (`tam welcome`), having the root guide index say "Welcome" creates confusion. It should be titled **"The Tamlinux User Guide"** and serve as the authoritative root portal.

2. **Absence of an Overarching Table of Contents:**
   `knowledge/index.html` currently lists only 16 chapters. Specifically, out of the 14 chapters authored in `knowledge/ecosystem/`, only two (`ecosystem/index.html` and `ecosystem/patches.html`) are linked from the home page. The other 12 deep architectural chapters (distributions, compositors, packaging, init, desktop toolkits, desktop apps, libc, Lua runtime, virtualization, AI tooling, and sustainability) are completely absent from the home page. There is no overarching Table of Contents that lays out the entire system documentation structure.

3. **Undiscoverable Explanation Topics:**
   Milestone 10 implemented `tam explain topics` and the virtual document URI `tam:topics`, which dynamically renders an interactive, hyperlinked table of all 29 curated explanation topics. However, `knowledge/index.html` does not link to this catalog. Readers in the guide pager cannot find the list of topics without already knowing the terminal command `tam explain topics`. Furthermore, running `tam show topics` from the shell fails with `tam: guide page not found: topics.html`.

4. **Missing `tam` Command Reference and Manual Page:**
   While `tam` provides `tam man [SECTION] PAGE` to browse hyperlinked system manual pages, `tam` itself has no manual page. Running `man tam` reports `No manual entry for tam`, and `tam man tam` reports `tam: no manual page found for 'tam'`. The `Makefile` does not define `MANDIR` or install man pages. In addition, `knowledge/man/index.html` discusses man pages in the abstract but does not link to `tam(1)` or mention `tam man`.

---

## 2. Decided Architecture and Scope

### 2.1 The Home Page: "The Tamlinux User Guide"

- Retitle `knowledge/index.html` to **The Tamlinux User Guide** (both `<title>` and `<h1>`).
- Clarify opening text to frame it as the complete offline reference for the workstation.
- Elevate `index.html` to be the primary **Overarching Table of Contents** of the system.

### 2.2 Overarching Table of Contents Organization

`knowledge/index.html` will structure the entire workstation guide into seven clean, logical volumes centered around Fred's tripartite workstation model: **Using**, **Maintaining**, and **Improving** (writing software, coding, hacking, and AI-assisted engineering):

- **Part I: Getting Started**
  - Welcome and orientation (`welcome.html`)
  - The three paths: Using, Maintaining, and Improving
  - Installing (`install/index.html`)

- **Part II: Guided Reading Paths**
  - Beginner's path (`beginner.html`)
  - Intermediate path (`intermediate.html`)
  - Expert's path (`expert.html`)

- **Part III: Using Your Workstation**
  - *Foundations & Command Center:*
    - Shell and streams (`shell/index.html`)
    - Shell pipelines and filters (`shell/pipelines.html`)
    - Processes, jobs, and signals (`shell/jobs.html`)
    - Command center & `tam-*` tools (`shell/tam-commands.html`) *(Plan 26)*
    - Terminal tools & `tmux` session multiplexing (`shell/tools.html`) *(Plan 26)*
    - Text editors: Neovim, Micro, and Helix (`shell/editors.html`) *(Plan 26)*
  - *Filesystem & Storage:*
    - Filesystem hierarchy (`filesystem/index.html`)
    - File permissions and ownership (`filesystem/permissions.html`)
  - *Desktop Environment & Navigation:*
    - Tamlinux desktop basics (`desktop/index.html`)
    - Transitioning from macOS or Windows (`desktop/transition.html`) *(Plan 26)*
    - Window layout mechanics, tabs, and scratchpad (`desktop/windows.html`) *(Plan 26)*
    - Authoritative keyboard shortcuts reference (`desktop/hotkeys.html`) *(Plan 26)*
    - Workspaces and multi-monitor sets (`desktop/workspaces.html`)
    - The top bar & `fred.*` plugin panels (`desktop/bar.html`) *(Plan 26)*
    - Quick toggles, idle, and screensaver (`desktop/toggles.html`) *(Plan 26)*
  - *Desktop Utilities & Workflows:*
    - Screen capture & recording (`desktop/capture.html`) *(Plan 26)*
    - Text extraction, OCR & QR reader (`desktop/text-tools.html`) *(Plan 26)*
    - Unified clipboard history (`desktop/clipboard.html`) *(Plan 26)*
    - Desktop notifications & reminders (`desktop/notifications.html`) *(Plan 26)*
    - Standalone web applications (`desktop/webapps.html`) *(Plan 26)*
  - *Appearance & Customization:*
    - Themes & background switcher (`desktop/theming.html`) *(Plan 26)*
    - Desktop dotfiles & hooks hierarchy (`desktop/config.html`) *(Plan 26)*
    - Common interface tweaks (`desktop/tweaks.html`) *(Plan 26)*
  - *Hardware, Input & Power:*
    - Resources and performance (`hardware/resources.html`)
    - Display positioning & HiDPI scaling (`hardware/monitors.html`) *(Plan 26)*
    - Keyboard, mouse, and trackpad gestures (`hardware/input.html`) *(Plan 26)*
    - Networking, Wi-Fi QR, and DNS (`hardware/networking.html`) *(Plan 26)*
    - System sleep, suspend, and power profiles (`hardware/power.html`) *(Plan 26)*
    - Hardware authentication & security keys (`hardware/security.html`) *(Plan 26)*

- **Part IV: Maintaining Your Workstation**
  - Maintaining and upgrading overview (`maintain/index.html`)
  - Non-disruptive updates & package tiers (`maintain/updates.html`)
  - Snapshots and generation rollbacks (`maintain/rollback.html`)
  - System diagnostics and journals (`maintain/logs.html`)
  - Progressive power ladder (`maintain/power.html`)
  - Emergency disaster recovery (`maintain/recovery.html`)
  - Workstation troubleshooting runbook (`maintain/troubleshooting.html`) *(Plan 26)*
  - Frequently asked questions (`faq.html`) *(Plan 26)*

- **Part V: Improving Your Workstation: Coding, Hacking, and AI Agents**
  - Improving overview: writing software and extending Tamlinux (`improve/index.html`)
  - **Coding with AI agents on Tamlinux** (`improve/ai-coding.html`): Pair programming with Claude, Codex, and Antigravity; isolated worktrees; scope claiming (`tam-work`); public provenance; and autonomous testing
  - **Writing tools and extending features** (`improve/software.html`): Scripting conventions (`tam-*` portable system utilities vs `fred-*` personal commands), strict Bash headers, and input safety
  - **Authoring desktop plugins** (`desktop/plugins.html` / `improve/plugins.html`): Quickshell plugin architecture, QML panels, and `tam-plugin` distribution
  - **Adding and packaging software** (`improve/packaging.html`): Creating Nix packages, Home Manager modules, and native Arch/Void packages
  - **Native toolchains and compilation** (`improve/toolchains.html`): Native C compilation, `libtam` linking, fast turnaround loops, and zero-polling architecture

- **Part VI: Architecture and Ecosystem (Why Tamlinux Is So)**
  - Ecosystem overview and design choices (`ecosystem/index.html`)
  - Conceptual framework: the botanical analogy (`ecosystem/terminology.html`)
  - Philosophy: free software and resource efficiency (`ecosystem/philosophy.html`)
  - Base distributions: Arch, Void, and antiX (`ecosystem/distros.html`)
  - Compositors and windowing: Sway, Hyprland, wlroots (`ecosystem/compositors.html`)
  - Package management: Nix, Home Manager, and native (`ecosystem/packaging.html`)
  - Init and supervision: runit and systemd (`ecosystem/init.html`)
  - Desktop toolkits: Quickshell, Qt Quick, QML (`ecosystem/toolkits.html`)
  - Desktop application suites: KDE and GNOME (`ecosystem/desktop-apps.html`)
  - C standard libraries: glibc versus musl (`ecosystem/libc.html`)
  - Lua runtime and desktop scripting (`ecosystem/lua.html`)
  - Carried patches and public forks (`ecosystem/patches.html`)
  - Virtual machines and containers (`ecosystem/virtualization.html`)
  - AI tooling and human agency (`ecosystem/ai.html`)
  - Hardware sustainability and longevity (`ecosystem/sustainability.html`)

- **Part VII: Reference, Command Manuals, and Indexes**
  - **Explanation Topics Catalog** (`tam:topics`): Complete alphabetical index of all curated topics with summaries and level badges.
  - **The `tam(1)` Manual Page** (`man:tam(1)`): Complete command syntax, subcommands, options, environment variables, and pager keys.
  - **Reading Manual Pages** (`man/index.html`): Guide to manual conventions, section numbers, and `tam man`.
  - **Guide Search and Inquiries**: Fast topic search via `tam search` and grounded question answering via `tam ask`.

In `knowledge/index.html`, active links will point to existing authored pages; planned chapters will be clearly distinguished in the outline so the overarching scope of the User Guide is immediately visible without dead links.

---

### 2.3 Tamlinux Terminology: The Botanical Analogy

Authored at `knowledge/ecosystem/terminology.html` (Topic: `terminology`, with aliases/keywords `greenhouse`, `pot`, `nut`, `root-stock`, `cultivar`, `analogy`, `framework`, `concept`).

The conceptual framework analogy used by the developer (Fred) to describe system structure:

1. **Greenhouse (Hardware):**
   The physical hardware providing the environmental conditions and resources (power, CPU, memory, storage, thermal budget, displays) that the system needs to function and grow.

2. **Pot (Boot Environment / VM):**
   The containing vessel placed inside the greenhouse—the UEFI firmware, boot loader, or virtual machine—that isolates and holds the bootable seedling.

3. **Nut (Minimal Bootable Image / UKI):**
   An extremely minimal bootable system image (a Unified Kernel Image [UKI] or minimal initramfs/kernel/shell image). When the system transfers control to the nut, it sprouts: running a Linux kernel and minimal text shell with the essential commands to establish network connectivity, verify storage, and prepare the ground so a larger, feature-rich system can be loaded next.

4. **Root-stock (Base Linux Distribution):**
   A robust, general-purpose Linux distribution (such as Void Linux or Arch Linux) that provides the underlying operating system layer, kernel, libc, core system libraries, and package manager required by the graphical shell and applications. The nut may bootstrap the root-stock across the network, or the root-stock may already be rooted on disk.

5. **Cultivar (Tamlinux Desktop & Application Suite):**
   The cultivated variety grafted onto the root-stock. A cultivar is a specific release of Tamlinux, comprising the graphical desktop shell (Wayland/Sway/Quickshell), user tools (`tam`, `libtam`), and application suite. If the root-stock and the cultivar are compatible, the cultivar will thrive.

### 2.4 Explanation Topics Integration

1. **Direct Link in Guide Index:**
   Add `<a href="tam:topics">Explanation Topics Catalog</a>` under Part VII of `knowledge/index.html`.
   When followed in the interactive pager, `corpus_load()` triggers `tam_topics_document()`, immediately rendering the interactive topic list. Pressing `u` returns back to `index.html`; pressing `` ` `` resets to the top of `index.html`.

2. **CLI and Pipe Support for `tam show topics`:**
   Update `cmd_show()` in `src/tam.c` so that running `tam show topics` or `tam show tam:topics`:
   - In interactive mode: launches the interactive topics document in the pager.
   - In `--pipe` mode: prints the topics catalog as plain text.

---

### 2.5 The `tam(1)` Manual Page

1. **Roff Manual Source (`man/tam.1`):**
   Author a standard Unix Section 1 manual page formatted with standard `groff_man(7)` macros.
   Sections:
   - `NAME`: Command name and one-line purpose.
   - `SYNOPSIS`: Complete syntax for all 9 subcommands and standard flags.
   - `DESCRIPTION`: Offline-first design principles, startup speed (<5ms), resource bounds (<4MB RSS), zero daemons, dual-mode text/TUI execution.
   - `COMMANDS`: Detailed descriptions of `welcome`, `show`, `search`, `explain`, `explain topics`, `man`, `install inspect`, `ask`, and help/version.
   - `PAGER NAVIGATION`: Table of interactive navigation keybindings (`j`/`k`, `Space`/`b`, `g`/`G`, `Tab`/`Shift-Tab`, `Enter`, `u`, `` ` ``, `/`, `q`).
   - `OUTPUT MODIFIERS`: Explanation of `--pipe`, `--terse`, `--verbose`, `--json`, and `--level`.
   - `ENVIRONMENT`: Variables controlling behavior (`TAM_KNOWLEDGE_DIR`, `TAM_DATADIR`, `TAM_ASK_MODEL`, `TAM_ASK_SERVER`, `TAM_ASK_ALLOW_FETCH`, `XDG_DATA_HOME`, `XDG_DATA_DIRS`, `XDG_STATE_HOME`).
   - `FILES`: Installed data paths and user state files.
   - `SEE ALSO`: Cross-references to `tam-man(1)`, `man(1)`, `apropos(1)`.

2. **Build and Installation Integration:**
   - In `Makefile`:
     - Add `MANDIR ?= $(PREFIX)/share/man`
     - Under `install:` target:
       ```make
       install -d $(DESTDIR)$(MANDIR)/man1
       install -m 644 man/tam.1 $(DESTDIR)$(MANDIR)/man1/tam.1
       ```
     - Under `uninstall:` target:
       ```make
       rm -f $(DESTDIR)$(MANDIR)/man1/tam.1
       ```

3. **Lookup Resilience in `src/man.c`:**
   Enhance `locate_man_file()`:
   - First probe via standard `man -w [sec] name`.
   - If not found (as in development builds under `~/.local/dev` where `~/.local/dev/share/man` is not in system `MANPATH`), inspect `<prefix>/share/man/man<sec>/<name>.<sec>` derived from the binary prefix or corpus root.
   - This ensures `tam man tam` functions reliably in development environments as well as packaged installations.

4. **Guide Integration (`knowledge/man/index.html`):**
   Update `knowledge/man/index.html` to:
   - Introduce `tam man` as the system's hyperlinked manual viewer.
   - Provide an explicit example linking to `man:tam(1)`.
   - Provide direct links to foundational manual pages (e.g. `man:ls(1)`, `man:sh(1)`).

---

## 3. Workstream Lanes and Implementation Sequence

### Lane A: Corpus Lane (`make corpus`)
1. Create `knowledge/ecosystem/terminology.html`:
   - Document the botanical analogy (greenhouse, pot, nut, root-stock, cultivar).
   - Add metadata tags (`topic="terminology"`, keywords, level, track="improve").
   - Include standard breadcrumb: Guide / The Tamlinux Ecosystem / Conceptual Framework.
2. Edit `knowledge/index.html`:
   - Change title to "The Tamlinux User Guide".
   - Expand into 7-part overarching Table of Contents uniting Using, Maintaining, Improving, Architecture, and Reference.
   - Add Part VII linking `tam:topics`, `man:tam(1)`, `man/index.html`, and search commands.
3. Edit `knowledge/man/index.html`:
   - Add description of `tam man` and live link to `man:tam(1)`.
4. Update `tests/guide.sh`:
   - Add `tam:*` to internal link exception list alongside `man:*`.
   - Add `ecosystem/terminology` to list of corpus pages.
   - Update title assertion to check for "The Tamlinux User Guide".
5. Run `tools/build-index.py` to regenerate `knowledge/index.tsv` and `knowledge/index.json`.
6. Update text snapshots in `tests/snapshots/index.txt`, `man-index.txt`, and generate `ecosystem-terminology.txt`.

### Lane B: Command Lane (`make command`)
1. Create `man/tam.1`.
2. Update `Makefile` with `MANDIR` variable, `install`, and `uninstall` rules.
3. Update `src/tam.c` to route `topics` and `tam:topics` in `cmd_show()`.
4. Update `src/man.c` (`locate_man_file`) to check binary prefix `share/man/` fallback.
5. Update `tests/run.sh` to verify `tam man tam` and `tam show topics`.

---

## 4. Verification and Acceptance Criteria

- `make test-corpus` passes: all breadcrumbs validate, index builds cleanly, snapshots match.
- `make test-command` passes: `tam man tam` displays `tam(1)` correctly; `tam show topics --pipe` outputs the topic catalog; all screen budgets stay within bounds.
- Full `make test` runs 100% green.
- Interactive PTY verification:
  - From `index.html`, following the `Explanation Topics Catalog` link loads the topics list; pressing `u` returns to `index.html`.
  - From `index.html`, following `man:tam(1)` loads the manual page; pressing `u` or `` ` `` returns to `index.html`.
  - From `knowledge/man/index.html`, following `man:tam(1)` opens `tam(1)`.
- Staged installation verification: `make install PREFIX=... DESTDIR=...` installs `man1/tam.1` with mode 644.
