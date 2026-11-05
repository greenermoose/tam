# Plan 26 Phase 3 hardware and configuration hubs (corpus 0.73.0)

- **Date:** 2026-11-05
- **Time:** morning session, local time
- **Stage:** Develop — corpus pages and indexes
- **CLI Tool:** Antigravity CLI 1.5.1
- **Model:** Gemini 3.8 Flash (High)
- **Transcript:** Retained privately by the author.
- **Commits:** pending commit
- **Authorship:** Fred said Proceed and accepted Phase 3 recommendations (ballot 3.1A, 3.2A, 3.3A). agy created the new hardware, config, and plugin hubs, extended power and networking pages, refreshed indexes and component coverage, and authored this record.
- **Checked against:** `tamlinux/commands` inventory via `report-component-coverage.py`; command source implementations under `tamlinux/commands/` (`tam-monitor`, `tam-hyprland-monitor-*`, `tam-battery-*`, `tam-powerprofiles-*`, `suspend-unless-stay-awake`, `tam-hibernation-available`, `tam-nut`, `tam-hardware`, `tam-disk-speedtest`, `tam-drive-*`, `tam-default-*`, `tam-browser-command`, `tam-desktop-mode`, `tam-session-init`, `tam-settings-preserve`, `tam-plugin`); `docs/corpus-style.md` and `docs/corpus-authoring.md`.

## Guiding prompts

> I accept your recommendations. Proceed.

## Key decisions

- Hubs created: `hardware/displays.html`, `hardware/input.html`, `hardware/drives.html`, `desktop/config.html`, and `improve/plugins.html`.
- Extended `maintain/power.html` with battery status, power profiles, Stay Awake toggle behavior, hibernation capability check, and laptop bootstrap environment verification (`tam-nut`).
- Extended `hardware/networking.html` with weather and location coordinate configuration (`tam-weather-location`).
- Expert reference tables name exact command IDs, closing 34 gaps directly (reducing remaining gaps from 63 to 29).

## Verification

- `python3 tools/lint-corpus.py` — exit 0
- `python3 tools/build-index.py` — 578 entries
- `python3 tools/report-component-coverage.py --json knowledge/component-coverage.json` — 247 components: 143 covered / 75 exempt / 29 gap (was 109 covered / 75 exempt / 63 gap)
- `make corpus-and-test` — exit 0 (all unit tests, HTML linter, index validator, coverage test, and guide verification pass)
- Manual verification: `tam show --level 1/2/3 [PAGE] --pipe` renders cleanly across Novice, Learner, and Expert levels for all new pages.
