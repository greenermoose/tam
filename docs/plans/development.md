# tam development plan

**Stage:** Develop — milestones 1–4 (command foundation, offline guide,
orientation and explanation, installation inspection) implemented. The
executable builds in C against an installed `libtam` and installs as `tam`,
rendering the bundled guide itself.

## Purpose and ownership

Give users one useful entry point to learn, maintain and improve Tamlinux,
including offline and before the graphical desktop is running.

This repository owns the C command, bundled knowledge, tests and component
versioning. `libtam` owns the reusable C library. The `tamlinux` product owns
workstation assembly, installed cultivar metadata and the installation
framework. Existing component tools retain their owners; document or call them
through deliberate subcommand contracts rather than moving every `tam-*`
helper into this repository.

## First command contract

| Invocation | Behavior |
| --- | --- |
| `tam` | Concise introduction and starting points; exit 0 without launching a browser or writing state. |
| `tam --help` | Concise usage and implemented commands; works without guide data or cultivar metadata. `tam help` is the same invocation. |
| `tam --version` | Separate command version and installed Tamlinux cultivar version; report unavailable cultivar metadata explicitly. |
| `tam show` | Open the local guide in the built-in pager when interactive; readable text when piped or not a terminal. (Replaces `tam browse`; `browse` remains supported as an explain topic). |
| `tam welcome` | Explicit orientation; record welcome only after successful display. |
| `tam explain <topic>` | Match curated local topics; clear miss message and useful next step. Never execute topic input. |
| `tam install inspect` | Read-only host facts and recognized installation capabilities, through the product installation contract. |

`--pipe` explicitly requests text. Redirected output contains no terminal
controls or browser prompts. Bare `tam`, `show`, help, version and install
inspection never write welcome state. Successful explicit `welcome` uses
`$XDG_STATE_HOME/tamlinux/welcome_seen`, falling back to
`~/.local/state/tamlinux/welcome_seen`.

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
   `libtam` (see
   [the toolchain report](../../../improvements-fred-tamlinux/reports/2026-10-09-tam-c-toolchain-and-build.md)).
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
5. **Engine enhancements, table layout, and dynamic input (milestone 5 — planned).**
   Add standard input reading (`tam show -`), definition lists (`<dl>`, `<dt>`,
   `<dd>`) and space-padded table layout (`<table>`, `<tr>`, `<td>`, `<th>`) to
   `src/render.c` (D133). Prioritize the "Use" track first in the knowledge corpus.
   Prepare for dynamic hyperlinked man page traversal.

The first slice can develop independently of desktop migration. Later
installer plan/apply/verify work follows the product framework. Compound
shell parsing, AI estimates and writable settings retain separate scope.

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
  resolution, control-sequence-free piped output and argument errors. Toolchain
  options and the packaging path are recorded in
  [`2026-10-09-tam-c-toolchain-and-build.md`](../../../improvements-fred-tamlinux/reports/2026-10-09-tam-c-toolchain-and-build.md).
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
- **2026-10-09:** Knowledge architecture, man page strategy, and priorities (D133).
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




