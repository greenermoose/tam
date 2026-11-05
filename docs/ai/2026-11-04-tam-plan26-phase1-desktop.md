# Plan 26 Phase 1 desktop hubs (corpus 0.72.0)

- **Date:** 2026-11-04
- **Time:** evening session, local time
- **Stage:** Develop — corpus pages and indexes
- **CLI Tool:** cursor 3.27.22
- **Model:** Composer (`composer`)
- **Transcript:** Retained privately by the author.
- **Commits:** (pending commit that adds this record)
- **Authorship:** Fred accepted plan 26 Phase 1 ballot (full ladders; four hubs;
  name all plugins and binding files; Omarchy for ideas only). Cursor wrote
  the pages, indexes, provenance, and coverage refresh.
- **Checked against:** `tamlinux` repository
  `desktop/session/hypr/tamlinux/bindings/*.lua`,
  `desktop/plugins/fred.*/` manifests; existing
  `knowledge/desktop/index.html` and related desktop pages; `docs/corpus-style.md`.

## Guiding prompts

> I accept all your recommendations. Next?

(Prior turns in the same session accepted W0/W1 style and exemption ballots
and authorized Phase 1 desktop pages.)

## Key decisions

- Transition page lives at `desktop/transition.html` (Using track) because
  the separate Learning track was retired (D221); plan 26's old
  `learn/transition.html` path was adjusted.
- Novice/learner/expert ladders with short Learner walkthroughs (D225).
- Binding file basenames and every `fred.*` plugin id named for coverage.

## Verification

- `python3 tools/lint-corpus.py` — exit 0
- `python3 tools/build-index.py` — 567 entries
- `python3 tools/report-component-coverage.py --json …` — plugins and
  binding files all `covered`; gaps reduced (69 covered / 91 exempt / 87 gap
  at generation time)
- `python3 -m unittest -q tests.test_component_coverage` — OK
