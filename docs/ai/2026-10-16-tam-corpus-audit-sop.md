# SOP: Recurring Tam Knowledge Corpus Audit

- **Date:** 2026-10-16
- **Status:** Proposal for adoption (reviewed by the maintainer before use).
- **Applies to:** the `tam` repository, **corpus lane** (`knowledge/`) as the
  subject, with read-only use of the command lane (`src/`, `Makefile`) and the
  product repository as the authority.
- **Companion:** `docs/ai/2026-10-16-tam-corpus-audit.md` (the first run).

## Why

The corpus is product documentation rendered by a bespoke renderer. It drifts from
the product in three recurring ways, all seen in the first audit:

1. **Behaviour drift** — a page describes default keybindings, commands, or paths
   that the product no longer ships (e.g. Omarchy keybindings, superseded session
   command names).
2. **Status drift** — planned or stubbed features are written as if implemented
   (e.g. `tam install plan/apply/verify`).
3. **Render drift** — the page uses markup the renderer cannot display (entities
   outside the whitelist, LaTeX math).

A single author reading their own page cannot reliably catch (1)–(3). The audit is
therefore run on a cadence by a **different agent each cycle**, and findings are
advisory: the fix is separate Develop work.

## Independence rule (most important)

- **The auditor must not have authored the pages under audit.** Rotate across the
  agent families (Claude, Codex, agy, opencode) so each cycle sees the corpus with
  fresh eyes and different tooling.
- **One auditor per cycle**, reporting to the maintainer/orchestrator, who reviews
  findings and decides which become fixes. An audit never edits the corpus.
- **No self-audit of a cycle's own fixes:** a later cycle, by a different agent,
  verifies that prior findings were actually resolved.

## Cadence and triggers

- **Cadence:** at each corpus minor version, and at least monthly during active
  corpus expansion.
- **Triggers:** a corpus milestone lands; a product change renames commands,
  keybindings, paths, or versions; a new agent joins; before a release candidate
  that freezes the corpus.

## Preconditions

1. Claim read scope with `tam-work`. An audit is read-only outside its own output
   record, but the corpus lane is claimed (or a read-only claim recorded) so the
   tree is not changed under the auditor mid-run.
   - New output file conflicts with nobody: claim
     `docs/ai/<date>-tam-corpus-audit.md`.
2. Freeze the reference points and record them in the report:
   - corpus version (`knowledge/version`) and command version (`src/version.h`);
   - the product repository commit used as authority;
   - the installed command used for rendering checks (`tam --version`), and its
     known gap (the installed build may lag the checkout — note it).
3. If rendering behaviour must be observed rather than inferred, install the
   development build per the repository `AGENTS.md` and confirm `tam --version`
   shows the checkout's versions.

## Procedure

Work in passes. Each pass has a mechanical check and produces evidence
(`file:line` plus the authoritative `file:line`).

### Pass 1 — Inventory and metadata
- List every page; run `python3 tools/build-index.py` and confirm it validates
  metadata and breadcrumbs with no error.
- Confirm page count and first-level nav links are consistent.

### Pass 2 — Command and flag truth
- Extract every `tam …` invocation, subcommand, alias, option, and `check` ID from
  the corpus.
- Cross-check each against `src/tam.c`, `src/install.c`, and the subcommand help.
- Flag: subcommands that do not exist; planned/stubbed verbs written as working;
  flags that no parser accepts; `check` IDs not in the contract.

### Pass 3 — Environment truth
- Extract every claim about the desktop, keybindings, session/login, packages,
  versions, and file paths.
- Cross-check keybindings against the product bindings
  (`desktop/session/hypr/tamlinux/bindings/`), **not** the live desktop
  configuration (which carries personal overrides).
- Cross-check session/login and packaging names against the product's
  `desktop/native/integration/` and `desktop/tests/`.
- Cross-check package versions with the system (`pacman -Q`, `readelf`), noting the
  test machine.
- Flag any place a **planned** stack (e.g. a future compositor) is written as the
  **current** one.

### Pass 4 — Internal references
- Extract every capitalised identifier and parenthetical citation (e.g. Virtue
  Vnn, Decision Dnnn) and confirm each is defined in the corpus.
- Flag undefined identifiers, and any reference to an internal/private record
  (boundary concern — public corpus must not carry private identifiers).

### Pass 5 — Render truth
- Extract every HTML entity and every `$…$`/LaTeX fragment.
- Cross-check entities against the renderer whitelist in `src/render.c`; flag any
  outside it.
- Flag LaTeX/TeX markup, since the renderer has no math support.
- For a sample of pages, render through the command (`tam show --pipe <page>` and
  the depth ladder `tam explain --level …`) and read the output as a user would.

### Pass 6 — Prose and consistency
- Terminology consistency (one name/spelling per concept).
- Grammar and article errors.
- Example blocks: are command and expected output clearly labelled and current?
- Cross-page contradictions (the same fact stated two ways).

## Severity rubric

| Severity | Meaning |
| --- | --- |
| High | User-visible wrong behaviour, or a policy/boundary violation (wrong keybindings, non-existent commands presented as working, private references). |
| Medium | Correct concept, wrong detail, or broken rendering (superseded names, unrendered markup, planned-vs-current confusion). |
| Low | Style, grammar, stale example, undefined term that a paragraph could fix. |

## Output

- One dated record under `docs/ai/` named `<date>-tam-corpus-audit.md`.
- A findings table with IDs, severity, area, one-line summary.
- Per finding: the page claim with `file:line`, the authoritative source with
  `file:line`, and a fix direction.
- A "verified correct" section so the audit also records what held.
- Recommended follow-up grouped by lane (corpus fix / command fix / maintainer
  ruling), each a separate scoped change. **The audit does not bump a version**;
  fixes take a patch bump per the repository `AGENTS.md`.
- AI provenance per the repository `AGENTS.md`: tool and model, and a row in
  `AI_PROVENANCE.md`.

## Rotation and follow-through

- Record the auditor (tool/model) and the previous cycle's auditor in the report so
  rotation and independence are auditable.
- The maintainer/orchestrator triages findings into the backlog, assigning fixes as
  separate Develop work on the correct lane.
- The next cycle's auditor, from a different family, first re-checks the previous
  report's findings that were marked fixed.

## Tooling (suggested, optional)

A small harness reduces drudgery and makes passes repeatable:
- Pass 2/3/5 are largely mechanical: extract `tam …` tokens, `&entity;` tokens, and
  `$…$` runs from `knowledge/**/*.html`, then grep the governing source files.
- Keep such a script in the corpus lane (e.g. `tools/`) as a new file so it follows
  the lane's tests; it is a development aid, not part of the shipped corpus.

## Anti-patterns

- Auditing by reading only (misses Pass 2-5 mechanical drift).
- Checking keybindings against the live desktop rather than the product bindings.
- Fixing findings inside the audit; that couples review and authorship and defeats
  the independence rule.
- Trusting "the page looks fine in a browser": the reader sees the **rendered**
  text, not the HTML.
