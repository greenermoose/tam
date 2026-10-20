# Plan: Common pitfall → Best practice

**Plan status:** Completed
**Reviewed:** 2026-10-20
**Scope:** Corpus 0.35.2: the 16 `Common pitfall` sections become
`Best practice` sections that lead with the correct action.
**Authority:** The developer's instruction of 2026-10-20 to audit the
`Common pitfall` sections, plan the fixes and do the work; the
[audit report](../ai/2026-10-20-common-pitfall-to-best-practice-audit.md);
[plan maintenance procedure](../plan-maintenance.md).
**Next action:** None; delivered in `ae1fa41` (pages), `ca25c8f` (index) and
corpus 0.35.2 ([session record](../ai/2026-10-20-tam-common-pitfall-best-practice.md)).
Package stages remain developer-directed.

## Goal
Rename all `Common pitfall` sections to `Best practice` in the tam knowledge corpus and reorder content so each leads with the positive/correct action first, then explains what bad things happen if done the other way.

## Scope
16 pages under `knowledge/`:
- knowledge/ecosystem/lua.html
- knowledge/filesystem/permissions.html
- knowledge/hardware/resources.html
- knowledge/improve/ai-performance.html
- knowledge/improve/efficiency.html
- knowledge/improve/scripting.html
- knowledge/improve/terminal.html
- knowledge/improve/toolchains.html
- knowledge/improve/zram.html
- knowledge/maintain/logs.html
- knowledge/maintain/power.html
- knowledge/maintain/recovery.html
- knowledge/maintain/rollback.html
- knowledge/maintain/updates.html
- knowledge/shell/jobs.html
- knowledge/shell/pipelines.html

## Approach
Follow the fix procedure in `docs/ai/2026-10-20-common-pitfall-to-best-practice-audit.md`:
1. Claim each page (or batch) with `tam-work begin`
2. Read full page for context
3. Change `<h3>Common pitfall</h3>` → `<h3>Best practice</h3>`
4. Reorder: lead with correct action, move consequences/bad outcomes after
5. Preserve links/formatting/HTML validity
6. Test rendering if possible
7. Commit only edited pages
8. Bump corpus version if needed per `corpus-authoring.md`

## Verification
- `grep -rn "<h3>Common pitfall" knowledge/` returns nothing
- `grep -rn "<h3>Best practice" knowledge/ | wc -l` equals 16
- Render a few pages with `tam show --pipe` to check formatting
- `make corpus-and-test` passes

## Notes
- Style audit SOP: `docs/style-audit.md`
- Audit report: `docs/ai/2026-10-20-common-pitfall-to-best-practice-audit.md`
