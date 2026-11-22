# B6 — Failed back or forward load frees a live history entry

- **Status:** Fix available
- **Severity:** high
- **Affected versions:** Confirmed in 0.67.8
- **Fixed in:** Open (source fixed, not yet in a build Fred ran)
- **Reported:** 2026-11-22, Codex (R1 source finding), reproduced by Claude
- **Found by:** [R1](../reviews/R1-command-and-corpus-code.md)

## What happens

When the previous page cannot be loaded, back reports "cannot open"
and the next back press crashes the pager. Forward has the same code.

## What should happen

The entry stays intact. Each press reports the failure again.

## Steps to reproduce

Open a temporary guide page A with a link to page B. Follow the link,
delete A while the pager is open, then press Left twice. Under ASan the
second press is a heap-use-after-free in go_back. Without ASan it is
undefined behaviour, and the exit cleanup frees the strings again.

## Cause

go_back and go_forward pop an entry by struct copy. On a failed load
they increment the count to put the entry back, then free the strings
through the copy. The restored entry still points at the freed memory.
[Back and forward](../src/pager_nav.c#L938)

## Fix and verification

The failed-load branch keeps the entry on its stack and frees nothing.
The regression check is check_vanished_history in tests/pager_screen.py.
It passes under ASan on the fixed source and aborts on the previous
source. Only the back direction is exercised by the test.
