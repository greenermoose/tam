# Plain guide openings (corpus 0.45.2)

- **Date:** 2026-10-27
- **Time:** 14:58 to 15:04 -04:00
- **Stage:** Develop, corpus and authoring guidance
- **CLI Tool:** Codex CLI `0.164.0`
- **Model:** `gpt-6.1-terra`
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record and the subsequent index refresh.
- **Authorship:** Fred directed the writing standard and Codex revised the openings and guidance.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/templates/`, the existing corpus pages and their navigation, `tools/lint-corpus.py`, and the existing learning plan.

## Guiding prompts

> Okay, we have a problem. This page starts with a bunch of jargon and hard-to-parse lines.
>
> Revise based on the following:
>
> 1) The header and the first sentence immediately after it must be free of jardon and written in a way that a person with no background at all can understand what the page is about. Any technical terms, such as "terminal", "file", and "task" must be defined (or at least link to a definition).
> 2) The lines Gain: and For you if: and Levels: are well intentioned but fail because it is completely unclear why they are there and what they mean.
>
> I suggest the following standardized approach on every page:
>
> 1) Clear heading immediately after the header lines (La b or c)
> 2) Plain English "orientation" sentence immediately after the header. No special formatting, prefixed or suffixes. Just a plain English sentence that explains to a reader what the page is about. Assume only a high school education, no specialized technical knowledge.
>
> Ask if you have questions about this.

> Those refer to navigation to Guide, then the levels (beginner, intermediate, or expert). The pager code gives each line displayed either a letter (the top line is a, followed by b, and optionally c). Then the header is in line 1 (shown as L1 in the pager footer).

> Read and revise every page in the corpus to ensure none start with gibberish, cryptic labels, or difficult-to-understand technical language.

> Install this version of the tam corpus.

## Key decisions

Reviewed all 128 page openings and gave each an unformatted orientation sentence. Rewrote dense introductory context, made headings and their navigation trails consistent, and removed the labeled introductions from the learning paths. Retained the rescue path's safety guidance. Updated the authoring guidance and templates so future pages use the same opening. This is a source review, not a claim of learner acceptance.

## Verification

- Checked all 128 pages have a plain paragraph immediately after the page heading.
- `make corpus-and-test`: corpus lint, index tests and guide checks passed.

The requested development installation exposed command-suite assertions tied to former page titles. The follow-up change updates those existing assertions to the new titles and checks the resource search by its destination; it does not reorganize the held live-corpus tests.
