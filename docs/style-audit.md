# Style audit (tam knowledge corpus)

Adopted 2026-10-20.

## Purpose

The corpus checker (`make corpus-and-test` / `tools/lint-corpus.py`) catches mechanical issues (entities, links, metadata). A **style audit** catches presentation and pattern issues that affect readability and teaching effectiveness:

- **Anti-patterns**: sections that lead with prohibitions instead of positive guidance
- **Naming consistency**: heading names that don't match the intended tone/pattern
- **Ordering**: explanations that bury the recommended action
- **Tone**: instructional ordering that teaches "what not to do" before "what to do"

## Rules

- **Independent.** The auditor did not write the pages under audit. One auditor per cycle.
- **Read-only.** The audit writes only its own report. Fixes are separate Develop work; an audit never edits corpus pages or bumps versions.
- **Advisory.** The developer decides which findings become fixes.
- **Follow-through.** The next audit re-checks findings marked fixed.

## When to run

- When the developer requests a style/pattern audit
- When adding new section patterns and consistency matters
- After style guide changes
- Periodically during corpus growth (opportunistic)

## Before you start

- Claim only your report file under `docs/ai/`.
- Note corpus version (`knowledge/version`) and command version (`src/version.h`) if relevant.
- Understand the target pattern from the style guide/request.

## Procedure

1. **Define scope**: What pattern/naming/style issue to look for (be specific).
2. **Inventory**: Use `grep`/`glob` to find all occurrences of the pattern across `knowledge/**/*.html`.
3. **Classify**: For each occurrence, read the section and classify structure (current vs desired). Note special cases.
4. **Document**: Create a report in `docs/ai/<date>-<slug>-audit.md` with findings table, per-item fix directions, fix procedure, and verification checklist.
5. **Suggest follow-up**: Group as corpus fixes; no automatic edits.

## Report format

Follow `docs/ai/TEMPLATE.md`. Include:

- **Summary**: count, pattern found, desired pattern, note if SOP exists/needed
- **Findings table**: ID, page, current state, structure notes
- **Per-page fix directions**: concrete before/after guidance for each
- **Fix procedure**: claim paths, read context, make changes, test, commit (only edited pages), verification
- **Verification checklist**: grep commands to confirm completion

## Anti-patterns

- Fixing inside the audit (couples review and authorship)
- Vague findings without concrete fix directions
- Editing without reading surrounding context
- Bulk changes without verification

## Example

The 2026-10-20 audit for `Common pitfall` → `Best practice` (leading with positive guidance) is recorded in `docs/ai/2026-10-20-common-pitfall-to-best-practice-audit.md`.
