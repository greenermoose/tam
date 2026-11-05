# Plan 26 Phase 2 utilities and shell hubs (corpus 0.72.1)

- **Date:** 2026-11-04
- **Time:** evening session, local time
- **Stage:** Develop — corpus pages and indexes
- **CLI Tool:** cursor 3.27.22
- **Model:** Composer (`composer`)
- **Transcript:** Retained privately by the author.
- **Commits:** (pending Fred's Commit word)
- **Authorship:** Fred said Proceed for Phase 2. Cursor wrote the six hubs,
  index links, coverage refresh, plan 26 status, and this record.
- **Checked against:** `tamlinux/commands` inventory via
  `report-component-coverage.py`; packaged bindings in
  `desktop/session/hypr/tamlinux/bindings/utilities.lua` and
  `clipboard.lua`; existing `use/desktop/{screenshots,clipboard,lock-and-idle,night-light,notifications}.html`
  recipes; `docs/corpus-style.md`.

## Guiding prompts

> Proceed.

(Prior turns in the same session accepted Phase 1 and authorized Phase 2
utilities/shell hubs.)

## Key decisions

- New hubs link day-to-day recipes under `use/desktop/` rather than
  rewriting them.
- Expert tables name exact command ids for coverage (launch/tui/webapp,
  toggle, notification, capture families).
- `shell/tools.html` and `shell/editors.html` stay under the learn-track
  parent `shell/index.html` (breadcrumb and track match).

## Verification

- `python3 tools/lint-corpus.py` — exit 0
- `python3 tools/build-index.py` — 573 entries
- `python3 tools/report-component-coverage.py --json …` — 109 covered /
  75 exempt / 63 gap (was 69 / 91 / 87 after Phase 1)
- `python3 -m unittest -q tests.test_build_index tests.test_component_coverage`
  — OK (fixture also copies the coverage tool and stubs its unit test for
  `make corpus-and-test`)
