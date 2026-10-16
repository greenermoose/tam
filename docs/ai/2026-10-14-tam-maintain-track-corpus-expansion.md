# tam "Maintain" track knowledge corpus expansion (tam 0.19.0)

- **Date:** 2026-10-14
- **Stage:** Develop — Milestone 11: Comprehensive "Maintain" track knowledge corpus expansion across 5 core topics (`updates.html`, `rollback.html`, `logs.html`, `power.html`, `recovery.html`) plus `maintain/index.html` hub; full beginner/intermediate/expert fading template ladders; two-pass exact-topic alias resolution in `src/index.c`; updated tests and snapshots; version bumped to 0.19.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.4
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred directed: "I have another session of agy pondering some improvements to tam. While it is doing that, please work on the next milestone." Antigravity identified Milestone 11 (Version 0.19.0 — "Maintain" Track Knowledge Corpus Expansion) as the next unstarted milestone in `tam/ROADMAP.md` and `tam/docs/plans/development.md`. Antigravity claimed repository paths using `tam-work`, authored the 6 comprehensive "Maintain" track knowledge pages, implemented two-pass exact-topic alias resolution in `src/index.c` to prevent keyword shadowing, updated `install/index.html` cross-links, bumped the version in `src/version.h` to 0.19.0, updated test suites (`tests/run.sh` and `tests/guide.sh`) and golden text snapshots, and verified full test suite passes (`make test`).

## Guiding prompt

> I have another session of agy pondering some improvements to tam. While it is doing that, please work on the next milestone.

## Decisions and scope

- **Concurrency Claim & Workstream Discipline:**
  - Claimed repository paths using `tam-work begin --scope src/version.h --scope knowledge --scope tests --scope ROADMAP.md --scope docs/plans/development.md --scope AI_PROVENANCE.md --note 'Milestone 11: Maintain track knowledge corpus expansion (tam 0.19.0)'`.
  - Avoided conflicting with the other concurrent agy session, which was pondering guide home page titles, TOC structures, and `tam` manual page proposals in `docs/plans/user-guide-and-man-page.md`.
- **Maintain Track Knowledge Corpus Authoring (`knowledge/maintain/`):**
  - `knowledge/maintain/index.html`: Track portal and overview outlining the five maintenance pillars, built-in automated inspection with `tam install inspect`, and navigation pathways.
  - `knowledge/maintain/updates.html`: Explains why Linux updates in place without rebooting via POSIX inode unlinking (`unlink(2)` semantics, directory hardlinks vs open inode reference counts), the three-tier package model (base OS via pacman, Tamlinux packages via tamlinux-packages/Nix, user tools via mise), inspecting unlinked files with `lsof +L1`, and safe update workflows.
  - `knowledge/maintain/rollback.html`: Details Btrfs Copy-on-Write (CoW) subvolume snapshot mechanics, taking read-only snapshots, inspecting storage headroom via `tam install check storage_btrfs` / `storage_root_space`, switching Home Manager desktop generation profiles, and atomic root subvolume rollback from live rescue environments.
  - `knowledge/maintain/logs.html`: Establishes truthful diagnostics through empirical logging; filters systemd journal streams by boot (`-b`), unit (`-u`), and syslog priority ranges (`-p err..emerg`); reads kernel ring buffer via `dmesg -T`; explains cyclic printk buffer sizing and verifies binary journal file integrity with `journalctl --verify`.
  - `knowledge/maintain/power.html`: Explains the three-stage power progression (individual DPMS monitor blanking, low-power RAM suspend S3/s2idle, and zero-watt S4 disk hibernation); establishes the safe display wake convention using the `Shift` key (avoiding unintended clicks or terminal execution); details kernel power modes in `/sys/power/`, Wayland output control (`wlopm`), and hibernation `image_size=0` tuning to avoid ENOMEM allocation failures.
  - `knowledge/maintain/recovery.html`: Provides a structured recovery escalation ladder: dropping to Linux text virtual consoles (`Ctrl+Alt+F2`–`F6`) to kill hung processes, executing the kernel Magic SysRq `REISUB` sequence to cleanly unraw, terminate, kill, sync, unmount, and reboot without data loss, and performing live USB chroot system recovery.
- **Content-Depth Ladder Integration:**
  - Applied Beginner, Intermediate, and Expert fading template sections with worked examples and completion tasks across all five maintenance topics, enabling deep-linking from `tam install inspect` check findings, `tam explain --level LEVEL <topic>`, and `?level=` query parameters.
- **Two-Pass Alias Lookup in `src/index.c`:**
  - Discovered and resolved an alias lookup shadowing issue in `tam_index_lookup_alias()`: previously, if an earlier page listed a word in its keywords (e.g. `install/index.html` carrying keyword `maintain`), that keyword match shadowed a later page whose exact `topic` matched the query.
  - Refactored lookup into two distinct passes: Pass 1 evaluates exact topic matches across all entries; Pass 2 evaluates keyword aliases. Exact topic matches now reliably take precedence.
- **Version Bump:**
  - Bumped `TAM_COMMAND_VERSION` in `src/version.h` to `0.19.0`.
- **Corpus Index & Test Verification:**
  - Regenerated `knowledge/index.tsv` and `knowledge/index.json` (35 entries).
  - Updated `tests/guide.sh` with the 6 new pages and extensionless resolution checks; regenerated golden text snapshots in `tests/snapshots/`.
  - Updated `tests/run.sh` to assert `0.19.0` version output and verify `explain` resolution across all new Maintain track topics.

## Verification

- `make all` built cleanly: C binary compiled under `-Wall -Wextra -Wpedantic -O2`, index regenerated with 35 entries.
- `tests/guide.sh` passed 100%: all 35 pages rendered, extensionless resolution verified, zero escape sequences in piped output, all internal links validated against disk, snapshots matched.
- `tests/run.sh` passed 100%: version reported `0.19.0`, all Maintain topics resolved accurately (`maintain`, `updates`, `rollback`, `logs`, `power`, `recovery`).
- `make test` executed completely green.
