# Tam corpus audit remediation (corpus 0.75.1)

- **Date:** 2026-11-05
- **Time:** 18:00 to 18:45 -05:00
- **Stage:** Develop — corpus 0.75.1 audit remediation (A01–A12)
- **CLI Tool:** Antigravity CLI 1.5.2, checked from the running binary (`agy --version`).
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Content and index commit for corpus 0.75.1
- **Authorship:** Fred directed the remediation of findings A01–A12 from the independent audit report. The AI tool verified each finding against product sources in `tamlinux`, edited all affected corpus pages, bumped the patch version, updated `ROADMAP.md`, regenerated indexes, and verified test suites without delegation.
- **Checked against:**
  - `docs/ai/2026-11-05-tam-corpus-audit.md` (audit report, preserved intact)
  - `tamlinux/desktop/session/hypr/tamlinux/bindings/applications.lua`
  - `tamlinux/desktop/session/hypr/tamlinux/bindings/utilities.lua`
  - `tamlinux/desktop/session/hypr/tamlinux/bindings/media.lua`
  - `tamlinux/desktop/session/hypr/tamlinux/input.lua`
  - `tamlinux/commands/tam-plugin/tam-plugin`
  - `tamlinux/commands/tam-restart-shell/tam-restart-shell`
  - `tamlinux/commands/tam-drive-info/tam-drive-info`
  - `tamlinux/commands/tam-drive-select/tam-drive-select`
  - `tamlinux/commands/tam-hardware/format.c`
  - `tamlinux/commands/tam-disk-speedtest/tam-disk-speedtest`
  - `tamlinux/commands/tam-workstation-upgrade/cli.py`
  - `tamlinux/commands/tam-setup-security-fido2/tam-setup-security-fido2`
  - `tamlinux/commands/tam-setup-security-fingerprint/tam-setup-security-fingerprint`
  - `tamlinux/commands/tam-sudo-passwordless/tam-sudo-passwordless`
  - `tamlinux/commands/tam-sudo-docker/tam-sudo-docker`
  - `tamlinux/commands/tam-install-font/tam-install-font`
  - `tamlinux/commands/tam-dev/tam-dev`
  - `tamlinux/commands/tam-launch-tui/tam-launch-tui`
  - `tamlinux/commands/tam-launch-webapp/tam-launch-webapp`
  - `tamlinux/commands/tam-launch-editor/tam-launch-editor`
  - `tamlinux/commands/tam-default-editor/tam-default-editor`
  - `tamlinux/commands/tam-theme-colors-from-alacritty/tam-theme-colors-from-alacritty`
  - `tamlinux/commands/tam-theme-refresh/tam-theme-refresh`
  - `tamlinux/commands/tam-toggle-enabled/tam-toggle-enabled`
  - `tamlinux/commands/tam-hyprland-toggle-enabled/tam-hyprland-toggle-enabled`
  - `tamlinux/commands/tam-hyprland-toggle-disabled/tam-hyprland-toggle-disabled`
  - `tamlinux/commands/tam-notification-wait/tam-notification-wait`
  - `tamlinux/commands/tam-notification-time/tam-notification-time`
  - `tamlinux/desktop/theme/engine.py`
  - `tamlinux/commands/tam-hyprland-monitor-scaling/tam-hyprland-monitor-scaling`
  - `docs/corpus-pedagogy.md`
  - `docs/corpus-authoring.md`
  - `src/render.c`

## Guiding prompt

> Read tam/docs/ai/2026-11-05-tam-corpus-audit.md. Fix A01–A12, prioritizing the high-severity findings. Verify against product sources, preserve the audit report, and record each finding’s resolution and test results.

## Key decisions

- Preserved `docs/ai/2026-11-05-tam-corpus-audit.md` intact without alteration.
- Prioritized High-severity findings (A01–A06), followed by Medium-severity findings (A07–A12).
- Bounded risk labels strictly to the schema recognized by `src/render.c:842` and defined in `docs/corpus-pedagogy.md:304` (`read-only`, `scratch`, `user`, `system`, `boot`), removing all unsupported `data-risk="low"` instances.
- Replaced invented subcommands with real CLI verbs verified by executing product tools and reading product dispatch logic.
- Avoided private home-directory references in public corpus text (using portable `~/.config` paths).
- Bumped `knowledge/version` patch version to 0.75.1 per `AGENTS.md` version rules.

## Finding resolutions

### High severity (A01–A06)

- **A01 (Incorrect packaged shortcuts):**
  - `knowledge/desktop/theming.html`: replaced Meta + Shift + W with Meta + Ctrl + Space (`tam-menu toggle background`) for background switching in worked example and quiz; updated theme menu chord to Meta + Shift + T (`tam-menu toggle theme`).
  - `knowledge/hardware/displays.html`: replaced Meta + Shift + D with Meta + Ctrl + D (`tam-shell tamlinux-shell toggle fred.monitor`) for the display panel toggle.
  - `knowledge/hardware/input.html`: replaced Meta + Alt + T with the hardware `XF86TouchpadToggle` key and `tam-toggle-input-device touchpad`.
- **A02 (Invented plugin operations and output):**
  - `knowledge/improve/plugins.html`: replaced fabricated plugin list with real output of `tam-plugin list` (removing fictitious `fred.bar`); replaced invented `tam-plugin status` with authentic `tam-plugin info fred.clock`; replaced non-existent `dev link` with `tam-plugin dev fred.clock on`; replaced non-existent `dev reload` with `tam-restart-shell` (and `tam-qmlcache-purge`); redirected logs to `tamlinux-shell.service`; noted that plugin package testing is performed via `tam-deploy`.
- **A03 (Drive-info requires an argument):**
  - `knowledge/hardware/drives.html`: taught discovering block device names with `lsblk -dno NAME,TYPE` before passing the device node to `tam-drive-info /dev/nvme0n1`; updated practice task and quiz to reflect the required device argument.
- **A04 (Upgrade is not verification):**
  - `knowledge/maintain/troubleshooting.html`: separated upgrade inspection from execution by teaching `tam-workstation-upgrade --check` (show plan without mutation) and `--status` (inspect last receipt) instead of bare mutating execution.
- **A05 (Unsupported authentication and recovery guarantees):**
  - `knowledge/hardware/security.html`: clarified that FIDO2 keys and fingerprints offer convenient alternative authentication with password fallback rather than mandatory 2FA; explained the actual scope of `tam-sudo-passwordless` (unprompted root execution for any user process; timeout limits duration, not authority); removed false claims of automatic PAM backups and reminded readers to record rollback points before mutating authentication stacks.
- **A06 (Risk labels understate mutations):**
  - Eliminated `data-risk="low"` across the corpus:
    - `knowledge/desktop/theming.html`: font installation with `tam-install-font` labeled `system`.
    - `knowledge/hardware/displays.html`: monitor scaling adjustment labeled `user`.
    - `knowledge/hardware/drives.html`: disk speedtest labeled `scratch`.
    - `knowledge/improve/plugins.html`: development overrides labeled `user`.
    - `knowledge/maintain/troubleshooting.html`: QML cache purge labeled `user`.
    - `knowledge/improve/contributing.html`: `tam-work begin` and `tam-dev check tam` labeled `user` (build/state mutations).
    - `knowledge/desktop/config.html`: `tam-default-browser` setter labeled `user`.
    - `knowledge/hardware/security.html`: `tam-sudo-docker` probe labeled `read-only`.

### Medium severity (A07–A12)

- **A07 (Wrapper help and palette-recovery instructions are false):**
  - `knowledge/shell/tools.html`: replaced `--help` instructions on wrappers with their actual invocation patterns (`tam-launch-tui <command>`, `tam-launch-webapp <url>`).
  - `knowledge/shell/editors.html`: replaced `--help` calls with `tam-default-editor` query/set and `tam-launch-editor <path>`.
  - `knowledge/desktop/theming.html`: replaced bare `tam-theme-colors-from-alacritty` with `tam-theme-refresh` / `tam-theme-switcher` for live terminal color synchronization; accurately documented `tam-theme-colors-from-alacritty` as generating a theme's `colors.toml` from its `alacritty.toml` directory.
- **A08 (Docker predicate semantics and output):**
  - `knowledge/hardware/security.html`: taught `tam-sudo-docker` as a silent exit status predicate (exit 0 when sudo is needed, exit 1 when accessible directly); removed fabricated text output; documented that `--configured` tests user account group configuration rather than socket reachability.
- **A09 (Worked output incompatible with implementation):**
  - `knowledge/hardware/drives.html`: updated `tam-hardware` output to match `commands/tam-hardware/format.c` formatting; updated `tam-disk-speedtest` output to match phase/rate emission (`disk ...`, `read ...`, `write ...`); added system and date capture attributions.
  - `knowledge/hardware/displays.html`: updated `tam-hyprland-monitor-scaling` output to show normalized `1` and explicit readback after adjusting scale, with capture attribution.
- **A10 (Reference-table roles inferred incorrectly from command names):**
  - `knowledge/desktop/toggles.html`: documented `tam-toggle-enabled`, `tam-hyprland-toggle-enabled`, and `tam-hyprland-toggle-disabled` as file-existence predicates rather than setters.
  - `knowledge/desktop/notifications.html`: documented `tam-notification-wait` as waiting for server and shell IPC readiness; documented `tam-notification-time` as sending a date/time notification toast.
  - `knowledge/desktop/config.html`: documented `tam-branding-about` as editing, resetting, or transcoding custom ASCII artwork for the about dialog.
- **A11 (Nonexistent input source files):**
  - `knowledge/hardware/input.html`: removed nonexistent `bindings/keyboard.lua` and `bindings/touchpad.lua` references; referenced real product files `desktop/session/hypr/tamlinux/input.lua`, `desktop/session/hypr/tamlinux/bindings/media.lua`, and `tam-toggle-input-device`.
- **A12 (Unsupported universal command contract):**
  - `knowledge/improve/scripting.html`: replaced unsupported universal guarantees (`--help`, `--version`, uniform streams) with accurate per-tool contract descriptions, contrasting complex utilities with direct wrappers and silent status predicates.

## Verification

- `tools/lint-corpus.py`: passed cleanly (all 582 pages valid, zero HTML, risk, or private string warnings).
- `python3 -m unittest -q tests/test_lint_corpus.py`: 35 tests passed.
- `python3 -m unittest -q tests/test_build_index.py`: 22 tests passed.
- `python3 -m unittest -q tests/test_component_coverage.py`: 6 tests passed.
- `python3 tools/report-component-coverage.py --quiet --check`: 0 uncovered components, gap check passed.
- `tests/guide.sh build/tam`: all 12 guide verification checks passed.
- `make corpus-and-test`: completed exit 0.
