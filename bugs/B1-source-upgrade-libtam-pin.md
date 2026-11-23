# B1 — Source upgrade pins an incompatible libtam

- **Status:** Fix available
- **Severity:** high
- **Affected versions:** Confirmed in 0.67.8
- **Fixed in:** Unreleased
- **Reported:** 2026-12-03, Codex
- **Found by:** [R1](../reviews/R1-command-and-corpus-code.md)

## What happens

The source updater builds the pinned libtam 0.4.0, then the command build
rejects that dependency. This blocks upgrades to the reviewed source.

## What should happen

The source manifest selects a library revision that can build the command.

## Steps to reproduce

Extract `include/tam.h` from the exact libtam revision in
`source-build.json` into an empty scratch prefix under `include/`. Run
`make libtam-check LIBTAM_PREFIX=<scratch-prefix>` in `tam`. Make exits 2
and says the prefix lacks libtam 0.8.0 or later. The same check accepts
the header from libtam `725dd1f`.

## Cause

Commit `9ff5a36` began using `tam_install_pending` and raised the minimum
to 0.8.0. The source pin remained at `a051ebf` (0.4.0), which has no
declaration for that function.
[Manifest](../source-build.json#L3) · [Make check](../Makefile#L39)
· [Updater](../bin/tam-upgrade#L669)

## Fix and verification

Pinned `source-build.json` to libtam 0.8.0 at
`725dd1f848e4717d18db3e6f228e2a75cb9d21bb` and added a
`libtam_pin_incompatible` check in `bin/tam-upgrade`
`source_build_manifest()` so schema 1 manifests below libtam 0.8.0 are
rejected before build.

Verified with the B1 reproduction: extracting `include/tam.h` from the
pinned libtam revision into a scratch prefix makes
`make libtam-check LIBTAM_PREFIX=<scratch-prefix>` pass. A full isolated
source-upgrade build remains to be verified.
