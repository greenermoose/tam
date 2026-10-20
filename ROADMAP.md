# tam Roadmap

**Reviewed:** 2026-10-19. **Owner:** Fred (developer).
**Current Command Version:** `0.31.3`. **Current Corpus Version:** `0.33.1`.
Versions above come from `src/version.h` and `knowledge/version`; future targets
below are not delivered versions.

This roadmap sets priority and outcome, not a second implementation specification.
Read the [plan index](docs/plans/README.md) for status, the
[development contract](docs/plans/development.md) for behavior/invariants, and the
[plan maintenance procedure](docs/plan-maintenance.md) when changing any plan.

## 1. Vision and architecture

`tam` is the terminal entry point to Tamlinux's offline knowledge system. Its
primary journey has five parts: **Installing, Learning, Using, Maintaining,
Improving**. Learning routes cross shared knowledge; Beginner, Intermediate
and Expert are reading depths. Search, Topics, Man pages, Info manuals and Help
serve every part. Corpus 0.31.0 implements this organization and command 0.30.0
provides shared navigation and discovery. Command 0.31.0 adds explicit source
version checks and switching; 0.31.1 makes the fixed breadcrumb row keyboard
reachable. Corpus 0.32.0 documents the workflow.

Retain the native C/libtam reader, offline default, read-only browsing, semantic
HTML and indexed metadata, clean pipe output, explicit opt-in boundary for local
AI, and independent command/corpus versions. No external viewer, network fetch
or background daemon is required for knowledge browsing. Startup/memory targets
are goals to measure, not unconditional claims about every host or manual.

## 2. Delivered history

Since 2026-10-09, `tam` progressed rapidly through Milestones 1–16, link ergonomics, in-pager search, shell completions, content-depth ladder, self-inspection findings, check contract, grounded ask, output ergonomics, maintain-track corpus expansion, user guide architecture, improve-track corpus expansion, native GNU info subsystem, dynamic topic guides, command architecture, decoupled command/corpus versioning, attention line navigation, renderer layout fixes, pager navigation state restoration with numeric level synonyms, interactive pager depth-switching keys, and build-time Git revision and timestamp derivation:

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
| **0.18.0** | Milestone 10: Navigation & Output Ergonomics | Done (2026-10-14) | Quick Guide index return shortcut (`` ` ``) in `src/pager.c` with history push; Goldilocks screen budget (bare `tam explain` ≤15 lines, `tam --help` ≤20 lines); interactive hyperlinked `tam explain topics` with pager navigation, `--level` filtering, `--terse` (slugs only) and `--verbose` modifiers; `--terse` check status for `tam install check`; Bash completions. |
| **0.19.0** | Milestone 11: "Maintain" Track Corpus Expansion | Done (2026-10-14) | Authored comprehensive "Maintain" track knowledge pages: `maintain/index.html` (hub), `maintain/updates.html` (inode unlinking, package tiering), `maintain/rollback.html` (Btrfs CoW snapshots, Home Manager generations), `maintain/logs.html` (journalctl filtering, kernel ring buffer), `maintain/power.html` (power ladder, Shift-key wake), `maintain/recovery.html` (virtual consoles, REISUB, chroot repair); Beginner/Intermediate/Expert fading templates; two-pass exact-topic alias resolution in `src/index.c`. |
| **0.20.0** | Milestone 12: User Guide TOC, Botanical Analogy, & tam(1) Man Page | Done (2026-10-14) | Re-architected `knowledge/index.html` into comprehensive 7-part Table of Contents ("The Tamlinux User Guide") reflecting Fred's triad (Using, Maintaining, Improving); authored `knowledge/ecosystem/terminology.html` (Greenhouse, Pot, Nut, Root-stock, Cultivar analogy) with fading templates; authored UNIX section 1 manual page `man/tam.1` installed to `$(MANDIR)/man1/tam.1`; updated `src/man.c` and `src/tam.c` for `tam show topics` and dev man page lookup; bumped version to 0.20.0. |
| **0.21.0** | Milestone 13: "Improve" Track Knowledge Corpus Expansion | Done (2026-10-14) | Authored comprehensive "Improve" track knowledge pages: `improve/index.html` (hub), `improve/efficiency.html` (zero-polling, inotify, epoll, Wayland sockets), `improve/zram.html` (compressed RAM swap, zsmalloc, LZ4/zstd), `improve/terminal.html` (sub-10ms shell startup, Micro editor live reload Virtue V11, foot terminal), `improve/scripting.html` (strict Bash headers set -euo pipefail, safe arguments "$@", tam-* vs fred-* Decision D131), `improve/toolchains.html` (native C compilation, -O2, -march=native, isolated Git worktrees, tam-work concurrency), and `improve/ai-performance.html` (physics of local autoregressive generation, memory bandwidth bounds, quantization tradeoffs); Beginner/Intermediate/Expert fading templates; updated User Guide Part V links; bumped version to 0.21.0. |
| **0.22.0** | Milestone 14: Native GNU Info Subsystem | Done (2026-10-15) | Native C GNU Info reader (`src/info.c`, `src/info.h`) parsing `.info` and `.info.gz` files along `$INFOPATH` and `/usr/share/info/`; top-level directory browser (`tam info`), targeted manual and node lookup (`tam info MANUAL [NODE]`), tag table and indirect subfile support; hyperlinked menus, cross-references (`*Note`), and man links (`man:`); dedicated pager navigation keys (`n`, `p`, `u`, `t`, `i`); `a`/`s`/`d`/`f` 2D hyperlink navigation pad with first-visible-link selection; cross-system `info:` URI resolution in `tam show` and `tam man`; `bin/tam-info` wrapper; Bash shell completions; User Guide and man page cross-links; bumped version to 0.22.0. |
| **0.23.0** | Milestone 15: Topic Guides & tam Architecture | Done (2026-10-15) | Dynamic topic guides for `tam explain`; `tam explain` always displays a topic guide in the built-in pager, never taking you directly to a page (`tam show` is for going directly to a page); topic guides list matching pages with each page explained; bare `tam explain` dynamically collects all topic keywords from metadata and presents them in the pager; devoted `knowledge/tam.html` page (`tam show tam`) detailing C implementation, all Makefile targets, library linking, compiler chain, and the Develop -> Test -> Run -> Release lifecycle via `tamlinux-packages` and Nix/Home Manager; bumped version to 0.23.0. |
| **0.24.0** | Milestone 16: Decoupled Versioning & Corpus Subcommands Alignment | Done (2026-10-15) | Decoupled version numbers of `tam` command (`src/version.h`) and knowledge corpus (`knowledge/version`); dynamic runtime corpus version and build ID discovery; dual version reporting in bare `tam` help and `tam --version`; added `tam hello` and `tam hi` aliases; carefully reviewed and updated every page in knowledge corpus to incorporate `tam inspect` and `tam check` across maintenance, rollback, power, recovery, hardware, welcome, index, and `tam.html`; updated `tam.html` with full subcommands reference table; incorporated Fred's 6 guiding principles into philosophy and botanical Fruit application layer into terminology; updated man pages, completions, and test suites. |
| **0.25.0–0.25.2** | Attention Line, Lane Release Independence, & Renderer Layout | Done (2026-10-16) | Attention line cursor (`cursor_line`), charcoal tint, directional link catching, and safe `Enter` activation (0.25.0); independent command/corpus release tests (0.25.1); extended HTML entity whitelist and display-column (`dwidth`) measurement in `src/render.c` (0.25.2). |
| **0.26.0** | Pager Navigation Position Stack, Numeric Level Synonyms, & Punctuation Spacing Fix | Done (2026-10-16) | Full `nav_state_t` (`{ href, top, cursor_line }`) in `src/pager.c` restoring exact viewport and cursor line on back (`u`/Left Arrow) and forward (Right Arrow) across page and `#anchor` jumps; numeric and `all` depth synonyms (`--level 0\|all\|1\|2\|3` and `?level=0\|all\|1\|2\|3`) across `tam show`, `tam explain`, and `tam explain topics` plus Bash completions and `man/tam.1`; renderer fix in `src/render.c` so tokens starting with punctuation keep their preceding space when separated by whitespace in the source (e.g. `git add .`). |
| **0.27.0** | Pager Reading Depth Keys & Retired 1–9 Link Jumping | Done (2026-10-16) | Bound keys `0`, `1`, `2`, `3` inside `src/pager.c` to switch the active page's reading depth (`all`, `beginner`, `intermediate`, `expert`) with history push and status feedback, and retired legacy `1`–`9` link jumping in favor of the attention line, `Tab`/`Shift-Tab`, and `a`/`s`/`d`/`f`. |
| **0.28.0** | Build-Time Git Page Revisions & Update Timestamps | Done (2026-10-16) | Deterministic build-time derivation of per-page revision count (`git rev-list --count HEAD -- knowledge/<page>`) and last-updated timestamp (`git log -1 --format=%cd --date=format:%Y-%m-%d %H:%M -- knowledge/<page>`) in `tools/build-index.py`, stored in `knowledge/index.tsv` and `knowledge/index.json` and displayed as `Rev <N> Last updated <YYYY-MM-DD HH:MM>` on every rendered corpus page via `tam_depth_apply_rev()` and in `tam explain topics --verbose`. |
| **0.29.0** | Portable Pager Display | Done (2026-10-17) | Terminfo capability profiles, incremental row painting, Unicode cell geometry, retained-layout resize reflow, and terminal-safe fallbacks. |
| **0.29.1** | Selected Link Focus | Done (2026-10-17) | The selected hyperlink now uses bold reverse video, with underline fallback where reverse video is unavailable, making the Enter/Right Arrow target easier to identify. |
| **0.29.2** | Highlight Selected Link Spaces | Done (2026-10-17) | Spaces inside the selected hyperlink now receive the same emphasis as its other characters, keeping multiword link labels visually continuous. |
| **0.29.3** | Keep Link Boundary Spaces Unhighlighted | Done (2026-10-17) | Only spaces between visible characters in a selected hyperlink receive emphasis; layout whitespace before the link remains unhighlighted. |
| **0.29.0 corpus** | Linux Boot Fallbacks & Rescue Routes | Done (2026-10-17) | New Maintain guide distinguishes firmware paths, alternate kernels and initramfs images, snapshot boots and external rescue media by their shared boot dependencies. |

| **0.30.0 corpus** | Snapper guide | Done (2026-10-18) | Dedicated snapshot-management page and maintenance/recovery cross-links (`d13c1f6`). |
| **0.30.0 command** | Shared reader navigation and discovery | Done (2026-10-18) | Common Guide/Back/Help/Quit navigation; in-reader Man, Info and Guide search discovery (`0b3f008`, `52e71f7`). |
| **0.31.0 corpus** | Five-part knowledge system and guided learning | Done (2026-10-18) | Five-part root, logical homes for all 50 existing pages, five new learning/navigation pages, route and search acceptance (`597cb71`, `f4fa539`). |
| **0.31.1 command** | Breadcrumb keyboard navigation | Done (2026-10-18) | Up Arrow focuses the fixed breadcrumb row; `a`/`f`, `s`/`d`, Tab and Shift-Tab navigate its links. |
| **0.32.2 corpus** | Reliable corpus builds | Done (2026-10-19) | TCK9: `make corpus` rechecks all pages and Git metadata; `make corpus-and-test` builds and checks. Unchanged outputs retain timestamps. |
| **0.31.2 command** | Wrapped tables and clearer topic lists | Done (2026-10-19) | Tables wider than the terminal wrap their widest columns instead of stacking every row as `Header: value` records; stacking remains only for tables that cannot fit unbroken words. Topic lists are titled Topics A to Z and Keyword Index. |
| **0.32.4 corpus** | Keyword corrections | Done (2026-10-19) | Removed topic keywords that misdirect or mean nothing as lookups (for example `2006`, `v11`, `zram` on the sustainability page); `top` now leads to the resources page. |
| **0.33.0 corpus** | Operating Systems reference | Done (2026-10-19) | Twelve pages under Learning > Ecosystem (`ecosystem/os/`): kernel architectures, the Linux families, BSDs, illumos, other independent systems, and the Tam 2.0 microkernel research, readable offline. Roles come from the `tamlinux` operating-system horizons plan. |
| **0.31.3 command** | Shorter default message | Done (2026-10-19) | Bare `tam` fits a half-height terminal (30 lines): it drops `tam search`, `--terse` and `--verbose`, which `tam --verbose` still lists. Usage reads `tam <subcommand> [arguments] [--options]`; `--terse` drops `tam info`. |
| **0.33.1 corpus** | Welcome page wording | Done (2026-10-19) | The welcome page says why Tam exists and who else it may help. |


## 3. Current work and follow-up

The approved [corpus build plan](docs/plans/corpus-build.md) makes
`make corpus` sufficient after edits and commits, and renames the combined
build/check entry point to `make corpus-and-test` (TCK9, reliable corpus builds).
Source implementation and focused, parallel and integrated checks passed for
corpus 0.32.2; command 0.31.1 is unchanged. Package stages remain separate.

The accepted [source-upgrade and stage-switching plan](docs/plans/upgrade.md) covers
Development/Test/Run/Release discovery, source installation into the approved
user development prefix, exact release pins and rollback (TCK6–TCK8). New code
builds for the developer receive independently maintained command/corpus version
bumps; reinstalling existing sources preserves their versions. Release tags
record the command/corpus version pair. The developer approved the complete
design on 2026-10-18. TCK6–TCK8, the managed isolated source build, live metadata
check, and developer-directed first activation in the real development prefix
are complete. Disposable acceptance builds and live metadata checks have
standing developer authorization. The live metadata check resolves
development and test; `main` predates `knowledge/version`, so the run stage
cannot be described until it is promoted. See the
[Luna handoff](docs/handoffs/tam-upgrade-luna.md).

The [five-part knowledge-system plan](docs/plans/knowledge-system-0.31.0.md)
is implemented and its TCK1–TCK5 source and automated acceptance work is
complete. Its command/corpus pair is **0.30.0 / 0.31.0**. The exact source
commits and checks are recorded in that plan and its
[implementation session record](docs/ai/2026-10-18-knowledge-system-implementation.md).

No package assembly, stage transition, installation or deployment was part of
this work. The owning assembly can consume the Tam source at `f4fa539`; real
Linux-console and Foot review of the exact packaged candidate remains
developer-directed. Installer execution and stateful progression remain
deferred.

Separately, [portable pager display](docs/plans/pager-display-portability.md) has
implemented source and automated tests (command 0.29.0), but exact packaged
Linux-console and Foot visual acceptance remains pending. Only the developer
can direct that stage/review. Do not treat it as a new source-work assignment
or claim it passed during this planning cleanup.

## 4. Deferred direction, without reserved versions

These remain ideas or separate dependencies, not approved implementation slices
of corpus 0.31.0:

| Direction | Boundary / prerequisite |
| --- | --- |
| Non-executing shell syntax dissection | A separately reviewed parser/CLI design; all input remains inert. Its previous command 0.30.0 reservation is replaced by knowledge navigation/discovery. |
| Installer plan/apply/verify | Product installation/deployment framework and separately approved implementation. Previous command 0.31.0 reservation is withdrawn. Installing documentation must truthfully describe what exists now. |
| Stateful `tam learn` and automated assessment | Separate opt-in state and assessment design. Guided learning in corpus 0.31.0 does not require it. |
| Rationale, deep-system traces and CS learning | Exploratory content/commands, not committed milestones. Existing depth filtering is already implemented. Old proposed 0.26/0.27/0.28 milestones are not future reservations. |
| Wider distribution/terminal-only profiles and stable 1.0 | Follow the product's capabilities and evidence; no promised feature set or version allocation from this roadmap. |
| Merged full-text search over installed manuals | Separate scalability/search work. This milestone includes explicit Man keyword discovery and Info directory browsing. |

A future scope decision creates or updates an authoritative plan and allocates
versions against then-current files. Do not infer a scheduling commitment from
an old proposed sequence or a version number.

## 5. Supersession and closure

The developer's 2026-10-18 decision replaces the old seven-part contents and
three-track organizational model with the five-part spine. The former future
version sequence and feature-reference matrix are retired as scheduling
authorities; delivered history above remains history. The
[plan index](docs/plans/README.md) records every plan's disposition and links
old designs to their successor at stable paths.

Plans close only with evidence or an explicit replacement/retirement. Update
this roadmap, the development contract and index together whenever scope or
completion changes. No plan update moves Test, Run or Release or changes the
package-only delivery rule.
