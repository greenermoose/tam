# Update `tam:topics` wording and alphabetize topics (command 0.59.1)

- **Date:** 2026-11-14
- **Time:** 20:52 to 20:55 -05:00
- **Stage:** Develop — update `tam_topics_document` in `src/views.c` (command 0.59.1)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit of this change and record
- **Authorship:** Fred directed updating the `tam:topics` wording, removing the "Choose a subject" line, and alphabetizing the topics; Antigravity updated `src/views.c`, bumped `TAM_COMMAND_VERSION` to `0.59.1`, updated tests and roadmap/provenance records, and verified the build and tests.
- **Checked against:** `src/views.c`, `src/version.h`, `tests/run.sh`, `tests/test_library_discovery.py`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> Change the wording on the tam:topics page in the tam knowledge corpus from
>
> Guide > Browse subjects
>
> Browse subjects
>
> Choose a subject. Its opening page introduces the ideas, practical tasks, and reading order.
>
> to
>
> Guide > Browse topics
>
> Topics Covered in Your Tamlinux User Guide
>
> Remove the "Choose a subject" line. Alphabetize the topics, i.e. move Operating Systems to be after Networking and before Programming.

## Key decisions

- Updated `tam_topics_document` in `src/views.c` to use `"Browse topics"` for the page title and breadcrumb, `"Topics Covered in Your Tamlinux User Guide"` for the `<h1>` heading, and removed the `<p>Choose a subject...</p>` paragraph.
- Collected matching subject hub entries into an array and sorted them with `qsort` using `entry_title_order` so topics appear in alphabetical order by title.
- Bumped `TAM_COMMAND_VERSION` in `src/version.h` from `0.59.0` to `0.59.1`.

## Verification

- `make command`, `make test-command`, `python3 -m unittest tests/test_library_discovery.py`, and `make check-repo`: pass.
