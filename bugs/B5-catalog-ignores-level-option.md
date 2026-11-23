# B5 — The catalog ignores the reading-level option

- **Status:** Fix available
- **Severity:** medium
- **Affected versions:** Confirmed in 0.67.8
- **Fixed in:** 0.68.3
- **Reported:** 2026-12-03, Codex
- **Found by:** [R1](../reviews/R1-command-and-corpus-code.md)

## What happens

`tam show --level expert tam:catalog` renders the bare catalog. Its
heading and links omit the requested level.

## What should happen

The option reaches the catalog and applies the same level as a
`level=expert` address parameter.

## Steps to reproduce

Set `TAM_KNOWLEDGE_DIR` to `tests/fixtures/guide`. Compare the output
of `tam show --pipe tam:catalog` and
`tam show --pipe --level expert tam:catalog`. They are identical.
`tam show --pipe 'tam:catalog?level=expert'` shows the expert level
and adds it to the links.

## Cause

The CLI uses `tam_addr_takes_lens` to decide whether to add the level
parameter. The catalog loader supports the parameter and its route is
absent from that eligibility function.
[Eligibility](../src/address.c#L41) · [Catalog](../src/views.c#L171)

## Fix and verification

Added `tam:catalog` to `tam_addr_takes_lens` in `src/address.c` so
`--level` is applied to catalog addresses that do not already name a
level. Added `test_catalog_level_option` in `tests/test_library_discovery.py`
covering flag/param equivalence and explicit level preservation.
Verified with `python3 tests/test_library_discovery.py` (7 tests, pass).
