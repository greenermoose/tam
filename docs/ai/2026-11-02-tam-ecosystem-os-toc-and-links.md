# Table of contents, alphabetization, and website links for ecosystem/os pages (corpus 0.58.1)

- **Date:** 2026-11-02
- **Time:** 08:56 to 09:22 -05:00
- **Stage:** Develop; corpus 0.58.1
- **CLI Tool:** Antigravity CLI 1.4.12
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** `<pending>`
- **Authorship:** Fred Horch directed the additions, scope clarification, and field formatting; Antigravity drafted the plan, added the tables of contents, alphabetized the sections, and added the official website links.
- **Checked against:** `knowledge/ecosystem/os/*.html`, `src/pager_nav.c`, `src/render.c`, `tools/lint-corpus.py`, `tools/build-index.py`, and `tests/guide.sh`.

## Guiding prompts

> For the ecosystem/os pages in the tam knowledge corpus, do the following:
>
> 1) Provide a table of contents for pages that list operating systems. This should be a list of the systems described on the page and allow you to quickly see all of them and jump to the section that describes each one.
> 2) Alpbabetize so that it is easier to find the one you want.
> 3) Add external links to learn more about each OS. I should be able to follow a link with a web browser to get to the best page on the Internet for learning more about that operating system.
>
> Ask if you have questions about this project.

> ready

> 1 yes, include microkernels and environment-minded alongside the 9 core OS catalog pages.
> 2 Website:
>
> Plan looks good. Close the review window. Proceed.

## Key decisions

In-page tables of contents (`<h2>Contents</h2>` with `href="#<id>"` anchors) were added to all 11 operating system catalog pages in `knowledge/ecosystem/os/`: the 9 primary family pages (`arch-family.html`, `bsd.html`, `debian-family.html`, `declarative.html`, `fedora-family.html`, `illumos.html`, `independent-linux.html`, `opensuse-family.html`, `other-systems.html`) plus `microkernels.html` and `environment-minded.html`. All described operating systems and distributions were sorted alphabetically, and authoritative project homepage links were added using the `Website:` field. The hub page `ecosystem/os/index.html` was updated so its bullet summaries reflect the new alphabetical ordering.

## Verification

`make corpus` cleanly regenerated the corpus indexes.
`make corpus-and-test` passed: `tools/lint-corpus.py`, `tests/test_lint_corpus.py`, `tests/test_build_index.py`, and `tests/guide.sh` all reported 0 errors. Verified `tam show --pipe` rendering on all modified pages.
