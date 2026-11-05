# Plan 26 Phase 5 maintenance, W5 contributing and zero-gap coverage (corpus 0.75.0)

- **Date:** 2026-11-05
- **Time:** midday session, local time
- **Stage:** Develop — corpus pages and indexes
- **CLI Tool:** Antigravity CLI 1.5.1
- **Model:** Gemini 3.8 Flash (High)
- **Transcript:** Retained privately by the author.
- **Commits:** pending commit
- **Authorship:** Fred said Proceed to Phase 5. agy created `maintain/troubleshooting.html` and `improve/contributing.html` (W5), folded remaining tool coverage into `desktop/capture.html`, `improve/scripting.html`, and `desktop/config.html`, refreshed indexes and coverage, and authored this record.
- **Checked against:** `tamlinux/commands` inventory via `report-component-coverage.py`; command source implementations under `tamlinux/commands/` (`tam-crash-watch`, `tam-agent-crash`, `tam-qmlcache-purge`, `tam-desktop-test`, `tam-workstation-upgrade`, `tam-work`, `tam-dev`, `tam-fleet`, `tam-transcode`, `tam-transcode-ascii`, `tam-show-logo`, `tam-show-done`, `tam-branding-about`); `docs/corpus-style.md` and `docs/corpus-authoring.md`.

## Guiding prompts

> Commit phase 4. Proceed to phase 5.

## Key decisions

- Added `maintain/troubleshooting.html` covering crash diagnostics, core dumps, QML cache invalidation, desktop testing, and text-console workstation upgrades.
- Added `improve/contributing.html` (W5) covering bug reports, path claims via `tam-work`, local component checks with `tam-dev`, and fleet deployment with `tam-fleet`.
- Folded media transcoding (`tam-transcode`, `tam-transcode-ascii`) into `desktop/capture.html`, terminal banner tools (`tam-show-logo`, `tam-show-done`) into `improve/scripting.html`, and about-dialog branding (`tam-branding-about`) into `desktop/config.html`.
- Closes the final 12 component coverage gaps, achieving 0 remaining gaps (247 components: 176 covered / 71 exempt / 0 gap).
- Verified `python3 tools/report-component-coverage.py --check` exits with code 0.

## Verification

- `python3 tools/lint-corpus.py` — exit 0
- `python3 tools/build-index.py` — 582 entries
- `python3 tools/report-component-coverage.py --json knowledge/component-coverage.json` — 247 components: 176 covered / 71 exempt / 0 gap
- `python3 tools/report-component-coverage.py --check` — exit 0
- `python3 -m unittest -v tests/test_component_coverage.py` — 6 tests, OK
- `make corpus-and-test` — exit 0 (all unit tests, HTML linter, index validator, coverage test, and guide verification pass)
- Manual verification: `tam show --level 1/2/3 [PAGE] --pipe` renders cleanly across Novice, Learner, and Expert levels for all new pages.
