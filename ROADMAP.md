# tam roadmap

`tam` is Tamlinux's native C17 terminal entry point to its offline
knowledge system, combining an offline HTML/man/info/Arch-Wiki pager with
a three-level teaching corpus.

- **Current version:** command `0.72.0` / corpus `0.140.1` (branch `dev`)
- **Current Command Version:** `0.72.0`.
- **Current Corpus Version:** `0.140.1`.
- **Progress:** 3 of 4 phases done (P1–P3 done on `dev`; P4 deferred on
  Fred's word)
- **Versioning:** Command (`src/version.h`) and corpus
  (`knowledge/version`) versions move independently; detailed feature
  plans live in `docs/plans/` (indexed by `docs/plans/README`, contract
  in `docs/plans/development.md`). Full pre-1.3 release log is archived
  read-only in `work/history/roadmap-pre-idol-1.3.md`.

| Branch | Command / corpus | Meaning |
| --- | --- | --- |
| `dev` | 0.72.0 / 0.140.1 | Active development on `dev`: C17 compile standard, learning-path presentation, `libtam` 0.14.0, derived CLI-aware search, canonical IDs, expanded curricula including the typography series, and the expanded SFNT glossary title. |

## Phase summary

| Phase | Versions (Cmd / Corpus) | Goal | Plan | Status |
| --- | --- | --- | --- | --- |
| **P1** | 0.1–0.34 / 0.1–0.35 | Native C offline reader, five-part knowledge system, corpus build, pager keys, dev loop | [P1-knowledge-foundation.md](plans/P1-knowledge-foundation.md) | Done |
| **P2** | 0.35–0.56 / 0.36–0.89 | Three-level pedagogy, sticky lens, folds/quizzes, goal paths, glossary & security curricula | [P2-pedagogy-and-levels.md](plans/P2-pedagogy-and-levels.md) | Done |
| **P3** | 0.57–0.72 / 0.90–0.140 | Subject library, offline Arch Wiki, interactive sandboxes, derived search, `libtam`/C17, expanded curricula | [P3-subject-library-and-sandboxes.md](plans/P3-subject-library-and-sandboxes.md) | Done (2026-11-28) |
| **P4** | 0.73+ / 0.141+ | Pending console/Foot candidate review, selected `ideas/synthesis.md` plans, deferred items | [P4-future-horizons.md](plans/P4-future-horizons.md) | Deferred |

## Decided

- Native C17/`libtam` reader, offline by default, read-only browsing,
  semantic HTML and indexed metadata, clean pipe output, explicit opt-in
  boundary for local AI, and independent command/corpus versions.
- Subject-based reference library supersedes the earlier five-part
  organization while keeping Novice/Learner/Expert levels and
  learning-path contracts.
- Stage promotion (`test`, `main`, tags, releases) requires Fred's exact
  word.

## Phases and milestones

### Phase P1: Knowledge foundation and pager workflow (0.1–0.34 / 0.1–0.35) — [P1-knowledge-foundation.md](plans/P1-knowledge-foundation.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **0.1–0.24** | Initial unified native C reader, man/info/help/topic browsing, clean pipe output | Done |
| **0.30–0.34 / 0.31–0.35** | Decoupled command/corpus versions, five-part knowledge system, source-upgrade, corpus build, home-row/quick keys, dev loop | Done |

### Phase P2: Teaching levels, goal paths, and glossary (0.35–0.56 / 0.36–0.89) — [P2-pedagogy-and-levels.md](plans/P2-pedagogy-and-levels.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **0.37–0.44 / 0.37–0.45** | Teaching contract, sticky level lens, folding/quizzes, skills/glossary/bibliography, external links, incremental build | Done |
| **0.45–0.56 / 0.46–0.89** | Dynamic multi-path engine, path-contract validation, cryptography/security curricula, 404-term glossary pass, link stepping | Done |

### Phase P3: Subject library, sandboxes, and libtam/C17 (0.57–0.72 / 0.90–0.140) — [P3-subject-library-and-sandboxes.md](plans/P3-subject-library-and-sandboxes.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **0.57–0.62 / 0.90–0.99** | Subject-based library (`tam:hubs`/`tam:topics`), offline Arch Wiki pre-processor & reader, three-level glossary, botanical analogy | Done |
| **0.63–0.69 / 0.100–0.128** | Interactive `<tam-sandbox>` simulations across 229 pages, `tam-bud` & Info support, networking/windowing/kernel curricula | Done (2026-11-25) |
| **0.70–0.71 / 0.129–0.138** | `libtam` 0.10–0.14 adoption, canonical IDs, derived CLI-aware search, databases/email/texting/lifecycle curricula, goal-path contracts | Done (2026-11-26) |
| **0.72.0 command** | Learning-path presentation: friendly intro, in-page menu, fixed categories, and outcome/prerequisites/time details in `tam:paths` | Done (2026-11-26) |
| **0.139.1 corpus** | Retarget Tamlinux C compile standard from C11 to C17 across active code and current docs (following 0.139.0 path metadata) | Done (2026-11-26) |
| **0.140.0 corpus** | Desktop > Typography series from the beginning of writing to research practice: formats, kernel fonts, shaping, foundries, printing history, glossary, sources and three paths | Done (2026-11-28) |
| **0.140.1 corpus** | Expand the SFNT title and breadcrumb to "SFNT (Scalable Font)" and carry the expansion in the summary and keywords | Done (2026-11-28) |

### Phase P4: Future horizons (0.73+ / 0.141+) — [P4-future-horizons.md](plans/P4-future-horizons.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **TBD** | Real Linux-console and Foot review of portable pager display on an exact packaged candidate | Deferred |
| **TBD** | Follow-up documentation plans selected by Fred from `ideas/synthesis.md` | Deferred |
| **TBD** | Installer execution and stateful progression | Deferred |

## How a milestone is done

A milestone is done when `make command` / `make corpus` and
`python3 tools/check-repo.py` pass and Fred has reviewed the build. Bump
`src/version.h` and/or `knowledge/version`, update the relevant plan, and
update this roadmap in the same change.
