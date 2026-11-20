# Independent knowledge corpus audit (corpus 0.75.0)

- **Date:** 2026-11-05
- **Time:** 17:42 to 17:52 -05:00
- **Stage:** Develop — independent, read-only audit under `docs/corpus-audit.md`.
  This report is the only authored file; no content fixes or version bump.
- **CLI Tool:** Codex CLI (`codex`) 0.166.0, checked from the running binary.
- **Model:** `gpt-6.1-sol`, confirmed by runtime detection.
- **Transcript:** Retained privately by the author.
- **Commits:** Audited `tam` at `5633418b1f5900f909ffc028f8353eb8d8fc8d40`;
  product authority `tamlinux` at `206d3d5b5b99f87818234833daae7cbc5d8a69f1`;
  packaging authority `tamlinux-packages` at
  `944cb241ec616b21f3dfa91ad7f42bcc49b4b093`. The report commit follows these.
- **Authorship:** The developer requested the audit. This session wrote none of
  the pages reviewed and performed the audit without delegation.
- **Checked against:** `knowledge/version` (0.75.0), `src/version.h` (0.48.2),
  `docs/corpus-audit.md`, `docs/corpus-authoring.md`, `docs/corpus-style.md`,
  `docs/corpus-pedagogy.md`, `AGENTS.md`, `ROADMAP.md`, `docs/plans/README.md`,
  `docs/plans/development.md`, the October 16 and November 2 audit records,
  `src/install.c`, `src/render.c`, `tools/report-component-coverage.py`,
  `knowledge/coverage.json`; product binding files, command implementations,
  `desktop/theme/engine.py`, `desktop/session/hypr/tamlinux/input.lua`, plugin
  manifests; packaging `docs/console-startup.md`. Findings give exact source
  locations below.

## Guiding prompt

> Audit the tam knowledge corpus. I believe agy, cursor, and opencode just did a bunch of work and now agy is suggesting that per corpus-audit.md, another AI agent should do an audit.

## Scope and conclusion

The required mechanical checks pass. The current corpus nevertheless contains
**12 factual findings: six High and six Medium**, concentrated in the recent
component-coverage expansion. Correct the actionable instructions and security
assurances before relying on these pages as teaching material. This is advisory;
the audit does not gate a candidate or authorize remediation.

Read all 19 new hubs added in the five expansion phases, including their expert
tables, and reviewed their related additions to power, networking, scripting,
AI and navigation pages. Rechecked earlier findings and sampled older boot,
session, ecosystem, glossary and learning-path material. Mechanical verification
covered all 582 indexed entries. This is not a sentence-by-sentence certification
of all 398 glossary pages or the entire older corpus, nor a learner mastery test.

The component map accurately reports 247 components: 176 covered, 71 exempt,
zero gaps. Its definition of coverage is a matching name in HTML
(`tools/report-component-coverage.py:126`), not verified behavior or sufficient
teaching. Several findings below occur on pages counted as covered.

References beginning `knowledge/`, `src/`, `tools/` or `docs/` are in `tam`.
References beginning `commands/` or `desktop/` are in the product repository at
the authority commit above. Line numbers refer to those audited revisions.

## Recheck of the previous audit

| Previous ID | Result this cycle |
| --- | --- |
| H1, Apps menu | Fixed: `knowledge/desktop/index.html:99` says Ctrl + F12; matches product `bindings/utilities.lua:2`. New, different shortcut defects are A01. |
| H2, F9 dictation | Fixed: `knowledge/desktop/voice.html:26`, `:53`, `:99` and the voice recipe teach hold/release; product `bindings/voxtype.lua` has press/start and release/stop. |
| H3, login | Fixed in the sampled SDDM, design-path and boot-phases pages: console-first plus explicit `tam-desktop`; supported by packaging `docs/console-startup.md:7`. |
| M1, Omarchy independence | Fixed: `knowledge/ecosystem/toolkits.html:64` states runtime independence is complete. |
| M2, stale version examples | Sampled `tam`, toolchains and package-versions captures are now dated examples. Older package-path illustrations remain; an example need not equal today's version. No new finding merely for an old number. |
| M3, First steps | Fixed: reader keys are explained and `pwd`, `ls`, temporary-directory creation and `cd` precede permissions (`knowledge/learn/paths/first-steps.html:19`, `:77`). |
| M4, path gaps | Fixed mechanically: the current learning coverage report shows no skill-cell page/path gaps. This does not establish mastery. |
| L1, repeated introductory rootstock text | Still appears on multiple hubs. Retained as the previous stylistic observation, not counted again as a new factual finding. |
| L2, Meta note status | Fixed: `docs/plans/corpus/keyboard-modifiers.md:3` says Completed and records delivered pages. |
| G1–G4, missing scaffolding | The matter/energy page, OS/session bridge pages, recipes, symptoms, FAQs and requested glossary terms now exist. Presence verified; the previous aspiration of first-principles expertise is not thereby validated. |

The older October 16 installer/worktree/status findings were also sampled:
`knowledge/install/index.html:24` correctly marks install execution planned;
`src/install.c:1191` and `:1216` still classify/reject those verbs accordingly.
The toolchains page now teaches shared clones and claims. Terminology no longer
presents Sway as the running compositor. The checker covers the earlier
entity/LaTeX/private-string defects; those searches were not repeated manually.

## Findings

| ID | Severity | Page | Finding |
| --- | --- | --- | --- |
| A01 | High | theming, displays, input | Four purported packaged shortcuts do different jobs or do not exist. |
| A02 | High | improve/plugins | Plugin walkthrough invents subcommands, output and a plugin ID. |
| A03 | High | hardware/drives | Drive inspection omits its required device argument and cannot perform the taught exercise. |
| A04 | High | maintain/troubleshooting | A real upgrade is presented as diagnostic verification. |
| A05 | High | hardware/security | Hardware authentication and PAM backup assurances exceed the implementation. |
| A06 | High | theming and other new hubs | Invalid or understated risk labels hide package changes and mislabel builds as read-only. |
| A07 | Medium | shell/tools, shell/editors, theming | Suggested help/recovery invocations do not have the documented behavior. |
| A08 | Medium | hardware/security | Docker predicate output and configured-versus-live semantics are wrong. |
| A09 | Medium | drives, displays | Worked output disagrees with formatter code and has no capture attribution. |
| A10 | Medium | toggles, notifications, config | Command tables confuse predicates with setters and misidentify notification/branding jobs. |
| A11 | Medium | hardware/input | Two claimed packaged binding files do not exist. |
| A12 | Medium | improve/scripting | Universal help/version and exit-stream guarantees are not product contracts. |

### A01 — Incorrect packaged shortcuts

- **Claims:** `knowledge/desktop/theming.html:57` assigns wallpaper cycling to
  Meta + Shift + W; `:58` assigns themes to Meta + Ctrl + T; the wrong wallpaper
  answer repeats at `:86`. `knowledge/hardware/displays.html:70` assigns the
  display menu to Meta + Shift + D. `knowledge/hardware/input.html:63` assigns
  touchpad toggling to Meta + Alt + T.
- **Authority:** `desktop/session/hypr/tamlinux/bindings/applications.lua:7`
  assigns Meta + Shift + W to a private browser; `:16` assigns Meta + Shift + D
  to the Docker TUI when preinstalled bindings are enabled. In
  `bindings/utilities.lua:106`, Meta + Ctrl + T opens btop. Background selection
  is Meta + Ctrl + Space (`:17`), themes Meta + Shift + T (`:18`), and Display
  Meta + Ctrl + D (`:102`). `bindings/media.lua:13` binds the hardware
  XF86TouchpadToggle key to `tam-toggle-input-device touchpad`; none of the six
  packaged binding files binds Meta + Alt + T.
- **Fix direction:** Use the actual bindings and describe background selection
  accurately instead of inventing a next-wallpaper shortcut. Correct the quiz
  answer with the instruction.

### A02 — Invented plugin operations and output

- **Claims:** `knowledge/improve/plugins.html:59` shows a plugin list including
  nonexistent `fred.bar`; `:69` teaches `tam-plugin status fred.clock`; `:77`
  teaches `tam-plugin dev link my-plugin`; `:90` recommends `tam-plugin dev reload`;
  `:118` says `tam-plugin test` runs isolated test suites. `:88` also sends the
  reader to the wrong systemd unit, `quickshell`.
- **Authority:** `commands/tam-plugin/tam-plugin:102` documents `list`, `info`,
  `dev <id> on|off`, `verify` and `restore`. Dispatch at `:683` has no `status` or
  reload verb; `:708` explicitly rejects `test`/`run` in favor of package testing.
  `:607` validates a `fred.*` ID and an `on|off` mode. Product manifests include
  the IDs listed on `knowledge/desktop/bar.html:97`, with no `fred.bar`.
  `commands/tam-restart-shell/tam-restart-shell:6` names `tamlinux-shell.service`.
- **Fix direction:** Rebuild the walkthrough from actual help, real captured
  output and the current developer-trial/package distinction. Use the actual
  shell unit for logs. Do not replace these invented verbs with guessed ones.

### A03 — Drive-info requires an argument

- **Claim:** `knowledge/hardware/drives.html:67` runs bare `tam-drive-info` and
  shows a disk inventory. The practice task at `:82` says it scans all devices.
- **Authority:** `commands/tam-drive-info/tam-drive-info:32` prints usage and
  exits 1 when no argument is supplied. Safe reproduction returned exactly
  `Usage: tam-drive-info [/dev/drive]`, exit 1. Drive enumeration is in
  `commands/tam-drive-select/tam-drive-select:35`; it is not the bare info command.
- **Fix direction:** Teach how to discover a real device name, then pass that
  name to `tam-drive-info`. Replace the output and corresponding practice task.

### A04 — Upgrade is not verification

- **Claim:** `knowledge/maintain/troubleshooting.html:82` directs a reader whose
  upgrade needs a restart to run bare `tam-workstation-upgrade` for “staged
  upgrade verification.”
- **Authority:** `commands/tam-workstation-upgrade/cli.py:220` defines `--check`
  as show-plan-only and `:221` defines offline receipt status. At `:243`, only
  the `--check` branch returns the plan; without it, `:251` invokes `apply_plan`.
  An apply flow can ask for confirmation; that does not make it verification.
- **Fix direction:** Separate diagnosis, plan inspection, receipt inspection,
  application and reboot. Use the appropriate explicitly non-applying mode for
  an inspection lesson, with its actual prerequisites and limitations.

### A05 — Unsupported authentication and recovery guarantees

- **Claims:** `knowledge/hardware/security.html:28` says keys/fingerprints prevent
  unauthorized access even when a password is known. At `:94` and `:113`, setup
  tools purportedly back up PAM/configuration state before modifying it. The
  same lesson recommends a passwordless sudo window at `:65` without explaining
  the scope of the grant.
- **Authority:** `commands/tam-setup-security-fido2/tam-setup-security-fido2:46`
  installs `auth sufficient pam_u2f.so`; its created Polkit stack at `:56`
  retains password authentication. Fingerprint setup at
  `commands/tam-setup-security-fingerprint/tam-setup-security-fingerprint:49`
  likewise adds a sufficient factor with password fallback (`:73`). These setup
  implementations directly edit PAM and contain no PAM backup operation.
  `commands/tam-sudo-passwordless/tam-sudo-passwordless:77` warns that the grant
  lets any process running as the user execute any root command without a
  password; expiry limits duration, not authority.
- **Fix direction:** Explain alternative authentication versus a required second
  factor, remove the nonexistent automatic-backup assurance, and describe the
  actual sudo grant before offering it as maintenance practice. A real backup
  feature would be separate product development, not a documentation correction.

### A06 — Risk labels understate mutations

- **Claims:** The package-install example at
  `knowledge/desktop/theming.html:60` uses `data-risk="low"`. The same unsupported
  value labels changes at `knowledge/hardware/displays.html:65`,
  `knowledge/hardware/drives.html:72`, `knowledge/improve/contributing.html:63`
  and `knowledge/maintain/troubleshooting.html:64`.
  `knowledge/improve/contributing.html:68` labels `tam-dev check tam` read-only.
- **Authority:** `docs/corpus-pedagogy.md:304` defines only `read-only`, `scratch`,
  `user`, `system` and `boot`; package changes require `system` or `boot`.
  `src/render.c:842` recognizes only those values, so `low` prints no risk label.
  The rendered font-install example indeed has no “[changes the system]” label.
  `commands/tam-install-font/tam-install-font:51` invokes `tam-pkg-add` and
  `tam-font-set`. `commands/tam-dev/tam-dev:462` creates state and `:482` builds
  before running checks; it is not a reads-only command.
- **Fix direction:** Use the documented risk vocabulary and classify each
  operation by actual effects. Preserve the distinction between writing user
  development artifacts and changing system packages. The linter currently
  accepts the unknown value and misses the wrapped font install.

### A07 — Wrapper help and palette-recovery instructions are false

- **Claims:** `knowledge/shell/tools.html:56` recommends `tam-launch-tui --help`
  and `tam-launch-webapp --help`; `knowledge/shell/editors.html:53` recommends
  `tam-default-editor --help` and `tam-launch-editor --help` as wrapper help.
  `knowledge/desktop/theming.html:72` recommends bare
  `tam-theme-colors-from-alacritty` to synchronize terminal colors.
- **Authority:** `commands/tam-launch-tui/tam-launch-tui:41` forwards its argument
  as the executable; `commands/tam-launch-webapp/tam-launch-webapp:43` forwards
  it as `--app=--help`; `commands/tam-launch-editor/tam-launch-editor:54` passes
  it to the selected editor. The default-editor setter rejects it at
  `commands/tam-default-editor/tam-default-editor:52`. The palette helper requires
  a theme directory at `commands/tam-theme-colors-from-alacritty/tam-theme-colors-from-alacritty:12`
  and generates a missing `colors.toml`; it is not a bare live-sync command.
- **Verification:** Isolated launch interception confirmed forwarding without
  opening any real application. Default-editor help and the bare palette helper
  both returned usage and exit 1.
- **Fix direction:** Document actual wrapper arguments and real discovery/help
  routes. Use the real theme recovery procedure rather than a conversion helper
  with no argument.

### A08 — Docker predicate semantics and output

- **Claims:** `knowledge/hardware/security.html:56` supplies a diagnostic message
  as output of `tam-sudo-docker`. At `:73`, `--configured` supposedly tests
  whether the daemon socket is configured and accessible.
- **Authority:** `commands/tam-sudo-docker/tam-sudo-docker:39` distinguishes current
  socket access from configured account groups after login. Its branches at `:51`
  are silent predicates: 0 means sudo is needed, 1 means it is not; `--configured`
  checks group membership, not socket reachability. Both audited invocations
  produced no output on the audit host.
- **Fix direction:** Teach the exit status and its polarity, and distinguish
  live access from configured future access. Remove the fabricated message and
  the inappropriate system-change label on the read-only probe.

### A09 — Worked output is incompatible with the implementation

- **Claims:** `knowledge/hardware/drives.html:60` shows `Model: Tamlinux
  Workstation` and `CPU: 8 cores`; `:74` shows a benchmark heading and colon-bearing
  `Write:`/`Read:` results. `knowledge/hardware/displays.html:60` shows `1.00`,
  while `:67` shows `1.25` as output of setting scale. These captures have no
  system/date attribution.
- **Authority:** `commands/tam-hardware/format.c:10` emits a CPU brand, separate
  physical/logical core counts at `:21`, and `Product:` at `:48`, not this format.
  `commands/tam-disk-speedtest/tam-disk-speedtest:148` prints `disk ...`; `:203`
  and `:232` print phase/rate, not the shown transcript. The scaling command's
  `normalize_scale` uses `%g` at
  `commands/tam-hyprland-monitor-scaling/tam-hyprland-monitor-scaling:98`;
  its `set_scale` function at `:102` performs the change and audit rather than
  printing the new scale. `docs/corpus-pedagogy.md:182`
  requires actual captures and system/date identification for variable output.
- **Fix direction:** Capture real output, identifying its environment; explicitly
  show a separate readback after a setter. Treat A03's failing drive command and
  A08's silent predicate as their own behavioral defects, not merely captions.

### A10 — Reference-table roles inferred incorrectly from command names

- **Claims and authority:**
  - `knowledge/desktop/toggles.html:76`, `:84`, `:85` describe
    `tam-toggle-enabled` and the Hyprland enabled/disabled helpers as setters.
    Their implementations end with file-existence predicates at
    `commands/tam-toggle-enabled/tam-toggle-enabled:31`,
    `commands/tam-hyprland-toggle-enabled/tam-hyprland-toggle-enabled:26`, and
    `commands/tam-hyprland-toggle-disabled/tam-hyprland-toggle-disabled:26`.
  - `knowledge/desktop/notifications.html:76` says `tam-notification-wait` waits
    for an event; its implementation at `commands/tam-notification-wait/tam-notification-wait:36`
    waits for server readiness. At `:77`, the page calls `tam-notification-time`
    a timing helper; `commands/tam-notification-time/tam-notification-time:31`
    sends a notification containing the current date/time.
  - `knowledge/desktop/config.html:113` says `tam-branding-about` queries branding
    and version metadata. `desktop/theme/engine.py:646` instead edits, resets or
    converts the about-dialog art; `:660` rejects other operations. It is not a
    version query.
- **Fix direction:** Describe exact jobs and predicate polarity from source;
  include required arguments where needed. A matching component name alone is
  not enough to approve its table row.

### A11 — Nonexistent input source files

- **Claim:** `knowledge/hardware/input.html:97` and `:98` identify packaged
  `bindings/keyboard.lua` and `bindings/touchpad.lua` as configuration authorities.
- **Authority:** The product binding directory contains only `applications.lua`,
  `clipboard.lua`, `media.lua`, `tiling.lua`, `utilities.lua` and `voxtype.lua`.
  `desktop/session/hypr/tamlinux/input.lua:50` begins the real input configuration;
  touchpad toggle bindings are in `bindings/media.lua:13`.
- **Fix direction:** Point to actual source files and separate XKB/input options
  from action bindings. Do not create new files merely to match the lesson.

### A12 — Unsupported universal command contract

- **Claims:** `knowledge/improve/scripting.html:142` says all product helpers must
  support `--help` and `--version` and operate cleanly in noninteractive pipelines.
  At `:150`–`:152`, success always has empty stderr, 1 always means runtime error
  with a diagnostic, and 2 always means usage error with a `--help` direction.
- **Authority:** The actual helper implementations cited in A07 do not expose
  that interface. A08 and A10 provide silent status predicates where exit 1 is
  a normal negative result. `commands/tam-drive-info/tam-drive-info:32` reports
  missing-argument usage on stdout and exits 1, another direct counterexample.
  `docs/corpus-authoring.md:34` requires a source for project choices/contracts;
  a proposed uniform future interface is not today's behavior.
- **Fix direction:** Teach per-command contracts and predicate semantics. Label
  any proposed normalization as a proposal until separately approved and
  implemented; do not impose a new product-wide policy through the corpus.

## Recorded-checks pass

Three recent session records were traced to their stated sources:

| Record | Result |
| --- | --- |
| `docs/ai/2026-11-04-tam-plan26-phase2-utilities.md:12` | Named binding files support clipboard/menu chords. They do not establish the launcher `--help` behavior. Source inspection yields A07; command inspection yields A10. |
| `docs/ai/2026-11-05-tam-plan26-phase3-hardware-config.md:11` | Named command implementations contradict drive-info and plugin walkthroughs (A02/A03), and formatter code contradicts example output (A09). Binding/config sources contradict A01/A11. |
| `docs/ai/2026-11-05-tam-plan26-phase5-maintenance-w5.md:11` | Crash/cache/dev tools exist and the coverage counts reproduce. The upgrade implementation contradicts verification wording (A04); the developer command writes build/state artifacts (A06). |

The Phase 4 record was also inspected: its named security implementations do
not establish automatic PAM backups (A05). Recorded claims that pages render
cleanly are compatible with these findings: successful rendering does not
validate the instructions. This review does not reconstruct earlier tool calls
or assert what an author actually read.

## Verification and correct material

- `timeout 240 make corpus-and-test` passed, exit 0, under a short exclusive
  claim. The linter, 35 linter tests, 22 index tests, six component-map tests,
  component gap check and all guide checks passed. Test timing summary: 18.56s,
  excluding command compilation and corpus/map generation. Generated tracked
  indexes remained unchanged.
- Coverage reports no skill-cell page/path gaps and five existing anatomy
  warnings across clipboard, notifications, toggles and shell tools/editors.
  Those mechanical warnings are recorded, not duplicated as factual findings.
- Read 12 source-selected renders using
  `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe --level LEVEL PAGE`:
  theming, input, displays, plugins, drives, security, troubleshooting, shell
  tools, contributing, windows, voice and SDDM. Eleven were Learner samples;
  windows was Expert. All exited 0. The errors above survive into reader text.
- Wrapper reproduction used temporary dependency stubs to intercept launching,
  never a real terminal/browser/editor. Privileged authentication, package,
  upgrade and desktop-setting examples were checked from source, not executed.
- The new desktop hub, hotkeys, transition and windows pages correctly teach
  Meta + Q close, Meta + T float, directional focus and the packaged bar toggle.
  The bar's nine `fred.*` IDs match the product manifests.
- Earlier console-first, F9 and installer-stub corrections survive. Network
  helper descriptions distinguish configured state, active state and recovery;
  the recent weather-location addition matches its helper interface. Power's
  `tam-nut inspect` reference correctly names a read-only bootstrap inspector.

## Follow-up

- **Corpus fixes, if directed:** A01–A12. Prioritize wrong actions, upgrade
  wording, authentication/recovery guarantees and package-risk labels; then
  recapture examples and correct reference tables. Re-render corrected pages
  and check the named source behavior rather than accepting name coverage.
- **Command/tooling fixes, if directed:** Validate the risk-value enumeration in
  `lint-corpus.py` and recognize relevant package-install wrappers. Broader helper
  help/version normalization would be a separately designed product change.
  No new command behavior is required to correct the current corpus.
- **Developer ruling:** Which findings to implement, as the audit SOP requires.
  No stage transition, installation, version allocation or remediation was
  performed or inferred from this audit request.

Outside the numbered corpus findings, maintenance references lag: `ROADMAP.md`
still identifies corpus 0.72.2, the development contract header names much older
versions, and `docs/corpus-authoring.md` says the component gap check is not yet
required although the audited Makefile now runs it. These should be reconciled
during separately directed follow-up; this audit has not changed them.

## Key decisions

Applied the audit SOP's report-only rule. The report carries its own provenance;
shared provenance indexes and product files were left untouched. No fixes were
folded into the review.
