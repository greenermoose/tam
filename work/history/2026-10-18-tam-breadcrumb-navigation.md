# Breadcrumb navigation (tam command 0.31.1)

- **Date:** 2026-10-18
- **Time:** 14:46 to 14:53 -04:00
- **Stage:** Develop; corrected pager breadcrumb focus and navigation
- **CLI Tool:** Codex CLI `0.163.0`
- **Model:** `gpt-6-luna`
- **Commit:** `e0854f4`
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred requested the behavior and version bump; Codex implemented the pager change, tests and documentation.
- **Checked against:** `src/pager.c`, `man/tam.1`, `src/version.h`, `tests/run.sh`, `tam --version`

## Guiding prompt

> I can't reach the Guide and Man pages hyperlinks in the breadcrumb top bar of tam version 0.31.0. Is this intentional? If so, that is bad UI/UX design. I find it frustrating. Make it so I can up the up arrow to get to the breadcrumb top bar and can use the regular hyperlink navigation keys (a, s, d, f, and Tab and Shift + Tab) to reach the hyperlinks in the breadcrumb. This should be done for tam command version 0.31.1, correcting this bug in 0.31.0.

## Key decisions

Up Arrow at the first document line focuses the last linked breadcrumb; repeated Up Arrow moves toward earlier breadcrumbs. Down Arrow returns to page links. `a`/`f` move horizontally among breadcrumb links, `s`/`d` move vertically between the breadcrumb and page links, and Tab/Shift-Tab include the breadcrumb links in their normal sequence. Command version is 0.31.1; corpus version remains 0.32.0.

The source-upgrade plan's first activation had also been completed. Its acceptance records were closed with the plan index, roadmap and development contract.

## Verification

- `make test PREFIX="$HOME/.local/dev" LIBTAM_PREFIX="$HOME/.local/dev"`: passed command, upgrade, renderer, corpus and guide suites. Live Ollama checks were skipped because no server was running.
- PTY regression coverage on `tam man`: verified Up, `a/s/d/f`, Tab, Shift-Tab and Enter across both breadcrumb links.
- `git diff --check`: passed.
