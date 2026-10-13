# Scaling Plan: Explainable Topics in tam

**Stage:** Plan & Reference. Companion to [orientation and explanation](orientation-and-explanation.md)
and [development plan](development.md).

> **Revised in part (2026-10-12).** Milestones 6–9 of the
> [development plan](development.md) supersede parts of this roadmap. The single
> search verb is now **`tam search`**, not Phase 4's `tam explain search`
> (the naming clash is settled in favour of `search`), and the corpus-driven
> metadata of Phase 5 is promoted from last to first (milestone 6's corpus
> index). Phase 2's domain grouping remains useful for bare `tam explain`, but
> it is no longer the primary scaling answer; the index and search carry that
> load. Read the rest of this document as the historical rationale for the
> pre-index design.

## 1. Context and Problem Statement

`tam explain <topic>` provides offline, terminal-based explanations of Tamlinux
concepts, system tools, and shell constructs. When run without arguments (`tam explain`),
the command displays a list of available topics rather than reporting a usage error.

Currently, the curated topic catalog is small (~5 guide target pages, ~35 keywords).
Printing the entire catalog directly to stdout fits comfortably on a standard 80x24
terminal window without scrolling.

As Tamlinux documentation expands, the explainable catalog will grow substantially:
- Core Unix utilities, file formats, and pipes (`find`, `awk`, `sed`, `tar`, `systemd`, `fstab`)
- Tamlinux cultivars, packaging tiers, and lifecycle (`pacman`, `nix`, `home-manager`, `mise`)
- Wayland / Hyprland desktop integration and keybindings (`hyprland`, `quickshell`, `waybar`, `foot`)
- System hardware, monitors, and audio (`pipewire`, `wireplumber`, `seatd`, `udev`, `edid`)
- Agent workflows, provenance, and delegation (`tam-work`, `claim`, `agy`, `opencode`)

Without a scaling strategy, an unconstrained dump of hundreds of topics would:
1. **Overflow the viewport:** Scroll off the screen immediately, requiring terminal scrollback.
2. **Impede discovery:** Create visual clutter where primary concepts are lost among dozens of aliases.
3. **Overwhelm scripts:** Produce excessively verbose output when piped or inspected non-interactively.
4. **Cause code bloat:** Hardcoding hundreds of topic mappings in C source (`src/tam.c`) makes documentation maintenance brittle.

---

## 2. Phased Scaling Roadmap

```
Phase 1: Current (<50 keywords)        → Formatted grouped list to stdout
Phase 2: Growth (20–60 topics)         → Domain categorization & progressive disclosure
Phase 3: Mature (60–200 topics)        → Interactive TUI pager + plain-text fallback
Phase 4: Large Catalog (200+ topics)   → Search / "apropos" + fuzzy suggestions
Phase 5: Extensible Architecture       → Corpus-driven metadata + shell completions
```

### Phase 1: Curated Listing to stdout (Current State, 0.6.0)
- **Scope:** Catalog has < 20 primary topics and < 50 aliases.
- **Behavior:** `tam explain` outputs an organized list grouping primary topics, 1-line summaries, and aliases directly to stdout, exiting 0.
- **Constraints:** Must not exceed 24 lines so it fits on any standard terminal screen without scrolling. Zero ANSI control characters in plain output.

### Phase 2: Domain Categorization & Progressive Disclosure (20 – 60 Topics)
- **Problem:** Topic count exceeds single-screen height.
- **Behavior:**
  - Group topics into 5–8 high-level functional domains:
    - `Orientation & Basics`
    - `Shell & Scripting`
    - `System Administration & Hardware`
    - `Packaging & Cultivars`
    - `Desktop Environment`
    - `Agent & Automation`
  - Default `tam explain` displays a **Domain Summary View**:
    ```text
    tam explain - available topic domains

      shell/         Pipelines, redirection, filters, job control (14 topics)
      install/       Installation, updates, layers, host inspection (8 topics)
      desktop/       Hyprland, wayland, monitor configuration (6 topics)
      man/           Manual pages, apropos, command documentation (5 topics)

    Run 'tam explain <domain>/' to list topics in a domain.
    Run 'tam explain --all' to list every available topic.
    Run 'tam explain <topic>' to view an explanation.
    ```
  - Filtering by domain: `tam explain shell/` or `tam explain --domain shell` lists only topics in that domain.
  - `--all` flag: `tam explain --all` emits the complete uncompressed list for users or scripts wanting exhaustive output.

### Phase 3: Interactive TUI Pager Integration (60 – 200 Topics)
- **Problem:** Browsing a multi-page catalog requires interactive scrolling and navigation.
- **Behavior:**
  - **Interactive Terminal (`isatty(STDOUT_FILENO)` and not `--pipe`):**
    `tam explain` automatically renders the curated Topic Index in the built-in TUI pager (`tam_pager_run`).
    - Standard navigation keys: `j`/`k`, arrows, `PageUp`/`PageDown`.
    - Hyperlinked entries: Users select a topic with `Enter` or Right Arrow to navigate directly to that topic's full documentation page.
    - Quick exit: `q` or `Esc` returns directly to the shell prompt.
  - **Non-interactive / Piped (`--pipe` or redirected stdout):**
    Falls back to compact, clean plain-text output (domain summary or columnated list), maintaining POSIX pipe friendliness.

### Phase 4: Search, "Apropos", and Fuzzy Matching (200+ Topics)
- **Problem:** Manual scrolling or category browsing becomes slower than direct search.
- **Behavior:**
  - **Keyword Search:**
    ```sh
    tam search <term>     # searches topic names, summaries, and aliases
    ```
    Behaves like a fast, local `apropos` for the Tamlinux knowledge base. The
    earlier `tam explain search <term>` / `tam explain --search <term>` spellings
    are **superseded**: `tam search` is the single search verb (milestone 6).
    Fuzzy matching is shown as a bounded suggestion, never an auto-redirect.
  - **Intelligent Miss Handling ("Did you mean?"):**
    When `tam explain <query>` does not find an exact match:
    - Computes Levenshtein edit distance and substring matching against the topic catalog.
    - Emits suggestions instead of a blank failure:
      ```text
      tam: no exact explanation found for 'pipelin'

      Did you mean one of these?
        pipelines    Shell pipelines, redirection, and core filters
        pipe         Shell pipeline operator

      Run 'tam explain pipelines' to view.
      ```
  - **Disambiguation:** If a query matches multiple topics partially, presents a concise list of candidate topics with invocation hints.

### Phase 5: Corpus-Driven Metadata & Shell Auto-Completion
- **Promoted (2026-10-12):** corpus-driven metadata is now milestone 6's corpus
  index, the first step of the new sequence, not the last. Shell completions
  remain future work.
- **Problem:** Hardcoding topic tables in `src/tam.c` requires recompilation for documentation updates and separates content from code.
- **Architecture:**
  - **Metadata in Knowledge Pages:**
    Topic titles, summaries, keywords, and categories are extracted directly from the HTML corpus (e.g. via `<meta name="keywords" content="...">`, `<meta name="summary" content="...">` or a build-time/staged `knowledge/topics.json`).
  - **Build / Runtime Generator:**
    During package build, a compact lookup structure is generated, allowing `tam` to resolve topics dynamically while preserving instantaneous (<5ms) startup.
  - **Shell Completions:**
    Provide generated completion files for Bash (`/usr/share/bash-completion/completions/tam`), Zsh (`_tam`), and Fish, allowing tab completion for `tam explain <TAB>`.

---

## 3. Guiding Invariants

1. **Strict Offline Execution:** No network calls, telemetry, or remote lookups are ever introduced.
2. **Zero External Dependencies:** Built purely with C and `libtam`; no external pagers or browsers required.
3. **Pipe and Redirect Safety:** Piped output always emits plain, deterministic text without ANSI escapes, cursor codes, or interactive prompts.
4. **Input Safety:** Topic arguments are treated purely as data; shell metacharacters (`|`, `>`, `;`, `$()`) are never evaluated.
5. **No State Pollution:** `tam explain` (with or without arguments) never creates state files or modifies `$XDG_STATE_HOME`.
