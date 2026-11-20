# Update `tam:catalog` wording and table headers (command 0.59.2)

- **Date:** 2026-11-14
- **Time:** 21:41 to 21:46 -05:00
- **Stage:** Develop — update `tam_catalog_document` in `src/views.c` (command 0.59.2)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit of this change and record
- **Authorship:** Fred directed updating the `tam:catalog` wording and table headers; Antigravity updated `src/views.c`, bumped `TAM_COMMAND_VERSION` to `0.59.2`, updated tests and roadmap/provenance records, and verified the build and tests.
- **Checked against:** `src/views.c`, `src/version.h`, `knowledge/index.tsv`, `tests/run.sh`, `tests/test_library_discovery.py`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> Update tam:catalog in the tam knowledge corpus. The glossary page names have changed and the new names should be reflected in this list.
>
> Change
>
> Guide > Topics
> Topics A to Z
>
> Every topic tam can explain, one line each.
> Select a topic with Tab and press Enter to read it. Press ` for the guide index, or q to quit.
>
> Topic                           Covers
>
> to
>
> Guide > Catalog
> All Pages A to Z
>
> Page                            Subject Matter
>
> Every page in the knowledge corpus should be listed, sorted alphabetically by title.

## Key decisions

- Updated `tam_catalog_document` in `src/views.c` to use `"Catalog"` for the breadcrumb, `"All Pages A to Z"` for the page title and `<h1>` heading, removed the `<p>Every topic tam can explain...</p>` paragraph, and changed the table headers to `"Page"` and `"Subject Matter"`.
- Verified that `tam_catalog_document` already loads all entries from `index.tsv` (including the updated glossary page titles) and sorts them alphabetically by title via `entry_title_order`.
- Bumped `TAM_COMMAND_VERSION` in `src/version.h` from `0.59.1` to `0.59.2`.

## Verification

- `make command`, `make test-command`, `python3 -m unittest tests/test_library_discovery.py`, and `make check-repo`: pass.
