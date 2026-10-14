# tam read-only self-inspection findings and check contract (tam 0.16.0)

- **Date:** 2026-10-14
- **Stage:** Develop — Milestone 8: read-only self-inspection findings and check contract implemented; version bumped to 0.16.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.4
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested: "Examine the plan in the tam repo for the tam command. Work on the next milestone. I can't remember which one it is, but you should be able to tell from examining the tam repo. In parallel I've asked opencode to add a page about lua to the knowledge corpus. I hope you and opencode can both work in the tam repo together without stepping on each others' toes." Antigravity examined `docs/plans/development.md` and `ROADMAP.md`, identified Milestone 8 (read-only self-inspection findings and check contract) as the next unstarted milestone for the command, claimed the command lane paths (`src/`, `completions/`) via `tam-work`, and coordinated concurrently with OpenCode (working in the corpus lane) without scope conflict. Antigravity implemented the complete Milestone 8 deliverables in `src/install.c`, `src/version.h`, `completions/bash/tam`, and `tests/run.sh`, bumped `TAM_COMMAND_VERSION` to `0.16.0`, and verified the full test suite (`make test`).

## Guiding prompts

> Examine the plan in the tam repo for the tam command. Work on the next milestone. I can't remember which one it is, but you should be able to tell from examining the tam repo. In parallel I've asked opencode to add a page about lua to the knowledge corpus. I hope you and opencode can both work in the tam repo together without stepping on each others' toes.

## Decisions and scope

- **Structured Finding Model (`src/install.c`):**
  - Evolved `tam install inspect` from a basic hardware/OS info reporter into a structured system diagnostic engine.
  - Implemented severity levels: `info`, `warn`, `error`, `critical`.
  - Defined finding attributes: `id` (stable task identifier), `severity`, `summary`, `evidence` (captured slice / command parameters), and `help_link` (deep-link to Maintain-track guide pages).
  - Findings deep-link directly into relevant maintenance documentation:
    - `storage_btrfs` and `storage_root_space` -> `maintain/rollback.html`
    - `privilege_pkexec` and `kernel_release` -> `maintain/recovery.html`
    - `init_systemd` -> `maintain/logs.html`
    - `graphics_card` -> `maintain/power.html`
    - `package_manager` -> `maintain/updates.html`
- **Offline Check Contract (`src/install.c`):**
  - Implemented the versioned Check Contract schema:
    `{task_id, name, status, pass, fail, evidence, severity, help_ref, source, schema_version}`.
  - Returns `pass: true/false`, `fail: true/false`, and `status: "pass" | "fail" | "unknown"`.
  - Emitted as clean JSON for consumption by automated verification, scripts, and the deferred progression framework (`tam learn`).
  - Supported querying specific checks via `tam install inspect --check <task_id>` or `tam install check <task_id>`.
  - Supported listing available checks via `tam install check` and emitting full arrays of check contracts via `--json`.
- **Safe Execution Envelope (`safe_probe_exec`):**
  - All external probes execute under `LC_ALL=C`, bounded timeouts (≤500ms via `tam_exec`), captured streams, detached TTY, and neutralized color.
  - Whitelisted only fixed, deterministic read-only system tools (`findmnt`, `systemctl`, `udevadm`, `pkexec`). Arbitrary user commands are never accepted or executed.
  - Backed by POSIX syscall primitives (`statvfs`, `uname`) for filesystem metrics and architecture details.
- **Privilege Detect-and-Degrade Rule (R-A5):**
  - `pkexec` presence is detected without assuming privilege or executing root commands.
  - On systems without `pkexec` (or minimal rescue environments), gracefully degrades to the non-privileged subset and emits a truthful `warn` finding: `"privilege tool unavailable: limited inspection"`.
  - Never spawns an interactive `sudo` prompt.
- **Command Line Surface & Bash Completion:**
  - `tam install inspect`: displays Host Information, Installation Capabilities, Installation Profiles, and Inspection Findings.
  - `tam install inspect --json`: emits full machine-readable inspection report in JSON format.
  - `tam install inspect --check [ID]`: runs a discrete check (or lists checks) in Check Contract format.
  - `tam install check [ID]`: dedicated check runner / progression oracle subcommand (`--json` supported).
  - Updated `completions/bash/tam` to complete `inspect`, `check`, `--json`, `--check`, and check task IDs.
- **Version Bump:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` to `0.16.0`.
- **Testing & Verification:**
  - Expanded `tests/run.sh` to assert version `0.16.0`, inspect findings in text, JSON report schema, Check Contract schema, discrete check invocation, pkexec detect-and-degrade behavior with isolated `PATH`, unknown check error handling (exit 1), and Bash completion behavior.
  - Verified `make command`, `make test-command`, `make corpus`, and `make test`.

## Verification

- `make test` runs both `tests/run.sh` and `tests/guide.sh` cleanly with zero failures.
- `make command` compiles with zero warnings under `-Wall -Wextra -Wpedantic -std=c11`.
