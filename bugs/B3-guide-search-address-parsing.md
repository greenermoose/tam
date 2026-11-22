# B3 — Guide search address delimiters change the query

- **Status:** Confirmed
- **Severity:** medium
- **Affected versions:** Confirmed in 0.67.8
- **Fixed in:** Open
- **Reported:** 2026-12-03, Codex
- **Found by:** [R1](../reviews/R1-command-and-corpus-code.md)

## What happens

A literal URL in a Guide Search query sends the command to page lookup.
A fragment appended to a search address becomes part of the search term.

## What should happen

The leading scheme determines routing. The query and fragment each keep
their own boundaries.

## Steps to reproduce

Set `TAM_KNOWLEDGE_DIR` to `tests/fixtures/guide`. Run
`tam show --pipe 'tam:search?query=pipelines'` and repeat with
`#details` appended. The first command finds the fixture, while the
second searches for `pipelines#details`. Compare the literal and
percent-encoded URL values in the same query parameter.
`https://example.invalid` gives a page lookup error, while
`https%3A%2F%2Fexample.invalid` reaches Guide Search.

## Cause

The address classifier looks for `://` across the whole string before
examining the scheme. The Guide Search decoder splits parameters on
`&` alone and consumes the fragment.
[Classifier](../src/address.c#L21) · [Search decoder](../src/discovery.c#L83)

## Fix and verification

Share address splitting and parameter decoding. Add fixtures for
literal and percent-encoded URL values, fragments and encoded
delimiters. Check both command-line and pager entry points.
