# Absolute-beginner task coverage

**Plan status:** Completed
**Reviewed:** 2026-11-13
**Scope:** Corpus content plan for a reader who has never used a terminal; page set, command-reference scope and build order. Approved 2026-11-02 for implementation in slices.
**Authority:** [Plan maintenance procedure](../../plan-maintenance.md) and the developer's instructions of 2026-11-02 (audience: absolute beginner; deliverable: a plan document; command reference: a hand-written curated subset).
**Next action:** None. All four slices are delivered at corpus 0.81.0; learner review and packaged acceptance remain developer-directed and are not implied by this completion.

**Decisions recorded 2026-11-02:** page granularity is one page per job family (option B); the skill registry gains `devices`, `security` and `accounts` at beginner and intermediate level; the page set and curated command subset below are approved.

## Problem

The guide explains computers well but does not teach everyday jobs. A reader
who finishes the theory cannot take a screenshot, pair headphones, print, add a
second account or find the command behind a desktop action.

Measured on `develop` at corpus 0.60.0 / command 0.48.1:

| Evidence | Count |
| --- | --- |
| Indexed pages (`knowledge/index.json`) | 231 |
| Using pages (`knowledge/use/`) | 10 (hub, FAQ, 7 recipes, index) |
| Shell pages (`knowledge/shell/`) | 3 |
| Filesystem pages (`knowledge/filesystem/`) | 2 |
| Hardware pages (`knowledge/hardware/`) | 2 |
| Install pages (`knowledge/install/`) | 1 placeholder |
| `tam-*` commands in `tamlinux/commands/` | 226 |
| `tam-*` commands named anywhere in the corpus | 21 |
| Hand-written command reference pages | 0 |

The recipes are four-step stubs, not worked examples. The missing topics are
already implemented as commands, so the gap is documentation, not software.
The command inventory shows audio, Bluetooth, capture, notifications, power,
network, security, packages and display commands that no page mentions.

## Agreed outcome

A beginner who has never opened a terminal can complete a named set of
everyday jobs from the guide alone, recognise success, and undo the change.
For each job the page names the command or keybinding that performs it and
points to the existing theory page that explains why it works.

This plan covers content only. It does not promise an installer, a GUI, or
behaviour that the product repositories do not implement today.

## Audience and teaching rules

Write for **Sam**: arrived from macOS or Windows, has never used a terminal,
wants to finish work without breaking the machine. Use the existing contracts
in [`corpus-authoring.md`](../../corpus-authoring.md) and
[`corpus-pedagogy.md`](../../corpus-pedagogy.md):

- One page, one job. The title is the job, not the component.
- Beginner section is a complete worked example with captured output, then a
  faded example, then an independent task. Intermediate and expert sections
  add transfer, diagnosis and internals only where they earn their place.
- Plain language, short sentences, every term defined or linked at first use.
- Scratch directory for practice; risk labels for privileged steps; the undo
  step is stated before the change.
- Every Tamlinux statement traces to `tamlinux/commands/<name>`, a bindings or
  configuration file, real command output, or a public plan. General Linux
  knowledge is presented as general, not as a Tamlinux choice.
- No `SUPER`; the desktop modifier is **Meta**.
- No install-track promises beyond the real `tam install` surface.

## Proposed pages

Paths are relative to `knowledge/`. Parents are existing hubs unless a new hub
is listed first. `kind` follows the metadata contract; `skills` must exist in
`learn/skills/skills.json` (see the registry question below).

### Slice 1 — First day on the desktop

| Page | Kind | Level | Parent | Job taught | Main sources to check |
| --- | --- | --- | --- | --- | --- |
| `use/desktop/screenshots.html` | how-to | 1 | `use/index.html` | Capture the screen or a window, find the file, record a short clip | `commands/capture-screenshot`, `capture-region`, `capture-screenrecording`, `capture-edit` |
| `use/desktop/clipboard.html` | how-to | 1 | `use/index.html` | Find something copied earlier and paste it again | Desktop configuration and the clipboard-history component; verify before naming |
| `use/desktop/notifications.html` | how-to | 1 | `use/index.html` | Read a notice, send one, silence them, use the on-screen display | `commands/notification-send`, `notification-wait`, `notification-battery`, `toggle-notification-silencing`, `osd` |
| `use/desktop/audio.html` | how-to | 1 | `use/index.html` | Mute, choose output, set volume, restart audio | `commands/audio-*`, `restart-audio` |
| `use/desktop/bluetooth.html` | how-to | 1 | `use/index.html` | Turn Bluetooth on, pair headphones, reconnect | `commands/bluetooth-power`, `bluetooth-device`, `restart-bluetooth` |
| `use/desktop/displays.html` | how-to | 1 | `use/index.html` | Change brightness, scaling or text size, connect a second screen | `commands/brightness-*`, `display-text-size`, `hw-external-monitors`, `hyprland-monitor-*`; existing `use/recipes/external-display.html` |
| `use/desktop/keyboard-touchpad.html` | how-to | 1 | `use/index.html` | Type another layout, fix a touchpad, disable a device | `commands/toggle-input-device`, `hw-touchpad`, `hw-touchscreen`, `hw-fingerprint`; bindings |
| `use/desktop/lock-and-idle.html` | how-to | 1 | `use/index.html` | Lock, sleep, wake, stop the screen blanking too early | `commands/system-lock`, `system-sleep-lock`, `system-wake`, `toggle-idle` |
| `use/desktop/night-light.html` | how-to | 1 | `use/index.html` | Warm the screen colour in the evening and undo it | `commands/toggle-nightlight`, `restart-hyprsunset` |

Keep `use/recipes/external-display.html` and `use/recipes/appearance.html` as
short task recipes; the new pages carry the full worked examples and link
back to them.

### Slice 2 — Files, apps and media

| Page | Kind | Level | Parent | Job taught | Main sources to check |
| --- | --- | --- | --- | --- | --- |
| `use/files/finding-files.html` | how-to | 1 | `filesystem/index.html` | Find a file by name or content, open it, reveal where it lives | Existing `filesystem/`, `shell/`; general GNU tools |
| `use/files/usb-drives.html` | how-to | 1 | `filesystem/index.html` | Plug in a USB drive, find it, copy a file, remove it safely | `commands/drive-info`, `drive-select`; verify automount source |
| `use/files/archives.html` | how-to | 1 | `filesystem/index.html` | Make and open a `.zip` or `.tar.gz` | General GNU tar, `zip`; verify packages |
| `use/apps/browser.html` | how-to | 1 | `use/index.html` | Open the browser, set the default, copy a link | `commands/launch-browser`, `default-browser`, `browser-command`, `chromium-copy-url-host` |
| `use/apps/editor.html` | how-to | 1 | `improve/index.html` | Open a text file, change it, save without a mouse | `commands/launch-editor`, `default-editor`; existing `improve/terminal.html` |
| `use/apps/documents-and-pdf.html` | how-to | 1 | `use/index.html` | Read a PDF, fill a form, print a document | Verify installed viewers and CUPS before naming |
| `use/apps/media.html` | how-to | 1 | `use/index.html` | Play music or video, view photos, convert a file | `commands/transcode`, `transcode-ascii`, `menu-images`; verify players |
| `use/apps/web-apps.html` | how-to | 1 | `use/index.html` | Install a website as an app, launch it, remove it | `commands/webapp-install`, `webapp-remove`, `launch-webapp`, `webapp-handler-*` |

### Slice 3 — People, safety and help

| Page | Kind | Level | Parent | Job taught | Main sources to check |
| --- | --- | --- | --- | --- | --- |
| `use/security/passwords.html` | how-to | 1 | `maintain/index.html` | Change a password, authenticate for an admin step, keep secrets out of files | `commands/sudo-*`, `setup-security-*`; verify PAM and policy sources |
| `use/security/ssh.html` | how-to | 2 | `maintain/index.html` | Make a key, reach another computer, disable the server | `commands/setup-security-sshd`, `remove-security-sshd`; general OpenSSH |
| `use/security/fingerprint-and-fido.html` | how-to | 2 | `maintain/index.html` | Enrol a fingerprint or security key, remove it | `commands/hw-fingerprint`, `setup-security-fido2`, `remove-security-*` |
| `use/security/firewall.html` | how-to | 2 | `maintain/index.html` | See whether the firewall is on, allow one service, undo it | Verify Tamlinux firewall ownership before writing; drop if there is no source |
| `use/security/disk-encryption.html` | explanation | 2 | `maintain/index.html` | Understand what is encrypted, unlock at boot, recover a passphrase | `commands/drive-password`, `drive-select`; product plans |
| `use/people/accounts.html` | how-to | 2 | `maintain/index.html` | Add a second account, change its group membership, remove it | Verify product account tooling; general `useradd`/`usermod` |

### Slice 4 — Hand-written command reference

One hub plus grouped pages, not one stub per command:

| Page | Kind | Level | Parent | Covers |
| --- | --- | --- | --- | --- |
| `use/commands/index.html` | hub | 1 | `use/index.html` | What this reference is, how to read an entry, how to get the full list |
| `use/commands/desktop.html` | reference | 2 | `use/commands/index.html` | Capture, audio, Bluetooth, brightness, notifications, idle, lock, theme, fonts |
| `use/commands/network.html` | reference | 2 | `use/commands/index.html` | Status, password, QR, speed test, DNS, restart Wi-Fi |
| `use/commands/packages.html` | reference | 2 | `use/commands/index.html` | Query, install, remove, AUR, update, firmware, workstation upgrade |
| `use/commands/system.html` | reference | 2 | `use/commands/index.html` | Stats, drives, power profiles, battery, hibernate, session actions |

Each entry is hand-written and verified, with four fields: what it does, one
runnable example, the undo or safe direction, and the source file. Mark
privileged commands with the risk label. Do not copy `--help` wholesale.

Selection rule for the curated subset, about 40 commands:

1. It appears in a Slice 1–3 page, or the reader meets it in the desktop.
2. It is user-facing and safe to run on a normal account, or its risk is
   clearly labelled.
3. Its behaviour can be confirmed from `commands/<name>` or real output.
4. Exclude helpers, internal callers, agent-only tools, `tam-deploy`,
   `tam-work`, `tam-agent*`, `tam-hook`, `tam-fleet` and anything marked
   planned.

The remaining commands stay undocumented until a page needs them. The hub
states plainly that the corpus documents everyday commands, not all 226.

## Skill registry (decided 2026-11-02)

The current domains are desktop, shell, files, processes, software, updates,
boot, hardware, signals, networking, troubleshooting, customizing and
contributing. The developer approved three additions at beginner and
intermediate level: `devices`, `security` and `accounts`. There is no `apps`
domain; app pages map to `desktop` and `software`.

Proposed can-do statements:

| Cell | Can-do |
| --- | --- |
| `devices.beginner` | Connect, disconnect and choose an audio, display or input device |
| `devices.intermediate` | Diagnose a device that does not appear, sounds wrong or changes unexpectedly |
| `security.beginner` | Recognise a privileged action and protect an account, key or secret |
| `security.intermediate` | Choose between password, key and hardware authentication and recover access |
| `accounts.beginner` | Identify your account, its home folder and its groups |
| `accounts.intermediate` | Add or remove another account and choose its group membership |

Boundaries keep the domains from overlapping:

- `hardware` stays with physical evidence, cables and resource limits.
- `devices` covers connecting and choosing working peripherals.
- `customizing` covers who owns a setting and how to undo it.
- `security` covers authentication, keys, encryption and the firewall.
- `accounts` covers identity, home folder and group membership.

Page mapping:

- Screenshots, clipboard, notifications, audio, Bluetooth, displays,
  keyboard/touchpad, lock/idle and night light map to `devices` plus
  `desktop`.
- Passwords, SSH, fingerprint/FIDO, firewall and disk encryption map to
  `security`, plus `customizing` where the page changes a setting.
- The accounts page maps to `accounts` plus `security`.
- Files and app pages keep `files`, `software` and `desktop` as appropriate.

Add the three cells in the same change as the Slice 1 pages, not before, so
the coverage report starts showing taught cells rather than empty ones. The
cell IDs become available to pages only after that change.

## Sourcing and verification

For every page:

- Check `tamlinux/commands/<name>` and its `--help` before naming a flag.
- Check bindings under `desktop/session/hypr/tamlinux/bindings/` before
  naming a chord.
- Run the example and paste the real output; do not write output from memory.
- Where the source does not exist, teach the general Linux behaviour and say
  it is general, or leave the page out.
- Record `Checked against:` in the session record, and list sources in the
  session record rather than on the page.
- Verify every glossary link and bibliography ID with the builder.

## Build order and handoff

1. Add the `devices`, `security` and `accounts` cells, then Slice 1: nine
   pages, then `use/commands/desktop.html`.
2. Slice 2, then `use/commands/network.html` and `packages.html`.
3. Slice 3, then `use/commands/system.html` and the reference hub.
4. Link each page from `use/index.html`, the relevant path (`first-steps`,
   `settle-in`, `session-and-desktop`, `make-it-yours`, `keep-it-healthy`),
   and the command hub.
5. Regenerate indexes with `make corpus-and-test`; read each page with
   `tam show --pipe`.

TCK identifiers are assigned from `CHECKLISTS.md` when the developer approves
the slices; this plan invents none.

## Acceptance and limits

Observable acceptance:

- A person who has never used a terminal completes each Slice 1 job by
  following the page, without asking for help.
- Each page shows success output and an undo step.
- `tam explain` finds the page for its job words.
- `make corpus-and-test` passes with no new warnings, and every glossary and
  bibliography link resolves.
- The command reference contains no command that has not been checked against
  its source, and no planned command is described as available.

Limits:

- The pager is text-only; pages use captured text, not screenshots.
- Live desktop behaviour and key chords need the developer's verification on
  the running session.
- Learner review and packaged acceptance remain developer-directed and are not
  claimed by this plan.
- This plan does not add install-track content, a GUI, or new commands.

## Disposition

Completed at corpus 0.81.0. The developer approved the page set, the curated
command subset, job-family granularity and the `devices`, `security` and
`accounts` skill domains on 2026-11-02. Slice 1 landed at corpus 0.61.0; the
remaining slices landed at corpus 0.79.0–0.81.0
([record](../../ai/2026-11-13-tam-dynamic-multipath-and-beginner-pages.md)).

All 28 planned pages exist: nine `use/desktop/`, three `use/files/`, five
`use/apps/`, five `use/security/`, one `use/people/` and five `use/commands/`.
The three new skill domains are registered and the coverage report shows no page
or path gaps at corpus 0.81.0.

Learner review and packaged acceptance remain pending. The plan's acceptance
criterion that a person who has never used a terminal completes each job is a
learner test, not a structural check, and has not been run.
