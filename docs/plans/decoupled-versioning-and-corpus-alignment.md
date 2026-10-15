# Plan: Decoupled Versioning and Knowledge Corpus Alignment

**Status:** Approved & Implemented (2026-10-15)  
**Command Version:** `0.24.0`  
**Corpus Version:** `0.24.0`  
**Milestone:** 16  
**Owner:** Fred

---

## 1. Context & Architectural Requirements

Milestone 16 addresses three interrelated architectural goals for the `tam` workstation utility and its offline knowledge corpus:

1. **Decoupled Lifecycle Versioning:**
   Prior to `0.24.0`, the `tam` command binary and the offline documentation corpus were tightly bound to a single compile-time version macro in `src/version.h`. However, the C binary (parser, terminal pager, layout engine, index search, and diagnostic inspect/check execution) and the offline documentation corpus iterate at different cadences. The versioning of the command binary and the knowledge corpus must be decoupled so both start at `0.24.0`, but can diverge cleanly in the future as either the tool or the knowledge base evolves.

2. **Knowledge Corpus Subcommand Alignment & Perfection:**
   When top-level `tam inspect` and `tam check` were introduced, many pages across the knowledge corpus (in `maintain/`, `install/`, and `hardware/`) continued to reference older subcommand patterns (such as `tam install inspect` or `tam install check`) or lacked concrete examples of check task IDs (`storage_btrfs`, `storage_root_space`, `graphics_card`, `privilege_pkexec`). A comprehensive audit was needed to align all documentation pages with the modern subcommand contract.

3. **Discernment & Incorporation of Fred's Ideas:**
   Review notes in `improvements-fred-tamlinux/ideas/fred/`, discern sound architectural concepts from tentative suggestions, file them into permanent structured specifications in `improvements-fred-tamlinux/ideas/`, author an audit record (`ideas/fred/HISTORY`), and incorporate the good ideas into `0.24.0`:
   - Decoupled command and corpus versioning.
   - First-run orientation aliases (`tam hello`, `tam hi`) and shell alias guidance.
   - Botanical analogy completion with the 6th layer (**Fruit**) and command varieties (`*-nut`, `*-root`, `*`).
   - Fred's 6 Guiding Principles for Tamlinux.

---

## 2. Decoupled Versioning Architecture

### A. Source Authorities
- **Command Binary Version:** Declared in `src/version.h` via `TAM_COMMAND_VERSION` (`"0.24.0"`).
- **Corpus Version:** Declared in plain text in `knowledge/version` (`0.24.0\n`).
- **Corpus Build ID:** Stamped dynamically at index generation time by `tools/build-index.py` into `knowledge/.build_id` (git commit hash or UTC timestamp).

### B. Dynamic Runtime Discovery
The C binary dynamically discovers the active corpus root using `tam_find_knowledge_root()`:
1. `$TAM_KNOWLEDGE_DIR` (development / testing override).
2. `$DATADIR/tamlinux/knowledge` (system installation).
3. Relative fallback to `./knowledge` (in-tree execution).

Two runtime discovery functions were added in `src/tam.c` (declared in `src/version.h`):
- `tam_corpus_version(const char *root)`: Reads `$root/version`. If absent or unreadable, falls back to `TAM_DEFAULT_CORPUS_VERSION`.
- `tam_corpus_build_id(const char *root)`: Reads `$root/.build_id`. If absent, reports `tam_build_id()`.

### C. Truthful Dual-Version Reporting
- **Bare `tam` & `tam --help`:**
  ```text
  This is tam version 0.24.0 (dev ...) showing corpus version 0.24.0 (dev ...)
  ```
- **`tam --version`:**
  ```text
  tam 0.24.0 (dev ...)
  corpus: 0.24.0 (dev ...)
  libtam: 0.1.0
  cultivar: Tamlinux cultivar blue sparkler 0.4.3
  ```

---

## 3. Subcommand Perfection Across the Knowledge Corpus

Every HTML document in `knowledge/` was reviewed and aligned with the current command contract:

| Page | Updates & Alignments |
| :--- | :--- |
| `knowledge/maintain/index.html` | Updated quick-reference card to feature top-level `tam inspect` and `tam check`; added Check Contract overview. |
| `knowledge/maintain/updates.html` | Replaced legacy `tam install inspect` with top-level `tam inspect` for verifying package managers and system tiering. |
| `knowledge/maintain/rollback.html` | Updated health validation commands to `tam check storage_btrfs` and `tam check storage_root_space`. |
| `knowledge/maintain/power.html` | Updated GPU power diagnostics to `tam check graphics_card`. |
| `knowledge/maintain/recovery.html` | Updated privilege escalation diagnostic references to `tam check privilege_pkexec`. |
| `knowledge/install/index.html` | Documented top-level `tam inspect` and `tam check` as the modern diagnostic interfaces alongside `tam install inspect`. |
| `knowledge/hardware/resources.html` | Added worked examples for memory and disk health checks using `tam inspect` and `tam check storage_root_space`. |
| `knowledge/welcome.html` | Added `tam inspect` and `tam check` under workstation maintenance; noted `tam hello` and `tam hi` shortcuts. |
| `knowledge/shell/index.html` | Added "User aliases and the one-place-to-look principle", documenting `tam hello` / `tam hi` and best practices for user-defined shell aliases in `~/.bashrc`. |
| `knowledge/tam.html` | Added comprehensive reference table for all 14 `tam` subcommand invocations; documented decoupled versioning architecture. |
| `knowledge/index.html` | Updated Part IV (Maintenance) and Part VII (Installation) of The Tamlinux User Guide to showcase `tam inspect` and `tam check`. |

---

## 4. Ideas Review & Ecosystem Incorporation

### A. Ideas Discernment & Filing in `improvements-fred-tamlinux`
Four raw notes in `improvements-fred-tamlinux/ideas/fred/` were evaluated and filed into permanent design documents:
1. `ideas for tam` $\rightarrow$ `ideas/decoupled-tam-command-and-corpus-versioning.md` (Accepted: decoupled versioning).
2. `aliases for Tamlinux` $\rightarrow$ `ideas/user-shell-aliases-and-onboarding.md` (Accepted: one place to look; caution on 1-letter `h`).
3. `nut root stock cultivar` $\rightarrow$ `ideas/botanical-analogy-and-command-varieties.md` (Accepted: Fruit layer and command varieties).
4. `guiding principles` $\rightarrow$ `ideas/tamlinux-guiding-principles.md` (Accepted: 6 core principles).

Original notes were removed from `ideas/fred/` and an audit record `ideas/fred/HISTORY` was committed to preserve rationale.

### B. Ecosystem Documentation Enrichments
- **Botanical Taxonomy Completion (`knowledge/ecosystem/terminology.html`):**
  Added the 6th layer, **Fruit** (end-user graphical and CLI applications), completing the botanical lifecycle analogy:
  $$\text{Greenhouse (Hardware)} \rightarrow \text{Pot (Firmware/VM)} \rightarrow \text{Nut (UKI)} \rightarrow \text{Root-stock (Base OS)} \rightarrow \text{Cultivar (Shell/System)} \rightarrow \text{Fruit (Applications)}$$
  Documented the three varieties of commands in Tamlinux:
  - `*-nut`: Minimal, statically linked emergency binaries embedded in the UKI.
  - `*-root`: Foundational maintenance tools that manipulate root-stock infrastructure.
  - Unprefixed / regular commands: Cultivar and Fruit applications for standard user workflows.
- **Tamlinux Guiding Principles (`knowledge/ecosystem/philosophy.html`):**
  Added Fred's 6 Guiding Principles for Tamlinux:
  1. *Fewer Parts, Deeper Trust*
  2. *Offline Self-Sufficiency*
  3. *Zero Unearned Work*
  4. *One Clear Way*
  5. *Truthful Diagnostics*
  6. *Reversible Evolution*

---

## 5. Verification & Ergonomics Checks

- **Dual-Version Assertions:** `tests/run.sh` asserts both `0.24.0` command version and `0.24.0` corpus version. Tested mock corpus with divergent version string (`9.9.9-test`) to guarantee complete runtime decoupling.
- **Screen Budget Invariants:**
  - `tam --help`: 14 lines (limit $\le 20$ lines).
  - Bare `tam`: 11 lines (limit $\le 20$ lines).
- **Test Suite:** Full test suite passes clean (`make check`: all unit tests, PTY tests, and corpus golden snapshots match).
