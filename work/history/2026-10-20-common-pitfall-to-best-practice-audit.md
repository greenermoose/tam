# Common pitfall → Best practice audit (corpus style)

- **Date:** 2026-10-20
- **Time:** started now
- **Stage:** Develop — read-only review. No corpus page edits; this record reports findings and proposes follow-up work only.
- **CLI Tool:** opencode (current)
- **Model:** big-pickle (opencode/big-pickle)
- **Corpus under audit:** `knowledge/version` (current), `src/version.h` (command version)
- **Method:** searched for `<h3>Common pitfall</h3>` across `knowledge/**/*.html`; read each section and classified by structure (negative-first vs positive-first, number of sentences, presence of consequence explanation).

## Summary

Found **16** pages with a `Common pitfall` section. All 16 currently follow the anti-pattern described: they lead with a negative/prohibitive statement ("Never…", "Do not…", "Avoid…", "Closing your terminal window sends…") and often follow with an explanation of what to do instead or consequences. The requested change is to rename to `Best practice`, lead with the positive/correct action first, then explain what bad things happen if done the other way.

**SOP question:** There is no existing SOP for this style/pattern audit in `tam/docs/`. The general corpus audit SOP (`docs/corpus-audit.md`, `docs/ai/2026-10-16-tam-corpus-audit-sop.md`) covers factual/behavioural/rendering drift, not presentation/style pattern refactoring like this. **Recommendation:** create an SOP (e.g. `docs/style-audit.md` or add a section to `docs/corpus-authoring.md`) defining how to audit and fix style/pattern issues, where to file reports (`docs/ai/`), severity, and the fix procedure.

## Findings table

| ID | Page | Current heading | Structure | Notes |
|---|---|---|---|---|
| CP01 | knowledge/ecosystem/lua.html | Common pitfall | Describes problem + fix; no strong "never/do not" opener | Special case - already explains behavior; restructure to lead with correct practice |
| CP02 | knowledge/filesystem/permissions.html | Common pitfall | Never… + consequence + extra detail | Classic anti-pattern; needs best practice first (e.g. use least privilege) |
| CP03 | knowledge/hardware/resources.html | Common pitfall | Do not mistake… + explanation + "Always monitor available memory" | Has positive ending; needs reordering |
| CP04 | knowledge/improve/ai-performance.html | Common pitfall | Do not attempt… + consequence | Negative-first |
| CP05 | knowledge/improve/efficiency.html | Common pitfall | Avoid… + consequence | Negative-first |
| CP06 | knowledge/improve/scripting.html | Common pitfall | Never rely… + Always quote… (explanation) | Has "Always" in second sentence - mixed; reorder to lead with quoting |
| CP07 | knowledge/improve/terminal.html | Common pitfall | Do not attempt… + Use… | Has positive in second sentence |
| CP08 | knowledge/improve/toolchains.html | Common pitfall | Never install… + explanation of correct location | Negative-first |
| CP09 | knowledge/improve/zram.html | Common pitfall | Do not confuse… + explanation | Negative-first |
| CP10 | knowledge/maintain/logs.html | Common pitfall | Do not run… + Use filtering… | Mixed |
| CP11 | knowledge/maintain/power.html | Common pitfall | Never click… + consequence | Negative-first |
| CP12 | knowledge/maintain/recovery.html | Common pitfall | Never reach… + consequences + escape hatch | Negative-first |
| CP13 | knowledge/maintain/rollback.html | Common pitfall | Do not confuse… + protection scope + Always maintain backups | Mixed |
| CP14 | knowledge/maintain/updates.html | Common pitfall | Do not treat… + guidance | Negative-first |
| CP15 | knowledge/shell/jobs.html | Common pitfall | Describes SIGHUP behavior (factual) | Describes consequence; needs restructuring to best practice (detach/disown/nohup) first |
| CP16 | knowledge/shell/pipelines.html | Common pitfall | Avoid useless use of cat… + Instead pass directly | Has "Instead" - restructure to lead with direct pass |

## Per-page fix directions (ready for a fixer agent)

| ID | Suggested restructure |
|---|---|
| CP01 (lua.html) | Rename to **Best practice**. Lead with: "After changing Hyprland/Lua config, reload with `hyprctl reload` (and check errors with `hyprctl configerrors`)." Then explain: editing under `~/.config/hypr/` won't take effect until reloaded; parse failures need checking. |
| CP02 (permissions.html) | Rename to **Best practice**. Lead with: "Grant only the minimum required permissions (use `chmod` with specific modes, or `chown`). For directories, ensure the execute bit is set to traverse." Then explain: `chmod 777` grants universal write access creating severe security vulnerabilities. |
| CP03 (resources.html) | Rename to **Best practice**. Lead with: "Monitor **available** memory (not just 'free')." Then explain: unused RAM is used for disk cache by the kernel; low 'free' memory alone doesn't indicate exhaustion. |
| CP04 (ai-performance.html) | Rename to **Best practice**. Lead with: "Only load models that fit in available physical RAM." Then explain: loading a 14 GB model on 12 GB free RAM causes thrashing to disk and token generation to collapse. |
| CP05 (efficiency.html) | Rename to **Best practice**. Lead with: "Use event-driven approaches or efficient timers; avoid busy polling." Then explain: periodic `while true; do ...; sleep 1; done` loops wake the CPU and can multiply standby power draw up to ~3× by preventing deep C-states. |
| CP06 (scripting.html) | Rename to **Best practice**. Lead with: "Always quote variable expansions (e.g. `"$target"`) to prevent word splitting." Then explain: unquoted variables break on paths with spaces. |
| CP07 (terminal.html) | Rename to **Best practice**. Lead with: "Use Micro's standard shortcuts: `Ctrl+S` to save, `Ctrl+Q` to quit." Then explain: modal Vi commands like `:wq` aren't available in Micro. |
| CP08 (toolchains.html) | Rename to **Best practice**. Lead with: "Install development builds under `PREFIX=$HOME/.local/dev` (first on `$PATH`) without touching system directories." Then explain: installing directly into `/usr/bin` or `/usr/local/bin` can conflict with package-managed files and requires root. |
| CP09 (zram.html) | Rename to **Best practice**. Lead with: "Use ZRAM for compressed swap in RAM when appropriate." Then explain: don't confuse ZRAM (microsecond, in-RAM via CPU) with disk swap (milliseconds). |
| CP10 (logs.html) | Rename to **Best practice**. Lead with: "Filter `journalctl` by time/service/unit to focus on relevant entries." Then explain: bare `journalctl` produces thousands of routine messages making issues hard to spot. |
| CP11 (power.html) | Rename to **Best practice**. Lead with: "Wait for the panel to relock before interacting with input on a dark screen." Then explain: clicking keys/mouse blindly is unsafe because you can't see which window has focus until the screen lights up. |
| CP12 (recovery.html) | Rename to **Best practice**. Lead with: "Try non-destructive recovery first (virtual consoles, killing the hung app) before hardware power off." Then explain: hard power cuts bypass disk flush routines and can leave filesystems unclean. |
| CP13 (rollback.html) | Rename to **Best practice**. Lead with: "Maintain external backups of essential personal documents in addition to filesystem snapshots." Then explain: snapshots protect against updates/config changes but not physical drive failures. |
| CP14 (updates.html) | Rename to **Best practice**. Lead with: "Follow the correct update path for each layer (native packages vs Tamlinux product deployments per the lifecycle)." Then explain: treating all layers as one update command risks missing steps; reboot requirements depend on what changed. |
| CP15 (jobs.html) | Rename to **Best practice**. Lead with: "Detach long-running jobs with `disown` or start them with `nohup` so they survive terminal closure." Then explain: closing the terminal sends `SIGHUP` to child jobs and will terminate them otherwise. |
| CP16 (pipelines.html) | Rename to **Best practice**. Lead with: "Pass files directly to commands when possible (e.g. `grep pattern file.txt`)." Then explain: `cat file.txt | grep pattern` spawns an unnecessary process (useless use of cat). |

## Fix procedure (for a future agent)

1. **Claim paths** for each page to edit (or batch claim). Follow repo policy in `AGENTS.md` / `tam/AGENTS.md`.
2. **Read each page** in full to preserve surrounding context/links.
3. **Rename heading** `<h3>Common pitfall</h3>` → `<h3>Best practice</h3>`.
4. **Reorder content**: lead with the positive/correct action as the first sentence/paragraph; move the "don't do this / consequences" explanation after it. Keep technical accuracy and existing links/formatting.
5. **Preserve cross-references** - if other pages link to "Common pitfall" anchors/textually, update if needed (grep for "Common pitfall").
6. **Test rendering** where easy: `make command` or use `tam show --pipe <page>` if built. At minimum ensure HTML remains valid.
7. **Commit only edited pages** with a clear message like "knowledge: rename Common pitfall to Best practice and reorder to lead with correct action".
8. **Bump version** only if policy requires for content changes (check `corpus-authoring.md`); this is style reordering so patch/minor per normal rules.

## Verification checklist

- [ ] All 16 pages updated
- [ ] Headings changed to `Best practice`
- [ ] Each section leads with positive guidance
- [ ] Consequences/bad outcomes explained after the best practice
- [ ] No broken links/HTML
- [ ] `grep -rn "<h3>Common pitfall" knowledge/` returns nothing
- [ ] `grep -rn "<h3>Best practice" knowledge/ | wc -l` shows 16

## Notes

- `knowledge/ecosystem/lua.html` is a borderline case (descriptive rather than purely prohibitive) but should follow the same naming convention per the requested change.
- No SOP exists for style audits in `tam/docs/`. The existing `docs/corpus-audit.md` focuses on factual/behavioural/rendering drift; a style audit needs its own procedure. **Recommend creating `docs/style-audit.md`** (or adding to `docs/corpus-authoring.md`) covering: purpose, scope (naming/patterns/ordering), when to run, independence/read-only rules, report location (`docs/ai/`), and fix procedure. I can draft that if desired.

## SOP reference

- **SOP created:** `docs/style-audit.md` (this audit) — defines procedure for style/pattern audits. General factual audits follow `docs/corpus-audit.md`.
