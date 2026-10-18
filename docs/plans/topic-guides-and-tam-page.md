# Plan: Topic Guides, Metadata Topic Keywords, and tam Architecture

**Plan status:** Completed
**Reviewed:** 2026-10-18
**Scope:** Command 0.23.0 topic guides and command architecture page.
**Authority:** [Current development contract](development.md).
**Next action:** None for this closed plan; use the overhaul for new work.
**Completion evidence:** `a32f983`; [verification record](../ai/2026-10-15-tam-topic-guides-and-tam-page.md).

> Historical design snapshot. The show/explain separation survives. The
> [corpus 0.31.0 plan](knowledge-system-0.31.0.md) replaces the three-track labels,
> relocates user guidance logically, and owns new search/navigation behavior.

## Original design record

**Version:** `0.23.0`  
**Milestone:** 15  
**Owner:** Fred

---

## 1. Context & Architectural Problem

Prior to version `0.23.0`, the boundary between `tam explain` and `tam show` was conflated:
1. `tam explain <topic>` would look up a matching topic alias and immediately open that documentation page directly in the pager (e.g. `view_knowledge_page`).
2. There was no distinct notion of a **Topic Guide**—a curated or dynamic index explaining what pages exist on a subject and what each page covers before diving in.
3. Bare `tam explain` displayed a bounded usage synopsis, while topic browsing required typing `tam explain topics`.
4. There was no dedicated documentation page devoted to the `tam` command itself explaining its C architecture, Makefile build targets, compiler chain, library dependencies, and integration into the Tamlinux product release lifecycle.

## 2. Invariants & New Contract

### A. Clear Subcommand Separation of Concerns
- **`tam show [PAGE]`** is strictly for navigating **directly to a page**. It preserves path-based semantics (`knowledge/tam.html`, `knowledge/shell/pipelines.html`, `index.html`) and opens the target file in the pager (or prints plain text when piped).
- **`tam explain [TOPIC]`** **always displays a topic guide**; it **never** takes the user directly to a page.

### B. Topic Guides Structure
- A **Topic Guide** is a structured, hyperlinked document listing all pages in the Tamlinux corpus that match the queried topic or keyword.
- Each page entry in the Topic Guide includes:
  - A live hyperlink to the page (`<a href="...">`)
  - The architectural track (`Use`, `Maintain`, `Improve`)
  - A clear explanation/summary derived from the page's `<meta name="summary">` metadata.
- Pager navigation:
  - In an interactive terminal, `tam explain TOPIC` opens the topic guide directly in the built-in ANSI pager (`src/pager.c`).
  - Selecting a page link with `Tab`/`Shift-Tab` and pressing `Enter` follows the link to view the page.
  - Pressing `u` (or `Left Arrow`) returns back from the page to the Topic Guide.
  - Pressing `` ` `` (backtick) returns to the main Guide Table of Contents (`index.html`).
  - When output is redirected or `--pipe` is supplied, clean plain text with an indexed link table and zero terminal escape codes is emitted.

### C. Dynamic Topic Keywords Aggregation
- The list of topics is dynamically constructed at runtime from the metadata tags (`<meta name="topic">` and `<meta name="keywords">`) of all HTML pages in the knowledge base.
- When the user runs bare `tam explain`:
  - It opens the dynamic list of all topic keywords in the pager.
  - Each topic keyword in the list is an active link to its corresponding Topic Guide (`tam:explain?topic=<keyword>`).
  - Users can smoothly page through all keywords, select any keyword to view its topic guide, and from there navigate to any specific page.

### D. Devoted Page for `tam` (`knowledge/tam.html`)
A comprehensive documentation page is added for `tam` at `knowledge/tam.html`:
- Reachable via `tam show tam` (direct page display).
- Discovered via `tam explain tam` (Topic Guide listing pages with `tam` in metadata).
- Authored with Beginner, Intermediate, and Expert depth levels:
  - **C Implementation:** Pure C (C11 standard), sub-5ms startup, <4MB RSS, zero background daemons.
  - **Makefile Targets:** Comprehensive description of all targets: `all`, `command`, `corpus`, `test`, `test-command`, `test-corpus`, `check`, `install`, `uninstall`, `clean`, and `FORCE`.
  - **Compiler Chain & Libraries:** GCC/Clang flags (`-O2 -Wall -Wextra -Wpedantic -std=c11`), preprocessor definitions (`-DTAM_DATADIR`, `-DTAM_BUILD`), `libtam` linking (`-I$(LIBTAM_PREFIX)/include`, `-L$(LIBTAM_PREFIX)/lib -ltam`, `-Wl,-rpath`), and `zlib` (`-lz`). The exact compilation command executed by the makefile.
  - **Tamlinux Release Lifecycle:** The full develop -> test -> run -> release cycle, managed deployment via `tamlinux-packages` and Nix/Home Manager, immutable `/nix/store` package outputs, single managed endpoint activation, and generational rollback.
