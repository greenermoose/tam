# tam development plan

**Stage:** Develop — local repository scaffold. No command implementation,
installation, publication or release yet.

## Purpose and ownership

Give users one useful entry point to learn, maintain and improve Tamlinux,
including offline and before the graphical desktop is running.

This repository owns the C command, bundled knowledge, tests and component
versioning. `libtam` owns the reusable C library. The `tamlinux` product owns
workstation assembly, installed product metadata and the installation
framework. Existing component tools retain their owners; document or call
them through deliberate subcommand contracts rather than moving every
`tam-*` helper into this repository.

## First command contract

| Invocation | Behavior |
| --- | --- |
| `tam` | Concise introduction and starting points; exit 0 without launching a browser or writing state. |
| `tam --help` | Concise usage and implemented commands; works without Lynx, guide data or product metadata. |
| `tam --version` | Separate command version and installed Tamlinux product version; report unavailable product metadata explicitly. |
| `tam browse` | Open the local guide in Lynx when interactive; readable text when piped or Lynx is unavailable. |
| `tam welcome` | Explicit orientation; record welcome only after successful display. |
| `tam explain <topic>` | Match curated local topics; clear miss message and useful next step. Never execute topic input. |
| `tam install inspect` | Read-only host facts and recognized installation capabilities, through the product installation contract. |

`--pipe` explicitly requests text. Redirected output contains no terminal
controls or browser prompts. Bare `tam`, `browse`, help, version and install
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

Resolve knowledge and product metadata through packaging configuration,
with documented development overrides. Do not assume one fixed `/usr/share`
path or a sibling source checkout at runtime. Support staged installation
with `PREFIX` and `DESTDIR`; workstation integration pins the command and
library separately. The package and XDG namespace stay `tamlinux` even
though the executable and source repository are named `tam`.

Lynx is optional. Interactive browsing must use its terminal directly;
do not send it through a subprocess helper that captures output or imposes
a short command timeout. Local guides must open without fetching network
content or automatically installing anything. Show base-appropriate Lynx
installation guidance when it is missing.

## Milestones

1. **Command foundation.** Implement bare introduction, help, version and
   argument errors; establish the build and explicit library dependency.
2. **Offline guide.** Add corpus lookup, `browse`, `--pipe`, optional Lynx
   integration and text fallback. Verify a relocated staged installation.
3. **Orientation and explanation.** Add explicit `welcome`, successful-display
   state and curated `explain` topics.
4. **Installation inspection.** Connect read-only `install inspect` to the
   product contract; do not pretend unimplemented installer operations work.

The first slice can develop independently of desktop migration. Later
installer plan/apply/verify work follows the product framework. Compound
shell parsing, AI estimates and writable settings retain separate scope.

## Acceptance evidence

- Bare invocation exits 0, prints useful starting points and never launches
  Lynx or creates state, including with terminal input/output.
- Piped output and `--pipe` are readable and contain no control sequences.
- Missing Lynx falls back to text; browser failure does not mark welcome as
  shown. A successful welcome records state under an isolated XDG home.
- Missing corpus or product metadata produces useful, truthful diagnostics;
  help remains usable.
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
