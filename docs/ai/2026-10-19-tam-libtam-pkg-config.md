# Find libtam with pkg-config (build only; no version change)

- **Date:** 2026-10-19
- **Time:** 21:33 to 21:50 -04:00
- **Stage:** Develop: the Makefile finds `libtam` with pkg-config and stops with instructions when it is missing
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `8bed0ed` and the commit that adds this record; with `libtam` `aada139` and `tamlinux-tools` `e864442`
- **Authorship:** Fred asked why building `tam` could not find `libtam` and chose pkg-config discovery with a clear error; Claude Code wrote the Makefile and README changes.
- **Checked against:** `Makefile`, `bin/tam-upgrade` (its source builds pass `PREFIX` and `LIBTAM_PREFIX`), `source-build.json`, the `libtam` and `tamlinux-tools` Makefiles, the `tamlinux-packages` recipes (`LIBTAM_PREFIX` passed explicitly)

## Guiding prompts

> I've made some edits to tam. But I'm running into some issues finding where libtam is installed when I try to make the files. Why can't we install things to /usr/local so it's easy to develop and make files?

> Explain to me why we'd do an optional, untracked local.mk instead of setting things up so it's easy on every system to develop tam. What are my options to make the Makefile smarter so that I can just say "make all" from the tam repo clone as long as I have already installed libtam somewhere. Where would libtam go? Is $HOME/.local/dev the best place for libtam? Is it there already?

> Yes, go ahead with pkg-config and the clear error

## Key decisions

- `LIBTAM_PREFIX` no longer defaults to `PREFIX`: where `tam` is installed and
  where `libtam` is found are separate questions. pkg-config answers the
  second; an explicit `LIBTAM_PREFIX` still overrides it, so package recipes
  and `tam-upgrade` source builds work unchanged.
- Rejected: an untracked per-checkout `local.mk` (fixes one machine only) and
  probing a fixed list of prefixes (can pick up a stale copy silently).
- The check runs only for the command build, so `make corpus` and
  `make clean` do not need `libtam`.
- No command version bump: the binary is unchanged.

## Verification

- With no `libtam` visible, `make command` stops with the instructions; a
  nonexistent `LIBTAM_PREFIX` stops the same way.
- With `PKG_CONFIG_PATH` naming a scratch `libtam` 0.3.0 install, `make all`
  links it and sets the matching RUNPATH; `LIBTAM_PREFIX` builds as before.
- `tests/run.sh`, `tests/render.sh`, `tests/guide.sh` and `test_upgrade.py`
  (15 tests) pass on the new build. `make test` itself stops at an unrelated,
  uncommitted command version bump in the working tree.
