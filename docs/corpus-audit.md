# Auditing the tam knowledge corpus

Adopted by the developer on 2026-10-16, narrowed from the proposal in
`docs/ai/2026-10-16-tam-corpus-audit-sop.md`, which also records the first
audit.

## Purpose

Mechanical drift, such as unrendered markup, private strings and nonexistent
subcommands, belongs to the corpus checker that `make corpus-and-test` runs. An
audit finds what the checker cannot:

- **behaviour drift:** a page describes bindings, commands, names or paths the
  product no longer ships;
- **status drift:** planned or stubbed work written as if it exists;
- **unsupported claims:** statements about Tamlinux, including its history and
  choices, that no source records (see `docs/corpus-authoring.md`).

## Rules

- **Independent.** The auditor did not write the pages under audit. One
  auditor per cycle.
- **Read-only.** The audit writes only its own report. Fixes are separate
  Develop work, and an audit never bumps a version.
- **Advisory.** The developer decides which findings become fixes.
- **Follow-through.** The next audit first re-checks the previous report's
  findings marked fixed.

## Who and when

- **Auditor:** an independent agent chosen and directed by the developer.
  Any tool may audit; it must not have written the pages under review.
  Delegation was retired on 2026-10-16; do not dispatch another agent.
- **When:** after about ten new or substantially changed pages since the last
  audit, or monthly, whichever comes first, and whenever the developer asks.
- An audit never gates a Tamlinux candidate.

## Before you start

- Claim only your report file.
- Record the corpus version (`knowledge/version`), the command version
  (`src/version.h`), and the product repository commit you used as the
  authority.

## Passes

1. **Checker.** Run `make corpus-and-test` and record the result. Do not repeat
   its checks by hand; until the checker exists, check entities against
   `entities[]` in `src/render.c` and search for LaTeX.
2. **Behaviour and status.** Check each page's descriptions of commands,
   bindings, login, packaging and paths against "Where facts live" in
   `docs/corpus-authoring.md`. Check keybindings against the product
   bindings, never the live desktop. Flag planned work written as current.
   Use the current plan index and contract, not a historical plan's task list.
   Corpus 0.31.0's organization and learning review follows its authoritative
   plan; it does not replace this independent factual audit.
3. **Unsupported claims.** Every sentence saying what Tamlinux chose, uses,
   evaluated or rejected needs a source. Flag those without one.
4. **Recorded checks.** Pick three recent session records and confirm that
   their `Checked against:` sources support the pages they cover.
5. **Reading.** Render a sample of pages with `tam show --pipe` and read them
   as a user would, looking for contradictions between pages and unlabelled
   example output.

## Severity

| Severity | Meaning |
| --- | --- |
| High | Wrong behaviour a user will act on, or a public-boundary violation |
| Medium | Right concept, wrong detail; planned written as current |
| Low | Style, grammar, stale example, undefined term |

## Report

- One record, `docs/ai/<date>-tam-corpus-audit.md`, with the usual
  provenance fields from `docs/ai/TEMPLATE.md`.
- A findings table: ID, severity, page, one line.
- For each finding: the page's claim at `file:line`, the authority at
  `file:line`, and a fix direction. Do not restate the page.
- A short list of what was verified correct.
- Follow-up grouped as corpus fix, command fix, or developer ruling.
