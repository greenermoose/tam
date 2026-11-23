# B4 — Resize selects an earlier link with the same target

- **Status:** Fix available
- **Severity:** medium
- **Affected versions:** Confirmed in 0.67.8
- **Fixed in:** 0.68.2
- **Reported:** 2026-12-03, Codex
- **Found by:** [R1](../reviews/R1-command-and-corpus-code.md)

## What happens

A terminal resize can move link focus to an earlier link with the
same destination.

## What should happen

Focus stays on the same link occurrence when it survives reflow.

## Steps to reproduce

Open a fixture page with two adjacent links named First label and
Second label and with the same target. In a 64-column PTY, press Tab
twice to focus Second label. Resize to 56 columns and deliver SIGWINCH.
The focus moves to First label.

## Cause

The reflow loop accepts the first link with a matching ID or destination.
An earlier destination match wins before the original ID is visited.
[Reflow](../src/pager_nav.c#L109)

## Fix and verification

`pager_reflow` now restores the selected link by ID first. If the ID
disappeared, it falls back to the same-href link closest to the old
link's logical position. Added `check_b4_same_href_resize` in
`tests/pager_screen.py` with fixture `tests/fixtures/guide/b4-same-href.html`.
Verified with `python3 tests/pager_screen.py build/tam tests/fixtures` (pass).
