# `tam-bundle` page and bundle glossary entry (corpus 0.78.0, 0.78.1)

- **Date:** 2026-11-09
- **Time:** about 21:15 to 21:55 -05:00
- **Stage:** Develop: a new explanation page and glossary entry for
  `tam-bundle`, a tool in development in `tamlinux-tools`.
- **CLI Tool:** Claude Code 2.1.321
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `ecd5238`, `e25add2`, and the 0.78.1 correction with its index follow-up.
- **Authorship:** Fred asked for the `tam` pages; Claude wrote both pages,
  the links, the version and roadmap rows, and this record.
- **Checked against:** `tamlinux-tools` `src/tam-bundle/*.c`, `tb.h`,
  `README.md`, `man/tam-bundle.1`, `docs/plans/tam-bundle.md`,
  `tests/tam-bundle/run.sh`, `tam-bundle --help`; the worked example was
  captured by running it with `tam-bundle` 0.4.0 in a `mktemp -d` folder,
  removed afterwards.

## Guiding prompts

> Continue plan 53 step 4: the man page and tam pages.

## Key decisions

- **Marked as in development throughout.** No Tamlinux release delivers
  `tam-bundle`; the README says so. Every statement about Tamlinux using it
  (the product slot, the native package, `tam-deploy` calling it) is written
  as a plan, sourced from the public design.
- **Not added to a learning path.** The linter warns that the Learner section
  has no learning-path links. Putting an undelivered tool on "Understand the
  design" would send readers to a command they do not have; it can join the
  path when Tamlinux delivers it.
- **The design and the code disagreed on one point.** The design said every
  command runs the recovery check first; in `tam-bundle` 0.4.0 only `switch`
  and `back` did. The developer accepted the recommendation to change the
  code; `tam-bundle` 0.4.1 makes every changing command recover first, and
  corpus 0.78.1 corrects the expert section to match.

## Follow-up prompts

> Ponder this question: shoudl the design be corrected or the code changed? Do some research. Then give me your recommendation backed up with evidence from credible authorities.

> I accept your recommendation. Make those changes.

## Verification

- `make corpus-and-test`: linter, unit tests, component coverage check,
  guide tests and repository check pass; one warning, the learning path
  above.
- Read both pages with `build/tam show --pipe`.
