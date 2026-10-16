# Tam Knowledge Corpus Audit (corpus 0.26.1)

- **Date:** 2026-10-16
- **Stage:** Develop — read-only review. No corpus page, renderer, or command edits;
  this record reports findings and proposes follow-up work only.
- **CLI Tool:** opencode 1.19.x
- **Model:** `big-pickle`
- **Corpus under audit:** `knowledge/version` = `0.26.1` (command `src/version.h` = `0.25.1`)
- **Method:** every reviewable page read in full; each specific claim (command,
  subcommand, flag, check ID, keybinding, path, version, package, entity, internal
  reference) checked against the authoritative source — the product repository
  (`src/`, `Makefile`, `desktop/`), the generated index (`tools/build-index.py`),
  the HTML renderer (`src/render.c`), and the deployed system (the live desktop
  configuration, `pacman`, `readelf`). Rendering behaviour was derived from `src/render.c`, since
  the installed command (0.25.0) predates the corpus under audit.

## Summary

The corpus is factually strong overall: the subcommand list, `check` IDs, Makefile
targets, `tam --version` output shape, and the Lua/Hyprland package facts all match
the product. The defects cluster in three places: (1) pages that describe **stock
Omarchy/Hyprland behaviour rather than Tamlinux's**, (2) pages that describe the
**installation framework as if it were implemented**, and (3) **markup the renderer
cannot display** (HTML entities outside its whitelist and LaTeX math). Multi-agent
**worktree** guidance also now contradicts current agent policy.

| ID | Severity | Area | One-line |
| --- | --- | --- | --- |
| F1 | High | desktop | `desktop/index.html` keybinding table lists Omarchy defaults, not Tamlinux's |
| F2 | High | install, tam | `tam install inspect`/`plan`/`apply`/`verify` described as working; `inspect` is not a subcommand and the others are stubs |
| F3 | High | improve, tam, index | Worktree guidance contradicts current "never create a Git worktree" policy |
| F4 | Medium | sddm | Session command names (`tam-session`, `tam-hyprland-session`) superseded by `tamlinux-*` |
| F5 | Medium | rendering | HTML entities outside the renderer whitelist render literally (`&rarr;`, `&times;`, `&le;`, `&asymp;`) |
| F6 | Medium | rendering | LaTeX math (`$...$`, `\approx`, `\times`, `\text{}`) renders literally |
| F7 | Medium | terminology | "Cultivar" row states Sway/wlroots as the current stack; guide elsewhere says Hyprland is current and Sway is planned |
| F8 | Low | package-versions | Grammar: "A operating system" |
| F9 | Low | package-versions | "User Environment Module (Home Manager)" is not the project's name |
| F10 | Low | toolchains | Stale version example (`0.21.0`); compile example omits documented warning flags |
| F11 | Low | multiple | Undefined internal references: Virtue V03/V04/V11/V15, Decision D53/D131/D135 |
| F12 | Low | ai-performance | `--kv-quant-type q8_0` is not a llama.cpp flag |
| F13 | Low | terminology, philosophy | Inconsistent naming of Fred |
| F14 | Low | terminology | Unexplained proper noun "Suspra" |
| F15 | Low | power | `wlopm` used in an example but not present on this system |
| F16 | Low | jobs | One expected-output block concatenates the output of three commands unlabelled |

---

## High severity

### F1 — `desktop/index.html` documents Omarchy keybindings, not Tamlinux's

The "Essential keybindings" table (`knowledge/desktop/index.html:30-36`) and the
floating note (line 45-46) present these as Tamlinux's bindings. They are the stock
Omarchy/Hyprland defaults; Tamlinux ships a different set
(`desktop/session/hypr/tamlinux/bindings/tiling.lua`, `utilities.lua`).

| Page says | Actual Tamlinux binding |
| --- | --- |
| `SUPER + Space` — "Open the application launcher" (line 31) | `SUPER + SPACE` opens the **Tamlinux menu** (`tam-menu toggle`); the apps menu is `SUPER + ALT + SPACE` |
| `SUPER + Q` — "Close the active window" (line 32) | No binding; close is **`SUPER + W`** |
| `SUPER + H / J / K / L` — "Move focus …" (line 33) | Focus is **`SUPER + arrow keys`**; `J` = toggle split, `K` = keybindings, `L` = toggle workspace layout |
| `SUPER + Shift + H/J/K/L` — "Swap the position" (line 34) | Swap is **`SUPER + SHIFT + arrow keys`** |
| `SUPER + Shift + Space` — "Toggle floating mode" (line 46) | `SUPER + SHIFT + SPACE` toggles the **top bar**; float toggle is **`SUPER + T`** |
| `SUPER + RETURN` (line 30) | Correct |
| `SUPER + F` — fullscreen (line 35) | Correct |
| `SUPER + K` — keybindings reference (line 36) | Correct |

Also, the page's `keywords` meta includes `sway` (line 7) while the page describes
the Hyprland desktop; harmless as a search alias, but worth aligning if the
compositor migration lands.

Fix direction: correct the table from `desktop/session/hypr/tamlinux/bindings/`, or
replace the hard-coded list with a pointer to `tam-menu-keybindings`, which is the
authoritative, always-current reference.

### F2 — Installation lifecycle presented as implemented

`knowledge/install/index.html:26-29` describes four subcommands, and line 43 offers
`tam install inspect` as an alias of `tam inspect`. `knowledge/tam.html:60` repeats
the four-step lifecycle.

Reality (`src/install.c`):

- `tam install inspect` is **not a subcommand** — `cmd_install` rejects it with
  "unrecognized argument 'inspect'" and a usage exit (the correct command is
  `tam inspect`).
- `tam install plan`, `apply`, and `verify` exist but are **stubs**: each prints
  "'<verb>' is not implemented yet" to stderr and returns failure. The help text
  groups them under "Planned component commands:" (`src/install.c:1014-1062`).

Fix direction: mark `plan`/`apply`/`verify` as planned (as the command's own help
does), remove `tam install inspect` in favour of `tam inspect`, and align
`install/index.html:43`.

### F3 — Worktree guidance contradicts current agent policy

Current repository policy tells agents never to create a Git worktree or a copy of
the repository, and to work inside the single shared clone under path claims.
`knowledge/improve/toolchains.html` teaches the opposite across the whole page:
the summary/keywords (lines 6-7), the intermediate section "Isolated Git worktrees
for concurrent development" (lines 70-115) with worked examples
(`git worktree add ../tam-feature-work branch-name`, lines 82, 106) and the note
that "Worktrees share the main `.git` database…" (line 115).

The same recommendation is echoed in `knowledge/tam.html:223,259`,
`knowledge/index.html:102,105` (`105` is at least marked *planned*),
`knowledge/improve/index.html:49`, `knowledge/improve/terminal.html:159`,
`knowledge/improve/scripting.html:146`, and `knowledge/improve/ai-performance.html:162`.

Fix direction: rewrite the concurrency guidance around the shared-clone path-claim
workflow (`tam-work`), which the corpus already mentions, and remove the worktree
instruction. This may be a policy question (whether worktrees are ever advisory);
if so it needs the maintainer's ruling before editing.

---

## Medium severity

### F4 — Superseded session command names in `sddm.html`

`knowledge/ecosystem/sddm.html` documents the login flow in terms of
`/usr/bin/tam-session` and the user endpoint `~/.local/bin/tam-hyprland-session`.
On the deployed system the session entry is `/usr/bin/tamlinux-session` and the
user endpoint is `~/.local/bin/tamlinux-hyprland-session` (the endpoint is the
`desktop/session/start` script). The product repository is itself mid-rename: its
`desktop/native/integration/tamlinux.desktop` still sets `Exec=/usr/bin/tam-session`,
while `desktop/tests/test_native_login.py` already expects the `tamlinux-*` names.
Because the sources disagree, the page should be reconciled with the packaging
decision rather than fixed in isolation.

### F5 — HTML entities outside the renderer whitelist render literally

`src/render.c:285-294` decodes only: `amp`, `lt`, `gt`, `quot`, `apos`, `nbsp`,
`mdash`, `ndash`, `hellip`, `copy`, `reg`, `lsquo`, `rsquo`, `ldquo`, `rdquo`, plus
numeric entities. Any other named entity passes through as literal text. The corpus
uses four outside that set:

| Entity | Occurrences | Pages |
| --- | --- | --- |
| `&rarr;` | 11 | `shell/pipelines.html:111-118` (10), `ecosystem/package-versions.html:167` |
| `&times;` | 6 | `improve/ai-performance.html:80,142` (and the math lines below) |
| `&le;` | 1 | `improve/zram.html:133` |
| `&asymp;` | 1 | `improve/ai-performance.html:80` |

A reader using `tam show` sees `&rarr;`, `&times;` verbatim. The clean fix is to
extend the renderer whitelist with `rarr`, `larr`, `times`, `le`, `ge`, `asymp`
(recommended, helps every future page), or to spell the characters and use
`&gt;`, etc., in the pages.

### F6 — LaTeX math renders literally

The renderer has no math handling, so TeX markup shows as source text:

- `knowledge/improve/index.html:53`: `speeds ($Tokens/s \approx Bandwidth / ModelSize$)`
- `knowledge/improve/efficiency.html:125-126`: `$O(1)$`, `$O(N)$`
- `knowledge/improve/ai-performance.html:91-93,107`: `$2 \times 64\text{ bits}…$`,
  `$\approx 67.2\text{ GB/s}$`, `… / 4.5\text{ GB} \approx 14.9\text{ tokens/second}$`

Fix direction: rewrite as plain Unicode/ASCII prose (e.g. "≈", "×", "O(1)") which
the renderer already passes through as literal UTF-8.

### F7 — `terminology.html` states Sway as the current workstation stack

`knowledge/ecosystem/terminology.html:82` lists the **Cultivar** (Tamlinux
Workstation) as "Sway/wlroots, Quickshell, `fred.*` plugins, `tam`, Nix/Home
Manager userland". Elsewhere the guide is explicit that Tamlinux "runs on Hyprland
today" and has *selected* Sway as a future target
(`knowledge/ecosystem/compositors.html:19-20,42-45`, with migration at 0.7/0.8/1.0).
The terminology table presents a planned target as the present implementation.

---

## Low severity / style

- **F8** `knowledge/ecosystem/package-versions.html:19`: "A operating system" →
  "An operating system".
- **F9** `knowledge/ecosystem/package-versions.html:105`: "User Environment Module
  (Home Manager)" expands Home Manager to a name the project does not use.
- **F10** `knowledge/improve/toolchains.html:44-45`: version example `tam command
  0.21.0 (dev abc1234)` is stale (command is 0.25.1), and the compile line omits
  `-Wextra -Wpedantic -std=c11` that the page's own companion, `tam.html:135`,
  documents as Tamlinux's flags.
- **F11** Undefined internal references. "Virtue V03/V04" (`terminology.html:119`),
  "Virtue V11" (`index.html:100`, `improve/index.html:38`, `improve/terminal.html:20,81`),
  "Virtue V15" (`improve/index.html:62`), and "Decision D53"
  (`improve/index.html:60`), "Decision D131" (`improve/scripting.html:119`),
  "Decision D135" (`ecosystem/package-versions.html:233,260`) are cited with no
  defining page in the corpus. Beyond being unexplained to a reader, publishing
  internal decision-record identifiers in a public corpus is a boundary concern.
  Either define them in a corpus page or drop the parenthetical citations.
- **F12** `knowledge/improve/ai-performance.html:147`: "Enabling 8-bit KV cache
  quantization (`--kv-quant-type q8_0` in `llama.cpp`)". `llama.cpp` does not accept
  `--kv-quant-type`; the flags are `--cache-type-k`/`--cache-type-v` (or `-ctk`/`-ctv`).
- **F13** Naming is inconsistent: "developer Fred" (`terminology.html:21`) vs
  "Developer Fred Horch" (`philosophy.html:40`). Pick one convention.
- **F14** "Suspra" appears three times (`terminology.html:58,83,106`) with no
  introduction. It is a real Tamlinux-adjacent application, but a reader has no way
  to know that; introduce it or replace it with a generic example.
- **F15** `knowledge/maintain/power.html:97` uses `wlopm --toggle DP-1` as an
  example; `wlopm` is not installed on this workstation. Either note it as an
  optional tool or prefer a shipped command.
- **F16** `knowledge/shell/jobs.html:34-51`: a single `<pre>` block concatenates the
  expected output of three different commands (`sleep 100`, the resume, and `jobs`)
  without labels, which is easy to misread as one transcript.

---

## Out of scope (noted, not counted)

`README.md:10,24,44` repeats the `tam install inspect` claim of F2. The README is
not part of the knowledge corpus, but the same correction applies.

## Verified correct (spot checks that passed)

- Subcommand set and aliases (`src/tam.c`): `show`, `welcome`/`hello`/`hi`,
  `search`, `explain`, `inspect`, `check`, `install`, `man`, `info`, `ask`.
- `check` IDs (`src/install.c`): `privilege_pkexec`, `storage_btrfs`,
  `storage_root_space`, `init_systemd`, `graphics_card`, `package_manager`,
  `kernel_release` — `resources.html` and others cite valid IDs.
- `tam --version` output shape (`src/tam.c:278-309`) matches the sample in
  `package-versions.html:66-70`.
- Makefile targets and `SRC` list vs `tam.html`.
- Lua facts in `ecosystem/lua.html`: `lua` 5.5.1, `luajit` 2.1.x, `lua51` 5.1.5,
  `lua54` 5.4.9, `luarocks` 3.13.0; Hyprland links `liblua.so.5.5`.
- Hyprland configuration is Lua (`hyprland.lua`, `bindings.lua`, `monitors.lua`,
  `input.lua`, `looknfeel.lua`).
- Helper tools referenced by pages exist: `tam-update`, `tam-deploy`, `tam-work`,
  `tam-file-select`.

## Provenance

- Read-only audit; no files under `knowledge/`, `src/`, `tools/`, or `tests/` were
  modified. This record and the companion SOP are the only additions.
- Tool and model as stated in the header. The maintainer's originating instruction
  for this audit preceded the visible session window and is not reproduced here
  verbatim; the `AI_PROVENANCE.md` row and verbatim prompt are recorded separately
  with the maintainer's confirmation.

## Recommended follow-up (each a separate, scoped change)

1. Corpus-lane fix: F1, F2, F5 (page side if not rendered), F6, F7, F8-F16.
2. Command-lane change: extend `src/render.c` entity whitelist (F5) and refresh
   golden snapshots.
3. Policy ruling from the maintainer: worktree guidance (F3) and the session-name
   reconciliation (F4).
4. Adopt the companion SOP (`docs/ai/2026-10-16-tam-corpus-audit-sop.md`) so this
   class of drift is caught on a cadence by rotating agents.
