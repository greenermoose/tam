# Learning levels are paths, not routes (corpus 0.33.3)

- **Date:** 2026-10-19
- **Time:** 22:45 to 22:55 -04:00
- **Stage:** Develop; eight corpus pages, `docs/corpus-authoring.md`, roadmap header
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer chose the term. Claude Code found every
  learning-level use of "route" and wrote the edits.
- **Checked against:** `grep -rn route knowledge/`, `tam show --pipe learn/index`
  from the checkout build, `make corpus-and-test`

## Guiding prompts

> I see we sometimes use "route" and sometimes "path" to describe our three
> levels of understanding that you can follow in the learning section
> (learn/index.html). Let's standardize on the term "path." So, "Choose a
> path", "Beginner path", etc.

## Key decisions

- Titles, headings, breadcrumbs and links now read "Beginner path",
  "Intermediate path", "Expert path" and "Choose a path". This replaces the
  mixed "The beginner's path" / "Beginner route" wording. Topic slugs
  (`beginner-path` and the others) and page addresses are unchanged.
- Boot recovery pages keep "fallback route" and "rescue route", which name a
  different idea. `tam:search` and other reader URIs are unaffected.
- `docs/corpus-authoring.md` gains a Terms section so later pages keep the
  rule. Wording correction only: corpus patch bump to 0.33.3.

## Verification

`make corpus-and-test` passed, including lint and all guide tests (the staged
install check is skipped when make is unavailable inside the test).
