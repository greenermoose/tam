# B2 — File ownership lookup claims nonexistent paths

- **Status:** Confirmed
- **Severity:** medium
- **Affected versions:** Confirmed in 0.67.8
- **Fixed in:** Open
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

Check the deployment manifest for specific entries and give versioned
trees an existence and canonical-path check. Exercise a missing child,
an existing unowned child, a known manifest member and a native package
lookup using a fixture.
