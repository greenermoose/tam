# tam Roadmap

**Last Update:** 2026-10-30
**Owner:** Fred
**Current Command Version:** `0.45.2`.
**Current Corpus Version:** `0.50.1`.

Versions above come from `src/version.h` and `knowledge/version`; future targets
below are not delivered versions.

This roadmap sets priority and outcome.
Read the [plan index](docs/plans/README.md) for status, the
[development contract](docs/plans/development.md) for behavior/invariants, and the
[plan maintenance procedure](docs/plan-maintenance.md) when changing any plan.

## 1. Vision and architecture

`tam` is the terminal entry point to Tamlinux's offline knowledge system. Its
primary journey has five parts: **Installing, Learning, Using, Maintaining,
Improving**. Learning paths cross shared knowledge; Beginner, Intermediate
and Expert are difficulty levels the reader can choose to show. Search, Topics,
Man pages, Info manuals and Help serve every part.

Retain the native C/libtam reader, offline default, read-only browsing, semantic
HTML and indexed metadata, clean pipe output, explicit opt-in boundary for local
AI, and independent command/corpus versions. No external viewer, network fetch
or background daemon is required for knowledge browsing. Startup and memory
targets are goals to measure.

## 2. Current state and follow-up

| Branch | Command / corpus | Meaning |
| --- | --- | --- |
| `develop` | 0.45.2 / 0.50.1 | Newest source; pager level line and line labels, See also -> relatives leader, pager resize cursor fix, TCK23 review pending, TCK24, TCK25 Using, Maintaining, and Improving fully migrated and enforced, rootstock standardized, voice dictation updated to F9 tap toggle, shell voice commands, Fred Horch bio page, pager italic sitm, and terminfo/ncurses ecosystem pages. Development installation is a separate developer-directed step. |
| `origin/test` | 0.43.2 / 0.43.0 | Latest cached remote Test source at `eea4509`; not an installed-candidate claim. |
| `origin/main` | 0.38.1 / 0.42.0 | Latest cached remote Run source at `c161190`; not an installed-candidate claim. |

This table is a source snapshot taken on the review date. Remote-tracking rows
use the cached `origin/` refs because local `test`/`main` refs can be stale;
`git show <ref>:knowledge/version` and `src/version.h` are authoritative for
those refs. Source refs do not prove an installed package selection. Stage
promotion still requires the developer's word through the Tamlinux lifecycle.

Completed plans: the [five-part knowledge system](docs/plans/knowledge-system-0.31.0.md)
(TCK1–TCK5, command 0.30.0 / corpus 0.31.0), the
[source-upgrade and stage-switching plan](docs/plans/upgrade.md) (TCK6–TCK8,
command 0.31.0 / corpus 0.32.0, first activated in the development prefix),
[reliable corpus builds](docs/plans/corpus-build.md) (TCK9, corpus 0.32.2),
[home-row pager keys](docs/plans/pager-home-row-keys.md) (TCK10, command 0.32.0 /
corpus 0.33.5), the [developer loop](docs/plans/dev-loop.md) (TCK11–TCK14,
tooling and tests only: `make dev`, `make check-repo`, pager tests on fixtures),
and [pager quick keys, address display and status row](docs/plans/pager-quick-keys.md)
(TCK15–TCK19, command 0.34.0 / corpus 0.35.1). Disposable acceptance builds and
live metadata checks have standing developer authorization.

Approved next outcome: [learning levels, goal paths and self-quizzes](docs/plans/learning-levels.md)
(TCK20–TCK25). TCK20, the teaching contract in
[`docs/corpus-pedagogy.md`](docs/corpus-pedagogy.md) and its templates, and TCK21,
the sticky level lens (command 0.37.0, banner ranking fixed in 0.37.1), and
TCK22 (folding, self-quizzes and block-level levels, command 0.38.0 / corpus
0.37.1), are delivered. TCK23 source (pilot pages and two goal paths) is
delivered as corpus 0.38.0. The developer's review of all pilot levels and
First steps remains pending. His subsequent "Continue" directed TCK24
scaffolding, implemented as corpus 0.39.0: skills, placement, glossary,
bibliography, additive metadata and a coverage/gap report. TCK25's Using part
and staged level paths are implemented in corpus 0.40.0. Corpus 0.41.0
migrates Maintaining's rollback and emergency-recovery lessons and adds
three glossary terms. Corpus 0.42.0 migrates boot, boot-fallback and Snapper
and adds the staged Rescue path. Corpus 0.42.3 migrates logs and power.
Corpus 0.43.2 completes Maintaining and enables track-wide enforcement;
corpus 0.43.8 completes Improving and its enforcement. Corpus 0.44.0 adds
twelve introductory glossary entries and a Guide index link. Corpus 0.44.1 makes practice cleanup limited to the known file and empty directory. Corpus 0.45.0 revises the tagged Learning lessons, classifies Learning and Installing pages, adds the remaining goal paths and enables tagged-section enforcement in all parts. TCK25 source implementation is complete. Continue defining unfamiliar concepts and expanding the glossary; pilot and learner review remain pending. Allocate versions when
each slice starts. The deferred items in section 4 are candidates, not assignments.

The approved [external-links work](docs/plans/external-links.md) is integrated in command 0.44.0. Fred reports that external links work; the candidate, environments and full acceptance matrix remain unrecorded.

Still pending, and only on the developer's direction: real Linux-console and
Foot review of an exact packaged candidate, including the
[portable pager display](docs/plans/pager-display-portability.md) work (command
0.29.0). Do not treat it as a new source-work assignment or record it as passed
without that review. Installer execution and stateful progression remain
deferred (section 4).

## 3. Delivered history

Every delivered version since 2026-10-09, oldest first. Through 0.24.0 the
command and corpus shared one number; after that, rows marked *corpus* are
corpus versions and the others are command versions. "Done" means the source
is complete and tested; which pair a Tamlinux candidate packages is recorded
in that candidate, not here.

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
| **0.18.0** | Milestone 10: Navigation & Output Ergonomics | Done (2026-10-14) | Quick Guide index return shortcut (Backtick) in `src/pager.c` with history push; Goldilocks screen budget (bare `tam explain` ≤15 lines, `tam --help` ≤20 lines); interactive hyperlinked `tam explain topics` with pager navigation, `--level` filtering, `--terse` (slugs only) and `--verbose` modifiers; `--terse` check status for `tam install check`; Bash completions. |
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
| **0.31.0 command / 0.32.0 corpus** | Version checks, source upgrade and stage switching | Done (2026-10-18) | `tam upgrade`: bounded version discovery, transactional source generations, explicit stage switching and rollback; the corpus documents the workflow (`f721120`). |
| **0.31.1 command** | Breadcrumb keyboard navigation | Done (2026-10-18) | Up Arrow focuses the fixed breadcrumb row; `a`/`f`, `s`/`d`, Tab and Shift-Tab navigate its links. |
| **0.32.1 corpus** | Welcome page rewrite | Done (2026-10-18) | A warmer first page: what Tamlinux is, why it is called Tam, where the project stands, and where to go next (`86b9da5`). |
| **0.32.2 corpus** | Reliable corpus builds | Done (2026-10-19) | TCK9: `make corpus` rechecks all pages and Git metadata; `make corpus-and-test` builds and checks. Unchanged outputs retain timestamps. |
| **0.32.3 corpus** | Welcome page revision | Done (2026-10-19) | Further welcome-page wording (`b7a838f`). |
| **0.31.2 command** | Wrapped tables and clearer topic lists | Done (2026-10-19) | Tables wider than the terminal wrap their widest columns instead of stacking every row as `Header: value` records; stacking remains only for tables that cannot fit unbroken words. Topic lists are titled Topics A to Z and Keyword Index. |
| **0.32.4 corpus** | Keyword corrections | Done (2026-10-19) | Removed topic keywords that misdirect or mean nothing as lookups (for example `2006`, `v11`, `zram` on the sustainability page); `top` now leads to the resources page. |
| **0.33.0 corpus** | Operating Systems reference | Done (2026-10-19) | Twelve pages under Learning > Ecosystem (`ecosystem/os/`): kernel architectures, the Linux families, BSDs, illumos, other independent systems, and the Tam 2.0 microkernel research, readable offline. Roles come from the `tamlinux` operating-system horizons plan. |
| **0.31.3 command** | Shorter default message | Done (2026-10-19) | Bare `tam` fits a half-height terminal (30 lines): it drops `tam search`, `--terse` and `--verbose`, which `tam --verbose` still lists. Usage reads `tam <subcommand> [arguments] [--options]`; `--terse` drops `tam info`. |
| **0.33.1 corpus** | Welcome page wording | Done (2026-10-19) | The welcome page says why Tam exists and who else it may help. |
| **0.33.2 corpus** | Current versus planned wording | Done (2026-10-19) | Pages separate what Tamlinux does today from what is planned: Void and Sway marked planned, Limine (not systemd-boot) as the boot loader, no Tamlinux install medium, real version output, and no unsourced compatibility or release-tag claims. |
| **0.32.0 command / 0.33.5 corpus** | Home-row pager keys and attention column | Done (2026-10-20) | TCK10 (D175): `d`/`k` and `s`/`l` move the attention line one row; `a`/`j` and `f`/`;` move an attention column from link to link along it; `g` follows, `h` helps; `x`/`.` and `c`/`,` jump to the top and bottom, and `G` is retired. The welcome page teaches the keys. |
| **0.33.6 corpus** | Guide root index wording | Done (2026-10-20) | Retitled the Guide root index to "Your Tamlinux User Guide", added the Welcome link to the primary list, and refined the introductory description and footer prompt. |
| **0.34.0 corpus** | POSIX Programmer's Manual guide page | Done (2026-10-20) | Dedicated `man/posix.html` guide page under Learning > Manual Pages covering IEEE Std 1003.1 sections `0p`, `1p`, and `3p`, standard interfaces versus GNU/Linux extensions, `APPLICATION USAGE` and `RATIONALE` sections, and browsing in `tam` (`08b166a`). |
| **0.34.1 corpus** | Appendices on Guide index one per line | Done (2026-10-20) | Appendices on Guide index one per line |
| **0.33.0 command / 0.35.0 corpus** | Native `tldr` pages subsystem and curated corpus | Done (2026-10-20) | Bundled AI-curated English `tldr` pages for Tamlinux under `knowledge/tldr/` (`linux` and `common` sections, `index.tsv`, `LICENSE.md`, `SOURCE.json`); native C `tam tldr` reader and index/discovery browser (`src/tldr.c`, `src/tldr.h`, `bin/tam-tldr`, `tam:tldr`) rendering both short and long option forms (`[-s|--long]`), hyperlinked `See also:`/alias/`man:` references, and fixed top breadcrumb navigation (`Guide > tldr pages > <cmd>`). |
| **0.34.0 command / 0.35.1 corpus** | Pager quick keys, address display, status row, and Guide index | Done (2026-10-20) | Pager link keys `]` and `[` with Tab/Shift-Tab aliases; search-only `N`; status row `L<cursor> <first>-<last>/<total>`; `=` address display (`Path = <address>`); `page:` addresses in `tam show`; `\` guide search; labelled prompts; `tam:help` and prompt action links; Guide index Quick keys table; corpus 0.35.1 (TCK15–TCK19). |
| **0.35.2 corpus** | Common pitfall sections become Best practice | Done (2026-10-20) | The 16 `Common pitfall` sections are `Best practice` sections that lead with the correct action and then explain what goes wrong otherwise ([plan](docs/plans/common-pitfall-to-best-practice.md), `ae1fa41`). |
| **0.36.0 corpus** | Pare down welcome page and add dedicated About page | Done (2026-10-20) | Paring down `welcome.html` to a focused orientation page and moving project background and developer notes into a new dedicated `about.html` page (`tam show about`). |
| **0.35.0 command / 0.36.1 corpus** | Operating system and Tamlinux cultivar identity | Done (2026-10-20) | `tam inspect` shows the distribution (`Arch Linux (rolling)`), an `OS version` row only with `VERSION_ID`, and `Tamlinux cultivar: <Name> <version>` from the invoking user's version and cultivar files (the `pkexec` caller under `pkexec`); `tam --version` matches; new `os_identity` and `tamlinux_version` findings; corpus pages document the rows. |
| **0.37.0 corpus** | Environment-minded systems and desktops | Done (2026-10-20) | New Operating Systems Reference page `ecosystem/os/environment-minded.html` (`tam show environment-minded`): wattOS, Emmabuntüs, GALPon MiniNo, KDE Eco and the Blue Angel label, the Green Software Foundation SCI standard, Nura, /e/OS, Collapse OS and Dusk OS, permacomputing and End of 10, none with a Tamlinux role; linked from the reference index and the sustainability page. |
| **0.36.0 command** | Search results open in the pager | Done (2026-10-20) | On a terminal, `tam search TERM...` opens the `tam:search?query=TERM` results page in the pager, so each result link can be followed and searched again; `--pipe`, `--json` and non-terminal output keep the plain text and JSON contracts; a miss still prints suggestions and exits 1. |
| **0.37.0 command** | Sticky level lens (TCK21) | Done (2026-10-21) | Keys `0`-`3` set one reading level that holds across links, Back and restarts, saved in `$XDG_STATE_HOME/tamlinux/tam/level` (`TAM_LEVEL` and `--level` override it); a level bar under the title, hidden-section markers, the level in the status row, and a "Written for" banner with links at the reader's level on pages without it, so no level key dead-ends; `welcome_seen` moves to `tamlinux/tam/` ([plan](docs/plans/learning-levels.md)). |
| **0.37.1 command** | Banner links related pages by parent | Done (2026-10-21) | The "Written for" banner links up to three pages at the reader's level from the index's parent links (children, then siblings, then the parent, each by shared prerequisites and title); a page with no such relative links only the level path ([plan](docs/plans/learning-levels.md)). |
| **0.38.0 command / 0.37.1 corpus** | Folding and self-quizzes (TCK22) | Done (2026-10-21) | Answers fold in the pager with Enter, `r` and `*`, reset on navigation, and collect under Answers in pipe output; block-level filtering, generated section labels, named asides and command risk labels; 23 pages lose hand-typed level prefixes and lint checks enforce the markup ([record](docs/ai/2026-10-21-tam-folding.md)). |
| **0.38.1 command** | Pager resize cursor preservation | Done (2026-10-21) | Reflow preserves whether the attention line is on text or a blank separator when both share a logical position; PTY screen tests cover text and blank anchors ([record](docs/ai/2026-10-21-tam-pager-resize-drift.md)). |
| **0.39.0 command** | Pager level line | Done (2026-10-21) | One dimmed, right-aligned level line above the page heading, `<Level> text [on <Level> path] [(no <Level> text)]`, replaces the level bar and the "Written for" banner; the path is linked, and a page without the lens level adds `<Level> text: A \| B \| C` with up to three related pages (never the level path itself). Keys `0`-`3` still switch levels ([record](docs/ai/2026-10-21-tam-pager-level-line.md)). |
| **0.44.0 command** | External links | Done (2026-10-27) | Confirm and hand HTTP(S) links to the reader’s browser; OSC 8 in supported terminals. Fred reports external links work; full acceptance details remain open. |
| **0.43.2 command** | Pager relatives line leader | Done (2026-10-23) | Changed the line c relatives leader for all lenses to `See also -> <links>` so readers understand the list contains followable navigation links rather than introducing the text below ([record](docs/ai/2026-10-23-tam-see-also-leader.md)). |
| **0.43.1 command / 0.42.2 corpus** | Arrow-key hints on answers | Done (2026-10-22) | Answers read `[+] Answer (-> to show)` and `[-] Answer (<- to hide)`; Right Arrow toggles the answer on the attention line even when its link is not selected. `+`, `-`, Enter, `r` and `*` still work; the help overlay, man page and Learning paths page name the arrows ([record](docs/ai/2026-10-21-tam-pager-level-line.md)). |
| **0.43.0 command** | Left Arrow hides a shown answer | Done (2026-10-22) | On a shown answer's `[-]` line, Left Arrow hides it, as `-` does; elsewhere, including a hidden answer's `[+]` line, it goes back as before ("no earlier page" on the first page) ([record](docs/ai/2026-10-21-tam-pager-level-line.md)). |
| **0.42.0 command / 0.42.1 corpus** | `+` and `-` show and hide answers | Done (2026-10-22) | Folded answers read `[+] Answer (Press + to show)` and `[-] Answer (Press - to hide)`; `+` and `-` act on the answer on the attention line or selected, and say where to go elsewhere. Enter and Right Arrow still toggle, unmentioned. The help overlay, man page and the Learning paths page teach the new keys ([record](docs/ai/2026-10-21-tam-pager-level-line.md)). |
| **0.41.0 command** | Man page cache (refactoring step 7) | Done (2026-10-22) | groff's HTML for each man page source is cached in `$XDG_CACHE_HOME/tamlinux/tam/man/`, one 0600 file per source behind a stamp of format, size, modification time and path; a changed source is rendered again into the same file. `man bash` revisits drop from about 1.1 s to 55 ms; output is unchanged. Tests isolate `XDG_CACHE_HOME` and `XDG_RUNTIME_DIR` ([record](docs/ai/2026-10-22-tam-refactor-step-2.md)). |
| **0.40.5 command** | `tam.c` and `pager.c` split into modules (refactoring step 6) | Done (2026-10-22) | `tam.c` (1,717 lines) becomes the command line (`tam.c`), Tamlinux and corpus identity (`identity.c`), the offline guide (`corpus.c`) and generated views (`views.c`); `pager.c` (2,524 lines) becomes the run loop (`pager.c`), screen and status row (`pager_draw.c`), keys (`pager_input.c`), link selection and line movement (`pager_links.c`), following, history, folds and lens (`pager_nav.c`), and search and prompts (`pager_search.c`). Function bodies move unchanged; only functions another module uses are exported, through private `corpus.h` and `pager_int.h`. No behaviour change ([record](docs/ai/2026-10-22-tam-refactor-step-2.md)). |
| **0.40.4 command** | One HTML tag scanner (refactoring step 5) | Done (2026-10-22) | New `src/html_scan.c` reads tags and attributes for the renderer, the level filter, the fold transform and the breadcrumb, replacing four scanners with different rules. Attributes are parsed as name=value pairs, so `data-href` or `href=` inside another value no longer becomes a link, and `<abbr>` no longer breaks a breadcrumb. All guide pages at every level, generated pages, 25 man pages and 83 Info manuals render byte-identically ([record](docs/ai/2026-10-22-tam-refactor-step-2.md)). |
| **0.40.3 command** | One address module and loader (refactoring step 4) | Done (2026-10-22) | New `src/address.c` classifies every address (guide, man, Info, tldr, view, action, external), reads its query parameters, and routes it through one table; `tam show`, the pager and the `man`, `tldr` and `info` commands share that loader, replacing four partial loaders and five `tam show` branches. Guide links opened from `tam man`, `tam tldr` or `tam info` now get levels, folds, the level line and the corpus path check; `man:` opens the Man pages view; `--level` reaches every view that takes the lens. Scheme comparisons fall from 76 to 31 ([record](docs/ai/2026-10-22-tam-refactor-step-2.md)). |
| **0.40.2 command** | One HTML escaper and page builder (refactoring step 3) | Done (2026-10-22) | New `src/html.c` holds the only HTML escaper and the page builder that topics, keywords, topic guide, search, man, Info and tldr pages now share; libtam's path, JSON and XDG state helpers replace `tam`'s private copies. Every generated page checked renders byte-identically; Info names and topic summaries are now escaped. About 175 fewer lines ([record](docs/ai/2026-10-22-tam-refactor-step-2.md)). |
| **0.40.1 command** | Subprocesses without a shell (refactoring step 2) | Done (2026-10-22) | Man page lookups, rendering and apropos run through libtam 0.4.0's `tam_exec` with deadlines and output caps; the `popen` shell pipelines and the private runner are gone. gzip man pages decompress in process; groff's temporary files and images go to a private directory under `$XDG_RUNTIME_DIR`, removed after each page, not `/tmp`. libtam links statically (no RUNPATH), and `source-build.json` pins libtam 0.4.0 ([review](docs/ai/2026-10-22-tam-refactor-step-2.md)). |
| **0.40.0 command** | Status row with version | Done (2026-10-22) | The status row reads `<Title> \| L<line> <range>/<total> \| <Level> text 0123 \| h Help  Esc Quit  [] Select links  -> <link>` with `v<command> (<corpus>)` at the right edge; the `< Back`, `` ` Guide ``, `q Quit` and key-list hints are gone (the help overlay keeps them). Narrow rows drop the title, then the link, then the key hints ([record](docs/ai/2026-10-21-tam-pager-level-line.md)). |
| **0.39.2 command** | Top of page is the title | Done (2026-10-22) | `x`, `.`, Home, an empty `#` link and `` ` `` on the Guide index go to the title (`L1`) with the level lines in view, not to the first level line ([record](docs/ai/2026-10-21-tam-pager-level-line.md)). |
| **0.39.1 command** | Line labels and lens selector | Done (2026-10-22) | A page opens on its title as `L1`; the level lines above it are `Lb`, `Lc` and the breadcrumb row `La`, and the range and total count only numbered lines. The status row shows the lens as `All text 0123` with the lens digit underlined (bracketed without underline support) ([record](docs/ai/2026-10-21-tam-pager-level-line.md)). |
| **0.38.0 corpus** | Learning pilot (TCK23 source) | Source done; developer review pending (2026-10-21) | Three laddered pilot pages with captured examples and folded quizzes; goal-path hub, First steps and Keep it healthy linked from Welcome and Learning. Developer review remains unverified; subsequent "Continue" directs TCK24 ([record](docs/ai/2026-10-21-tam-learning-pilot.md)). |
| **0.39.0 corpus** | Learning scaffolding (TCK24 source) | Source implemented (2026-10-21) | 36 can-do skill cells, self-placement, 12 per-level glossary terms including the botanical analogy, bibliography, validated optional kind/skills and teaching references, additive index sidecars and a coverage/gap report. Developer "Continue" directed this slice; pilot/learner review remains unverified ([record](docs/ai/2026-10-21-tam-learning-pilot.md)). |
| **0.40.0 corpus** | Using migration and path format (TCK25 first part) | Source implemented (2026-10-21) | All six Using pages classified, owned-child jobs exercises, sourced reversible desktop/workspace tasks, strict anatomy checks for Using, staged level paths and Command-line fluency. Other parts retain migration warnings; learner and GUI acceptance remain unverified ([record](docs/ai/2026-10-21-tam-learning-pilot.md)). |
| **0.41.0 corpus** | Recovery lessons (TCK25 Maintaining first slice) | Source implemented (2026-10-21) | Rollback and emergency-recovery ladders with captured scratch/read-only examples, folded quizzes, three glossary terms and Intermediate recovery planning in Keep it healthy. Other Maintaining pages and learner/packaged acceptance remain pending ([record](docs/ai/2026-10-21-tam-recovery-learning.md)). |
| **0.42.0 corpus** | Boot, Snapper and Rescue (TCK25 Maintaining second slice) | Source implemented (2026-10-21) | Three migrated ladders with read-only/scratch practice, corrected Snapper type/option semantics and a four-stage Intermediate-to-Expert Rescue path. Remaining Maintaining migration and learner/packaged acceptance stay pending ([record](docs/ai/2026-10-21-tam-boot-rescue-learning.md)). |
| **0.42.3 corpus** | TCK25 logs and power learning ladders | Done (2026-10-22) | Captured read-only journal/sysfs examples, folded quizzes and diagnosis tasks; truthful capability, integrity and energy limits. Command stays 0.43.1; resources, packaging and Maintaining hub remain. |
| **0.42.4 corpus** | Wordsmithed welcome page | Done (2026-10-22) | Wordsmithed welcome page |
| **0.42.5 corpus** | Tightened up where next on welcome page | Done (2026-10-22) | Tightened up where next on welcome page |
| **0.43.0 corpus** | Checkpoint concept guide and glossary entry | Done (2026-10-23) | Authored 'Checkpoints and Rollback Points' (`maintain/checkpoint.html`) and glossary entry (`learn/glossary/checkpoint.html`); explains checkpoint scopes across filesystem, boot, and user settings layers, boundary matching, quiescence, and avoiding churn via recovery anchors. Enables `tam explain checkpoint`. |
| **0.43.1 corpus** | Wordsmithing welcome and about | Done (2026-10-25) | Wordsmithing welcome and about |
| **0.43.2 corpus** | TCK25 Maintaining completion and enforcement | Done (2026-10-25) | Migrated resources and packaging lessons with full teaching anatomy and captured read-only examples; classified Maintaining hub; enabled Maintaining track-wide enforcement in skills registry. |
| **0.43.3 corpus** | TCK25 Improving track migration (first slice) | Done (2026-10-25) | Migrated scripting and lua lessons to full teaching anatomy with captured examples, quizzes and customizing skills; classified Improving hub. |
| **0.43.4 corpus** | TCK25 Improving track migration slice 2 | Done (2026-10-25) | Migrated terminal and efficiency lessons to full teaching anatomy with captured read-only examples, quizzes, and desktop.expert / hardware skills; resolved desktop.expert page gap. |
| **0.43.5 corpus** | TCK25 Improving track migration slice 3 | Done (2026-10-25) | Migrated zram and toolchains lessons to full teaching anatomy with captured read-only examples, quizzes, and hardware / contributing skills; resolved all contributing page gaps. |
| **0.43.6 corpus** | TCK25 Improving track migration slice 4 | Done (2026-10-25) | Migrated ai-performance and patches lessons to full teaching anatomy with captured read-only examples, quizzes, and hardware / customizing skills; alphabetized Glossary. |
| **0.43.7 corpus** | TCK25 Improving track migration slice 5 | Done (2026-10-25) | Migrated package-versions and virtualization lessons to full teaching anatomy with captured read-only examples, quizzes, and software / contributing skills. |
| **0.43.8 corpus** | TCK25 Improving track completion and enforcement | Done (2026-10-25) | Migrated final Improving lesson (tam.html) to full teaching anatomy with captured read-only examples, quizzes, and contributing skills; enabled track-wide build enforcement for Improving in skills registry. |
| **0.44.0 corpus** | Foundational glossary | Done (2026-10-27) | Adds twelve foundational terms, explains and links first-use vocabulary, and links the glossary from the Guide index. |
| **0.44.1 corpus** | TCK25 practice cleanup | Done (2026-10-27) | Removes the known exercise file and empty temporary directory without recursive deletion. |
| **0.45.0 corpus** | TCK25 Learning and Installing | Done (2026-10-27) | Revises login and console lessons, classifies remaining pages, enforces tagged lesson anatomy in all parts, and adds six goal paths. |
| **0.45.1 corpus** | First steps review corrections | Done (2026-10-27) | Restores Welcome links, clarifies terminal access, fixes capstone output order and updates skills-map reading links. Learner review remains pending. |
| **0.45.2 corpus** | Plain guide openings | Done (2026-10-27) | Gives each page a plain orientation sentence and removes the labeled path introductions; developer and learner review remain pending. |
| **0.45.3 corpus** | Use tam hi | Done (2026-10-27) | Uses `tam hi` in corpus instructions and examples. |
| **0.45.4 corpus** | Pilot review corrections | Done (2026-10-27) | Aligns example output with commands, clarifies scratch setup and revises a First steps checkpoint. Learner and developer review remain pending. |
| **0.45.5 corpus** | Beginner guide clarity | Done (2026-10-27) | Explains reader navigation, returning to practice, scratch setup, and introductory shell and filesystem terms. Learner review remains pending. |
| **0.46.0 corpus** | Boot phases | Done (2026-10-28) | Adds a Maintaining page naming the boot phases, separating disk unlock from login and hibernation resume from a fresh boot. Learner and packaged acceptance remain pending. |
| **0.46.1 corpus** | Boot phases topic slug | Done (2026-10-28) | Renames the boot phases topic to `boot-phases`, so `tam explain boot-phases` works and terse topic lists stay one word per line, and drops its `desktop` keyword. |
| **0.46.2 corpus** | More wordsmithing of first steps page | Done (2026-10-28) | More wordsmithing of first steps page |
| **0.46.3 corpus** | Wordsmithing welcome and first steps pages | Done (2026-10-28) | Wordsmithing welcome and first steps pages |
| **0.45.0 command** | Short version option | Done (2026-10-29) | `tam -v` reports the same version information as `tam --version`. |
| **0.44.1 command** | Wordsmithing welcome and first steps pages | Done (2026-10-28) | Wordsmithing welcome and first steps pages |
| **0.46.4 corpus** | Explaining you start in a CLI and can launch a graphical shell | Done (2026-10-28) | Explaining you start in a CLI and can launch a graphical shell |
| **0.47.0 corpus** | Standardize rootstock terminology | Done (2026-10-29) | Standardizes on 'rootstock' across the knowledge corpus for the operating system Tamlinux depends on; explains rootstock, cultivar, third-party software boundary, Arch today, Void migration plan, and microkernel horizon; adds 'distro' glossary entry and renames 'root-stock' to 'rootstock'. |
| **0.48.0 corpus** | Voice dictation and shell voice commands | Done (2026-10-29) | Adds Using pages for desktop voice dictation (F9 / SUPER+CTRL+X) and safe shell voice commands (F10); explains on-device Whisper inference, PipeWire audio flow, and Readline vs wtype injection mechanisms. |
| **0.48.1 corpus** | Update F9 dictation to tap to start and stop | Done (2026-10-29) | Updates voice dictation documentation to reflect F9 tap-to-start and tap-to-stop toggle behavior instead of hold to speak. |
| **0.49.0 corpus** | Fred Horch bio page | Done (2026-10-30) | Adds `learn/about/fred.html` with a short bio of the Tamlinux developer; links from the About page and resolves the existing rootstock glossary link. |
| **0.45.1 command** | Pager italic sitm | Done (2026-10-30) | Emits terminfo `sitm` (CSI 3m) for `<i>` spans when the terminal description provides italic. |
| **0.50.0 corpus** | Terminfo/termcap and ncurses | Done (2026-10-30) | Adds ecosystem tutorials for terminfo/termcap display capabilities and ncurses, with hub cross-links and how `tam` uses terminfo helpers. |

## 4. Deferred direction, without reserved versions

These are ideas or separate dependencies. None is approved or scheduled, and
none has a reserved version:

| Direction | Boundary / prerequisite |
| --- | --- |
| Non-executing shell syntax dissection | A separately reviewed parser/CLI design; all input remains inert. An earlier reservation of command 0.30.0 for it was withdrawn. |
| Installer plan/apply/verify | Product installation/deployment framework and separately approved implementation. An earlier reservation of command 0.31.0 for it was withdrawn. Today read-only `tam inspect` and `tam check` are available; Installing pages must say so. |
| Stateful `tam learn` and automated assessment | Separate opt-in state and assessment design. Guided learning in corpus 0.31.0 does not require it. The learning-levels plan names the candidates: `tam tutor` (scratch-directory lessons that check each task), `tam quiz` (interleaved, spaced review of corpus quizzes, local only) and a level-aware `tam ask`; each needs its own plan. |
| Rationale, deep-system traces and CS learning | Exploratory content/commands, not committed milestones. Existing depth filtering is already implemented. Old proposed 0.26/0.27/0.28 milestones are not future reservations. |
| Wider distribution/terminal-only profiles and stable 1.0 | Follow the product's capabilities and evidence; no promised feature set or version allocation from this roadmap. |
| Non-interactive live-corpus assertions in `tests/run.sh` | On hold by the developer (2026-10-20). About 200 `tam explain`/`tam search` routing and output checks still read `knowledge/`, so a page edit can fail them; `make dev` then names the test. Sorting them into corpus contracts (`tests/guide.sh`) or command tests (fixtures) waits for his decision. |
| Merged full-text search over installed manuals | Separate scalability/search work. Command 0.30.0 already provides explicit Man keyword discovery and Info directory browsing. |

A future scope decision creates or updates an authoritative plan and allocates
versions against then-current files. Do not infer a scheduling commitment from
an old proposed sequence or a version number.

## 5. Supersession and closure

Fred's 2026-10-18 decision replaces the old seven-part contents and
three-track organizational model with the five-part spine. The former future
version sequence and feature-reference matrix are retired as scheduling
authorities; delivered history above remains history. The
[plan index](docs/plans/README.md) records every plan's disposition and links
old designs to their successor at stable paths.

Plans close only with evidence or an explicit replacement/retirement. Update
this roadmap, the development contract and index together whenever scope or
completion changes. No plan update moves Test, Run or Release or changes the
package-only delivery rule.
