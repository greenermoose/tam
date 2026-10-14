# tam Roadmap

**Reviewed:** 2026-10-14. **Owner:** Fred. **Current Command Version:** `0.17.0`.  
This roadmap sets the development sequence, future version numbers, and feature sets for the `tam` command and its bundled offline knowledge base. Component milestones derive from the [development plan](docs/plans/development.md) and its settled design decisions.

---

## 1. Vision & Architecture

`tam` is the terminal entry point and flagship showcase command for Tamlinux. Its mission is to empower users to **Use**, **Maintain**, and **Improve** their Linux system—running entirely offline, starting in under 5 milliseconds, consuming negligible memory (<4 MB RSS), generating zero background polling daemons, and operating reliably from legacy hardware (circa-2006 laptops) up to modern multi-core workstations.

### Core Architectural Invariants

1. **Strictly Offline by Default:** `tam` displays bundled documentation, parses manual pages, and inspects host state with zero network calls, zero tracking telemetry, and zero mandatory runtime daemons.
2. **Zero External Viewers:** Interactive browsing uses an internal ANSI terminal pager (`src/pager.c`) and semantic HTML layout engine (`src/render.c`) built in C with `libtam`. `tam` never requires Lynx, Less, or an external browser.
3. **Dual-Mode Unix Citizenship:** In an interactive terminal, `tam` presents a responsive, hyperlinked TUI. When standard output is piped or redirected (`--pipe`), `tam` emits clean, formatted plain text with an indexed link table and zero terminal escape sequences.
4. **Input & State Safety:** Input arguments are treated strictly as data; shell metacharacters are never evaluated. State is never written without explicit user intent (`tam welcome` records `welcome_seen` atomically; bare `tam`, `show`, `search`, and `explain` are strictly read-only).
5. **Version Contract:** Following semantic versioning in `src/version.h`, every new user-visible capability or subcommand advances the minor version (`0.y.0` → `0.y+1.0`). Patch releases (`0.y.z`) are reserved for bug fixes and internal maintenance.

---

## 2. What Has Been Done (Foundation: 0.0.1 – 0.16.0)

Between 2026-10-09 and 2026-10-14, `tam` progressed rapidly through Milestones 1–8, link ergonomics, in-pager search, shell completions, content-depth ladder, self-inspection findings, and check contract:

| Version | Milestone / Focus | Status | Key Delivered Capabilities |
| :--- | :--- | :--- | :--- |
| **0.0.1** | Milestone 1: Command Foundation | Done (2026-10-09) | Pure C implementation linking `libtam`; bare introduction (`tam`, exit 0), usage help (`tam --help`), version reporting (`tam --version`) with Tamlinux cultivar metadata, and clean non-TTY pipe output. |
| **0.1.0** | Milestone 2: Offline Guide | Done (2026-10-09) | Native C HTML-subset layout engine (`src/render.c`) and interactive ANSI terminal pager (`src/pager.c`), replacing external Lynx dependency; bundled HTML corpus under `knowledge/` (`index`, `welcome`, `install/`, `shell/pipelines`, `man/`). |
| **0.2.0** | Pager Navigation | Done (2026-10-09) | Keyboard navigation keys: `Esc`/`q` to exit, arrow scrolling, `Enter`/Right Arrow to follow links, and `u`/Left Arrow to go back. |
| **0.3.0** | Help Alias | Done (2026-10-09) | `tam help` exact alias for `tam --help`. |
| **0.4.0** | Milestone 3: Orientation & Explanation | Done (2026-10-09) | First-run onboarding (`tam welcome`) with atomic `$XDG_STATE_HOME/tamlinux/welcome_seen` recording only upon successful display; curated concept explainer (`tam explain <topic>`) treating shell metacharacters strictly as query data. |
| **0.5.0** | Milestone 4: Installation Inspection | Done (2026-10-09) | Read-only host inspection (`tam install inspect`) probing `/etc/os-release`, `uname()`, mounts, DRM graphics, package managers, and privilege tools; truthful exit 1 diagnostics for unimplemented installer verbs (`plan`, `apply`, `verify`). |
| **0.6.0** | Explain Catalog Listing | Done (2026-10-09) | Bare `tam explain` lists available explainable topics grouped with summaries and aliases (exit 0) rather than raising an argument error. |
| **0.7.0** | Subcommand Rename & Quoting | Done (2026-10-09) | Renamed guide subcommand from `tam browse` to `tam show`; shell quote-stripping for `tam explain` (e.g. `tam explain '|'`). |
| **0.8.0** | Milestone 5: Layout Engine & "Use" Track | Done (2026-10-09) | Table rendering (`<table>`, `<tr>`, `<th>`, `<td>`), definition lists (`<dl>`, `<dt>`, `<dd>`), stdin paging (`tam show -`, `tam explain -`); authored comprehensive "Use" track knowledge pages (`shell/`, `filesystem/`, `desktop/`, `hardware/resources.html`). |
| **0.9.0** | Native Manual Page Subsystem | Done (2026-10-09) | Native `tam man [SECTION] PAGE` subcommand; dynamic conversion of roff man pages via `groff -mandoc -Thtml` with automatic hyperlinking of `SEE ALSO` references (`man:name(sec)`) and intra-page anchor navigation. Lightweight `bin/tam-man` wrapper. |
| **0.9.1** | Build Isolation & Dev Prefix | Done (2026-10-09) | Stamped development builds with `git describe`; installed under user development prefix (`~/.local/dev`), prioritizing binary's own compiled datadir over system directories. |
| **0.10.0** | Milestone 6: Corpus Index & Search | Done (2026-10-13) | Replaced hardcoded `TOPIC_MAP` in `src/tam.c` with a build-time corpus index artifact (`knowledge/index.tsv`, `knowledge/index.json`) generated from HTML metadata by `tools/build-index.py`; implemented `tam search [--json] TERM...` with field-weighted ranking, prefix matching, bounded fuzzy suggestions ("Did you mean"), and miss banners offering `tam man`. Multi-word fallback in `tam explain`. |
| **0.11.0** | Pager Backward Link Circulation | Done (2026-10-13) | Added `Shift-Tab` backward link navigation in `src/pager.c` across ANSI CSI sequences (CBT, Kitty, modifyOtherKeys), updated status line to `Tab/S-Tab link` with UTF-8 boundary truncation safety. |
| **0.12.0** | Pager Forward Path Traversal | Done (2026-10-13) | Right Arrow forward navigation along previously visited exploration paths when no hyperlink is selected. |
| **0.13.0** | Hyperlink Display Fixes | Done (2026-10-13) | Corrected hyperlink leading-space underline suppression and multi-line wrapped link continuation styling in `src/render.c` and `src/pager.c`. |
| **0.14.0** | In-Pager Search, Telemetry & Bash Completions | Done (2026-10-13) | Interactive keyword search prompt (`/`) in `src/pager.c` with in-buffer hit highlighting, next/previous navigation (`n`/`N`), and `Esc` clearing; reading position telemetry (`L...`, percentage, `Top`, `Bot`, `All`); static shell completions for Bash without dynamic file I/O (Tamlinux supports Bash only). |
| **0.15.0** | Milestone 7: Content-Depth Ladder & Fading Templates | Done (2026-10-14) | Beginner / Intermediate / Expert fading templates across core topics (`shell/jobs.html`, `shell/pipelines.html`, `filesystem/permissions.html`, `hardware/resources.html`); dynamic section filtering in `src/depth.c` and depth navigation (`?level=`); `tam explain --level LEVEL <topic>` and catalog depth filtering; index metadata projection for level lists, tracks, and prerequisites; depth indicators in `tam search` and JSON output; Bash completion for `--level`. |
| **0.16.0** | Milestone 8: Self-Inspection Findings & Check Contract | Done (2026-10-14) | Comprehensive read-only system diagnostic and health finding engine in `tam install inspect`; structured finding model (`id`, `severity`, `summary`, `evidence`, `help_link`) deep-linking to Maintain-track help; versioned offline Check Contract schema (`{task_id, name, status, pass, fail, evidence, severity, help_ref, source, schema_version}`); Safe Execution Envelope (`safe_probe_exec`) running whitelisted probes under `LC_ALL=C`, bounded timeouts (≤500ms), captured streams, detached TTY; `pkexec` detect-and-degrade (no interactive `sudo`); `tam install inspect --json`, `tam install inspect --check [ID]`, `tam install check [ID]` (`--json` supported); updated Bash completions. |
| **0.17.0** | Milestone 9: Opt-in Grounded Ask | Done (2026-10-14) | Opt-in grounded question-answering CLI surface (`tam ask [OPTIONS] <question>`) over the corpus index with mandatory citations; POSIX socket HTTP/1.1 client connecting to already-running local inference server (e.g. Ollama); strictly offline by default with D53 zero-daemon invariant; non-interactive refusal contract (exit code 2) and interactive consent flow; `tam ask --json` structured output; Bash completions for `ask`. |

---

## 3. What Needs to Be Done Next

The immediate priority for `tam` is implementing **Version 0.18.0** (Quick Guide Index Return Shortcut & "Maintain" Track Knowledge Corpus Expansion). With the content-depth ladder, self-inspection findings, and grounded ask in place, the guide provides clear pedagogical progression that will frame system health and maintenance documentation alongside pager ergonomics.

### Version 0.18.0 — Quick Guide Index Return (`\``) & "Maintain" Track Knowledge Corpus Expansion

- **Quick Guide Index Return Keybinding (`\``):**
  - Bind backtick (`` ` ``) in the interactive pager (`src/pager.c`) to return immediately to the Guide root index (`index.html`) from anywhere in the corpus or manual page viewer.
  - Pushes the current page to the history stack before navigation so users can return with `u` if desired.
  - Eliminates repetitive back-navigation when deep in subtopics or manual pages.
- **Non-Disruptive Updates & Package Lifecycle:**
  - `knowledge/maintain/updates.html`: Explaining inode unlinking (why Linux updates in place without rebooting), package tiering (OS base vs Tamlinux packages vs Mise user tools), and `tam-update` workflow.
- **Snapshots & Generation Rollbacks:**
  - `knowledge/maintain/rollback.html`: Copy-on-Write (CoW) mechanics, taking and restoring Btrfs snapshots, and Home Manager `/nix/store` generation rollbacks.
- **Truthful System Diagnostics:**
  - `knowledge/maintain/logs.html`: Filtering `journalctl` by boot (`-b`), unit (`-u`), and priority (`-p err..emerg`); reading the kernel ring buffer (`dmesg -T`).
- **The Progressive Power Ladder:**
  - `knowledge/maintain/power.html`: 3-stage power progression: DPMS monitor blanking (idle monitors turn off individually), RAM suspend, and zero-watt S4 disk hibernation; safe display wake via `Shift` key.
- **Emergency Disaster Recovery:**
  - `knowledge/maintain/recovery.html`: Magic SysRq (REISUB) emergency sequence, dropping to virtual consoles (`Ctrl+Alt+F2`–`F6`), and live USB chroot recovery.

---

## 4. Future Version Sequence & Feature Sets

Following the immediate next steps, the future development of `tam` advances through system maintenance documentation, read-only self-inspection, optimization guides, architectural rationale, deep physics/CS ladders, opt-in AI querying, and full installation framework integration.

```text
  0.14.0 ──► 0.15.0 ──► 0.16.0 ──► 0.17.0 ──► 0.18.0 ──► 0.19.0 ──► 0.20.0 ──► 0.21.0 ──► 0.22.0 ──► 0.23.0 ──► 0.24.0 ──► 1.0.0
 (Search)   (Depth)    (Inspect)   (Maintain) (Improve)   (Why)     (Depth)    (Ask)     (Learn)   (Parser)  (Install)  (Stable)
```

### Version 0.17.0 — "Maintain" Track Knowledge Corpus Expansion
Expand the offline corpus to comprehensively cover system health, resilient updates, diagnostics, and disaster recovery.

- **Non-Disruptive Updates & Package Lifecycle:**
  - `knowledge/maintain/updates.html`: Explaining inode unlinking (why Linux updates in place without rebooting), package tiering (OS base vs Tamlinux packages vs Mise user tools), and `tam-update` workflow.
- **Snapshots & Generation Rollbacks:**
  - `knowledge/maintain/rollback.html`: Copy-on-Write (CoW) mechanics, taking and restoring Btrfs snapshots, and Home Manager `/nix/store` generation rollbacks.
- **Truthful System Diagnostics:**
  - `knowledge/maintain/logs.html`: Filtering `journalctl` by boot (`-b`), unit (`-u`), and priority (`-p err..emerg`); reading the kernel ring buffer (`dmesg -T`).
- **The Progressive Power Ladder:**
  - `knowledge/maintain/power.html`: 3-stage power progression: DPMS monitor blanking (idle monitors turn off individually), RAM suspend, and zero-watt S4 disk hibernation; safe display wake via `Shift` key.
- **Emergency Disaster Recovery:**
  - `knowledge/maintain/recovery.html`: Magic SysRq (REISUB) emergency sequence, dropping to virtual consoles (`Ctrl+Alt+F2`–`F6`), and live USB chroot recovery.

### Version 0.17.0 — Read-Only Self-Inspection Findings & Check Contract (Milestone 8)
Evolve `tam install inspect` into a comprehensive, read-only system diagnostic engine that reports machine health without modifying configuration or executing unvetted commands.

- **Read-Only Finding Model:**
  - Define structured finding schema: `{id, severity, evidence, help_link, machine_readable}` with severity levels `info`, `warn`, `error`, `critical`.
  - Deep-link findings directly into Maintain-track guide pages (e.g. storage exhaustion links to `maintain/rollback.html`).
- **Offline Check Contract:**
  - Establish versioned Check Contract: `{task_id, pass, fail, evidence, source, schema_version}` emitted as JSON for automated verification and progression tasks.
- **Privilege Detect-and-Degrade:**
  - Detect `pkexec` presence without assuming privilege; on hosts without `pkexec` (or minimal rescue environments), gracefully degrade to the non-privileged inspection subset and emit a `warn` finding.
  - Never spawn an interactive `sudo` prompt.
- **Safe Execution Envelope:**
  - All external inspection probes run under: `LC_ALL=C`, bounded timeouts (≤500ms), captured standard streams, detached TTY, and neutralized color (`--color=never`).
  - Whitelist only fixed, deterministic read-only system probes (`uname`, `findmnt`, `statvfs`, `systemctl is-active`, `udevadm`); never execute arbitrary user commands.

### Version 0.18.0 — "Improve" Track Knowledge Corpus Expansion
Equip users and developers to optimize performance, eliminate energy waste, configure tools, and understand local AI inference.

- **Zero-Polling Architecture:**
  - `knowledge/improve/efficiency.html`: Eliminating periodic polling loops; leveraging kernel `inotify`, `epoll`, and Wayland compositor event sockets to keep CPU idle and save battery.
- **Memory Compression with ZRAM:**
  - `knowledge/improve/zram.html`: How compressed RAM swap (LZ4/zstd) doubles effective memory capacity with near-zero latency compared to disk thrashing.
- **Terminal & Editor Ergonomics:**
  - `knowledge/improve/terminal.html`: Fast shell startup; using `micro` with auto-reload when AI agents modify files concurrently (Virtue V11).
- **Scripting & Tool Conventions:**
  - `knowledge/improve/scripting.html`: System-wide portable `tam-*` tools vs personal machine `fred-*` scripts, strict Bash headers (`set -euo pipefail`), safe argument handling (`"$@"`).
- **Native Toolchains & Multi-Agent Workflows:**
  - `knowledge/improve/toolchains.html`: Native C compilation (`-O2`, `-march=native`), isolated Git worktrees (`git worktree add`) for concurrent coding agents.
- **Physics of Local AI Generation:**
  - `knowledge/improve/ai-performance.html`: Memory bandwidth vs autoregressive token generation speed ($Tokens/s \approx Bandwidth / ModelSize$); quantization tradeoffs (Q4_K_M vs FP16).

### Version 0.19.0 — The "Why" Axis: Architectural Rationale & Excluded Packages
Demystify why Tamlinux is designed the way it is, explaining not only what is included, but what is deliberately excluded.

- **`tam why <topic|package>` Command:**
  - Provide concise, evidence-based architectural rationale for core Tamlinux decisions.
  - Uniform 4-part structure for every decision:
    1. **Decision:** What Tamlinux chose.
    2. **Alternatives Considered:** What else was evaluated and what merits it had.
    3. **Tradeoffs & Evidence:** Why the alternative was rejected here (citing Virtues V01–V14 and Faults A–G).
    4. **Reopen Trigger:** What specific technical condition would prompt reopening the choice.
- **Excluded Packages Registry:**
  - Explain deliberate omissions: why Sway instead of Hyprland (ABI stability), why runit instead of systemd (simplicity and deterministic speed), why native C instead of Electron/Python daemons (resource respect), why zero-polling.
  - Provide self-determination install routes: clear instructions on how users can install excluded packages if they choose.
- **Public Rationale Projection:**
  - Build-time tool generating the published rationale pages from reviewed decision summaries.

### Version 0.20.0 — The "Depth" Axis & Computer Science Fundamentals
Enable users to interrogate their system and descend from observable desktop behavior down to fundamental physics and computer science theory.

- **Progressive Depth Descent (`tam deeper` / `tam depth`):**
  - Interactively descend through abstraction layers one rung at a time:
    $$\text{User Action} \rightarrow \text{Application} \rightarrow \text{OS/Syscall} \rightarrow \text{Kernel/Driver} \rightarrow \text{ISA/Machine Code} \rightarrow \text{Microarchitecture} \rightarrow \text{Logic Gates} \rightarrow \text{Transistors} \rightarrow \text{Solid-State Physics} \rightarrow \text{Electromagnetism} \rightarrow \text{Quantum Fields}$$
  - Each rung explicitly defines: (1) what it explains, (2) what it treats as a black box, and (3) the link to the next rung down.
- **Guided System Traces (`tam trace <journey>`):**
  - `tam trace keystroke-to-pixel`: From key switch contact bounce, through USB HID / interrupt, `evdev`, compositor dispatch, GPU rasterization, to display refresh.
  - `tam trace source-to-electricity`: From C source code tokens, compiler IR, assembly, linked ELF binary, virtual memory page loading, CPU execution pipelines, CMOS voltage switching, to electron flow.
- **Computer Science Fundamentals Track (`knowledge/cs/`):**
  - Practical theory framed around **Possible / Impossible / Expensive**: computability (undecidable limits), complexity (tractable vs intractable), data representation (two's complement, IEEE-754 floating point rounding), memory hierarchy, and concurrency. Grounded directly in the user's host hardware (`lscpu`, cache topology, RAM bandwidth).

### Version 0.21.0 — Opt-in Grounded Question Answering (`tam ask` — Milestone 9)
Provide cited, grounded local AI assistance without sacrificing the offline, lightweight foundation.

- **Strict Offline-by-Default Boundary:**
  - Core binary remains 100% offline and standalone.
  - `tam ask <question>` connects via local HTTP API strictly to an **already-running** local inference server (e.g. Ollama); `tam` never embeds, bundles, starts, or runs background AI daemons.
- **Grounded Retrieval-Augmented Generation (RAG):**
  - Searches local corpus index for top-k relevant documentation snippets to seed model context.
  - Mandatory citation requirement: every answer must include explicit `Sources:` lines referencing local pages.
  - Grounding miss safety: if no relevant local documentation exists, `tam ask` responds with `"Not found in Tamlinux guide."` and suggests a `tam search` query, preventing hallucinations.
- **Interactive Consent for Model Retrieval:**
  - If a model is not present, `tam` reaches the network only upon explicit interactive TTY confirmation showing URL, download size, storage path, and expected SHA256 checksum.
  - SHA256 checksum verified immediately after download; mismatches automatically delete partial data.
- **Non-Interactive Pipeline Safety:**
  - Invoking `tam ask` under `--pipe` or non-interactive shells when a model or server is unavailable immediately fails with exit code 2 and a concise error message; never attempts network operations or hangs waiting for input.

### Version 0.22.0 — Offline Skills Progression & Self-Paced Learning (`tam learn`)
Provide self-paced, offline skill building that uses the Milestone 8 inspection engine as an objective assessment oracle.

- **Opt-in Progress State:**
  - Stores local learning progress in `$XDG_STATE_HOME/tamlinux/progress` (following the `welcome_seen` design). Zero network tracking or remote telemetry.
- **Hands-on Verification Tasks:**
  - Pairs Intermediate fading template tasks with automated checks: e.g. "Create a background pipeline that filters logs" $\rightarrow$ `tam learn verify` probes system state using the Check Contract.
  - Provides hints without spoiling answers, guiding users toward mastery of command-line tools and system navigation.

### Version 0.23.0 — Safe Command-Line Syntax Dissection
Extend `tam explain` to safely parse and explain compound shell commands, pipelines, and redirection.

- **Non-Executing Analytical Lexer:**
  - Safely tokenizes shell pipelines: command names, subcommands, option flags, arguments, redirects (`>`, `>>`, `2>&1`, `<`), and pipe chains (`|`).
  - Explains execution flow, data routing between standard streams, and file modifications in plain language.
  - Treats all input strictly as inert text data; never passes input to `/bin/sh` or evaluates expressions.

### Version 0.24.0 — Installation Framework Integration (`tam install plan/apply/verify`)
Complete the installation framework contract by connecting `tam install` to the product deployment engine.

- **`tam install plan --profile <profile>`:**
  - Evaluates host capabilities (from Milestone 4/8 inspection) and generates a reproducible, human-readable execution plan detailing exact package inputs, config files, required privileges, and Btrfs snapshot checkpoints without altering the system.
- **`tam install apply --profile <profile>`:**
  - Executes a reviewed plan with verified checkpoints and instant rollback hooks.
- **`tam install verify`:**
  - Post-installation verification auditing installed packages, checksums, system services, and desktop integration.
- **Profile Support:**
  - Arch with Hyprland (`arch-hyprland`), Arch with Sway (`arch`), and Void Linux (`void`).

### Version 1.0.0 — The Stable Showcase Release
Consolidate `tam` into a production-hardened, fully documented system utility.

- **Complete Tripartite Knowledge Hub:**
  - Exhaustive Use, Maintain, and Improve knowledge base covering the full Tamlinux workstation experience.
- **Rock-Solid Foundation:**
  - Dynamic man-page conversion with seamless cross-referencing, in-pager keyword search, progressive depth ladders, and offline self-inspection.
  - Full test coverage across interactive PTY fixtures, snapshot comparisons, and memory safety checks (Valgrind/AddressSanitizer clean).
- **Packaging Parity:**
  - Stable binary and man page packaging across Arch Linux (`PKGBUILD`), Nix (`tamlinux-packages`), and Void Linux (`xbps-src`).

---

## 5. Beyond 1.0: Multi-Distribution & Minimal Hardware

Following 1.0.0, `tam` evolves alongside the broader Tamlinux roadmap to support secondary distributions and resource-constrained machines:

| Version | Milestone Target | Focus & Feature Set |
| :--- | :--- | :--- |
| **1.1.0** | Gate C: Another Distribution | Adapt host inspection (`tam install inspect`), capability detection, and package tool wrappers for additional distributions (Debian, Fedora, Ubuntu) running systemd. |
| **1.2.0** | Gate D: Void Linux Pilot | Native inspection and documentation for the minimal Void Linux base: `runit` service supervision, `xbps` package querying, Btrfs subvolume rollback checks, and `musl` vs `glibc` reporting. |
| **1.3.0** | Repeatability & Terminal Profile | Dedicated terminal-only profile for legacy hardware (<512 MB RAM, circa-2006 processors, 32-bit architectures): ultra-compact memory footprint, terminal workspace cheatsheets, and headless administration guides. |

---

## 6. Version & Feature Reference Matrix

| Version | Milestone / Theme | Primary Subcommands & Surfaces | Key Deliverables |
| :--- | :--- | :--- | :--- |
| **0.14.0** | Pager Ergonomics & Completions | `src/pager.c`, completions | `/` in-pager text search, line/percentage telemetry, Bash completions. |
| **0.15.0** | Milestone 7: Content-Depth Ladder | `tam explain --level`, `tam search` | Beginner / Intermediate / Expert fading templates, depth line, ?level= navigation, index metadata. |
| **0.16.0** | Milestone 8: Self-Inspection | `tam install inspect`, check contract | Read-only findings model, versioned JSON check contract, `pkexec` detect-and-degrade. |
| **0.17.0** | Milestone 9: Grounded `ask` | `tam ask [OPTIONS] <question>` | Current baseline: Opt-in cited RAG question answering over corpus index, local server only (no daemon), interactive-only consent, --json. |
| **0.18.0** | Maintain Track Knowledge | `knowledge/maintain/` | Updates, Btrfs rollbacks, logs (`journalctl`/`dmesg`), power ladder, REISUB. |
| **0.19.0** | Improve Track Knowledge | `knowledge/improve/` | Zero-polling, ZRAM compression, `micro` autoreload, `tam-*` vs `fred-*`, AI tokens/s physics. |
| **0.20.0** | Architectural Rationale ("Why") | `tam why <topic>` | Dedicated top-level subcommand, 4-part rationale structure (decision, alternatives, tradeoffs, reopen triggers), excluded packages registry. |
| **0.21.0** | Fundamental Physics & CS Depth | `tam deeper`, `tam depth`, `tam trace` | 11-rung descent ladder down to quantum fields, guided traces (`keystroke-to-pixel`, `source-to-electricity`), CS theory track. |
| **0.22.0** | Offline Skills Progression | `tam learn`, `tam learn verify` | Opt-in `$XDG_STATE_HOME` progress tracking, automated offline task assessment using check contract. |
| **0.23.0** | Shell Syntax Dissection | `tam explain <command-line>` | Analytical non-executing shell parser explaining compound pipelines, redirects, and flags. |
| **0.24.0** | Installation Framework | `tam install plan/apply/verify` | Full installer contract implementation for `arch-hyprland`, `arch`, and `void`. |
| **1.0.0** | Stable Showcase Release | Complete command suite | Production release, unified knowledge graph, stable `libtam` ABI, multi-distro packages. |
| **1.1.0** | Multi-Distribution Adapter | `tam install inspect` | Host inspection across systemd distributions (Debian, Fedora, Ubuntu). |
| **1.2.0** | Void Linux Native Base | `tam install inspect`, `knowledge/` | Native Void + runit + Btrfs + seatd inspection and guides; musl/glibc analysis. |
| **1.3.0** | Minimal Hardware Profile | Terminal-only profile | Sub-512MB RAM optimizations, legacy/32-bit guides, headless administration. |
