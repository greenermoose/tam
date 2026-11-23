# B2 — File ownership lookup claims nonexistent paths

- **Status:** Verified
- **Severity:** medium
- **Affected versions:** Confirmed in 0.67.8
- **Fixed in:** 0.68.4 (427e4e6), verification accepted by Fred (2026-11-23)
- **Reported:** 2026-12-03, Codex
- **Found by:** [R1](../reviews/R1-command-and-corpus-code.md)

## What happens

`tam where` succeeds and reports cultivar ownership for a missing path
under a known directory.

## What should happen

A positive result should have ownership evidence for the specific path.
The native package lookup should remain available for other paths.

## Steps to reproduce

Choose a child name under `/usr/share/doc/tamlinux/` that does not exist.
Pass its absolute path to `tam where`. The command exits zero and
prints that Tamlinux owns it.

## Cause

The lookup tests the original input against directory prefixes and
returns true before consulting any records, even when `realpath`
failed. It treats location as proof of ownership.
[Lookup](../src/tam.c#L117)

## Fix and verification

`path_owned_by_cultivar` now checks deployment manifest entries first,
then only accepts cultivar prefix matches when `realpath` succeeds
(existence and canonical-path evidence). Added B2 tests in `tests/run.sh`
for a missing child under a cultivar directory, an existing unowned
child with native package lookup fixture, and a known manifest member.
Fred verified (2026-11-23).
