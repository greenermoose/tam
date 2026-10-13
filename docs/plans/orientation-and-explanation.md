# tam orientation and explanation — slice 3 plan

**Stage:** Develop — **implemented 2026-10-09.** Milestone 3 of the
[development plan](development.md): orientation (`welcome`), successful-display
state (`welcome_seen`), and curated explanation (`explain <topic>`).

**Decision (Fred, 2026-10-09):**
1. **1 combined slice** for orientation (`tam welcome`) and explanation (`tam explain <topic>`).
2. **Curated topic scope** based on existing curriculum pages (`welcome`, `install`, `pipelines`, `man`, redirection, and core tool mentions) is sufficient for this slice.
3. **Open in the pager**, falling back to clean text in pipes.

**Preceding work:**
- Milestone 1: Command foundation (`tam 0.0.1`, bare intro, `--help`, `--version`, argument handling).
- Milestone 2: Offline guide (`tam 0.1.0` -> `0.2.0`, `browse`, `--pipe`, native HTML renderer, TUI pager).
- Update: `tam help` exact alias of `--help` (`tam 0.3.0`).

---

## 1. Objectives and Scope

Milestone 3 delivers the remaining entry-point and orientation commands defined
in the first command contract:

1. **`tam welcome` (Orientation):**
   - Open orientation explicitly at any time.
   - Interactive terminal: open `welcome.html` in the built-in pager (`tam_pager_run`).
   - Piped / non-TTY stdout / `--pipe`: render `welcome.html` as plain text.
   - **Successful display state:** Record that welcome was displayed in
     `$XDG_STATE_HOME/tamlinux/welcome_seen` (falling back to
     `~/.local/state/tamlinux/welcome_seen`) **only after successful display**.
   - Bare `tam`, `show`, `help`, `version`, `explain`, and failed runs must
     **never** write or touch `welcome_seen`.

2. **`tam explain <topic>` (Curated Explanation):**
   - Look up curated local topics, concepts, and shell constructs.
   - Match curated topics without network access.
   - Interactive terminal: open the matched topic in the pager.
   - Piped / non-TTY stdout / `--pipe`: emit clean plain text.
   - Clear miss diagnostic on unrecognized topics with actionable suggestions
     (e.g., referring user to `tam show`).
   - **Strict input safety:** Topic input is treated purely as data. Input
     containing shell characters (pipes `|`, redirects `>`, semicolons `;`,
     subshells `$(...)`) is **never executed**.
   - Compound shell AST parsing is reserved for later slices; this slice focuses
     on curated lookup and truthful diagnostics.

---

## 2. Command Specifications

### 2.1 `tam welcome`

```sh
tam welcome          # interactive pager displaying welcome.html
tam welcome --pipe   # plain text output to stdout (also: tam welcome -p)
tam welcome | head   # non-TTY stdout automatically uses text mode
```

- **Arguments:**
  - Accepts optional `--pipe` / `-p`.
  - Any unexpected or extra arguments produce exit code 2 (usage error).
- **State effect:**
  - Upon successful display (exit code 0), writes an ISO 8601 UTC timestamp
    (e.g., `2026-10-09T16:30:00Z\n`) to `$XDG_STATE_HOME/tamlinux/welcome_seen`
    (or `~/.local/state/tamlinux/welcome_seen`).
  - Directory is created if missing (`0755`).
  - File is written atomically using `tam_fs_write_atomic(path, data, len, 0644)`.
  - If display fails (e.g., knowledge directory missing or unreadable), exits 1
    and writes no state.

### 2.2 `tam explain <topic>`

```sh
tam explain <topic>         # interactive pager displaying the matched topic
tam explain <topic> --pipe  # plain text output to stdout
tam explain                 # list available curated topics (exit 0)
tam explain <topic> extra   # error: too many arguments (exit 2)
```

- **Topic matching:**
  Curated dictionary matching case-insensitively or normalized (with outer quotes stripped):
  - Guide pages:
    - `welcome` -> `welcome.html`
    - `install`, `installation` -> `install/index.html`
    - `pipes`, `pipeline`, `pipelines`, `|` -> `shell/pipelines.html`
    - `man`, `manual` -> `man/index.html`
    - `guide`, `index`, `show`, `browse` -> `index.html`
  - Core Unix concepts & operators from the curriculum:
    - Pipelines & redirection: `|`, `>`, `>>`, `2>&1`, `<`, `redirection` -> `shell/pipelines.html`
    - Commands featured in examples: `grep`, `ps`, `wc`, `cat`, `whatis`, `apropos`
- **Miss handling:**
  When a topic is not in the curated set:
  ```text
  tam: no explanation found for '<topic>'
  Run 'tam explain' to see available topics, or 'tam show' to explore the offline guide.
  ```
  Exits with code 1.
- **Safety invariant:**
  `tam explain` never spawns a shell or executes input strings. A string like
  `tam explain "cat file | grep pattern"` is processed strictly as query text.

---

## 3. Work Items

1. **State directory & writer helper (`src/tam.c`):**
   - Resolve `$XDG_STATE_HOME` / `~/.local/state`.
   - Ensure `tamlinux/` directory exists.
   - Atomic write of ISO timestamp string via `tam_fs_write_atomic`.
2. **`tam welcome` command wiring:**
   - Parse `--pipe` / `-p`.
   - Resolve `welcome.html` in knowledge root.
   - Render to pager or text.
   - Record `welcome_seen` on success.
3. **`tam explain` curated topic lookup:**
   - Define curated topic table and alias mappings.
   - Parse `<topic>` argument and optional `--pipe`.
   - Render matched page/section to pager or text.
   - Emit clear diagnostic and exit 1 on miss.
4. **Command documentation & help updates:**
   - Update `print_help()` to include `welcome` and `explain <topic>`.
   - Update `print_intro()` to reflect that `welcome` and `explain` are available.
   - Bump minor version `0.3.0` -> `0.4.0` in `src/version.h` and tests.
5. **Testing suite:**
   - `tests/welcome.sh` (or additions to `tests/run.sh`):
     - `tam welcome` creates `welcome_seen` with valid timestamp.
     - Piped output has no escape sequences.
     - Negative isolation: bare `tam`, `browse`, `help`, `version` do not write state.
     - Missing corpus does not write state and reports error.
     - Extra arguments exit 2.
   - `tests/explain.sh`:
     - Matched topics render expected content without escapes when piped.
     - Unmatched topics exit 1 with clear miss diagnostic and reference `browse`.
     - Shell metacharacters (`|`, `>`, `;`, `$()`) are handled harmlessly as data.
     - Missing topic or extra arguments exit 2.
6. **Build & Installation:**
   - Verify `make && make test` pass cleanly with no warnings.
   - Run `pkexec make -C <tam checkout> install` to update `/usr/local/bin/tam`.
   - Record decision entry and AI provenance.

---

## 4. Verification and Acceptance Criteria

- `tam welcome` displays `welcome.html` interactively in Foot, and records
  `welcome_seen` only on success.
- `tam welcome --pipe` emits readable text and records `welcome_seen`.
- Bare `tam`, `tam show`, `tam help`, `tam --version` never touch `welcome_seen`.
- `tam explain <topic>` resolves curated topics and renders them accurately.
- `tam explain "cat | grep"` and malicious shell strings are never executed.
- Unmatched topics report a helpful error pointing to `tam show`.
- All tests pass cleanly under GCC `-Wall -Wextra -Wpedantic -std=c11`.
