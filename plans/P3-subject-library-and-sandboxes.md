# P3: Subject library, interactive sandboxes, and libtam/C17 alignment

- **Status:** done (command 0.57.0–0.72.0 / corpus 0.90.0–0.139.1, 2026-11-26)
- **Versions:** command 0.57.0–0.72.0, corpus 0.90.0–0.139.1

## Goal

Reorganize discovery into the subject-based reference library, integrate
offline Arch Wiki and interactive `<tam-sandbox>` simulations, expand
curricula (networking, databases, email, texting, hardware lifecycle),
derive CLI-aware search from corpus metadata, and align with `libtam`
0.10–0.14 and C17.

## Versions

- **Command 0.57.0–0.62.1 / Corpus 0.90.0–0.99.0 (done):** Subject-based
  library (`docs/plans/subject-library.md`), offline Arch Wiki reader and
  pre-processor, glossary level differentiation, and botanical analogy
  alignment.
- **Command 0.63.0–0.69.7 / Corpus 0.100.0–0.128.3 (done):** Interactive
  `<tam-sandbox>` exercises across 229 pages, `tam-bud` integration,
  Info discovery, `tam:hubs`/`tam:topics`/`tam:paths` redesign, and
  networking/windowing/kernel curricula.
- **Command 0.69.8–0.72.0 / Corpus 0.129.0–0.139.1 (done 2026-11-26):**
  `libtam` 0.10–0.14 adoption, canonical ID lookup, derived CLI-aware
  search, learning-path presentation, databases/email/texting/lifecycle
  curricula (`docs/plans/database-systems-curriculum.md`,
  `docs/plans/email-systems-curriculum.md`,
  `docs/plans/texting-curriculum.md`,
  `docs/plans/electronic-waste-and-recycling-curriculum.md`), and C17
  standard.

## Acceptance

`make command`, `make corpus`, `python3 tools/check-repo.py`, and PTY
tests pass.

## Dependencies

`libtam` 0.14.0+ (static) and POSIX libc; Python 3 for corpus build.

## Risks

None (completed on `dev`; stage promotion awaits Fred's word).

## Open points

- Stage promotion of `dev` (0.72.0 / 0.139.1) awaits Fred's word.
