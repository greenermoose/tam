# Plain text README conversion

- **Date:** 2026-11-13
- **Time:** 20:15 to 20:38 -0500 (session window).
- **Stage:** Develop; repository tooling and documentation, no component version change.
- **CLI Tool:** Codex CLI 0.169.0, checked live.
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`), verified from the transcript.
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record.
- **Authorship:** Fred approved the conversion plan; Codex converted the three
  README files and updated references, the checker and fixtures.
- **Checked against:** Tracked originals at `9f1d1d4df14c`, `tools/check-repo.py`,
  `tests/test_check_repo.py`, developer and coverage fixtures, and the plan index.

## Guiding prompt

> Read the approved plan at [redacted: local path] and implement it.

## Key decisions

Use plain text Plan, Status and Disposition records. Keep Plan lines intact,
including long titles and nested paths, while preserving each full disposition.
Retain missing, duplicate and nonexistent-plan checks. Check angle-bracket links
in plain README files as well as Markdown links in other documents. Preserve
historical filename records and upstream filenames. Component versions stay
unchanged because this changes repository tooling and documentation.

## Verification

- Checker fixtures: 13 tests pass, including missing index, long titles and
  nested paths; duplicate, missing and nonexistent plans remain covered.
- Developer workflow fixtures: 22 tests pass.
- Component coverage fixtures: 7 tests pass.
- `make check-repo` passes.
- Original prose, table cells, code blocks and links compared with replacements.
- `git diff --check` passes.
