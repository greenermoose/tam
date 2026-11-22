# B7 — Review, bug and work records carry a date in the future

- **Status:** Reported
- **Severity:** low
- **Affected versions:** Records added in `377c937`
- **Fixed in:** Open
- **Reported:** 2026-11-22, agy (Claude Opus 5.5), at Fred's request
- **Found by:** libtam and tam-view R1 reviews, which used tam's R1 as
  a guide

## What happens

R1, B1–B5 and the work record W40 give their date as 2026-12-03. They
were committed on 2026-11-22, and the revision R1 reviews (`4acc95a`) was
committed on 2026-11-21. The records therefore carry a date that had not
yet arrived when they were written.

## What should happen

Each record carries the date it was written.

## Steps to reproduce

Run `grep -rn '2026-12-0' reviews bugs work`. It finds R1's Date line,
the Reported line of B1–B5, and the Date line of W40. Then compare with
`git log -1 --date=iso 377c937`, which shows 2026-11-22 07:50, and
`git log -1 --date=iso 4acc95a`, which shows 2026-11-21.

## Cause

Needs investigation. All the dated records came from a single review
session, recorded in W40, and that session produced the same wrong date
everywhere. Find out where the session took its date from, so the same
date error is caught next time.

## Fix and verification

Confirm the correct date from the session's evidence (the commit says
2026-11-22). Correct the Date and Reported lines in a forward commit,
leaving the findings as written. Afterwards the grep above finds
nothing.
