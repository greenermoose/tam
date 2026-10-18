# Check, upgrade and switch tam source versions

**Plan status:** Completed
**Reviewed:** 2026-10-18
**Scope:** Four-stage version discovery, source installation and switching.
Implementation: command 0.31.0 / corpus 0.32.0; baseline was 0.30.0 / 0.31.0.
**Authority:** The developer requested source installation and Development/Test/Run/
Release switching, approved the development prefix and paired release tags, and
clarified independent code-based versions with no renumbering on installation.
The developer accepted the complete plan on 2026-10-18; see the
[approval record](../ai/2026-10-18-tam-update-check-plan.md#approval-and-handoff).
**Next action:** None for source implementation or local activation. Package
Test/Run/Release remain separate developer-directed stages.
**Implementation commits:** `f721120`, `5a84294`, `627c126`, `4afdea5` and
`5d5c128` (source, tests and acceptance follow-up); `09101b1` records the
handoff.

Follow [plan maintenance](../plan-maintenance.md). This plan owns the new
upgrade feature; the completed knowledge-system plan remains historical.

## Problem and recommended outcome

The user can see the version running with `tam --version`, but cannot discover
newer versions, install from source, or switch between software stages.
Use `tam upgrade` for checking and forward updates, and `tam switch` for
intentional stage/version selection, including downgrades.

| Invocation | Approved behavior |
| --- | --- |
| `tam upgrade` | Check all four stages; show running versions, selected stage, available versions and next commands. |
| `tam upgrade --check` | Explicit spelling of the same check. |
| `tam upgrade --channel STAGE [--json]` | Check one stage; optional structured output. |
| `tam upgrade --json` | Structured results for all four stages. |
| `tam upgrade --apply [--channel STAGE]` | Preview, confirm, build and activate a forward update on the tracked stage; explicit channel selects it if no stage is recorded. |
| `tam switch STAGE` | Preview, confirm, build and install the selected stage from source, allowing an explicitly confirmed downgrade. |
| `tam switch release --version X.Y.Z --corpus-version A.B.C` | Select and pin an existing exact release pair. |
| `tam switch --rollback` | Confirm return to the previous local source generation or recorded packaged fallback; no network. |
| `tam upgrade --help` / `tam switch --help` | Explain network, source builds, stages, destination, prerequisites and recovery; work offline. |

The CLI stages are `development`, `test`, `run`, `release`; accept `develop`
as an alias for `development`. Installation/rollback require confirmation in a
terminal, default no. Reject conflicting actions, unknown arguments, JSON with
mutation, and version-selection flags except for `switch release`. Global `tam --version`
remains unchanged; the subcommand parser handles the release selection option.
A non-interactive mutation fails before network or writes. There is no separate
`update` alias, unattended `--yes`, arbitrary remote/ref, system-wide install,
automatic downgrade or background updater in the first scope.

Example only; numbers do not claim current remote availability:

```text
Running:     tam 0.30.0 / corpus 0.31.0 (source abc1234)
Release:     command 0.29.0 / corpus 0.30.0 — running versions ahead
Run:         command 0.30.0 / corpus 0.31.0 — selected stage
Test:        command 0.31.0 / corpus 0.31.1 — feature complete; bug testing
Development: command 0.32.0 / corpus 0.32.0 — feature work in progress

Build and switch to development: tam switch development
```

Keep normal output within 24 lines, without a pager. Failed/unavailable stages
remain visible. Report command and corpus versions separately; a newer command
and a newer corpus are separate results. Normal checks need not compare libtam
or the whole Tamlinux product; installation receipts record all dependencies.

## Stages, versions and selection

| Stage | Meaning supplied by the developer | Source selector |
| --- | --- | --- |
| Development | The feature list is still being worked on. | `develop` branch |
| Test | Feature list believed complete; looking for bugs. | `test` branch |
| Run | Believed reliable for daily use. | `main` branch |
| Release | A noteworthy Run version worth upgrading to, pegged to a specific release. | Exact full release tag and resolved commit |

Releases are sparse: a Run version need not become a release. These commands
select existing versions locally; they never move stage branches, create tags
or releases, or infer reliability from numeric version order. Missing stages
are unavailable, never silently substituted with development.

**Developer's version rule:** version numbers follow code changes, independently
for command and corpus. Every newly prepared code build for the developer must
receive a version bump; do not deliver changed code under a reused version.
Stages do not determine the number or require stage suffixes. Preserve both
existing version numbers when compiling/installing an already numbered source
commit, including an older release selected with `switch`. A stage promotion
alone is not a code build. Full source SHAs distinguish candidates independently
of version labels. Keep existing minor-for-feature/patch-for-fix rules and
allocate numbers from the then-current files, not this proposal.

A receipt records the chosen stage, tam/corpus versions, full tam and libtam
SHAs, and selection policy. Branch stages track their branch but resolve one
immutable commit per install. `switch release` tracks latest full releases;
`switch release --version X.Y.Z --corpus-version A.B.C` pins that pair. `upgrade --apply` on a pinned
release reports the pin and requires an explicit new `switch release` to leave
it. An unknown/package-managed selection requires an explicit stage for apply;
never infer it from equal version text. Several stages may identify one commit.
Switching policy at the same owned commit updates the receipt after confirmation
without rebuilding the identical generation.

Switching covers tam, its matching corpus and private libtam, not the desktop or
OS. Whole-product selection still belongs to the assembly/deployment tool.

## Baseline and dependencies

Verified in source on 2026-10-18 at `b131bab`:

- `src/version.h` defines command 0.30.0; `knowledge/version` is 0.31.0.
- `src/tam.c` owns dispatch, help and version reporting. `TAM_BUILD` from the
  Makefile is a display label derived from `git describe`, potentially dirty;
  it is not a reliable full source revision and packages may omit it.
- The Makefile builds C against separately installed libtam, Python builds the
  corpus index, and installation copies the command, wrappers, guide, man page
  and Bash completion. `PREFIX`, `LIBTAM_PREFIX` and `DESTDIR` are supported.
  The default prefix is `/usr/local`; never use that default for this feature.
- libtam is a separate C project with its own build/test/install targets and
  D-Bus build dependency. tam also needs zlib and ncurses/terminfo development
  inputs. The current package assembly pins tam and libtam independently.
- No command-owned source dependency lock exists. `src/ask.c` has a local HTTP
  client, not a reusable HTTPS client or strict JSON parser. libtam's current
  subprocess helper captures output without a byte limit; do not assume it
  supplies the complete bounded transport contract below.

Implementation allocated command 0.31.0 for the new command behavior and
corpus 0.32.0 for the existing guide page update. Rebuilding older selected
sources preserves the versions recorded in those sources.

## Version discovery

Use the fixed public repository `greenermoose/tam`; no account or token is
required. An explicit check is consent to the documented GitHub metadata
requests, including in pipes. Existing help/version/browsing stay offline.
Do not read credentials or invoke an authenticated `gh` session.

1. Release: enumerate Git release tags with the Git refs API. The developer
   approved tags containing both independently versioned components, for example
   `tam-0.31.0-corpus-0.32.1`. Require complete numeric versions and validate
   both against `src/version.h` and `knowledge/version` at the resolved commit.
   Ignore floating/prerelease tags. For compatibility, accept historical
   `X.Y.Z`/`vX.Y.Z` tags only after deriving and validating their version pair
   from source; conflicting aliases for a pair are an error. Order latest by
   command SemVer, then corpus SemVer, with comparisons still reported separately.
   An explicit pair selects its exact tag; command-only selection is allowed
   only if one release pair matches, otherwise list choices and require the
   corpus version. Resolve annotated tags with bounded traversal.
   Verify release ancestry on main before offering installation; a full tag
   outside Run history is invalid release metadata. A GitHub Release may supply
   notes, but is not required: the maintainer's release tag establishes identity.
   Never substitute main or develop when a release is absent. Exhaust the refs
   response within the byte/ref limits (maximum 4096 refs); report incomplete
   discovery rather than calling a truncated set latest.
2. Development/Test/Run: resolve their branch to a full commit SHA, then read
   `src/version.h` and `knowledge/version` at that exact SHA. Fetching by that SHA prevents
   branch movement between requests from mixing version and source identity.
3. Running: compare the embedded `TAM_COMMAND_VERSION` in this process and
   the corpus version resolved by existing runtime discovery. Report corpus
   origin/overrides so a separately selected corpus is not mistaken for the
   installation receipt. Add
   separate full-source-revision and dirty metadata, preserving existing
   `--version` text compatibility. Do not run another `tam` from PATH or read
   a nearby checkout to infer the running command's version.

Use numeric SemVer comparison, including prerelease ordering and ignoring build
metadata. Validate lengths and numeric overflow; never compare strings or use
`sort -V`. Classify each channel as newer, same version, running ahead, absent,
or error. Track revision identity separately:

- Same version and same known clean commit: already this source.
- Same version with a different commit: version unchanged, revision differs;
  this alone does not prove a newer command. Permit an explicit confirmed
  source switch, explaining this distinction.
- Missing/dirty local revision: revision comparison unknown; numeric version
  comparison still works. Do not claim fully up to date from that evidence.
- Remote component version older than running: show running ahead and refuse ordinary
  upgrade application. `switch` permits an explicitly confirmed older remote
  version/stage; rollback selects an older local managed generation.

Recommend an optional runtime `curl` dependency for these operations, invoked
with argv directly from C. The checker remains available without Git or build
tools. Disable curl config/netrc use, require verified HTTPS, reject redirects,
set connect timeout 3 seconds and a shared total check deadline of 30 seconds,
and cap response bodies at 1 MiB, headers at 32 KiB and diagnostics at 8 KiB.
Enforce caps while reading, including chunked responses; terminate/reap the
child on timeout/overflow and cancellation. Never execute response text or
print remote control characters. Use a strict bounded JSON parser with nested
field/type validation; do not copy the substring parser in `ask.c`.

Use curl without automatic retries or a persistent cache in this scope. Show
missing curl, DNS/TLS failure, timeout, malformed metadata and rate limits as
failures, never as current. A release/stage 404 is absent only after repository
availability is established by another successful request (or an explicit
repository lookup); otherwise it is ambiguous and remains an error.

Check exit codes: 0 means all requested channels were checked successfully
(including confirmed absent releases/stages); 1 means any channel failed, while
preserving successful channel results; 2 means invalid invocation. Availability
does not change the exit code. JSON schema version 1 includes running identity,
per-stage command/corpus version and comparison, revision, error code, and
`update_available` as true, false or null when unknown. Either component being
newer counts as an available update; `upgrade --apply` refuses if either
component would be downgraded and directs the user to explicit `switch`.
Exit 1 also emits a complete JSON result;
human diagnostics go to stderr. Test automation reads channel status rather
than inferring freshness from process success.

## Source installation and recovery

**Destination approved by the developer:** use the established user
development prefix `~/.local/dev` as an explicit development override. It
takes precedence over the packaged command in terminal PATH. This operation
does not promote Test/Run/Release. Product delivery still belongs to a recorded
`tamlinux-packages` assembly. Remove the override through the approved component
workflow when the assembly delivers that version; never run a blanket uninstall
over unrelated development tools.

The source installer needs a transaction, not `git pull && make install` in
the developer's working clone:

1. Resolve the selected channel to a full SHA. Preflight compiler, make,
   pkg-config, Python, Git, curl and required headers/libraries; give an
   actionable missing-dependency list. Never install OS packages automatically.
2. Fetch `source-build.json` from that exact tam commit. Introduce this small
   versioned manifest in the implementation: a full compatible libtam commit,
   its expected version and required build dependencies. Repository URLs and
   commands stay fixed in code; the manifest cannot supply executable commands.
   Never select latest libtam independently. For existing older stage heads and
   releases without this new file, ship an audited compatibility table keyed by
   full tam SHA, with exact libtam SHA and supported build layout. Populate it
   from verified package/build evidence, covering the extant Test/Run/Release
   selections at implementation time. Missing records stop installation with
   an explanation; do not invent compatibility or silently remove old-stage
   switching from scope. The installed updater orchestrates and validates even
   when the selected older tam lacks the upgrade subcommands.
3. Show current/target versions, channel, full tam/libtam revisions, intended
   destination and rollback target, plus the fact that building runs the selected
   upstream Makefiles as this user. Freeze that previewed selection afterward.
   For acceptance builds in a disposable temporary home/prefix, Fred has granted
   standing authorization to fetch the pinned sources, run their checks and
   activate only inside that temporary prefix; agents should not interrupt him
   for per-build confirmation. The command continues to preview and require
   confirmation for ordinary switches into the user's configured prefix.
4. Use a private temporary build area and detached source snapshots fetched
   from the fixed repositories at those SHAs. Do not create worktrees, copy,
   reset, pull, switch or write into existing developer clones. Disable Git
   hooks/submodule recursion and do not run repository-configured commands.
   Apply a source-operation deadline and size limits separately from metadata
   checks; a failed transfer never proceeds to the build.
5. Build a private libtam and tam into one fresh generation under
   `~/.local/dev/opt/tam/<generation>/`. Its library and corpus stay together;
   the shared `~/.local/dev/lib/libtam.so` and other tools are untouched.
   Set the final generation prefix and rpath at build time. Do not rename a
   completed generation to a different absolute path after building it.
6. Run libtam checks and tam's command/corpus checks against the selected
   sources, with `TAM_KNOWLEDGE_DIR` explicit. Require tests to pass; provide
   a deterministic way to skip optional live Ollama integration. Stream bounded
   logs to an owned state file; report the failing step. Then smoke-check the
   installed generation by absolute path with corpus overrides cleared.
7. Record a manifest with source SHAs, versions, stage/pinning policy, dependency
   pins, installed files, checks, timestamps and prior target. fsync the completed record before
   activation. Expose the command and adjacent wrappers, man page and completion
   through owned links under the development prefix to one managed `current`
   pointer. Switch that pointer atomically after every check passes.
8. Refuse pre-existing unowned files at any endpoint, including old manual
   development installs; explain the conflict and leave them untouched. Use an
   install lock, record initial link creation, and recover interrupted initial
   activation idempotently. Failed later updates leave the prior target active.
9. Verify the activated command, bundled corpus and library. Print the actual
   version/path and `hash -r` guidance; a child cannot clear its parent shell's
   command cache. Do not claim an application/menu PATH changed automatically.
10. Preserve the previous generation. `switch --rollback` uses local recorded state,
    requires no network, and switches back atomically. On first-install rollback,
    remove only recorded owned exposure links to reveal the existing packaged
    command; verify that fallback before proceeding. Keep an absolute recovery
    command using the retained previous binary in the receipt, so rollback is
    possible even if the newly active command fails to start. Keep an installer-
    owned recovery launcher beside the retained updater generation: selecting
    an older tam without `upgrade`/`switch` must not strand the user. Print its
    absolute path and document using it to switch forward or roll back; do not
    add a new binary into the old source tree or mislabel its version.

Installation returns 0 only after activation verification, 1 for operational
failure, and 2 for invalid invocation. A declined confirmation exits 0 with
"Cancelled; installation unchanged". Non-interactive mutation exits 1 before
network or writes. Source/rollback actions serialize on the same lock. Retain
receipts and one previous generation; automatic deletion of old generations
or old hand-built overrides is outside this first implementation.

## Implementation and handoff

The developer accepted this design on 2026-10-18. Implementation is in the
shared clone on develop under path claims, following `AGENTS.md` and the
[implementation handoff](../handoffs/tam-upgrade-luna.md).

- [x] **TCK6 (version discovery and comparison):** Added `src/upgrade.c/.h` and
  any small transport/parser modules; pure version comparison, channel metadata,
  running revision metadata and bounded fixed-host transport. Evidence: 12
  deterministic upgrade tests cover semantic versions, strict metadata,
  partial channel failure, bounded HTTP, release compatibility and CLI offline
  behavior. Live check (2026-10-18, after `627c126` removed a trailing slash
  that made GitHub answer 404 for the repository root): development and test
  resolved with exact SHAs and versions; release is absent; run reports an
  error because `main` (0.9.1) predates `knowledge/version`.
- [x] **TCK7 (source build and transactional activation):** Added the versioned
  source-build manifest, audited compatibility pins, isolated generations,
  receipts, locking, owned links, validation, retained recovery helper and
  offline rollback. Evidence: the fake-HOME end-to-end suite covers activation,
  failure preservation, endpoint conflicts, lock refusal and rollback. A real
  isolated source build against tam `5d5c128804e9d5d754d9d21f94dd6feb6063fc24`
  and libtam `c081708b5ed2c8ade4ecdf171cebc110c2cb316f` passed all source tests,
  installed and verified the temporary generation, and wrote its receipt.
  `tam --version` reported 0.31.0 / 0.32.0 / libtam 0.2.0, and the bundled
  welcome page rendered from outside the build tree. The staged-install test
  now isolates XDG paths and matches current guide text. Fred explicitly
  directed the first activation in `~/.local/dev`; `make install` installed
  command 0.31.0 / corpus 0.32.0 from source commit `33e8e1f`, and
  `tam --version` verified both versions and the development prefix on `PATH`.
- [x] **TCK8 (CLI, documentation and delivery handoff):** Wired both commands,
  dispatch/help, static Bash completion, manual, README and `knowledge/tam.html`; document the
  temporary development override and bootstrap. Independent versions are
  command 0.31.0 / corpus 0.32.0. This plan, index, roadmap, contract and
  provenance are synchronized. Evidence: CLI tests and the complete
  offline command/corpus test suite. Live metadata resolved Development and
  Test; Run remains unparseable because its older `main` lacks
  `knowledge/version`.

Likely shared paths: `Makefile`, `tests/run.sh`, a new focused upgrade test
runner and fixtures, `src/version.h`, `src/tam.c`, `completions/bash/tam`,
`man/tam.1`, `README.md`, planning/provenance files and the named corpus page.
Claim only the slice in progress. Whole-tree builds/tests require an exclusive
claim; do not mutate adjacent libtam/product/package checkouts as a side effect.

**Bootstrap:** the running older tam cannot upgrade itself until this capability
has first been delivered. The implementation handoff must include the approved
component-development build procedure (or exact packaged candidate), prerequisite
list and rollback for installing this first version. Do not pretend the new
command already exists in the older binary. Planning alone installs nothing.

## Acceptance and limits

Use deterministic fixtures/fake transport with no live GitHub dependency in
the normal test suite. Test actual process output and persistent filesystem
state, not just internal helpers.

| Scenario | Required evidence |
| --- | --- |
| Older/equal/newer command and corpus; 0.9 versus 0.10; prereleases | Correct numeric order, independent component/stage results and exit codes; a corpus-only update is discoverable. |
| Same version/different SHA; dirty/unknown running SHA | Honest revision status; no claim that a different SHA proves newer command code. |
| Four stage mappings; paired and legacy tags; ambiguous pin; moving branch | No fallback masquerading as release; one resolved SHA used throughout. |
| Partial/all stage failures, TLS/DNS/rate limit, missing tools | Visible partial/unknown results, valid JSON, bounded time/memory and no installation. |
| Help and malformed/contradictory arguments | No network, writes, build or prompts before validation. |
| Source selection accepted/declined/non-interactive | Preview equals installed commit; cancellation and non-interactive calls leave installation intact. |
| Build/test/copy/smoke-check failure or interruption | Previous command/library/corpus still usable; receipt/log identifies failed stage. |
| Two simultaneous installs; unowned endpoint; broken pointer | Locking/refusal/recovery preserves user files and selects one complete generation. |
| Stage downgrade, pinned release, same-commit policy switch; install then offline rollback | Correct selected stage/pin and preserved source versions; absolute-path verification and previous generation/fallback restored. |
| Paths with spaces, malicious metadata, oversized/chunked data | Safe argv, bounded parser/transport, no injected commands or terminal controls. |
| Normal command browsing after installation | No network; installed corpus and private libtam resolve outside build directory. |

No implementation tests or installations were performed during planning. The
implementation, full source tests, isolated source build, live metadata check,
and developer-directed first activation in the real development prefix are
complete. The `run` source remains unparseable because `main` predates
`knowledge/version`; this is reported truthfully and is not a blocker to the
implemented checks. Package Test/Run/Release remain separate actions.

## References and disposition

The local source files above establish current behavior. The network design uses
GitHub's [Git references API](https://docs.github.com/en/rest/git/refs),
[annotated tag API](https://docs.github.com/en/rest/git/tags),
[commit API](https://docs.github.com/en/rest/commits/commits#get-a-commit),
[content API](https://docs.github.com/en/rest/repos/contents#get-repository-content)
and [curl options](https://curl.se/docs/manpage.html), consulted 2026-10-18.
Live tam release/develop API responses were unavailable through the research
tool; this plan makes no claim about the current remote release or branch head.

Closure evidence: TCK6–TCK8, the isolated real source build, live metadata
check, and explicit first activation are recorded above. Command 0.31.1 / corpus
0.32.0 is now installed in the development prefix after the breadcrumb fix;
package-stage acceptance remains developer-directed.
