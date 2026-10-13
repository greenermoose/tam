# tam installation inspection — slice 4 plan

**Stage:** Develop — **implemented 2026-10-09.** Milestone 4 of the
[development plan](development.md): installation inspection (`tam install inspect`),
command help (`tam install --help`), and contract alignment with the Tamlinux installation framework.

---

## 1. Context and Purpose

Milestones 1–3 established the `tam` command foundation (`0.0.1`), the offline
knowledge browser (`0.1.0`–`0.2.0`), `tam help` (`0.3.0`), and orientation and
curated explanation (`0.4.0`).

Milestone 4 connects the `tam install` entry point to the product installation
contract defined in [`tamlinux/docs/plans/installation-framework.md`](../../../../tamlinux/docs/plans/installation-framework.md):

> - `tam install inspect` reports OS, architecture, version, package tools, and supported profile.
> - `tam install plan --profile <profile>` prints exact changes, inputs, revisions, privileges, checkpoints, and recovery steps without changing anything.
> - `tam install apply --profile <profile>` applies only the reviewed plan; a stale or mismatched plan requires a new review.
> - `tam install verify` checks components, versions, command resolution, desktop integration, and failed or skipped steps.

As mandated by milestone 4 of the development plan:
> "Connect read-only `install inspect` to the product contract; do not pretend
> unimplemented installer operations work. The first slice can develop
> independently of desktop migration. Later installer plan/apply/verify work
> follows the product framework."

This slice implements read-only inspection of host facts and recognized installation
capabilities in C against `libtam`, wires up the `install` subcommand namespace,
clearly reports unimplemented operations (`plan`, `apply`, `verify`), and updates
the offline documentation.

---

## 2. Command Specification

### 2.1 Invocations

| Invocation | Behavior | Exit Code |
| :--- | :--- | :--- |
| `tam install inspect` | Probe and display host facts, package tools, capabilities, and supported profile. Read-only. | 0 |
| `tam install inspect --pipe` | Plain text output (identical to default text output; respects non-TTY pipes). | 0 |
| `tam install --help` (or `-h`, `help`) | Display usage instructions and subcommands for `tam install`. | 0 |
| `tam install` (bare) | Display `tam install` usage and available subcommands (or run `inspect` by default — see Open Questions). | 0 or 2 |
| `tam install plan ...` | Explicit diagnostic: `tam: 'plan' is not implemented yet` (describing framework milestone). | 1 |
| `tam install apply ...` | Explicit diagnostic: `tam: 'apply' is not implemented yet`. | 1 |
| `tam install verify` | Explicit diagnostic: `tam: 'verify' is not implemented yet`. | 1 |
| `tam install <unknown>` | Usage error: `tam install: unrecognized argument '<arg>'`. | 2 |

### 2.2 Output Design (`tam install inspect`)

The inspection output is structured into clear, readable sections with no terminal
control codes:

```text
Host Information:
  Operating system:  Omarchy (arch-like)
  OS version:        4.0.4
  Kernel release:    7.2.8-arch1-2
  Architecture:      x86_64
  Cultivar version:  unavailable

Installation Capabilities:
  Init system:       systemd
  Package tools:     pacman, nix, home-manager, mise
  Privilege tool:    pkexec, sudo
  Root filesystem:   btrfs
  Graphics device:   /dev/dri/card1 (available)

Installation Profiles:
  Supported:         arch-hyprland, arch
  Target base:       arch
```

If run on Void Linux:
```text
Host Information:
  Operating system:  Void Linux
  OS version:        rolling
  Kernel release:    6.6.50_1
  Architecture:      x86_64
  Cultivar version:  unavailable

Installation Capabilities:
  Init system:       runit
  Package tools:     xbps-install, xbps-query
  Privilege tool:    sudo
  Root filesystem:   btrfs
  Graphics device:   /dev/dri/card0 (available)

Installation Profiles:
  Supported:         void
  Target base:       void
```

### 2.3 Strict Isolation and Invariants

- **Read-only guarantee:** `tam install inspect` inspects only via read-only system
  calls (`uname`, `statvfs`, `access`) and `/proc`, `/sys`, `/etc/os-release`. It
  modifies no files, creates no state, and triggers no package installations.
- **State isolation:** Never writes `$XDG_STATE_HOME/tamlinux/welcome_seen` or
  modifies any configuration.
- **Pipe safety:** Output contains zero ANSI color or cursor escape sequences.
- **Honest contract:** Running `tam install plan`, `apply`, or `verify` truthfully
  reports that these operations are not implemented yet, rather than failing silently
  or pretending to succeed.

---

## 3. Host Detection and Capability Probes

All probes will be implemented in C using POSIX primitives and `libtam`:

1. **Operating System (`OS`):**
   - Read `/etc/os-release` (fallback `/usr/lib/os-release`) via `tam_fs_read_file`.
   - Parse `NAME`, `ID`, `ID_LIKE`, and `VERSION_ID` / `BUILD_ID`.
2. **Architecture & Kernel:**
   - Call POSIX `uname()` to obtain `machine` (e.g. `x86_64`) and `release`.
3. **Cultivar Version:**
   - Reuse existing `cultivar_version()` function (checks `$XDG_CONFIG_HOME/tamlinux/version`,
     `~/.config/tamlinux/version`, and `$TAMLINUX_VERSION`).
4. **Package Tools:**
   - Scan `$PATH` or test standard paths for recognized Tamlinux package managers:
     - `pacman` (Arch / Omarchy)
     - `xbps-install`, `xbps-query` (Void Linux)
     - `apt`, `dpkg` (Debian / antiX)
     - `nix` (Nix package manager)
     - `home-manager` (Home Manager)
     - `mise` (Mise tool manager)
5. **Supported Profiles:**
   - Map detected `ID` / `ID_LIKE` to known profiles defined in `installation-framework.md`:
     - Arch/Omarchy (`arch`, `omarchy`): `arch-hyprland`, `arch`
     - Void (`void`): `void`
     - antiX/Debian (`antix`, `debian`): `antix`
     - Other: `none (unsupported host distribution)`
6. **Installation Capabilities:**
   - **Init system:** Check `/run/systemd/system` (`systemd`) vs `/run/runit` (`runit`).
   - **Privilege tools:** Check for `pkexec` and `sudo` in `$PATH`.
   - **Root filesystem:** Inspect mount / `statvfs` on `/` to detect `btrfs` vs other
     filesystems (crucial for Btrfs snapshot checkpoint requirements).
   - **Graphics device:** Check `/dev/dri` for available DRM cards.

---

## 4. Architecture and File Changes

Following the repository's modularity guidelines:

| File | Proposed Changes |
| :--- | :--- |
| `src/install.h` | Header declaring `tam_cmd_install(int argc, char **argv)`. |
| `src/install.c` | Implementation of `tam install` subcommand dispatcher, `install inspect` host probes, capability checks, and formatting. |
| `src/tam.c` | Wire `install` subcommand into `main()`; update `print_intro()` and `print_help()`; add `inspect` to curated `explain` topic map. |
| `src/version.h` | Bump minor version from `0.4.0` to `0.5.0` (`TAM_COMMAND_VERSION`). |
| `Makefile` | Add `src/install.c` to objects list. |
| `knowledge/install/index.html` | Update offline guide to document `tam install inspect` and the installation framework lifecycle. |
| `tests/snapshots/install-index.txt` | Update snapshot to match revised `knowledge/install/index.html`. |
| `tests/run.sh` | Add test cases for `tam install`, `tam install inspect`, `--help`, unimplemented operations, state isolation, and error handling. Update version assertion to `0.5.0`. |
| `docs/ai/` | Add provenance record for slice 4. |

---

## 5. Verification and Acceptance Plan

1. **Compilation & Static Checks:**
   - Clean build with zero warnings under `-Wall -Wextra -Wpedantic -std=c11`.
2. **Behavioral Test Suite (`tests/run.sh`):**
   - `tam --version` asserts `0.5.0`.
   - `tam install inspect` exits 0 and outputs expected OS, architecture, kernel, package tools, and supported profile.
   - `tam install inspect` produces zero terminal escape sequences.
   - `tam install inspect` writes no state under an isolated `$XDG_STATE_HOME`.
   - `tam install --help` and `tam install help` display clear usage information.
   - `tam install plan`, `apply`, `verify` exit 1 with clear messages explaining they are not implemented yet.
   - Unrecognized flags (e.g. `tam install --bogus` or `tam install inspect extra`) exit 2 with usage error.
3. **Guide Test Suite (`tests/guide.sh`):**
   - Golden snapshot verification for `knowledge/install/index.html`.
   - `tam explain inspect` successfully displays the install guide.
4. **Live Installation:**
   - `pkexec make -C <tam checkout> install` to deploy `/usr/local/bin/tam`.
   - Verify `/usr/local/bin/tam install inspect` live on Fred's workstation.

---

## 6. Behavior

1. **Bare `tam install` invocation:**
   When invoked as simply `tam install` with no subcommand:
   Print `tam install` usage and available subcommands, exiting 0 (informative entry point).

2. **Output Formatting:**
   Display as the sectioned key-value layout shown in §2.2
