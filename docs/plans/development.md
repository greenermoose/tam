# tam development plan

**Stage:** Develop — milestones 1–10 (command foundation, offline guide,
orientation and explanation, installation inspection, engine enhancements,
corpus index and search, content-depth ladder and fading templates, read-only self-inspection findings and check contract, opt-in grounded ask, navigation & output ergonomics), link navigation, hyperlink display fixes, in-pager text search,
reading telemetry, and static Bash shell completions implemented; the command is at 0.18.0. The executable
builds in C against an installed `libtam`, installs as `tam`, and renders the
bundled guide itself.

## Purpose and ownership

Give users one useful entry point to learn, maintain and improve Tamlinux,
including offline and before the graphical desktop is running.

This repository owns the C command, bundled knowledge, tests and component
versioning. `libtam` owns the reusable C library. The `tamlinux` product owns
workstation assembly, installed cultivar metadata and the installation
framework. Existing component tools retain their owners; document or call them
through deliberate subcommand contracts rather than moving every `tam-*`
helper into this repository.

The corpus's journey spine is the three tracks — **Use**, **Maintain** and
**Improve** — while lookup for people who already know the word is a separate,
cross-cutting topic index plus a search surface (the two-axis model). The
2026-10-12 `tam` help and self-inspection requirements are the authority for
milestones 6–9 below. The private decision ledger remains authoritative for the
reasoning; this repository carries the public, publishable projection. The
`why`/depth axes and a computer-science track are exploratory ideas only and are
listed under [Excluded and deferred work](#excluded-and-deferred-work).

## Repository layout and workstream lanes

The repository has two independently developable workstreams:

- **Command (`src/`, `bin/`, `completions/`)** — the C program that renders
  the corpus and links `libtam`.
- **Corpus (`knowledge/`, `tools/`, `tests/snapshots/`)** — the authored
  semantic HTML, the index generator, and the golden snapshots.

They are separate at the source and runtime levels: the binary loads its
corpus from `TAM_KNOWLEDGE_DIR` or the installed `DATADIR`, so a corpus edit
needs no recompile and a source edit needs no corpus change. Two agents may
edit the two lanes at once, each claiming only its lane's paths with
`tam-work` (workspace `AGENTS.md`). Shared integration files (`Makefile`,
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
- `make corpus` — regenerate and validate `knowledge/index.{tsv,json}`.
- `make test-command` / `make test-corpus` — one lane's checks.
- `make test` / `make check` — the full, integrated suite.

## Command contract

| Invocation | Status | Behavior |
| --- | --- | --- |
| `tam` | Implemented | Concise introduction and starting points; exit 0 without launching a pager or writing state. |
| `tam --help` | Implemented | Concise usage and implemented commands; works without guide data or cultivar metadata. `tam help` is the same invocation. |
| `tam --version` | Implemented | Separate command version and installed Tamlinux cultivar version; report unavailable cultivar metadata explicitly. |
| `tam show [PAGE]` | Implemented | Open the local guide in the built-in pager when interactive; readable text when piped or not a terminal. Preserves path-based semantics and does not consult the topic index. (Replaces `tam browse`; `browse` remains an explain topic.) |
| `tam welcome` | Implemented | Explicit orientation; record welcome only after successful display. |
| `tam explain` | Implemented | Concise usage overview without spewing the full topic catalog (keeps output within the 24-line screen budget; ≤15 lines). |
| `tam explain <topic>` | Implemented | Match curated local topics; clear miss message and useful next step. Never execute topic input. From milestone 6, multi-word queries resolve through `tam search`. |
| `tam explain topics` | Implemented | Interactive hyperlinked topic browser in pager (or clean text when piped); supports `--level LEVEL`, `--terse` (slugs only for pipes/scripts), and `--verbose` (exhaustive keywords/summaries). |
| `tam man [SECTION] PAGE` | Implemented | System manual page, converted from roff to semantic HTML with hyperlinked cross-references; a surface separate from the corpus. |
| `tam install inspect` | Implemented | Read-only host facts and recognized installation capabilities. Grows into the read-only findings layer at milestone 8. |
| `tam install` `plan`/`apply`/`verify` | Not implemented | Unimplemented installer verbs report truthfully and exit non-zero; they follow the product installation framework. |
| `tam search [--json] TERM...` | Implemented | Single search verb over the corpus index. Returns a ranked list of pages; `--json` for scripting. |
| `tam ask <question>` | Implemented | Opt-in, grounded question answering over the corpus with mandatory citations; never starts a daemon. |

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
place where the offline default is amended.

## Implementation and packaging

Use C with `libtam`, following the selected Tamlinux language direction.
Keep the library separate; record its source revision and supported version
when introducing the dependency. Begin with a small Makefile and documented
development dependency paths. Builds must not modify a sibling library
checkout as a hidden side effect.

Ship semantic HTML with relative local links and matching readable text.
Start with the guide index, welcome, installation overview, shell pipelines
and man-page basics. Keep HTML/text parity checked as the corpus grows.
Verified component behavior supplies later plugin and AI pages.

Author topic metadata in the pages themselves and project it into a small
corpus index artifact at build time: each page declares a title, a one-line
summary and its keywords, plus authoring-only level, track and prerequisites.
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
  (a new subcommand, a new output mode, new bundled content). While the major
  is `0`, new features take the minor bump.
- **Patch** (`0.y.z` → `0.y.z+1`): backwards-compatible fixes and internal
  changes with no new capability.
- **Major**: reserved; not used before `1.0.0`.

Update the `--version` assertion in `tests/run.sh` to match, and note the
bump in the decision record below. Historical `docs/ai/` records keep the
version that shipped at the time; do not rewrite them.

## Milestones

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

Milestones 6–10 derive from the 2026-10-12 `tam` help and self-inspection
requirements. The reasoning still belongs in the private decision ledger; this
plan is the public projection and is revised when those decisions are recorded.

The first slice developed independently of desktop migration. Later installer
plan/apply/verify work follows the product framework.

## Excluded and deferred work

- **Installer `plan`/`apply`/`verify`.** Follows the product installation
  framework; still deliberately unimplemented here.
- **Stateful progression UI (`tam learn`).** Design only for now; it depends on
  the milestone-8 inspector as its oracle and would use opt-in XDG state on the
  `welcome_seen` pattern.
- **`why` and depth axes, and a computer-science track.** Exploratory ideas; by
  Fred's 2026-10-09 priorities they follow the "Use" foundation rather than pre-empt it. Not committed
  scope.
- **Fuzzy auto-redirect.** Rejected: fuzzy matches are shown, never applied.
- **Compound shell parsing, `tam ai` hardware estimates, writable settings and
  localisation.** Separate scope, each with its own review.
- **External browser or pager.** Dropped: the native renderer and pager are
  self-sufficient with zero external dependencies.

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




