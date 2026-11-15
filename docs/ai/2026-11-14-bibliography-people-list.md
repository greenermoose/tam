# Bibliography People Table (corpus 0.92.2)

- **Date:** 2026-11-14
- **Time:** 22:50 to 22:55 -05:00
- **Stage:** Develop; adds an alphabetized People table to `knowledge/bibliography/index.html` and bumps corpus version to `0.92.2`
- **CLI Tool:** Antigravity CLI `1.5.9` (`agy --version`)
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** See `git log` for the `bibliography/index.html`, `version`, `ROADMAP.md`, and index follow-up commits.
- **Authorship:** Fred directed the task; Antigravity CLI researched the people and websites and wrote the HTML table, version bump, and provenance record.
- **Checked against:** `knowledge/bibliography/*.html`, `knowledge/tamlinux/community/peer-communities.html`, `knowledge/tamlinux/developer.html`, `knowledge/**/*.html`, and web searches for researcher homepages, lab pages, and blogs.

## Guiding prompts

> The tam knowledge corpus refers to people by their last name in places like the bibliography, but has no list of people. Create an alphabetized list of people under the People heading of the Bibliography. Alphabetize by first name. Create a table: left column is name of person, right column is whatever you know about them.
>
> Look up to see who Saltzer & Shroeder are. Any people that are mentioned on any page should be listed in the bibliography. If you can get personal blog sites or professional sites (like a lab page), include that in the second column of the table under People.

## Key decisions

- Sorted all 106 entries alphabetically by first name (including handles such as `Aylur`, `Elkowar`, `outfoxxed`, and `Vaxry` for named project creators).
- Included personal blogs, academic homepages, and lab pages in the second column where verified.
- Used `Fred Horch is the Tamlinux developer` per `docs/corpus-authoring.md`.

## Verification

- `make corpus-and-test` and `make check-repo` run and passed.
