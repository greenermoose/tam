# Underline interior spaces in links (command 0.55.2)

- **Date:** 2026-11-14
- **Time:** 11:01 to 11:05 -05:00
- **Stage:** Develop — underline interior spaces in unselected links in `src/pager_draw.c` (`command 0.55.2`)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending
- **Authorship:** Fred reported the missing underline on interior spaces in unselected links and directed the fix; Antigravity CLI updated `src/pager_draw.c`, added a regression assertion in `tests/run.sh`, bumped the command version to `0.55.2`, and updated `ROADMAP.md` and `docs/plans/development.md`.
- **Checked against:** `src/pager_draw.c`, `src/version.h`, `tests/run.sh`, `tests/fixtures/link-leading-space.html`, `tests/fixtures/link-wrap.html`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> Change the tam command so that interior spaces in a link get underlines. See how Guide topics is one link, but the inside space is not underlined? That's a bug. That inside space should be underlined.

## Key decisions

None beyond the prompt: `link_space_is_interior` was already used for selected links in `emit_line`, so applying `interior_space` to all link spans preserves leading and trailing boundary spaces as unstyled while underlining interior spaces in unselected links.

## Verification

- `make command`: built `build/tam` with zero warnings.
- `make test-command`: `tests/run.sh` (all command integration tests including `link-leading-space` unselected interior-space underline check) and `test_upgrade.py` (15 tests) passed; `tests/render.sh` failed on pre-existing `blocks.txt` (`[1 Novice] Beginner section` vs `Beginner section`), unrelated to this change.
- `make check-repo`: passed.
