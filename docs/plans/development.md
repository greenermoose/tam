# tam development plan

**Plan status:** Active
**Reviewed:** 2026-10-21
**Scope:** Living integration contract; current command 0.40.5 and corpus 0.42.0.
**Authority:** The developer's current decisions; [roadmap](../../ROADMAP.md),
[plan index](README.md), [corpus 0.31.0 work plan](knowledge-system-0.31.0.md),
the [upgrade/stage-switching plan](upgrade.md), [corpus build plan](corpus-build.md),
[developer loop plan](dev-loop.md), [pager quick keys plan](pager-quick-keys.md) and
[learning levels plan](learning-levels.md).
**Next action:** Finish TCK25 Maintaining with logs, power, resources,
packaging and hub migration, then part-wide enforcement. Using, the five
Maintaining recovery/boot ladders and Rescue are implemented;
pilot/learner review remains unverified. See the
[learning levels plan](learning-levels.md). Package Test/Run/Release remain
developer-directed stages.
Disposable acceptance builds and live metadata checks have standing developer
authorization and need no per-run confirmation.

**Stage:** Develop. Milestones 1–16, the 0.30.0/0.31.0 knowledge overhaul and
the 0.31.0/0.32.0 upgrade workflow have landed in source. Package assembly and deployment remain separate,
developer-directed work. Maintain this living contract using the
[plan maintenance procedure](../plan-maintenance.md); closed plans are historical.

## Purpose and ownership

Give users one useful entry point for installing, learning, using, maintaining
and improving Tamlinux,
including offline and before the graphical desktop is running.

This repository owns the C command, bundled knowledge, tests and component
versioning. `libtam` owns the reusable C library. The `tamlinux` product owns
workstation assembly, installed cultivar metadata and the installation
framework. Existing component tools retain their owners; document or call them
through deliberate subcommand contracts rather than moving every `tam-*`
helper into this repository.

The delivered journey spine has five parts — **Installing, Learning, Using,
Maintaining, Improving**. Topic lookup/search and Guide/Man/Info reference access
are shared utilities; Beginner, Intermediate and Expert are independent reading
levels. Learning paths reuse pages across all parts. Every page has one
canonical logical home and may be linked contextually elsewhere. Corpus 0.31.0
declares five track values and explicit parent paths; older corpora may omit
parent metadata and retain directory-based hierarchy. Command 0.30.0 supplies
common Guide/Back/Help/Quit actions and in-reader Guide/Man discovery. The
[knowledge-system plan](knowledge-system-0.31.0.md) records the delivered
contracts and verification.

The 2026-10-12 requirements describe historical milestones 6–9; later decisions
below record their evolution. Rationale/depth commands, stateful learning and
installer execution remain deferred unless the developer assigns new work.

## Repository layout and workstream lanes

The repository has two independently developable workstreams:

- **Command (`src/`, `bin/`, `completions/`, `man/`)** — the C program that
  renders the corpus and links `libtam`, with its manual page.
- **Corpus (`knowledge/`, `tools/`, `tests/test_lint_corpus.py`)** — the authored
  semantic HTML, index generation and corpus checks. Corpus pages have no snapshots.

They are separate at the source and runtime levels: the binary loads its
corpus from `TAM_KNOWLEDGE_DIR` or the installed `DATADIR`, so a corpus edit
needs no recompile and a source edit needs no corpus change. Two agents may
edit the two lanes at once, each claiming only its lane's paths with
`tam-work` ([`AGENTS.md`](../../AGENTS.md)). Shared integration files (`Makefile`,
`tests/run.sh`, `tests/guide.sh`, `AI_PROVENANCE.md`, `README.md`,
`ROADMAP.md`, `docs/`) move by handoff, never concurrently. An agent working
on something that tightly couples the two — for example adding parser support
for a construct and then using it in the corpus — claims both lanes.

The lanes stay coupled at the test level. `tests/guide.sh` renders corpus
pages through the built binary, and `tests/run.sh` asserts corpus content
through `search` and `explain`, which read the generated index.

Build targets follow the lanes:

- `make` (or `make all`) — the whole project: the binary and the corpus index.
- `make command` — just the C binary; does not regenerate the corpus index.
- `make corpus` — always recheck all pages and Git metadata; regenerate the
  three corpus indexes and checkout build marker, preserving unchanged files.
- `make test-command` — command checks.
- `make corpus-and-test` — build the corpus and reader, then run corpus checks.
  This replaces `test-corpus` without an alias. No installation or stage promotion.
- `make test` / `make check` — the full, integrated suite.

## Command contract

| Invocation | Status | Behavior |
| --- | --- | --- |
| `tam` | Implemented | Concise introduction and starting points; reports command version and corpus version; exit 0 without launching a pager or writing state. |
| `tam --help` | Implemented | Concise usage and implemented commands; reports command version and corpus version; works without guide data or cultivar metadata. `tam help` is the same invocation. |
| `tam --version` | Implemented | Reports command version, bundled corpus version, libtam version, and installed Tamlinux cultivar version; report unavailable cultivar metadata explicitly. |
| `tam show [PAGE]` | Implemented | Open the local guide in the built-in pager when interactive (e.g. `tam show tam` opens `knowledge/tam.html`); readable text when piped or not a terminal. Preserves path-based semantics and navigates directly to a page. |
| `tam welcome` | Implemented | Explicit orientation; record welcome only after successful display. |
| `tam hello`, `tam hi` | Implemented | Friendly orientation shortcuts; exact aliases for `tam welcome`. |
| `tam explain` | Implemented | Always launch the built-in pager displaying the dynamic list of all topic keywords from corpus metadata (or clean text when piped). |
| `tam explain <topic>` | Implemented | Always display a dynamic topic guide for `<topic>` in the built-in pager (or clean text when piped), listing all matching pages with each page explained. Never takes you directly to a page. |
| `tam explain topics` | Implemented | Interactive hyperlinked topic browser in pager (or clean text when piped); supports `--level LEVEL`, `--terse` (slugs only for pipes/scripts), and `--verbose` (exhaustive keywords/summaries). |
| `tam inspect` | Implemented | Read-only host facts, capability detection, and system health findings (the old `tam install inspect` form is retired). Supports `--json`, `--check [ID]`. |
| `tam check [ID]` | Implemented | Structured offline Check Contract evaluation against system diagnostic probes (the old `tam install check` form is retired). Supports `--json`, `--terse`. |
| `tam man [SECTION] PAGE` | Implemented | Bare TTY use opens the manual discovery landing; named pages, sections, keyword lookup and section browsing route through the reader. Pipe mode prints landing guidance without prompting. |
| `tam info [MANUAL [NODE]]` | Implemented | System GNU Info manual, converted from Info format to semantic HTML with hyperlinked node hierarchy, menus, and cross-references; opens top-level dir or specific manual/node in pager. (Milestone 14, tam 0.22.0). |
| `tam install` `plan`/`apply`/`verify` | Not implemented | Unimplemented installer verbs report truthfully and exit non-zero; they follow the product installation framework. |
| `tam search [--json] TERM...` | Implemented | Single search verb over the corpus index. Returns a ranked list of pages; `--json` for scripting. |
| `tam ask <question>` | Implemented | Opt-in, grounded question answering over the corpus with mandatory citations; never starts a daemon. |
| `tam upgrade` / `tam switch STAGE` | Implemented in source; real source-build acceptance pending | Discover Development/Test/Run/Release, compare independent versions, build selected source into isolated development-prefix generations, pin releases and recover. [TCK6–TCK8](upgrade.md) records implementation and remaining acceptance. |

`--pipe` explicitly requests text. `--terse` provides minimal machine tokens for piping situations; `--verbose` provides exhaustive details and diagnostics; default output adheres to the "Goldilocks" screen budget (≤24 lines). Redirected output contains no terminal
controls or browser prompts. Bare `tam`, `show`, `search`, help, version and
installation inspection never write welcome state. Successful explicit
`welcome` uses `$XDG_STATE_HOME/tamlinux/welcome_seen`, falling back to
`~/.local/state/tamlinux/welcome_seen`.

## Offline identity and the `ask` boundary

`tam` is offline by default: it parses and displays its bundled corpus with no
network access, no daemon and no mandatory external dependency, and it emits no
terminal controls when piped. Milestone 9 adds one deliberate, opt-in exception.
`tam ask` is grounded in the corpus and cited, talks only to an
already-running local inference server, and never starts, bundles or embeds a
daemon or model. It reaches the network only after interactive consent that
discloses the URL, size and expected hash (verified on download). Under `--pipe`
or any non-interactive invocation it errors instead of fetching. The core binary
stays offline and dependency-free; the optional model runtime remains external
and user-controlled. This boundary is recorded here deliberately, as the one
place where the implemented offline default is amended. Explicit upgrade
subcommands perform fixed-host GitHub metadata/source operations; browsing,
help and version commands remain offline.

## Implementation and packaging

Use C with `libtam`, following the selected Tamlinux language direction.
Keep the library separate; record its source revision and supported version
when introducing the dependency. Begin with a small Makefile and documented
development dependency paths. Builds must not modify a sibling library
checkout as a hidden side effect.

Ship semantic HTML with corpus-root-relative local links; readable text is
derived at runtime from the same source. Corpus pages have no golden snapshots.
Renderer fixtures do. Verify component behavior before documenting it.

Author topic metadata in the pages themselves and project it into a small
corpus index artifact at build time: each page declares a title, a one-line
summary and its keywords, plus level, track and prerequisites. These fields
also drive reader-visible filtering and navigation. Corpus 0.31.0 requires
explicit logical parents for authored pages, keeps the Guide root track empty,
and validates parent reachability, cycles, part-track ownership and breadcrumb
agreement. The sidecar hierarchy projection preserves the established TSV
columns and lets older command readers ignore the additive data. Three-track
corpora without parent metadata retain their prior directory fallback.
The index is the single lookup authority for `search` and `explain`; it replaces
the hand-written keyword table in `src/tam.c` and keeps the existing aliases and
the explain catalog as projections of that one source. The index must load once
and hold the existing startup budget.

Resolve knowledge and cultivar metadata through packaging configuration,
with documented development overrides. Do not assume one fixed `/usr/share`
path or a sibling source checkout at runtime. Support staged installation
with `PREFIX` and `DESTDIR`; workstation integration pins the command and
library separately. The package and XDG namespace stay `tamlinux` even
though the executable and source repository are named `tam`.

The guide is self-contained. Interactive browsing uses the built-in terminal
pager directly; `tam` parses and displays the bundled HTML natively without
requiring an external browser or pager. Local guides open without fetching
network content or requiring external tools.

Read-only self-inspection runs fixed external tools, never user commands, inside
a bounded envelope: `LC_ALL=C`, pager and colour neutralised, a short timeout,
captured standard streams and never an attached terminal; structured output is
preferred and parseable text is read under a stable contract. Findings carry a
severity, the evidence that produced them and a link into corpus help.
Privileged reads use `pkexec` only when it is present, with absolute paths, and
otherwise degrade to the non-privileged subset with a truthful warning; never an
interactive `sudo`. The opt-in `ask` surface talks to an already-running local
server and bundles no model or daemon.

## Versioning

The command version lives in `src/version.h` (`TAM_COMMAND_VERSION`) and is
reported by `--version`. Follow semantic versioning and **bump the version in
the same change that adds a feature**:

- **Minor** (`0.y.0` → `0.y+1.0`): any new feature or user-visible capability
  (a new subcommand or output mode). Corpus content versions independently.
  While the major is `0`, new features take the minor bump.
- **Patch** (`0.y.z` → `0.y.z+1`): backwards-compatible fixes and internal
  changes with no new capability.
- **Major**: reserved; not used before `1.0.0`.

The developer clarified on 2026-10-18: every newly prepared code build for him
must receive a version bump; changed code must not be delivered under a reused
version. Command and corpus version independently. Stage labels express
readiness rather than a numbering scheme. Compiling/installing an existing
numbered source commit preserves its versions, including when switching back
to a release. Full commits identify candidates independently of version text.
Release tags record both component versions (for example,
`tam-0.31.0-corpus-0.32.1`), so a corpus-only release has an exact identity.

Update the `--version` assertion in `tests/run.sh` to match, and note the
bump in the decision record below. Historical `docs/ai/` records keep the
version that shipped at the time; do not rewrite them.

The corpus version lives only in `knowledge/version` and follows the same
rules for corpus content: new pages or chapters take the minor bump,
corrections take the patch bump. A corpus-only release changes nothing in the
command lane: `tests/run.sh` reads the expected corpus version from
`knowledge/version`, `tests/guide.sh` discovers every page under
`knowledge/`, and `TAM_DEFAULT_CORPUS_VERSION` in `src/version.h` is only
the `unversioned` label shown for a corpus that has no version file.

## Historical delivered milestones

- **2026-10-19:** Command 0.31.2 and corpus 0.32.4. The renderer wraps the
  widest columns of a table that is wider than the terminal, never below a
  column's longest word or 12 cells, and stacks rows as `Header: value`
  records only when the narrowed columns still cannot fit. Topic lists are
  titled Topics A to Z and Keyword Index. The corpus drops topic keywords
  that misdirect to a page that does not cover them or mean nothing as a
  lookup. Corpus 0.32.3 (`b7a838f`) revised the welcome page.
- **2026-10-19:** Corpus 0.32.2, TCK9 (reliable corpus builds). `make corpus`
  always rechecks pages and Git metadata, preserving unchanged outputs.
  `make corpus-and-test` rebuilds before checks and replaces `test-corpus`.
  Focused, parallel and integrated verification passed; see the
  [corpus build plan](corpus-build.md). Command 0.31.1 is unchanged.

These entries record the behavior and vocabulary at delivery time. For current
commands use the contract above; for new work use the overhaul. Original plans
are retained with explicit dispositions in the plan index. Historical text
about snapshots, aliases or deployment is not a current operating procedure.

1. **Command foundation — done (2026-10-09).** Bare introduction, help,
   version and argument errors are implemented; the build links an installed
   `libtam`.
   `--version` reads the installed cultivar version from
   `$XDG_CONFIG_HOME/tamlinux/version` (falling back to
   `~/.config/tamlinux/version`), then `$TAMLINUX_VERSION`, and reports
   unavailable metadata explicitly. Deferred within this milestone: a
   `libtam.pc` pkg-config file and a versioned `libtam` SONAME.
2. **Offline guide — done (2026-10-09).** Corpus lookup, `browse`, `--pipe`,
   a native HTML-subset renderer (`src/render.c`) and an interactive pager
   (`src/pager.c`) are implemented; a relocated staged installation is
   verified. See the [offline-guide plan](offline-guide.md). The native
   renderer and pager handle display without requiring an external browser.
3. **Orientation and explanation — done (2026-10-09).** Added explicit
   `welcome` (`tam welcome`), successful-display state (`welcome_seen`), and
   curated `explain` topics (`tam explain <topic>`). See the
   [orientation and explanation plan](orientation-and-explanation.md).
4. **Installation inspection — done (2026-10-09).** Connected read-only
   `install inspect` to the product contract and implemented host facts,
   package tool detection, and recognized installation capabilities in C.
   See the [installation inspection plan](installation-inspection.md).
   Later installer plan/apply/verify operations remain deliberately unimplemented.
5. **Engine enhancements, table layout, and dynamic input — done (2026-10-09).**
   Standard input reading (`tam show -`, `tam explain -`), definition lists
   (`<dl>`, `<dt>`, `<dd>`) and space-padded table layout (`<table>`, `<tr>`,
   `<td>`, `<th>`) landed in `src/render.c`, together with the "Use"
   track pages (tam 0.8.0). A native manual-page subsystem (`src/man.c`) added
   `tam man` and hyperlinked cross-reference traversal (tam 0.9.0); 0.9.1 moved
   development installs under `~/.local/dev` and preferred the binary's own
   corpus.
6. **Corpus index and search — done (2026-10-13).** Replaced the hand-written
   keyword table (`src/tam.c`) with a corpus index generated from page metadata
   (`<meta name="keywords">`, `<meta name="summary">`, `<meta name="level">`,
   `<meta name="track">`, `<title>`, `<h1>`) projected into `knowledge/index.tsv`
   and `knowledge/index.json` via `tools/build-index.py`. Implemented `tam search
   [--json] TERM...` as the single search verb: retrieval is alias → field-weighted
   word score → substring/prefix, with a bounded, shown "did you mean" suggestion
   and a miss banner that lists top candidates with reopen commands and offers
   `tam man <query>` when appropriate. Fuzzy matches are suggestions, never an
   auto-redirect. `tam explain` keeps single-topic semantics and resolves
   multi-word queries through search. Bumper version to 0.10.0.
7. **Content-depth ladder and fading templates — done (2026-10-14).** Added
   Beginner (one worked, annotated example with expected output),
   Intermediate (example plus a completion task) and Expert (terse reference with
   links to `man`) fading template sections across core curated topics
   (`shell/jobs.html`, `shell/pipelines.html`, `filesystem/permissions.html`,
   `hardware/resources.html`). Implemented `src/depth.c` and `src/depth.h` for
   dynamic section extraction and in-page depth switching (`?level=NAME`);
   added `tam explain --level LEVEL <topic>` and catalog depth filtering;
   extended `tools/build-index.py` and `src/index.c` with level lists, track,
   and prerequisites; updated `tam search` and `--json` with depth indicators;
   added Bash completions for `--level`. Version bumped to 0.15.0.
8. **Read-only self-inspection findings and check contract — done (2026-10-14).** Grew
   `tam install inspect` into a read-only findings layer: structured finding
   model (`id`, `severity` in info/warn/error/critical, `evidence`, `help_link`,
   `summary`) and a versioned offline Check Contract
   (`{task_id, name, status, pass, fail, evidence, severity, help_ref, source, schema_version}`)
   that the deferred progression system can use as its assessment oracle. Implemented
   the Safe Execution Envelope (`LC_ALL=C`, bounded timeouts ≤500ms, captured
   standard streams, detached TTY, neutralized color) with a fixed whitelist of
   read-only system probes (`findmnt`, `systemctl`, `udevadm`, `pkexec`), and pure C
   POSIX primitives (`statvfs`, `uname`). Implemented `pkexec` detect-and-degrade
   rule (never assume privilege, never spawn interactive `sudo`, truthful warning finding
   when `pkexec` is absent). Added `tam install inspect --json`, `tam install inspect --check [ID]`,
   and `tam install check [ID]` (`--json` supported) subcommands. Findings deep-link
   into Maintain-track help (`maintain/rollback.html`, `maintain/recovery.html`,
   `maintain/logs.html`, `maintain/power.html`, `maintain/updates.html`). Updated
   Bash auto-completions and test suite. Version bumped to `0.16.0`.
9. **Opt-in grounded `ask` — done (2026-10-14).** Added `tam ask <question>`: retrieval
   over the corpus index with mandatory citations, fallback to `tam search` when no
   model is available, and the offline-by-default boundary described above. Bounded context
   (≤4 KB) with top-k retrieved corpus pages. Non-interactive invocations (`--pipe`,
   redirected stdout) missing server/model exit immediately with code 2 and exact refusal message;
   interactive invocations prompt with full disclosure before model fetch. Added `tam ask --json`
   emitting structured output. Added bash completions for `tam ask`. Version bumped to `0.17.0`.
10. **Navigation & Output Ergonomics (Quick Guide Index Return, Goldilocks Screen Budget, and Output Modifiers) — done (2026-10-14).**
    - **Quick Guide index navigation (`\``):** Bind backtick (`` ` ``) in the interactive pager (`src/pager.c`) to immediately return to the root Guide index (`index.html`) from any corpus page, subpage, or manual page, pushing the current exploration position to the history stack before jumping to allow easy return with `u`.
    - **Goldilocks Screen Budget & `tam explain topics`:** Enforce the rule that usage guides and command overviews must not spew so much detail that critical orientation scrolls off an 80×24 screen. Refactor bare `tam explain` to emit strictly the usage synopsis and navigation pointers (≤15 lines) with an explicit notice that a special hyperlinked `tam explain topics` page is available, avoiding the 29-topic catalog dump.
    - **Interactive Hyperlinked Topics Pager:** In a terminal, `tam explain topics` launches inside `tam`'s built-in interactive pager, displaying available topics as live hyperlinks so users can easily select and navigate directly to any explanation. Under `--pipe` or non-TTY, it outputs clean formatted text.
    - **Piping Modifiers (`--terse` and `--verbose`):** Offer `--terse` for minimal machine-friendly tokens suited for shell pipelines and scripts (`tam explain topics --terse` emits one topic slug per line, ideal for `fzf`, `grep`, and `while read`; compact check statuses) and `--verbose` for exhaustive diagnostics and keyword arrays. Condense `tam --help` to ≤20 lines by consolidating redundant per-subcommand flag variants. Default outputs remain "Goldilocks" (human-friendly and bounded).
11. **Maintain track knowledge corpus expansion — done (2026-10-14).** Authored comprehensive "Maintain" track knowledge pages providing foundational system health and reliability documentation: `maintain/index.html` (hub page), `maintain/updates.html` (non-disruptive updates, inode unlinking, 3-tier package architecture: OS base vs Tamlinux packages vs Mise user tools), `maintain/rollback.html` (Btrfs Copy-on-Write snapshots, Home Manager generation rollbacks), `maintain/logs.html` (truthful system diagnostics, filtering `journalctl` by boot, unit, and priority, kernel ring buffer `dmesg -T`), `maintain/power.html` (3-stage power progression: DPMS monitor blanking, RAM suspend, zero-watt S4 disk hibernation, and safe display wake via `Shift` key), and `maintain/recovery.html` (emergency disaster recovery: virtual consoles `Ctrl+Alt+F2`–`F6`, Magic SysRq `REISUB` sequence, live USB chroot repair). Applied Beginner/Intermediate/Expert fading templates across core topics; integrated into depth ladder and `tam explain topics`; updated `tam_index_lookup_alias` in `src/index.c` with two-pass resolution (exact topic match taking precedence over keyword aliases); refreshed test suite and golden text snapshots. Version bumped to `0.19.0`.
12. **User Guide Table of Contents, Botanical Analogy, and tam(1) Manual Page — done (2026-10-14).** Re-architected the root offline User Guide (`knowledge/index.html`) into a comprehensive 7-part Overarching Table of Contents retitled to "The Tamlinux User Guide" and organized around Fred's core architectural triad: Using, Maintaining, and Improving your workstation (incorporating the full scope of planned user-facing chapters from Plan 26 and Omarchy manual). Authored `knowledge/ecosystem/terminology.html` introducing Tamlinux's core conceptual framework analogy (Greenhouse = hardware resources, Pot = boot loader or VM, Nut = minimal bootable system image / UKI, Root-stock = base Linux distro, Cultivar = Tamlinux desktop shell and application suite) with full Beginner/Intermediate/Expert fading templates. Authored the standard Section 1 manual page `man/tam.1` covering all subcommands, pager keyboard controls, output modifiers, environment variables, files, and cross-references. Updated `Makefile` to install `man/tam.1` into `$(MANDIR)/man1/tam.1`. Enhanced `src/man.c` to locate in-tree and installed development manual pages. Extended `cmd_show()` in `src/tam.c` to route `topics`, `tam:topics`, and `man:*` directly through the document loaders for seamless interactive and `--pipe` navigation. Bumped version to `0.20.0`.
13. **"Improve" Track Knowledge Corpus Expansion — done (2026-10-14).** Authored comprehensive "Improve" track knowledge pages empowering users and autonomous AI agents to optimize, script, and hack their Tamlinux workstations: `knowledge/improve/index.html` (hub page), `knowledge/improve/efficiency.html` (zero-polling architecture, inotify, epoll multiplexing, Wayland compositor event sockets), `knowledge/improve/zram.html` (memory compression with ZRAM, zsmalloc, LZ4 vs zstd tradeoffs), `knowledge/improve/terminal.html` (sub-10ms shell startup latency, Micro editor live disk reload Virtue V11, foot Wayland terminal architecture), `knowledge/improve/scripting.html` (defensive Bash headers `set -euo pipefail`, safe argument `"$@"` forwarding, `tam-*` vs `fred-*` namespace separation Decision D131), `knowledge/improve/toolchains.html` (native C compilation with GCC/Clang and libtam, `-O2` and `-march=native` optimization flags, isolated Git worktrees, `tam-work` scope reservations), and `knowledge/improve/ai-performance.html` (physics of local autoregressive token generation, memory bandwidth bounds, quantization formats Q4_K_M vs FP16, KV cache memory scaling). Applied Beginner/Intermediate/Expert fading templates across core subtopics; updated User Guide Part V links; updated `tests/guide.sh` and `tests/run.sh`; regenerated corpus index (43 entries) and golden snapshots; installed development build under `~/.local/dev`. Bumped version to `0.21.0`.
14. **Native GNU Info Subsystem (`tam info`) — done (2026-10-15).** Added GNU Info documentation as an authoritative, first-class documentation layer alongside system manual pages (`tam man`) and the curated guide (`tam show`). Implemented `tam info` to browse the system Info directory (`/usr/share/info/dir`) and `tam info [MANUAL [NODE]]` to navigate specific GNU manuals (Bash language specification, Coreutils, GCC, Make, GDB, GNU C Library 20-volume guide). Implemented `src/info.c` and `src/info.h` to locate `.info` and `.info.gz` files along `INFOPATH`, parse node headers (`\x1F`, `Node`, `Next`, `Prev`, `Up`), read tag tables for $O(1)$ random-access seeking, convert menus into hyperlinked lists, and rewrite cross-references (`*Note ...`) and manual links (`man:`) into live hyperlinks. Added virtual URI routing `info:MANUAL[/NODE]` in `corpus_load` for seamless cross-system navigation. Added dedicated node navigation keys in `src/pager.c` (`n` next, `p` prev, `u` up, `t` top, `i` info dir). Provided lightweight `bin/tam-info` wrapper script and Bash shell completions. Version bumped to `0.22.0`.
15. **Topic Guides, Dynamic Keywords, & tam Architecture — done (2026-10-15).** Redesigned `tam explain` to display dynamic topic guides rather than navigating directly to documentation pages (`tam show` remains the direct page navigation tool; `tam 0.23.0`). Invariant: `tam explain` always displays a topic guide in the built-in pager (or clean formatted plain text with link references under `--pipe`/non-TTY); it never opens a page directly. Topic guides are formatted lists of pages matching a topic or keyword, with each page explained via its metadata summary. Dynamically generated the list of topic keywords from `<meta name="keywords">` and `<meta name="topic">` tags across the corpus; typing bare `tam explain` launches the built-in pager with the full list of topic keywords to page through. Authored dedicated `knowledge/tam.html` page (`tam show tam`) detailing the C implementation, all Makefile targets (`all`, `command`, `corpus`, `test`, `test-command`, `test-corpus`, `check`, `install`, `uninstall`, `clean`, `FORCE`), libraries and compiler chain (`libtam`, `zlib`, `CC`, `CFLAGS`, `CPPFLAGS`, `LDFLAGS`, `RPATH`), and the complete Develop -> Test -> Run -> Release lifecycle via `tamlinux-packages` and Nix/Home Manager. Bumped version to `0.23.0`.
16. **Decoupled Versioning & Knowledge Corpus Alignment — done (2026-10-15).** Decoupled the version numbers of the `tam` command binary (`src/version.h`, `TAM_COMMAND_VERSION` = 0.24.0) and the offline knowledge corpus (`knowledge/version` = 0.24.0). Implemented dynamic discovery in `src/tam.c` (`tam_corpus_version`, `tam_corpus_build_id`) loading version and build metadata from the knowledge root. Bare `tam` and `tam --help` report command version alongside corpus version; `tam --version` reports command, corpus, libtam, and cultivar metadata. Added `tam hello` and `tam hi` as top-level orientation aliases. Reviewed and aligned all corpus pages with top-level `tam inspect` and `tam check` (updating `maintain/index.html`, `maintain/updates.html`, `maintain/rollback.html`, `maintain/power.html`, `maintain/recovery.html`, `install/index.html`, `hardware/resources.html`, `welcome.html`, `index.html`, `tam.html`). Enriched ecosystem documentation with the botanical Fruit application layer (`knowledge/ecosystem/terminology.html`) and command varieties (`*-nut`, `*-root`, `*`), Fred's 6 Guiding Principles (`knowledge/ecosystem/philosophy.html`), and user alias guidelines (`knowledge/shell/index.html`). Version bumped to `0.24.0`.

Milestones 6–16 derive from the 2026-10-12 `tam` help and self-inspection
requirements and Fred's 2026-10-15 topic guides, command architecture, and decoupled versioning guidance.
The reasoning still belongs in the private decision ledger; this
plan is the public projection and is revised when those decisions are recorded.

The first slice developed independently of desktop migration. Later installer
plan/apply/verify work follows the product framework.

## Current implementation and deferred work

The five-slice [knowledge-system overhaul](knowledge-system-0.31.0.md) delivered
command 0.30.0 and corpus 0.31.0. The [pager quick keys plan](pager-quick-keys.md)
delivered command 0.34.0 and corpus 0.35.1 (TCK15–TCK19): pager link keys `]`
and `[`, search-only `N`, status row `L<cursor> <first>-<last>/<total>`, `=`
address display (`Path = <address>`), `page:` addresses in `tam show`, `\` guide
search, labelled search prompts, `tam:help` and prompt action links, and the
Guide index Quick keys table. Full test evidence and remaining package review
limits are recorded in those plans.

The [learning levels plan](learning-levels.md) is in progress. TCK20 delivered
the teaching contract, [`docs/corpus-pedagogy.md`](../corpus-pedagogy.md), with
page templates in `docs/templates/`. TCK21 delivered the sticky level lens,
status row and level bar (command 0.37.0; banner ranking corrected in 0.37.1).
TCK22 delivers pager folds and self-quiz keys (Enter, `r`, `*`), section-end
pipe answers, block-level filtering, generated section labels, asides and
risk labels (command 0.38.0 / corpus 0.37.1). Fold state stays on the current
page, persists through lens changes and resets on page navigation. Section
labels alone identify levels in All; there is no extra gutter. Lint checks
reject manual level prefixes, incomplete quizzes and unlabeled privileged
examples. TCK23 source delivers the three pilot pages, goal-path hub, First
steps and Keep it healthy, with captured examples, folded quizzes and a
read-only maintenance capstone (corpus 0.38.0). After the review gate was
reported, the developer directed "Continue". TCK24 adds 36 can-do skill cells,
self-placement, 12 laddered glossary terms, an annotated bibliography,
validated kind/skills and teaching references, additive sidecars and a
coverage/gap report (corpus 0.39.0). Legacy pages remain valid; anatomy and
quiz gaps warn during migration. Pilot/learner review remains unverified.

TCK25's Using part is implemented as corpus 0.40.0. All six logical pages
declare their job, instructional pages map skill cells, and anatomy checks
are fatal for completed Using while other parts still warn. Jobs examples
target only owned disposable children; desktop tasks describe sourced defaults
and explicit undo without claiming GUI acceptance. The three level paths now
use stages, checkpoints and capstones; Command-line fluency is a new goal path.
Goal-path skills can span visited levels while page skills remain constrained
to declared levels. Pilot/learner and packaged acceptance remain unverified.

Corpus 0.41.0 migrates rollback and emergency recovery to the teaching
contract: captured scratch/read-only examples, folded quizzes and mapped
skills, plus Snapshot, Generation and Virtual console glossary pages.
Keep it healthy uses the Intermediate recovery-planning lesson. The lessons
separate product return, mutable data and native boot state, and teach SysRq
limits without invoking it. Maintaining remains partially migrated;
part-wide enforcement stays off. See the [verification record](../ai/2026-10-21-tam-recovery-learning.md).

Corpus 0.42.0 migrates boot, boot-fallback (`maintain/boot-recovery.html`)
and Snapper, with captured read-only/scratch exercises and folded quizzes.
It corrects Snapper types, comparison endpoints and option semantics and
separates checkpoint creation, menu generation, boot and restoration. Rescue
is a four-stage Intermediate-to-Expert goal path with a scratch decision
capstone. Logs, power, resources, packaging and hub migration remain before
Maintaining enforcement. See the [verification record](../ai/2026-10-21-tam-boot-rescue-learning.md).

Deferred work has no reserved command version: installer plan/apply/verify,
stateful `tam learn`, compound shell parsing, new `why`/depth/trace commands,
expanded CS content, AI hardware estimates, writable settings, localisation and
wider distribution profiles. See the roadmap for dependencies. Fuzzy
matches remain suggestions; external viewers remain unnecessary. TTY/VT,
boot-rescue and Snapper pages have already landed (corpus 0.27.0, 0.29.0 and
0.30.0 respectively); they are not future page requests.

Package delivery and Test/Run/Release follow developer-controlled stages.
Physical Linux-console/Foot acceptance left open by the pager portability
record remains explicit in that plan and the index; it is not silently closed.

## Acceptance evidence

- Bare invocation exits 0, prints useful starting points and never launches
  the pager or creates state, including with terminal input/output.
- Piped output and `--pipe` are readable and contain no control sequences.
- The built-in renderer needs no external viewer; a missing corpus or cultivar
  metadata produces useful, truthful diagnostics and help remains usable.
- Curated explanation handles misses and shell-looking strings as data.
- A staged package runs away from the source tree; relative guide links work
  offline. Inspect changes no configuration or installation state.
- Compile with warnings enabled and run focused command/integration tests
  and sanitizer checks. Measure startup, binary size and memory before
  describing performance targets as achieved.
- `tam search "change file owner"` reaches the permissions page from a
  canonical query set; a miss prints the bounded candidate banner and exits 1;
  a fuzzy match is shown, never auto-opened. The corpus index and the explain
  catalog derive from one source and stay consistent, and the existing aliases
  remain covered.
- `tam explain` resolves a curated topic and treats shell-looking input as data;
  multi-word queries resolve through search.
- Self-inspection emits machine-readable findings, never executes user
  commands, and degrades truthfully with a warning when `pkexec` is absent.
- `tam ask` cites corpus pages when a local model answers, refuses with the
  documented message under `--pipe`, and never fetches or starts a server
  without interactive consent.

## Decision record

- **2026-10-22:** Command 0.40.5 (patch): refactoring step 6 (private D188),
  a move with no behaviour change. Source layout: `tam.c` command line and
  subcommands; `identity.c` cultivar and corpus version; `corpus.c` corpus
  discovery, page resolution, page reading and related pages; `views.c`
  topic guide, keyword index and topics; `pager.c` run loop and globals;
  `pager_draw.c` screen, level and status rows; `pager_input.c` key and
  terminal-reply reading; `pager_links.c` link selection, breadcrumb focus
  and line movement; `pager_nav.c` following, history, anchors, folds and
  the lens; `pager_search.c` search and prompts. Function bodies moved
  unchanged by a script that cut at top-level function boundaries; only the
  5 command and 34 pager functions another module calls are exported, from
  the private headers `corpus.h` and `pager_int.h`; everything else stays
  `static`. Output matches 0.40.4 byte for byte and every PTY test passes.
- **2026-10-22:** Command 0.40.4 (patch): refactoring step 5 (private D188).
  `src/html_scan.c` is the one tag scanner: `tam_tag_read()` (quote-aware,
  comments as a unit, lower-case names in which a hyphen continues the name,
  a `complete` flag), `tam_tag_is()` and `tam_html_attr()`/`tam_tag_attr()`,
  which parse attributes as name=value pairs. The renderer's parse loop,
  attribute reader and breadcrumb extraction, `depth.c`'s helpers and
  `fold.c`'s scanner all use it; each caller keeps its own handling of an
  unterminated tag (the renderer shows a lone `<` as text). Fixed: an
  attribute named inside another value (`title="href=x"`) or with a longer
  name (`data-href`) was read as `href`, and `<abbr>` in a breadcrumb
  matched `<a`. Line count is about unchanged; four sets of tag rules
  become one. All 108 guide pages at four levels, 25 fixture and golden
  pages, 36 generated pages, 25 man pages and 83 Info manuals are
  byte-identical to 0.40.3.
- **2026-10-22:** Command 0.40.3 (patch): refactoring step 4 (private D188).
  `src/address.c` owns addresses: `tam_addr_kind()` (guide, man, Info, tldr,
  view, action, external, fragment, standard input), `tam_addr_takes_lens()`,
  `tam_addr_param()` (percent-decoded query values) and `tam_address_load()`,
  one route table for every scheme. `tam show`, the pager and `tam man`,
  `tam tldr` and `tam info` all use it; their three private loaders and
  `tam show`'s five scheme branches are gone, and `src/views.h` declares the
  corpus views and `tam_corpus_page_document()`. Fixed by sharing: guide
  links opened from the man, tldr and info commands were read raw (no
  levels, folds or level line) and joined without the corpus path check;
  `man:` (the man breadcrumb's link) failed under `tam show`; `--level`
  reached only `tam:topics`. Unknown schemes are treated as external links.
  Every generated page compared is byte-identical to 0.40.2.
- **2026-10-22:** Command 0.40.2 (patch): refactoring step 3 (private D188).
  `src/html.c` owns HTML escaping (`tam_html_escape`, `tam_html_cat`,
  escaping `& < > " '`) and the page builder (`tam_page_begin` with title,
  optional head links and breadcrumbs; `tam_page_end`; `tam_page_render`),
  used by every generated page; the six private escapers and hand-written
  framings are gone. libtam's `tam_path_join`, `tam_path_join_under`,
  `tam_xdg_path` and `tam_str_cat_json` replace `join_path`, `safe_join`,
  `tam_state_path`/`ensure_dir_exists` and `ask.c`'s JSON escaper. Corpus
  page paths still refuse URLs; `..` is now refused as a path component
  rather than anywhere in the name. A new state directory is created 0700.
  `install.c` keeps its bounded `json_escape_into`, which fits fixed-size
  evidence fields without splitting an escape. The Info navigation bar and
  the topics view now escape names and summaries; every page compared
  (36 generated pages and commands, 83 Info manuals) is byte-identical to
  0.40.1.
- **2026-10-22:** Command 0.40.1 (patch): step 2 of the developer-approved
  refactoring sequence (private D188). `man.c` runs `man -w`, `apropos` and
  groff through libtam 0.4.0's `tam_exec`, with `env LC_ALL=C PAGER=cat
  MANPAGER=cat` in the argv instead of the private fork runner, and no shell.
  gzip man pages are read with zlib (`tam_read_gzip_file`, shared with Info);
  bzip2, xz and zstd go through their decompressor as argv. groff gets
  `TMPDIR` and its image directory set to a fresh directory under
  `$XDG_RUNTIME_DIR/tamlinux/tam/` (else the cache directory, else `/tmp`),
  removed after each render. Output for every man, tldr and apropos page
  checked is byte-identical to 0.40.0. The build links libtam statically,
  drops the RUNPATH and requires libtam 0.4.0; `source-build.json` pins
  libtam `a051ebf` (0.4.0) for source upgrades, keeping `dbus-1` because
  building libtam still compiles its portal library.
- **2026-10-22:** Command 0.40.0 (minor): the developer's status row,
  `<Page Title> | L<line> <range>/<total> | <Level> text 0123 | h Help  Esc
  Quit  [] Select links  -> <link>`, with `v<command> (<corpus>)` right
  aligned "so I'll know which version I'm testing". The `< Back`, `` `
  Guide ``, `q Quit`, `u Up` and key-list hints are removed as confusing; the
  help overlay keeps every key, and `q` still quits. Messages take the link's
  place while they show. Narrow rows drop the title first (the breadcrumb
  repeats it), then the link, then the key hints; the line, range, lens and
  version stay.
- **2026-10-22:** Command 0.39.2 (patch): the top of a page is its title,
  `L1`. The developer: the level line "is not part of page content; it is a
  page display control line." `x`, `.`, Home, an empty `#` link and `` ` ``
  on the Guide index scroll to the top, keeping the level lines in view, and
  put the attention line on the title. Up still reaches `Lb` and `La`.
- **2026-10-22:** Command 0.39.1 (patch, the version the developer named):
  status-row line labels and lens selector. A page opens on its title, `L1`;
  the leading level lines are `Lb`, `Lc`, ... and the focused breadcrumb row
  is `La`. The visible range and total count only numbered lines, so the
  Welcome page reads `L1 1-31/48` where 0.39.0 read `L2 1-32/49`. The lens
  reads `All text 0123` (100 columns or wider), `All 0123` (60 to 99) or
  `0123`, with the lens digit underlined, or bracketed as `[0]123` where
  the terminal cannot underline; `(not on page)` still follows a missing
  lens level.
- **2026-10-21:** Command 0.39.0 (minor): the developer's pager layout. A
  dimmed, right-aligned level line sits between the breadcrumb and the page
  heading, `<Level> text [on <Level> path] [(no <Level> text)]`, with the
  path linked; it replaces the level bar and the "Written for" banner. A page
  without the lens level adds a second right-aligned line,
  `<Level> text: A | B | C`, linking up to three relatives at that level,
  separated by bars because titles hold commas, and never the level path,
  which the line above already links. A line too long for the width wraps
  left-aligned. Keys `0`-`3` and the help overlay remain the way to switch
  levels; no clickable level chooser replaces the bar's links. Rendered by a
  `<p class="level-line">` block and a new dim style (terminfo `dim`, skipped
  on the attention line).
- **2026-10-21:** Command 0.38.1 (patch): preserve the pager cursor's blank/text
  kind when resize reflow finds equal logical offsets. PTY screen tests cover
  text and blank anchors across width changes. The packaged Foot/Linux-console
  review remains pending; no development installation or package promotion is
  implied.
- **2026-10-21:** On the developer's explicit boot/Snapper/Rescue continuation,
  migrated three Maintaining ladders and added Rescue (corpus 0.42.0).
  Practice remains read-only or scratch; no actual boot or snapshot mutation
  is needed for completion. Remaining Maintaining pages and enforcement,
  learner review and packaged acceptance stay open.
- **2026-10-21:** Corpus 0.41.0 (minor): TCK25 Maintaining recovery slice,
  on the developer's continuation request. Migrate rollback and emergency
  recovery, add three glossary terms and connect Intermediate planning from
  Keep it healthy. Use captured scratch/read-only tasks instead of assumed
  disk repairs, arbitrary generation activation or emergency reboots.
  Command remains 0.38.0; remaining Maintaining migration, learner review
  and package acceptance stay open.

- **2026-10-21:** Corpus 0.40.0 (minor): TCK25 Using migration and new
  Command-line fluency path. Add validated per-part anatomy enforcement,
  preserve warning-only behavior elsewhere, and allow a path to map visited
  skill levels beyond its starting level. Convert the three level paths to
  the agreed format. Command remains 0.38.0; no live GUI, learner or packaged
  acceptance is inferred from source checks.

- **2026-10-21:** Corpus 0.39.0 (minor): TCK24 scaffolding, on the developer's
  "Continue" after the pilot-review status report. Optional learning metadata
  supports gradual migration: unknown supplied values/IDs and dangling teaching
  references are errors; anatomy gaps warn. JSON and new sidecars preserve the
  ten-column index.tsv. Coverage separates pages, paths and gaps; no learner
  score or stored progress. Command remains 0.38.0; review is not claimed.

- **2026-10-21:** Corpus 0.38.0 (minor): TCK23 pilot source. Rewrite pipelines,
  permissions and updates to the teaching contract; add two goal paths and
  their hub, linked from Welcome and Learning. Fixed-input exercises replace
  uncaptured output; maintenance tasks inspect ownership and prepare recovery
  without changing packages. Developer pilot review is pending before TCK24.
  Command remains 0.38.0; no deployment or acceptance is implied.

- **2026-10-21:** Command 0.38.0 (minor): TCK22, with corpus 0.37.1
  (patch), implements the approved folding, quizzes and block-level levels contract.
  Number folds before filtering so a level change never reveals another
  question's answer; keep the open set in the pager and pass it privately to
  the loader. Pipe output collects answers at section ends. Generated level
  labels replace manual heading prefixes on 23 pages. See the
  [implementation record](../ai/2026-10-21-tam-folding.md).

Entries are dated historical decisions; the current sections and roadmap above
resolve superseded schedules. Do not execute a former plan from this history.

- **2026-10-21:** Command 0.37.1 (patch): the "Written for" banner picks
  related pages from the index's parent links: children, then siblings, then
  the parent, each offering the lens level and ranked by shared
  prerequisites, then title, at most three. A page with no such relative
  links only the level path. The developer accepted this over the TCK21
  rule (same track, shared prerequisites, title), which gave poor picks.
- **2026-10-21:** Command 0.37.0 (minor) delivers TCK21, the sticky level
  lens. Keys `0`-`3` set the reader's lens and re-render the page in place
  without a history entry; the lens holds across links, Back and Forward
  and is saved in `$XDG_STATE_HOME/tamlinux/tam/level` only when the reader
  changes it. `TAM_LEVEL` overrides the file and is never written back;
  `--level` and an address with `level=` override both for one run, and a
  followed link with `level=` sets the lens. History and links keep
  addresses without `level=`; `=` shows the address with the lens. Guide
  pages and the generated topic views (`tam:explain`, `tam:topics`,
  `tam:keywords`) take the lens; man, Info, tldr, other `tam:` pages and
  standard input report that they have one level. Piped output shows only
  the level the command or address names, never the saved lens, so scripts
  stay reproducible. A level bar replaces the "Depth:" line, a one-line
  marker stands where a section is hidden, the status row shows the lens
  (with `(not on page)` where it applies), and a page or topic without the
  lens level is shown whole under a "Written for" banner linking up to
  three same-track pages at that level (ranked by shared prerequisites,
  then title) or the level path, so `tam show --level` and
  `tam explain --level` no longer fail there. `welcome_seen` moves to
  `tamlinux/tam/`; "Depth" is gone from reader-visible text.
- **2026-10-21:** The developer approved the learning levels plan (Tamlinux
  decision D184, amending D174). Keep three levels, with keys and numeric
  addresses 1-3 unchanged; novices get an on-ramp (the First steps goal
  path, folded "New to this?" asides, glossary links) instead of a fourth
  level. "Level" is the reader's 0-3 lens and "path" is a reading order;
  "Depth" is retired from reader-visible text. The lens is sticky across
  pages and sessions (default All), explicit `?level=` links set it, and
  filtering shows one level plus untagged content. Every page opens at
  every lens, with a banner where its level differs. Self-quizzes fold in
  the pager (Enter, `r`, `*`) and print collected answers in pipe output.
  Paths come in two families: level paths and goal paths. TCK20 (the
  contract) shipped with the plan; no component version changed.
- **2026-10-21:** `tam` keeps all its state in `$XDG_STATE_HOME/tamlinux/tam/`
  (fallback `~/.local/state/tamlinux/tam/`), Tamlinux decision D185. The
  level lens will be `tamlinux/tam/level`. TCK21 moves `welcome_seen` there
  from the top level of `tamlinux/`, with no migration because nothing reads
  it. This keeps `tam` in the `tamlinux` namespace it uses for its corpus
  and version file, with the subdirectory saying which component owns
  the files.

- **2026-10-20:** Command 0.36.0 (minor) opens `tam search` results in the
  pager. On a terminal the command shows the existing Guide Search page at
  `tam:search?query=<terms>`, whose result links are followable and whose
  `Search again` link reopens the guide search prompt. `--pipe` (formerly a
  no-op) and output that is not a terminal keep the plain-text listing;
  `--json` is unchanged; a search with no match still prints the miss
  suggestions to standard error and exits 1 without opening the pager.
- **2026-10-20:** Command 0.35.0 (minor) reports the operating system and
  Tamlinux separately (Tamlinux decision D181). `tam inspect` shows the
  distribution as `Operating system: Arch Linux (rolling)`, prints
  `OS version` only when os-release has `VERSION_ID`, and shows
  `Tamlinux cultivar: <Name> <version>` from the invoking user's
  `~/.config/tamlinux/version` and `cultivar` files (the `PKEXEC_UID` user
  under `pkexec`). `tam --version` prints the same cultivar line. Two findings
  join the Check Contract: `os_identity` (warns when `/etc/os-release` names a
  different distribution from `/usr/lib/os-release`) and `tamlinux_version`
  (info when the user has no deployment). The version file still wins over
  `$TAMLINUX_VERSION`. Corpus 0.36.1 (patch) documents the rows and updates
  the product-version description.
- **2026-10-20:** `make install` refuses a dirty checkout. An agent committed
  page edits, skipped the follow-up index commit and installed, so the
  development install reported corpus `ae1fa41-dirty`. The committed index
  records each page's commit history and is always one commit behind after a
  page commit; prose in `AGENTS.md` was the only safeguard. The new
  `check-clean-tree` target runs after the index rebuild and stops an install
  when any command or corpus path is uncommitted, naming the index follow-up
  commit when that is the only change. Staged (`DESTDIR`) installs and sources
  without their own `.git`, such as the Nix package, are not checked. Tooling
  and tests only; no component version change.
- **2026-10-20:** Command 0.34.0 (minor) adds pager link keys `]` and `[` with
  Tab and Shift-Tab kept as aliases, and restricts `N` to previous search hit
  when a search query is active (TCK15, [plan](pager-quick-keys.md)).

- **2026-10-20:** The developer approved the [pager quick keys plan](pager-quick-keys.md)
  (Tamlinux decision D176) and asked for it to be written for another agent to
  build. `]`/`[` select links, with Tab/Shift-Tab kept as aliases; `N` means
  only the previous search hit; `=` shows a `page:`, `man:`, `info:`, `tldr:` or
  `tam:` address that `tam show` reopens; `\` searches the Guide with labelled
  prompts; the status row reads `L65 3-78/79`; the Guide index gains a Quick
  keys table with `tam:help` and `tam:prompt:page-search` links and keeps tldr
  pages. He rejected Tab and Shift-Tab as jumps to `tam:topics` and `tam:tldr`.
  TCK15–TCK19; no version change until TCK15.
- **2026-10-20:** Command tests no longer depend on what the pages say
  ([developer loop](dev-loop.md), TCK13, TCK14). A correct reordering of the
  index links failed a pager test that asserted the live link order. Interactive
  tests in `tests/run.sh` now read `tests/fixtures/`, `make check-repo` rejects
  an unmarked live-corpus interactive test, and `make dev` names the failing
  check and says whether the pages or a test need fixing. Tooling and tests
  only; no component version change.
- **2026-10-20:** The developer approved the [developer loop](dev-loop.md)
  (TCK11, TCK12). `make dev` becomes his own path from saved edits to a
  committed, tested development install: it commits every changed path except
  other sessions' claimed paths, proposes the version bump for each changed
  lane, makes the separate index commit, installs to `~/.local/dev` and asks
  before pushing `develop`. Plain `make`, `make all` and `make install` stay
  non-interactive for packaging and agents. A drift checker in `make test`
  compares copied facts with their sources, and `docs/repo-map.md` says what
  each top-level entry is. Tooling only; no component version change.
- **2026-10-20:** Command 0.32.0 (minor) puts the pager's letter keys on the
  home row, mirrored on each hand (D175, TCK10, [plan](pager-home-row-keys.md)).
  `d`/`k` and `s`/`l` move the attention line one row down and up, sharing the
  arrows' breadcrumb behavior. `a`/`j` and `f`/`;` move an attention column
  from link to link along the attention line, wrapping; the column is drawn as
  a band as wide as the caught link on every content row and clears when the
  line, selection or page changes, or on `g`/Enter. `g` follows the selected
  link and `h` toggles help. `x`/`.` and `c`/`,` go to the top and bottom; `G`
  is retired. This replaces 0.31.1's `a`/`f` reading-order and `s`/`d`
  link-jump meanings. Corpus 0.33.5 (patch) teaches the keys on the welcome
  page.
- **2026-10-20:** Command 0.31.5 (patch) makes in-page links work. The
  renderer records `id=` and `<a name=>` targets by block, and Enter or Right
  Arrow on a `#fragment` link puts its target on the first content row; Left
  Arrow returns. Man pages were the visible case: groff's contents list links
  to `<a name>` targets inside headings, and the old text search matched the
  link itself, so following it did nothing. Pages without anchors fall back to
  a heading, then unlinked text. Regression fixture:
  `tests/fixtures/pager-anchors.html`.
- **2026-10-19:** Command 0.31.4 (patch) fixes Down Arrow on the breadcrumb
  row. Down (and `j`) now returns to the attention line where it was, instead of
  jumping to the first page link. On a page whose only links are breadcrumbs,
  the old behavior left Up and Down unable to move the attention line. Up from
  the first line still focuses the last breadcrumb link (the developer kept
  0.31.1's behavior); `s`/`d` still move between breadcrumb and page links.
  Regression fixture: `tests/fixtures/breadcrumb-only.html`.
- **2026-10-19:** Command 0.31.3 (patch) shortens the default message so it
  shows in full in a half-height window: bare `tam` drops `tam search`,
  `--terse` and `--verbose` (still listed by `tam --verbose`), and usage
  lines read `tam <subcommand> [arguments] [--options]`. Corpus 0.33.1
  (patch) rewords the welcome page. Both changes are the developer's.

- **2026-10-19:** Command 0.31.2 (patch) fixes table layout: a table wider than
  the terminal keeps its columns and wraps the widest ones instead of stacking
  every row. Topic list headings change to Topics A to Z and Keyword Index.
  Corpus 0.32.4 (patch) corrects topic keywords; the developer chose the patch
  bump so changed keywords are not delivered under 0.32.3.

- **2026-10-18:** The developer accepted [source upgrades and stage switching](upgrade.md)
  for a subsequent implementation session: TCK6–TCK8, development-prefix source
  installs, independent code-based component versions and paired release tags.
  Approval is recorded in the [planning session](../ai/2026-10-18-tam-update-check-plan.md#approval-and-handoff).
  No implementation, installation or stage promotion occurred during planning.
- **2026-10-18:** The developer approved Installing, Learning, Using, Maintaining,
  Improving as the spine, Learning routes through shared pages, and arbitrary
  man lookup within the guide. Corpus target 0.31.0 and companion command target
  0.30.0 are owned by [one work plan](knowledge-system-0.31.0.md). The seven-part
  contents and three-track design are superseded. Parser/installer version
  reservations are withdrawn; those features remain deferred.
- **2026-10-18:** Adopted [plan maintenance](../plan-maintenance.md): explicit
  status/evidence/replacement, stable historical addresses, complete plan index,
  synchronized roadmap/current contract, and a handoff in the owning plan.
- **2026-10-18:** Reconciled the current corpus to 0.30.0 (`d13c1f6`, Snapper).
  No version bump, code change, deployment or visual acceptance in this planning
  session. Pager portability physical acceptance remains pending.
- **2026-10-18:** Command 0.31.1 fixes fixed-row breadcrumb navigation in the
  interactive pager. From the first document line, Up Arrow focuses the last
  breadcrumb link; Up Arrow moves among breadcrumb links and Down Arrow returns
  to page links. `a`/`f` move horizontally among breadcrumb links, `s`/`d`
  move vertically between breadcrumb and page links, and Tab/Shift-Tab include
  breadcrumb links in the normal sequence. Regression coverage uses the
  `tam man` landing page and verifies both breadcrumb destinations. Corpus
  remains 0.32.0.

- **2026-10-17:** Command 0.29.3. Only whitespace with non-space hyperlink
  text on both sides receives selected-link emphasis. This keeps the layout
  separator before a hyperlink outside the highlighted region.

- **2026-10-17:** Command 0.29.2. Whitespace characters inside the selected
  hyperlink now receive the same bold reverse-video style as its other
  characters. Unselected links keep the existing leading-space behavior.

- **2026-10-17:** Command 0.29.1. Selected hyperlinks now use bold reverse
  video, with underline fallback where reverse video is unavailable. Added a
  PTY assertion for the selected style and kept the command and corpus
  versions independent; the corpus remains 0.29.0.

- **2026-10-17:** Pager display portability implemented as command 0.29.0.
  The existing pager now uses terminfo capabilities, buffered row updates,
  Unicode display-cell geometry and retained-layout reflow. Its focus uses a
  charcoal color where supported and reverse video on the Linux console and
  monochrome terminals. Because this is a user-visible capability, the planned
  shell syntax dissection and installation framework milestones move to
  0.30.0 and 0.31.0. See
  [pager display portability](pager-display-portability.md).

- **2026-10-08:** Separate `tam` component repository prepared locally.
- **2026-10-08:** Fred specified: "Bare tam should print a concise intro and
  exit. tam browse should open the guide."
- **2026-10-08:** C with `libtam` replaces the earlier Python proposal.
- **2026-10-09:** Fred approved this plan and requested committing the prepared
  work. Command implementation remains the next milestone.
- **2026-10-09:** Command foundation implemented (milestone 1). `tam 0.0.1`
  builds with GNU Make and `cc` (GCC) against an installed `libtam 0.1.0`,
  dynamically linked with an rpath for the `/usr/local` development install.
  `tests/run.sh` checks bare/help/version behavior, no-state, cultivar-version
  resolution, control-sequence-free piped output and argument errors.
- **2026-10-09:** The Tamlinux version reported by `--version` is labelled
  **"Tamlinux cultivar"**. Fred: "In the future, we will give each version of
  Tamlinux as a whole a cultivar name as well as a version number. There is no
  cultivar name yet, so we just call it 'Tamlinux cultivar'." The intended
  future shape is `Tamlinux cultivar <name> <version>` (e.g. `Tamlinux cultivar
  blue sparkler 1.0.0`); the name source is not yet defined.
- **2026-10-09:** Offline guide implemented (milestone 2). Fred approved a
  **native renderer** rather than an external browser (so `tam` works before the
  desktop and on minimal systems), with text derived at runtime from the same
  HTML and committed snapshots as golden fixtures. Corpus pages `index`, `welcome`,
  `install/`, `shell/pipelines` and `man/` were authored; links are
  root-relative and resolved against the corpus root. Recorded in the
  [offline-guide plan](offline-guide.md). Command version bumped to `0.1.0`
  for the new `browse` capability (Fred: bump the minor number when adding a
  feature), and the versioning convention above was recorded.
- **2026-10-09:** Pager navigation keys added (Fred's request): `Esc` as well
  as `q` quits; the right arrow follows the selected link (like `Enter`) and
  the left arrow goes back (like `u`). Version bumped to `0.2.0` (new input
  capabilities).
- **2026-10-09:** `tam help` added as an exact alias of `tam --help` (Fred's
  request: "update tam so tam help works the same as tam --help"). Extra
  arguments remain a usage error. Version bumped to `0.3.0` (new user-visible
  invocation).
- **2026-10-09:** Orientation and curated explanation implemented (milestone 3).
  Fred approved a combined slice with orientation (`tam welcome`) and curated
  explanation (`tam explain <topic>`). `tam welcome` opens the orientation guide
  in the interactive pager (or plain text when piped) and atomically records an
  ISO 8601 UTC timestamp in `$XDG_STATE_HOME/tamlinux/welcome_seen` only after
  successful display. Strict isolation is verified: bare `tam`, `browse`,
  `help`, `version`, and `explain` never touch welcome state. `tam explain`
  resolves curated topics (`pipelines`, `redirection`, `man`, `welcome`, `install`),
  emits clear diagnostics on misses, and treats arbitrary shell metacharacters
  strictly as query data without executing them. Command version bumped to `0.4.0`.
- **2026-10-09:** Installation inspection implemented (milestone 4). Fred approved
  the slice 4 implementation plan with bare `tam install` showing usage overview
  (exit 0) and sectioned key-value output for `tam install inspect`. Implemented
  modular `src/install.c` and `src/install.h` in C linking `libtam`. Probes
  `/etc/os-release`, `uname()`, `$PATH` for package tools and privilege tools,
  `/proc/mounts` for root filesystem, `/dev/dri` for graphics devices, and maps
  supported profiles (`arch-hyprland, arch`, `void`, `antix`). Unimplemented
  installer verbs (`plan`, `apply`, `verify`) truthfully report they are not
  implemented yet (exit 1). Updated offline guide `knowledge/install/index.html`
  and golden test snapshots. Command version bumped to `0.5.0`.
- **2026-10-09:** Bare `tam explain` topic listing implemented (0.6.0). Bare `tam explain`
  and `tam explain --pipe` now list available explainable topics grouped with
  summaries and aliases (exit 0) rather than erroring with missing topic argument.
  Miss messages point to `tam explain` and `tam browse`. A multi-phase scaling plan
  for large topic catalogs (domain grouping, TUI pager integration, apropos search,
  corpus-driven metadata) is recorded in [explain-topic-scaling.md](explain-topic-scaling.md).
  Command version bumped to `0.6.0`.
- **2026-10-09:** Subcommand rename to `tam show` and explain quoting (0.7.0).
  Fred requested replacing `tam browse` with `tam show` ("tam show by itself shows
  the index page, just like tam browse does now. I just like 'show' better than
  'browse'. We can keep browse as a topic that tam explain browse can handle").
  `tam show` now serves as the guide subcommand; `tam browse` is retired and
  produces an unrecognized argument error. `browse` and `show` were added as curated
  topics to `tam explain`, mapping to `index.html`. `tam explain` now strips outer
  matching single or double quotes, allowing shell-quoted inputs like `tam explain '|'`
  and `tam explain "|"` (as well as literal quotes `"'|'"`) to resolve cleanly
  to `shell/pipelines.html`. Added pipe symbol `|` to pipelines keywords. Miss diagnostics
- **2026-10-09:** Knowledge architecture, man page strategy, and priorities.
  Fred confirmed: (1) Provide a lightweight `man` wrapper so `man <command>`
  directly opens the hyperlinked `tam` pager; system roff man pages are converted
  to semantic HTML on the fly with cross-references (`SEE ALSO`) transformed into
  interactive hyperlinks for seamless traversal. (2) Prioritize the "Use" track
  first in the knowledge corpus. (3) A clean, space-padded terminal column layout
  is sufficient for table rendering in `src/render.c`. Recorded in milestone 5.
- **2026-10-09:** Milestone 5 implemented (tam 0.8.0). Implemented table rendering
  (`<table>`, `<tr>`, `<th>`, `<td>`) with dynamic space-padded column calculations
  and header underline separators, as well as definition lists (`<dl>`, `<dt>`, `<dd>`)
  with bold terms and indented definitions in `src/render.c`. Implemented standard
  input reading (`tam show -` and `tam explain -`) with interactive terminal control via
  `/dev/tty` in `src/pager.c` and `src/tam.c`. Authored comprehensive "Use" track knowledge
  pages (`shell/index.html`, `shell/jobs.html`, `filesystem/index.html`,
  `filesystem/permissions.html`, `desktop/index.html`, `desktop/workspaces.html`,
  `hardware/resources.html`) and refreshed `index.html` and `welcome.html`. Added
  lightweight `bin/tam-man` wrapper converting roff manual pages on the fly into
  hyperlinked terminal views. Updated test suites (`tests/run.sh` and `tests/guide.sh`)
  and refreshed golden snapshots. Version bumped to `0.8.0`.
- **2026-10-09:** Enhanced manual cross-referencing and `tam man` implemented (tam 0.9.0).
  Implemented Option C: native manual page subsystem in `src/man.c` and `src/man.h`.
  Roff manual pages are resolved via `man -w`, decompressed on the fly, converted to
  semantic HTML via `groff -mandoc -Thtml`, and parsed by an HTML linkifier that
  transforms manual cross references (such as `<b>awk</b>(1)` and `<b>sed</b>(1)` in
  `SEE ALSO` or throughout the manual body) into live `<a href="man:name(sec)">` links.
  Added built-in `tam man [SECTION] PAGE` subcommand and wired `man:` link traversal
  into `corpus_load` so following manual links in the pager immediately renders and
  switches to the target manual page, with full history navigation (`u` to go back).
  Added in-page anchor jumping in `src/pager.c` for table-of-contents headings (`#SEE ALSO`,
  `#SYNOPSIS`). Refactored `bin/tam-man` to delegate cleanly to `tam man` with standard
  man fallback. Updated test suite with `tam man`, section lookups, hyperlinked reference
  assertions, and error handling. Command version bumped to `0.9.0`.
- **2026-10-09:** Development installs under `~/.local/dev` (tam 0.9.1). Fred asked
  to install development builds to a user prefix that comes first on `PATH`, so
  `tam` runs the build under development until it is pinned in
  `tamlinux-packages`. The compiled data directory now precedes the XDG data
  directories in corpus lookup, so each binary renders the corpus installed with
  it rather than a packaged corpus linked into `~/.local/share`. Builds from a Git
  checkout report `git describe` in `--version` (`0.9.1 (dev 1a2b3c4-dirty)`);
  packaged builds report the plain version. The build is redone when `PREFIX` or
  other embedded configuration changes, and `make install` replaces the installed
  corpus instead of merging into it.
- **2026-10-12:** Plan revised for the second generation of work (milestones
  6–9). Following the `tam` help and self-inspection research and requirements
  (T32), the plan now commits to (6) a corpus index replacing the hand-written
  keyword table plus `tam search` as the single search verb, (7) a
  content-depth ladder with fading templates, (8) a read-only self-inspection
  findings layer and check contract, and (9) an opt-in, grounded `ask` surface.
  The strictly-offline identity is amended deliberately and opt-in only: `ask`
  never bundles or starts a daemon and fetches only under interactive consent.
  `tam search` supersedes the `tam explain search` variant in
  [explain-topic-scaling.md](explain-topic-scaling.md), and corpus-driven
  metadata (its Phase 5) is promoted from last to first. The `why`/depth axes
  and a computer-science track remain exploratory and are explicitly excluded.
  This revision is a proposal for Fred's review; the underlying design decisions
- **2026-10-13:** Milestone 6 implemented (tam 0.10.0). Replaced the
  hardcoded `TOPIC_MAP` in `src/tam.c` with a corpus index (`src/index.c`,
  `src/index.h`) generated from HTML page metadata (`<meta name="keywords">`,
  `<meta name="summary">`, `<meta name="level">`, `<meta name="track">`, `<title>`, `<h1>`)
  via `tools/build-index.py` into `knowledge/index.tsv` and `knowledge/index.json`.
  Added `tam search [--json] TERM...` as the single search verb with field-weighted
  ranking, prefix matching, bounded fuzzy suggestions (`Did you mean`), and
  machine-readable JSON output. `tam explain` keeps single-topic semantics and
  resolves multi-word queries through search. Clear miss banners report no-topic
  diagnostics, top candidates, and system manual page suggestions when available.
  HTML parser in `src/render.c` now skips quoted attribute values when detecting
  closing tag delimiters. Development build installed under `~/.local/dev`.
  Command version bumped to `0.10.0`.
- **2026-10-13:** Link navigation enhancement (tam 0.11.0). Implemented
  `Shift-Tab` in `src/pager.c` to circulate backward among links (standard ANSI
  CSI Z / CBT sequence `\033[Z`, plus CSI `1;2Z`, Kitty `9;2u`, modifyOtherKeys
  `27;2;9~`, and Alt-Tab `\033\t`), complementing `Tab` forward navigation and
  retaining `N` as a fallback. Pager status line updated to `Tab/S-Tab link` with
  display-column-aware truncation preventing UTF-8 boundary slicing. Updated
  corpus documentation, man page help text, and test coverage. Version bumped to
  `0.11.0`.
- **2026-10-13:** Forward path navigation enhancement (tam 0.12.0). Implemented
  right arrow (`\033[C`) forward navigation along previously visited paths in
  `src/pager.c`. When arriving at a page using left arrow (`\033[D`) or `u`, the
  referring page is remembered in a forward navigation stack. If no link is
  selected, right arrow proceeds forward toward the tip of an exploration. If a
  link is selected, right arrow follows that link to extend the exploration (or
  branch off). If at the tip of an exploration with no link selected, right arrow
  does nothing. Added automated PTY interactive tests in `tests/run.sh`. Version
  bumped to `0.12.0`.
- **2026-10-13:** Hyperlink display fixes (tam 0.13.0). Fixed two bugs in how
  hyperlinks are displayed: (1) Do not underline the first space in a hyperlink;
  start the underline (and selected highlight) with the first non-space character.
  (2) A hyperlink that wraps lines is treated as a single hyperlink rather than two
  separate hyperlinks, highlighting the continuation of the hyperlinked text on the
  next line rather than ending highlighting at the line break. Tagged each `<a>`
  anchor instance with a unique `link_id` in `src/render.c` and `src/render.h`.
  In `src/pager.c`, merged contiguous link spans sharing `link_id` into a single
  link entry, highlighted all lines of a wrapped hyperlink when selected, and
  skipped leading whitespace in `emit_line` when applying underline or reverse
  video styling. Added automated PTY tests in `tests/run.sh`. Version bumped to
  `0.13.0`.
- **2026-10-13:** In-pager text search, reading telemetry, and static Bash completions implemented (tam 0.14.0). Added in-pager keyword search in `src/pager.c` activated by `/` with interactive search prompt, case-insensitive line search, in-buffer standout hit highlighting, next match (`n`) and previous match (`N`) navigation, and `Esc` highlight clearing (or page navigation clearing). Updated status line reading position telemetry reporting exact line metrics and percentage bounds (`L.../total`, `All`, `Top`, `Bot`, `%`). Retained `N` backward link circulation fallback when no search query is active. Generated static shell completion script for Bash (`completions/bash/tam`) completing fixed subcommands and options with zero dynamic file I/O (Tamlinux supports Bash only; zsh and fish code was omitted). Updated `Makefile` install/uninstall targets and expanded automated PTY interactive tests in `tests/run.sh`. Version bumped to `0.14.0`.
- **2026-10-14:** Milestone 8 implemented (tam 0.16.0). Grew `tam install inspect` into a comprehensive, read-only system diagnostic and health finding engine and established the offline Check Contract. Defined structured finding model (`id`, `severity` in info/warn/error/critical, `summary`, `evidence`, `help_link`) deep-linking directly into Maintain-track guide pages (`maintain/rollback.html`, `maintain/recovery.html`, `maintain/logs.html`, `maintain/power.html`, `maintain/updates.html`). Defined the versioned offline Check Contract schema (`{task_id, name, status, pass, fail, evidence, severity, help_ref, source, schema_version}`) serving as the offline assessment oracle for progression tasks. Implemented the Safe Execution Envelope (`safe_probe_exec`) running fixed read-only whitelisted probes (`findmnt`, `systemctl`, `udevadm`, `pkexec`) under `LC_ALL=C`, bounded timeouts (≤500ms), captured standard streams, detached TTY, and neutralized color, backed by POSIX primitives (`statvfs`, `uname`). Implemented `pkexec` detect-and-degrade rule: detect presence without assuming privilege; on hosts without `pkexec`, gracefully degrade to non-privileged inspection subset and emit a `warn` finding ("privilege tool unavailable: limited inspection"); never spawn interactive `sudo`. Added `tam install inspect --json` for full machine-readable inspection reports, `tam install inspect --check [ID]` and `tam install check [ID]` (`--json` supported) for discrete check evaluation. Updated Bash completions (`completions/bash/tam`) and expanded automated test suite (`tests/run.sh`). Version bumped to `0.16.0`.
- **2026-10-14:** Milestone 9 implemented (tam 0.17.0). Added opt-in grounded question answering CLI surface `tam ask [OPTIONS] <question>` (`src/ask.c`, `src/ask.h`). Implemented pure POSIX socket HTTP/1.1 client connecting to already-running local inference server (e.g. Ollama); strictly honours D53 zero-daemon invariant (never starts, stops, or bundles a background daemon). Implemented RAG over corpus index (`tam_index_search`): retrieves top relevant corpus pages, extracts plain-text excerpts bounded to ≤4 KB, and enforces mandatory `Sources:` citation lines. Grounding miss safety: queries with zero corpus relevance return "Not found in Tamlinux guide." with `Sources: (none)` and suggest `tam search`, preventing hallucinations. Non-interactive pipeline safety: under `--pipe` or non-TTY shells, missing model/server immediately exits with code 2 and exact message `tam: ask requires a local model; run interactively to fetch or start Ollama`. Interactive consent: when connected to server with missing model, prompts user with full disclosure (URL, size, storage path) before download, with `$TAM_ASK_ALLOW_FETCH=false` override. Added `--json` machine-readable output schema. Added static Bash completions for `tam ask` (`--model`, `--server`, `--json`, `--pipe`). Version bumped to `0.17.0`.
- **2026-10-14:** Quick Guide index navigation keybinding planned (Fred's request). Bind backtick (`` ` ``) in the interactive pager (`src/pager.c`) to jump straight back to the Guide root index (`index.html`) from any corpus page, subpage, or manual page, saving the current exploration position to the history stack so users can re-orient to the home index with a single keystroke from any depth.
- **2026-10-14:** Output ergonomics and "Goldilocks" screen budget decided (Fred's request). Non-interactive usage guides and overviews must never spew unbounded output that scrolls critical syntax off a standard 80×24 terminal screen. Refactor bare `tam explain` to display strictly the usage synopsis and navigation pointers (≤15 lines) letting users know a special `tam explain topics` page is available, without dumping the full topic catalog. `tam explain topics` interactively launches inside `tam`'s built-in pager with each topic rendered as a live hyperlink so users can easily select and navigate directly to any topic; under `--pipe` or non-TTY, it outputs clean text. Establish `--terse` for minimal machine tokens emitting one topic slug per line (ideal for `fzf`/scripts) and `--verbose` for exhaustive diagnostics/keywords, while keeping default outputs in the "Goldilocks" just-right zone. Condense `tam --help` to ≤20 lines by consolidating redundant per-subcommand flag variants.
- **2026-10-14:** Milestone 10 implemented (tam 0.18.0). Implemented Quick Guide index navigation keybinding in `src/pager.c`: backtick (`` ` ``) returns immediately to `index.html` from any corpus topic, subtopic, or manual page (`tam man`), saving the current page to the history stack so `u` returns back. Updated pager status line to include `` ` index `` shortcut. Implemented Goldilocks screen budget: bare `tam explain` now strictly emits a concise 13-line usage overview (≤15 lines) pointing users to `tam explain topics` and topic queries rather than spewing the entire 29-topic catalog. Condensed `tam --help` to a 14-line synopsis (≤20 lines). Added `tam explain topics` which interactively launches inside `tam`'s built-in pager with each topic rendered as a live hyperlink (`<a href="...">`) for instant selection and navigation; under `--pipe` or non-TTY, emits clean text catalog. Added dynamic `tam:topics` loader routing in `corpus_load` to support back/forward pager navigation. Added `--terse` modifier (slugs only for `tam explain topics`; compact status for `tam install check`) and `--verbose` modifier (exhaustive diagnostics and keyword arrays). Added Bash completions for `topics`, `--terse`, and `--verbose`. Version bumped to `0.18.0`.
- **2026-10-14:** Milestone 11 implemented (tam 0.19.0). Authored the comprehensive "Maintain" track corpus expansion fulfilling the milestone 8 self-inspection deep-link contracts. Created `maintain/index.html` as the track portal and 5 core subtopics: `maintain/updates.html` (non-disruptive updates and package lifecycle), `maintain/rollback.html` (Btrfs snapshots and generation rollbacks), `maintain/logs.html` (truthful diagnostics and logs), `maintain/power.html` (progressive power ladder and safe display wake), and `maintain/recovery.html` (emergency disaster recovery). Authored Beginner/Intermediate/Expert fading templates with worked examples and completion tasks. Improved topic lookup in `src/index.c` (`tam_index_lookup_alias`) to prioritize exact topic names over keyword aliases. Regenerated corpus index (`knowledge/index.tsv`, `knowledge/index.json`) and golden text snapshots. Updated test suite (`tests/run.sh` and `tests/guide.sh`). Version bumped to `0.19.0`.
- **2026-10-14:** Milestone 12 implemented (tam 0.20.0). Restructured the root guide index (`knowledge/index.html`) into the comprehensive 7-part "The Tamlinux User Guide" table of contents integrating Fred's architectural triad (Using, Maintaining, and Improving your workstation) and the full planned chapter outline from Plan 26. Authored `knowledge/ecosystem/terminology.html` establishing the botanical conceptual analogy (Greenhouse, Pot, Nut, Root-stock, Cultivar) with fading templates. Authored the complete `tam(1)` UNIX manual page (`man/tam.1`) and updated `Makefile` to install it under `$(MANDIR)/man1/tam.1`. Updated `src/man.c` with development fallback paths and `src/tam.c` with direct `tam show topics` and `tam show tam:topics` routing. Regenerated corpus index (`knowledge/index.tsv`, `knowledge/index.json`) and golden text snapshots. Updated test suites (`tests/run.sh` and `tests/guide.sh`). Version bumped to `0.20.0`.
- **2026-10-14:** Milestone 13 implemented (tam 0.21.0). Authored comprehensive "Improve" track knowledge pages empowering users and autonomous AI coding agents to optimize performance, eliminate energy waste, write robust utilities, and understand local AI inference: `knowledge/improve/index.html` (hub page), `knowledge/improve/efficiency.html` (zero-polling architecture, inotify, epoll multiplexing, Wayland compositor event sockets), `knowledge/improve/zram.html` (memory compression with ZRAM, zsmalloc, LZ4 vs zstd tradeoffs), `knowledge/improve/terminal.html` (sub-10ms shell startup latency, Micro editor live disk reload Virtue V11, foot Wayland terminal architecture), `knowledge/improve/scripting.html` (defensive Bash headers `set -euo pipefail`, safe argument `"$@"` forwarding, `tam-*` vs `fred-*` namespace separation Decision D131), `knowledge/improve/toolchains.html` (native C compilation with GCC/Clang and libtam, `-O2` and `-march=native` optimization flags, isolated Git worktrees, `tam-work` scope reservations), and `knowledge/improve/ai-performance.html` (physics of local autoregressive token generation, memory bandwidth bounds, quantization formats Q4_K_M vs FP16, KV cache memory scaling). Applied Beginner/Intermediate/Expert fading templates across core subtopics; updated User Guide Part V links; updated `tests/guide.sh` and `tests/run.sh`; regenerated corpus index (43 entries) and golden snapshots; installed development build under `~/.local/dev`. Bumped version to `0.21.0`.
- **2026-10-15:** Milestone 14 implemented (tam 0.22.0). Added native GNU Info documentation reader (`tam info`) parsing Info manuals (`.info`, `.info.gz`) into semantic HTML with interactive hyperlinked menus, cross-references, and tag-table index seeking. Added dedicated Info pager keys (`n`, `p`, `u`, `t`, `i`), `a`/`s`/`d`/`f` link pad, `bin/tam-info` wrapper, Bash completions, and cross-system `info:` URI resolution. Version bumped to `0.22.0`.
- **2026-10-15:** Milestone 15 implemented (tam 0.23.0). Redesigned `tam explain` to display dynamic topic guides rather than navigating directly to documentation pages (`tam show` remains the direct page navigation tool). Bare `tam explain` dynamically aggregates all keywords across the corpus and launches the built-in pager. Authored dedicated `knowledge/tam.html` (`tam show tam`) detailing C implementation, Makefile targets, compiler chain, and the Develop -> Test -> Run -> Release lifecycle. Version bumped to `0.23.0`.
- **2026-10-15:** Milestone 16 implemented (tam 0.24.0). Decoupled `tam` command binary version (`src/version.h`) from offline knowledge corpus version (`knowledge/version`), allowing both to iterate and diverge independently. Implemented dynamic runtime discovery of corpus version and build ID in `src/tam.c` (`tam_corpus_version`, `tam_corpus_build_id`). Updated bare `tam` intro, `tam --help`, and `tam --version` to report both command and corpus versions. Added `tam hello` and `tam hi` aliases routing to first-run orientation (`cmd_welcome`). Carefully reviewed every page in `knowledge/` and aligned all references to use top-level `tam inspect` and `tam check` (updating `maintain/`, `install/`, `hardware/`, `tam.html`, `welcome.html`, `index.html`). Enriched ecosystem documentation with the botanical Fruit application layer (`knowledge/ecosystem/terminology.html`) and command varieties (`*-nut`, `*-root`, `*`), Fred's 6 Guiding Principles (`knowledge/ecosystem/philosophy.html`), and user alias guidelines (`knowledge/shell/index.html`). Reviewed Fred's private idea notes, filed them into structured idea docs, and recorded their removal. Version bumped to `0.24.0`.
- **2026-10-15:** Command 0.25.0. Added an attention line to the pager: one
  subtly tinted reading line that scrolling and link keys move. Sweeping down
  catches the first link on a line, sweeping up the last; `Enter` follows only
  a link on the attention line or a selected link still on screen.
- **2026-10-15:** Corpus 0.25.0. Added `ecosystem/package-versions.html`,
  which explains how a Tamlinux release pins the exact version of each
  component.
- **2026-10-15:** Command 0.25.1 and corpus 0.25.1. Made the lanes independent
  at release time: the tests read the corpus version from `knowledge/version`
  and discover pages instead of listing them, and the fallback corpus label
  became `unversioned`. The live `tam ask` checks skip, visibly, when no local
  Ollama server is running, instead of aborting the suite. Corpus corrections:
  the package-versions page now names `knowledge/version` as the only corpus
  version authority and describes the user's own configuration generically,
  and the toolchains example uses generic paths.
- **2026-10-16:** Corpus 0.26.0. Added `ecosystem/sddm.html`, which explains
  display managers, login greeters, the Tamlinux native Qt 6 greeter, autologin
  vs manual logout, Wayland session discovery, PAM authentication, and virtual
  console emergency recovery. Bumped corpus version to 0.26.0.
- **2026-10-16:** Corpus 0.26.1. Corrected the SDDM guide following review:
  clarified autologin as a user convenience preference distinct from disk
  encryption, documented /etc/sddm.conf precedence and SessionDir override
  mechanics including /usr/local/share, refined test-mode and headless
  execution notes, and neutralized display manager architecture descriptions.
  Bumped corpus version to 0.26.1.
- **2026-10-16:** Corpus 0.27.0. Added `ecosystem/ttys-and-virtual-consoles.html`,
  which explains terminal architecture across three layers (hardware TTYs,
  kernel virtual terminals/consoles VTs, and software pseudo-terminals PTYs),
  terminal line disciplines (canonical vs raw mode, POSIX signal generation
  Ctrl+C/Ctrl+\/Ctrl+Z/Ctrl+D), controlling terminals and session leaders,
  emergency console switching (Ctrl+Alt+F2/F1), kernel `drivers/tty/vt/`
  architecture (`KD_TEXT` vs `KD_GRAPHICS`), seat management with
  systemd-logind/seatd for unprivileged Wayland compositors, DRM KMS master
  handover (`DRM_IOCTL_DROP_MASTER` and `DRM_IOCTL_SET_MASTER`), and console
  troubleshooting. Cross-linked from User Guide index, ecosystem index, SDDM
  guide, and emergency recovery guide. Bumped corpus version to 0.27.0.
- **2026-10-16:** Command 0.25.2. Extended the HTML entity whitelist in
  `src/render.c` with `larr`, `rarr`, `times`, `le`, `ge`, and `asymp`, so the
  renderer decodes every named entity the corpus uses instead of printing it
  verbatim (finding F5 of the 2026-10-16 corpus audit). Numeric `&#…;`
  references were already decoded; no `knowledge/` page changed. The four
  affected golden snapshots were regenerated and reviewed.
- **2026-10-16:** Command 0.25.2 (renderer layout). Text measurement now counts
  display columns (`dwidth`) instead of UTF-8 bytes, so body, table and cell
  wrapping fill the requested width exactly; a full-width line previously
  stopped one column short for every multi-byte glyph it contained. Wrapping
  no longer inserts a space after an opening bracket, and a token made only of
  sentence punctuation (`.`, `,`, `:`, `)`, …) is kept with the word before it
  and never stranded alone on its own line; the break decision reserves room
  for such a token. Table columns stay aligned under `—`, `→` and `≤`. All 33
  affected golden snapshots were regenerated and reviewed: no line exceeds the
  former count of over-width lines and no page lost or gained content.


- **2026-10-16:** Fred settled how the corpus is written, checked and
  recorded, after a review of how agents had worked on it:
  1. Every statement about Tamlinux traces to a product file, a command's
     output or a recorded decision; general Linux knowledge is not presented
     as a Tamlinux choice; unbuilt work is marked planned. Sources go in the
     session record's `Checked against:` line, not on the page.
  2. The guide teaches the shared-clone, path-claim workflow; worktrees keep
     one short note in `improve/toolchains.html` explaining why Tamlinux's
     repositories do not use them.
  3. A version is spent when it is committed; corrections take a patch bump
     (no change to the rule above).
  4. A note about a missing page is a short file in `docs/plans/corpus/`
     from its template; the queries it records are re-checked when the page
     lands.
  5. Session records keep every required field, record key decisions only
     where the diff cannot show them, and add `Checked against:` and `Time:`
     lines (`docs/ai/TEMPLATE.md`); tam only for now.
  6. The audit SOP is adopted narrowed (`docs/corpus-audit.md`): about every
     ten changed pages or monthly, by a delegate that did not write the
     pages, never gating a candidate.
  7. Renderer goldens move to fixture pages; corpus pages are checked for
     properties and by the corpus checker instead of per-page snapshots.
  8. A corpus checker and the fixture tests are delegated to opencode, the
     audit fixes to agy; Claude wrote the authoring guide.
  Fred is called the developer of Tamlinux, never another title. The
  authoring guide is `docs/corpus-authoring.md`.
- **2026-10-16:** Corpus 0.27.1. Fixed the 2026-10-16 audit findings: the
  desktop keybinding table now matches the Tamlinux bindings; `tam install
  plan`, `apply` and `verify` are marked planned and `tam install inspect`
  is replaced by `tam inspect`; worktree guidance is replaced by the
  shared-clone, path-claim workflow, with one note on why; LaTeX and internal
  decision identifiers are removed; the terminology table names Hyprland as
  current and Sway as planned; smaller corrections to examples, names and
  wording. Finding F4 (session names) was withdrawn: the page was already
  right. `make test-corpus` and `make test` now run the corpus checker
  (`tools/lint-corpus.py`) and its tests, and `make test-command` and `make
  test` run the renderer goldens (`tests/render.sh`).
- **2026-10-16:** Command 0.26.0. (1) Replaced the bare `char **history` and
  `char **forward` arrays in `src/pager.c` with `nav_state_t` (`{ href, top,
  cursor_line }`), recording the viewport and attention-line position before
  following any page or intra-page `#anchor` link and restoring them on back
  (`u`/Left Arrow) and forward (Right Arrow) navigation while keeping
  `selected = -1` so Right Arrow traverses forward history until a link is
  explicitly selected. (2) Added numeric and `all` depth level synonyms
  (`0`|`all` for all levels, `1`|`beginner`, `2`|`intermediate`, `3`|`expert`)
  in `src/depth.c`, `src/depth.h`, and `src/tam.c` across `--level` and
  `?level=` for `tam show`, `tam explain`, and `tam explain topics`, with
  matching Bash completions and `man/tam.1` documentation. (3) Fixed
  punctuation joining in `src/render.c` (`layout_flow`, `cell_wrap`, and
  `peek_following_punct`) so tokens starting with `,.;:!?%)]}` only attach
  without a space when no whitespace separated them from the previous token in
  the HTML source (preserving the space in `<code>git add .</code>` while
  keeping `<code>x</code>.` joined). Bumped command version to `0.26.0`.
- **2026-10-16:** Command 0.27.0. Retired legacy `1`–`9` hyperlink jumping in
  `src/pager.c` and bound keys `0`, `1`, `2`, `3` inside the interactive pager
  to switch the current page's reading depth (`all`, `beginner`, `intermediate`,
  `expert`), pushing navigation state onto history and reporting status when a
  page is already at the requested depth or does not offer it. Updated
  `man/tam.1` and `tests/run.sh`, and bumped command version to `0.27.0`.
- **2026-10-16:** Command 0.28.0 and Corpus 0.28.0. Added deterministic
  build-time Git derivation (`git_page_revision()`) in `tools/build-index.py`
  computing each page's commit count (`git rev-list --count HEAD --
  knowledge/<page>`) and latest commit timestamp (`git log -1 --format=%cd
  --date=format:%Y-%m-%d %H:%M -- knowledge/<page>`), storing `rev` and
  `updated` in `knowledge/index.tsv` and `knowledge/index.json` without editing
  HTML source files. Updated `src/index.c`, `src/depth.c`
  (`tam_depth_apply_rev()`), and `src/tam.c` (`read_corpus_page()`) so every
  corpus page displays `Rev <N> Last updated <YYYY-MM-DD HH:MM>` as a document
  footer before `</body>` and in `tam explain topics --verbose`. Bumped command
  and corpus versions to `0.28.0`.
- **2026-10-17:** Corpus 0.29.0. Added `knowledge/maintain/boot-recovery.html`
  to explain how firmware fallback entries, alternate kernels and initramfs
  images, snapshot boots, virtual consoles and external rescue media differ.
  Linked the page from the maintenance and guide indexes and the existing boot
  and emergency-recovery pages. Added the boot and rescue source-authority row
  to `docs/corpus-authoring.md`; regenerated the searchable index.
- **2026-10-20:** Command 0.33.0 and Corpus 0.35.0 (minor): Added native `tam tldr`
  command-example subsystem (`src/tldr.c`, `src/tldr.h`, `bin/tam-tldr`),
  discovery and A–Z/platform index browser (`tam:tldr`, `tam:tldr:list`,
  `tam:tldr:keyword`, `tam:tldr:open`), both-form option placeholder rendering
  (`[-s|--long]`), hyperlinked `See also:`/alias/`man:` cross-references, top
  breadcrumb bar navigation (`Guide > tldr pages > <cmd>`), and the AI-curated
  Tamlinux subset of English upstream `tldr-pages` under `knowledge/tldr/`
  with `tools/build-tldr-index.py`.


