# Pager quick keys, address display, status row, and Guide index (command 0.34.0 / corpus 0.35.1)

- **Date:** 2026-10-20
- **Time:** 16:30 to 17:20 -04:00
- **Stage:** Develop; command 0.34.0 and corpus 0.35.1
- **CLI Tool:** agy 1.4.3
- **Model:** gemini-3.8-flash-high (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `c2bf223`, `6e2d809`, `2d2c074`, `775eb42`, and the commit that adds this record
- **Authorship:** The developer assigned implementation of plan `tam/docs/plans/pager-quick-keys.md` (TCK15–TCK19); Antigravity built each slice in order, ran `make test` before each commit, and authored the code, tests, documentation, and close-out.
- **Checked against:** `tam/docs/plans/pager-quick-keys.md`, `tam/docs/corpus-authoring.md`, `tam/AGENTS.md`, `tam/src/version.h`, `tam/knowledge/version`, `tam/man/tam.1`, `tam --help`, `build/tam show --pipe`

## Guiding prompts

> Develop: build tam/docs/plans/pager-quick-keys.md as written, TCK15
> through TCK19 in order. Read tam/AGENTS.md first. Claim only the paths
> each slice names, run make test before each commit, and commit after
> each slice. Follow the plan's "Stop and ask the developer" list. Do not
> push or install. When you finish, report plan adherence for each slice:
> followed as written, deviated (how and why), or needed rework.

## Key decisions

Plan adherence by slice:
- **TCK15:** Followed as written. Bound `]` and `[` to next/previous link anywhere; restricted `N` to previous search hit when search query is active; bumped command version to 0.34.0 in `src/version.h`, `tests/run.sh`, `docs/plans/development.md`, and `ROADMAP.md` (including delivered-history table to pass `tools/check-repo.py`); added tests in `tests/run.sh`.
- **TCK16:** Followed as written. Formatted status line position to `L<cursor> <first>-<last>/<total>`; removed `(All)`, `(Top)`, `(Bot)` qualifiers; handled empty documents (`0-0/0`); updated status assertions across tests and added viewport calculation tests in `tests/pager_screen.py`.
- **TCK17:** Followed as written. Added `show_address` to `pager_t`, implemented `pager_address()` handling stdin, prefixed schemes, and guide paths (`page:<path>`), formatted status line to `Path = <address>` with `status_byte_limit()`, added dismissal on `Esc`, page change, or second `=`, updated `tam show` to strip `page:` prefix (error exit 2 if empty) and route `tam:explain` and `tam:keywords`, updated `man/tam.1`, and added interactive and non-interactive tests in `tests/run.sh`.
- **TCK18:** Followed as written. Bound `\` to guide search; updated prompt labels to `Search in page /` and `Search in guide \`; updated help overlay in `src/pager.c`'s `draw()` to 8 lines detailing all quick keys; updated `print_man_help()` and `print_info_help()` to cite `] / [`; handled `tam:help` and `tam:prompt:page-search` in `follow()`; added `tests/fixtures/guide/quick-keys.html` and interactive PTY tests in `tests/run.sh`.
- **TCK19:** Followed as written. Updated `knowledge/index.html` with Guide topics, Learn more, and Quick keys table; updated `tests/guide.sh` link expectations; updated `knowledge/welcome.html` and `README.md` key descriptions and status row documentation; bumped corpus version in `knowledge/version` to 0.35.1; ran `make corpus` and `make test`; closed out plan, plan index, roadmap, development contract, AGENTS.md, AI provenance, and this session record.

## Verification

- `make test` run before each slice commit (TCK15, TCK16, TCK17, TCK18, TCK19). All test suites pass without warnings or regressions.
- `TAM_KNOWLEDGE_DIR=$PWD/knowledge build/tam show --pipe` verified: table renders cleanly with aligned columns.
- `python3 tools/lint-corpus.py` and `python3 tools/check-repo.py` pass cleanly.
