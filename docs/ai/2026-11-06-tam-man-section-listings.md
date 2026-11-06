# Complete manual section browsing (command 0.49.1)

- **Date:** 2026-11-06
- **Time:** 16:30 to 16:42 -05:00
- **Stage:** Develop; source fix, no installation or stage promotion.
- **CLI Tool:** Codex CLI 0.167.0, checked from the running binary.
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`), checked against runtime metadata.
- **Transcript:** Retained privately by the author.
- **Commits:** The commit containing this record.
- **Authorship:** Fred supplied the report and scope; Codex wrote the fix and regression checks.
- **Checked against:** `src/man.c`, `src/pager_search.c`, `src/pager_input.c`,
  `tests/run.sh`, the libtam execution API and implementation, and local
  `apropos --help` and installed manual indexes.

## Guiding prompts

> [Image #1] Investigate how tam interacts with man. Fix this bug. We are getting incomplete results and the results are not alphabetized. Expected output is a complete, alphabetized list of Manual sectio 1 pages that I can scroll through to find or search to find exactly what I am looking for. Instead, I get a sub-set presented in what appears to be a haphazard order.

> All sections of the man page should be able to be listed completely.

## Key decisions

Section browsing becomes one complete document so in-page search reaches every
entry. Capture grows with the installed index under the existing deadline;
failed lookups show an error instead of a partial section. Sort names without
case distinctions, break ties deterministically, and collapse duplicate
page/section entries across databases. Accept namespace/template names and
names up to the filesystem name limit. Keyword searches retain bounded paging. The provenance check also removed two
pre-existing private references from historical records, preserving the old
transcript mapping privately.

## Verification

- `make test-command`: command integration and terminal interaction tests,
  upgrade tests, and seven renderer goldens passed.
- Reversed fixtures exceeding 96 KiB cover sections 1–8, empty section 9,
  subsection 3p, duplicate/case/long names, failed lookups, and legacy offsets.
  Terminal checks search near the end and scroll to the alphabetical tail;
  long, namespace and template links open through the manual loader.
- Compared source-build output with every indexed page in local sections 1–9
  and subsection 3p: exact set equality, no duplicates, alphabetical order.
  Section 1: 2,749 entries; section 3: 20,023 entries.
- `git diff --check` passed. Package installation and Fred's visual acceptance
  remain separate.
