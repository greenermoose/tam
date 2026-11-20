# Arch host profile without an inherited OS ID (command 0.50.4)

- **Date:** 2026-11-09
- **Time:** 22:25 to 22:40 -05:00
- **Stage:** Develop; command 0.50.3 to 0.50.4
- **CLI Tool:** Claude Code 2.1.321
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer chose to make the change in tam rather than
  add patch support to bundle recipes; Claude Code wrote the change and
  this record.
- **Checked against:** `src/install.c`; the `tamlinux` repository's
  `desktop/native/providers/tam-arch-host-profile.patch`, which made the
  same change at packaging time.

## Guiding prompts

> Start on the recipes.

Answer to the question of how to handle the packaging patch:

> Change tam itself (Recommended)

## Key decisions

The new bundle recipes build each component from its exact commit with
`make` and `make install` only, and apply no patches. The packaging patch
existed only to drop the `omarchy` ID from the host-profile check, and its
header asked to be retired once the source carried the change. Arch hosts
are still recognized through `ID=arch` or an `ID_LIKE` containing `arch`.

## Verification

`make test`: all suites pass.
