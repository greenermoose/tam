# tam checklist item identifiers

Checklist items in this repository use permanent, repository-wide `TCK`
identifiers ("tam command and corpus"). A plan's implementation slices are
checklist items. The identifier belongs to the outcome, not to a plan, version
or wording, so it can be cited alone: `TCK2`, not "slice 2 of the knowledge
plan". This file provides the consolidated checklist of all `TCK` items across
the repository, linking each item to the plan that defines its detailed
procedure and design.

## Rules

- Mint top-level IDs in sequence (`TCK1`, `TCK2`, ...) across all plans; never
  restart per plan, reuse or renumber an ID. **Next free: TCK36.** Update this
  counter and add the item to this file in the same change that adds a new
  top-level item.
- Mint IDs only while holding a `tam-work` claim on this file and the plan
  being changed. Two sessions minting at once then conflict on the counter
  line in Git instead of sharing a number.
- Keep each item to one verifiable outcome, with a short name and stated
  completion evidence. Put detailed procedure in the plan body.
- When an outcome is clarified without changing what counts as done, keep its
  ID. If its done condition changes after evidence exists, record
  `revised YYYY-MM-DD: <reason>` on the item; earlier evidence no longer
  counts.
- For a split, retain the parent as a rollup and give children dotted IDs
  (`TCK4.1`, `TCK4.2`). Children cover the parent's full scope without
  overlap, and the parent completes only when every child does. One further
  level is allowed; deeper breakdown belongs in the plan body.
- For a merge, the older ID survives; mark the other `merged into TCK<n>`.
  When a plan is superseded, its open items keep their IDs in the replacement
  plan. For a drop, retain and strike through the item, then mark it
  `dropped: <reason>`.
- Do not create other letter-number series (`K1`, `S1`, `M1`) in this
  repository's plans; such labels collide with identifiers used elsewhere in
  Tamlinux.

On first mention in a message, commit or record, give the name too:
`TCK2 (shared reader navigation)`; use `TCK2` after that. Put the ID in the
subject of a commit that completes an item, so `git log --grep 'TCK2\b'` gives
its history. Find an item with `rg -n 'TCK2\b' docs/plans ROADMAP.md`.
Dictated variants (`TCK 2`, `T.C.K. 2`, `tee see kay two`) mean `TCK2`.

The knowledge-system plan's items were labelled `K1`-`K5` in commit `cbb6d04`
and renamed to `TCK1`-`TCK5` on 2026-10-18; older records may use the old
labels.

## Consolidated checklist

| ID | Item / Outcome | Plan | Status | Evidence / Commits |
| --- | --- | --- | --- | --- |
| `TCK1` | Index and hierarchy contract | [knowledge-system-0.31.0.md](docs/plans/knowledge-system-0.31.0.md) | Completed (2026-10-18) | `e0215fd` |
| `TCK2` | Shared reader navigation | [knowledge-system-0.31.0.md](docs/plans/knowledge-system-0.31.0.md) | Completed (2026-10-18) | `0b3f008` (command 0.30.0) |
| `TCK3` | Manual and search discovery | [knowledge-system-0.31.0.md](docs/plans/knowledge-system-0.31.0.md) | Completed (2026-10-18) | `52e71f7` |
| `TCK4` | Corpus migration and learning | [knowledge-system-0.31.0.md](docs/plans/knowledge-system-0.31.0.md) | Completed (2026-10-18) | `597cb71` (corpus 0.31.0) |
| `TCK5` | Knowledge system integration and handoff | [knowledge-system-0.31.0.md](docs/plans/knowledge-system-0.31.0.md) | Completed (2026-10-18) | `f4fa539` |
| `TCK6` | Version discovery and comparison | [upgrade.md](docs/plans/upgrade.md) | Completed (2026-10-18) | `1d741cb` |
| `TCK7` | Source build and transactional activation | [upgrade.md](docs/plans/upgrade.md) | Completed (2026-10-18) | `33e8e1f` |
| `TCK8` | Upgrade CLI, documentation and delivery handoff | [upgrade.md](docs/plans/upgrade.md) | Completed (2026-10-18) | `6c2bb20` (command 0.31.0 / corpus 0.32.0) |
| `TCK9` | Reliable corpus builds | [corpus-build.md](docs/plans/corpus-build.md) | Completed (2026-10-19) | `fb8c1ac` (corpus 0.32.2) |
| `TCK10` | Home-row pager keys and attention column | [pager-home-row-keys.md](docs/plans/pager-home-row-keys.md) | Completed (2026-10-20) | `c32ad9f` (command 0.32.0 / corpus 0.33.5) |
| `TCK11` | `make dev` developer loop | [dev-loop.md](docs/plans/dev-loop.md) | Completed (2026-10-20) | `ce4b617` |
| `TCK12` | Drift checks and repository map | [dev-loop.md](docs/plans/dev-loop.md) | Completed (2026-10-20) | `d608889` |
| `TCK13` | Pager tests on fixtures | [dev-loop.md](docs/plans/dev-loop.md) | Completed (2026-10-20) | `ee845af` |
| `TCK14` | `make dev` names the failing check | [dev-loop.md](docs/plans/dev-loop.md) | Completed (2026-10-20) | `e8da257` |
| `TCK15` | Link quick keys `]` and `[` | [pager-quick-keys.md](docs/plans/pager-quick-keys.md) | Completed (2026-10-20) | `d5c1dfa` (command 0.34.0) |
| `TCK16` | Unified status row (`L<cursor> <first>-<last>/<total>`) | [pager-quick-keys.md](docs/plans/pager-quick-keys.md) | Completed (2026-10-20) | `f9b5c32` |
| `TCK17` | `=` address display; `page:` CLI addresses | [pager-quick-keys.md](docs/plans/pager-quick-keys.md) | Completed (2026-10-20) | `94b0d06` |
| `TCK18` | `\` Guide search; prompts; `tam:help` action links | [pager-quick-keys.md](docs/plans/pager-quick-keys.md) | Completed (2026-10-20) | `9142f15` |
| `TCK19` | Guide index, welcome page, README, and corpus bump | [pager-quick-keys.md](docs/plans/pager-quick-keys.md) | Completed (2026-10-20) | `66aa7e7` (corpus 0.35.1) |
| `TCK20` | Pedagogy contract, templates and terms | [learning-levels.md](docs/plans/learning-levels.md) | Completed (2026-10-21) | `3c59cf5` |
| `TCK21` | Sticky level lens and status indicator | [learning-levels.md](docs/plans/learning-levels.md) | Completed (2026-10-21) | `ae21980` (command 0.37.0) |
| `TCK22` | Folding, self-quizzes and block-level levels | [learning-levels.md](docs/plans/learning-levels.md) | Completed (2026-10-21) | `8d6ec7c` (command 0.38.0 / corpus 0.37.1) |
| `TCK23` | Pilot pages, goal-path hub, two goal paths | [learning-levels.md](docs/plans/learning-levels.md) | Source done (2026-10-21) | `8859559` (corpus 0.38.0); developer review pending |
| `TCK24` | Corpus scaffolding (skills, placement, glossary) | [learning-levels.md](docs/plans/learning-levels.md) | Source done (2026-10-21) | `91b46d0` (corpus 0.39.0); learner review pending |
| `TCK25` | Track migration and tagged-section build enforcement | [learning-levels.md](docs/plans/learning-levels.md) | Source done (2026-10-27) | Corpora 0.40.0–0.45.0; review/acceptance pending |
| `TCK26` | Browser handoff, confirmation and OSC 8 source | [external-links.md](docs/plans/external-links.md) | Source integrated (2026-10-27) | `abf0262` (command 0.44.0); review pending |
| `TCK27` | External links documentation and integration checks | [external-links.md](docs/plans/external-links.md) | Source integrated (2026-10-27) | `abf0262`; review pending |
| `TCK28` | Avahi and Zeroconf page | [network-setup-curriculum.md](docs/plans/network-setup-curriculum.md) | Completed (2026-11-21) | Corpus 0.105.0 |
| `TCK29` | Network setup pages and path | [network-setup-curriculum.md](docs/plans/network-setup-curriculum.md) | Completed (2026-11-21) | Corpus 0.105.0 |
| `TCK30` | References and verification | [network-setup-curriculum.md](docs/plans/network-setup-curriculum.md) | Completed (2026-11-21) | Corpus 0.105.0; `make corpus-and-test` and `make check-repo` pass |
| `TCK31` | Web development curriculum | [javascript-web-development-curriculum.md](docs/plans/javascript-web-development-curriculum.md) | Completed (2026-11-24) | Corpus 0.123.0; ten teaching pages, hub, goal path, glossary, sources and skills |
| `TCK31.1` | Web development pages | [javascript-web-development-curriculum.md](docs/plans/javascript-web-development-curriculum.md) | Completed (2026-11-24) | Hub and ten level-tagged pages with steps 1–10 |
| `TCK31.2` | Web path and supporting corpus | [javascript-web-development-curriculum.md](docs/plans/javascript-web-development-curriculum.md) | Completed (2026-11-24) | Goal path, thirty glossary pages, five sources, hub links and `web.*` skills |
| `TCK31.3` | Web bookkeeping and verification | [javascript-web-development-curriculum.md](docs/plans/javascript-web-development-curriculum.md) | Completed (2026-11-24) | Corpus 0.123.0 records; corpus, repository and reader checks |
| `TCK33` | Databases and data storage curriculum | [database-systems-curriculum.md](docs/plans/database-systems-curriculum.md) | Completed (2026-11-25) | Corpus 0.130.0; hub, ten pages, path, glossary, sources and skills |
| `TCK33.1` | Databases hub and teaching pages | [database-systems-curriculum.md](docs/plans/database-systems-curriculum.md) | Completed (2026-11-25) | Hub and ten level-tagged pages with path steps 1–10 |
| `TCK33.2` | Databases path and supporting corpus | [database-systems-curriculum.md](docs/plans/database-systems-curriculum.md) | Completed (2026-11-25) | Five-stage goal path, thirty glossary pages, twelve sources, `data.*` skills |
| `TCK33.3` | Databases bookkeeping and verification | [database-systems-curriculum.md](docs/plans/database-systems-curriculum.md) | Completed (2026-11-25) | Corpus 0.129.0 records; corpus, repository and reader checks |
| `TCK33.4` | Database glossary whatis discoverability | [database-systems-curriculum.md](docs/plans/database-systems-curriculum.md) | Completed (2026-11-25) | Corpus 0.130.0; SQLite, PostgreSQL and MySQL glossary entries and pager whatis checks |
| `TCK34` | Email systems curriculum | [email-systems-curriculum.md](docs/plans/email-systems-curriculum.md) | In progress | Networking > Email series at corpus 0.132.0 |
| `TCK34.1` | Email hub and teaching pages | [email-systems-curriculum.md](docs/plans/email-systems-curriculum.md) | Planned | Hub and ten level-tagged pages with path steps 1–10 |
| `TCK34.2` | Email path and supporting corpus | [email-systems-curriculum.md](docs/plans/email-systems-curriculum.md) | Planned | Five-stage goal path, acronym and anti-spam glossary, sources, skills and cross-links |
| `TCK34.3` | Email bookkeeping and verification | [email-systems-curriculum.md](docs/plans/email-systems-curriculum.md) | Planned | Corpus 0.132.0 records; corpus, repository and reader checks |
| `TCK34.4` | Email glossary whatis discoverability | [email-systems-curriculum.md](docs/plans/email-systems-curriculum.md) | Planned | Protocol, MX record and anti-spam names resolve in the live corpus |
| `TCK35` | Texting and mobile messaging curriculum | [texting-curriculum.md](docs/plans/texting-curriculum.md) | Completed (2026-11-25) | Systems > Mobile communications series at corpus 0.133.0 |
| `TCK35.1` | Mobile communications and texting teaching pages | [texting-curriculum.md](docs/plans/texting-curriculum.md) | Completed (2026-11-25) | Hub, subhub, twelve level-tagged pages and path steps 1–12 |
| `TCK35.2` | Texting path and supporting corpus | [texting-curriculum.md](docs/plans/texting-curriculum.md) | Completed (2026-11-25) | Six-stage goal path, acronym and full practical glossary, sources, skills and cross-links |
| `TCK35.3` | Texting bookkeeping and verification | [texting-curriculum.md](docs/plans/texting-curriculum.md) | Completed (2026-11-25) | Corpus 0.133.0 records; corpus, repository and reader checks |
| `TCK35.4` | Texting glossary whatis discoverability | [texting-curriculum.md](docs/plans/texting-curriculum.md) | Completed (2026-11-25) | SMS, MMS, RCS and related names resolve in the live corpus |

### Knowledge system (corpus 0.31.0)

Owning plan: [knowledge-system-0.31.0.md](docs/plans/knowledge-system-0.31.0.md)

- [x] **TCK1 (Index and hierarchy contract):** Five tracks, optional logical parent with old-corpus fallback, root handling and hierarchy validation in `tools/build-index.py` and `src/index.[ch]`. Completed 2026-10-18 (`e0215fd`).
- [x] **TCK2 (Shared reader navigation):** Shared fixed location rows/actions (Guide, Back, Help, Quit) across guide, topics, man and Info in `src/pager.[ch]`, `tam.c`, `man.c`, `info.c`; command 0.30.0. Completed 2026-10-18 (`0b3f008`).
- [x] **TCK3 (Manual and search discovery):** Reader integration for `tam:man`, section/keyword actions and `tam:search` with bounded/safe helper execution; command 0.30.0. Completed 2026-10-18 (`52e71f7`).
- [x] **TCK4 (Corpus migration and learning):** Mapped baseline paths, added 5 learning/navigation pages, canonical homes, explicit parents and breadcrumbs across all 55 pages, Beginner/Intermediate/Expert route outcomes; corpus 0.31.0. Completed 2026-10-18 (`597cb71`).
- [x] **TCK5 (Knowledge system integration and handoff):** Reconciled documentation, version assertions, full automated test suites pass, verified command 0.30.0 / corpus 0.31.0 candidate ready for package consumption. Completed 2026-10-18 (`f4fa539`).

### Version checks, source upgrade and stage switching

Owning plan: [upgrade.md](docs/plans/upgrade.md)

- [x] **TCK6 (Version discovery and comparison):** Added `src/upgrade.c/.h`; pure semantic version comparison, channel metadata, running revision metadata, bounded fixed-host transport, and CLI offline behavior; command 0.31.0. Completed 2026-10-18 (`1d741cb`).
- [x] **TCK7 (Source build and transactional activation):** Versioned source-build manifest, compatibility pins, isolated generations, receipts, locking, owned links, and offline rollback; first activation in `~/.local/dev`. Completed 2026-10-18 (`33e8e1f`).
- [x] **TCK8 (Upgrade CLI, documentation and delivery handoff):** Wired `tam upgrade` / `tam stage`, dispatch/help, completions, manual, README, and `knowledge/tam.html`; command 0.31.0 / corpus 0.32.0. Completed 2026-10-18 (`6c2bb20`).

### Reliable corpus builds

Owning plan: [corpus-build.md](docs/plans/corpus-build.md)

- [x] **TCK9 (Reliable corpus builds):** Single generator invocation per Make run via phony `corpus` target, direct index dependencies, rebuild before corpus tests, preserved mtimes on unchanged outputs, validation before writes; corpus 0.32.2. Completed 2026-10-19 (`fb8c1ac`).

### Home-row pager keys and attention column

Owning plan: [pager-home-row-keys.md](docs/plans/pager-home-row-keys.md)

- [x] **TCK10 (Home-row pager keys and attention column):** Mirrored home-row navigation (`a`/`;` column outward, `s`/`l` line up, `d`/`k` line down, `f`/`j` column inward, `x`/`.` top, `c`/`,` bottom, `g` follow, `h` help, retired `G`); horizontal attention column band; command 0.32.0 / corpus 0.33.5. Completed 2026-10-20 (`c32ad9f`).

### Developer loop: `make dev` and drift checks

Owning plan: [dev-loop.md](docs/plans/dev-loop.md)

- [x] **TCK11 (`make dev` developer loop):** `tools/dev.py` interactive developer workflow: preflight, path claims, change grouping, version prompts, pre-commit test run, commit, index follow-up, development install, push check. Completed 2026-10-20 (`ce4b617`).
- [x] **TCK12 (Drift checks and repository map):** `tools/check-repo.py` drift validation for command/corpus versions, build inputs, plan index, relative links, lane paths, and fixture rules; `docs/repo-map.md`. Completed 2026-10-20 (`d608889`).
- [x] **TCK13 (Pager tests on fixtures):** Decoupled interactive pager tests from live corpus by introducing `tests/fixtures/guide/`; interactive tests in `tests/run.sh` read fixtures unless marked `# live-corpus:`. Completed 2026-10-20 (`ee845af`).
- [x] **TCK14 (`make dev` names the failing check):** Diagnostic error reporting in `tools/dev.py` (`explain_test_failure`), distinguishing page linter/index errors from command/tooling test failures. Completed 2026-10-20 (`e8da257`).

### Pager quick keys, address display and status row

Owning plan: [pager-quick-keys.md](docs/plans/pager-quick-keys.md)

- [x] **TCK15 (Link quick keys `]` and `[`):** Added `]` (next link) and `[` (previous link) as first-class aliases for Tab/Shift-Tab; made `N` search-only; bumped command to 0.34.0. Completed 2026-10-20 (`d5c1dfa`).
- [x] **TCK16 (Unified status row):** Formatted pager status row as `L<cursor> <first>-<last>/<total>` replacing the separate `(All)`/`(Top)`/`(Bot)` indicators; updated screen tests. Completed 2026-10-20 (`f9b5c32`).
- [x] **TCK17 (`=` address display and `page:` CLI addresses):** Key `=` toggles `Path = <address>` in status row; `tam show` supports `page:<path>`, `tam:explain`, and `tam:keywords`. Completed 2026-10-20 (`94b0d06`).
- [x] **TCK18 (`\` Guide search and prompt action links):** Key `\` opens guide search; labelled prompts (`Search in page /`, `Search in guide \`); `tam:help` and `tam:prompt:page-search` action links; updated help overlay. Completed 2026-10-20 (`9142f15`).
- [x] **TCK19 (Guide index, welcome page, and corpus version):** Quick keys table in guide index, updated welcome page and README; corpus 0.35.1. Completed 2026-10-20 (`66aa7e7`).

### Learning levels, goal paths and self-quizzes

Owning plan: [learning-levels.md](docs/plans/learning-levels.md)

- [x] **TCK20 (Pedagogy contract and templates):** Teaching contract in `docs/corpus-pedagogy.md`, templates (`ladder.html`, `layered.html`, `single.html`, `path.html`), authoring terms, and minting rules. Completed 2026-10-21 (`3c59cf5`).
- [x] **TCK21 (Sticky level lens):** Keys `0`–`3` set reader reading level (All, Beginner, Intermediate, Expert) preserved across links, Back, and restarts in `$XDG_STATE_HOME/tamlinux/tam/level`; level indicator and banner; command 0.37.0. Completed 2026-10-21 (`ae21980`).
- [x] **TCK22 (Folding, self-quizzes, and block-level levels):** Collapsible `<details>` answers folded with Enter, `r`, `*`; block-level `data-level` filtering; risk labels (`pre[data-risk]`) and named asides; hand-typed level prefix removal; command 0.38.0 / corpus 0.37.1. Completed 2026-10-21 (`8d6ec7c`).
- [ ] **TCK23 (Pilot pages, goal-path hub, two goal paths):** Laddered pilot pages (`shell/pipelines.html`, `filesystem/permissions.html`, `maintain/updates.html`), goal-path hub, and First steps / Keep it healthy paths; corpus 0.38.0. Source completed 2026-10-21 (`8859559`); developer review of pilot levels pending.
- [ ] **TCK24 (Corpus scaffolding):** 36 skill cells in `learn/skills/skills.json`, self-placement (`learn/placement.html`), laddered glossary definitions, annotated bibliography, metadata validation (`kind`, `skills`), and coverage report; corpus 0.39.0. Source completed 2026-10-21 (`91b46d0`); learner review pending.
- [ ] **TCK25 (Track migration and tagged-section enforcement):** Migration of Using (0.40.0), rollback/emergency recovery (0.41.0), boot/Snapper/Rescue (0.42.0), logs/power (0.42.3), Maintaining (0.43.2), Improving (0.43.3–0.43.8), foundational glossary (0.44.0), and tagged Learning revisions/goal paths (0.45.0) with build enforcement. Source completed 2026-10-27; pilot, learner, and packaged acceptance pending.

### External links in reader's browser

Owning plan: [external-links.md](docs/plans/external-links.md)

- [ ] **TCK26 (Browser handoff, confirmation, and OSC 8 links):** Offline-safe handoff of HTTP/HTTPS URLs to external browser with two-step keyboard confirmation and optional OSC 8 terminal hyperlink emission; command 0.44.0. Source integrated from `feature/external-links` at `abf0262` (2026-10-27); environment acceptance matrix pending.
- [ ] **TCK27 (External links documentation and integration checks):** Documentation, man page updates, manual/automated tests, and shared-checkout integration. Source completed at `abf0262` (2026-10-27); full desktop and text console verification pending.

### Network setup and discovery curriculum

Owning plan: [network-setup-curriculum.md](docs/plans/network-setup-curriculum.md)

- [x] **TCK28 (Avahi and Zeroconf page):** Three-level
  `networking/services/avahi-and-zeroconf.html` and folder hub covering local
  service discovery, captured `avahi-browse`/`avahi-resolve` practice and
  RFC 6762/6763 detail. Completed 2026-11-21 (corpus 0.105.0).
- [x] **TCK29 (Network setup pages and path):** `networking/setup/` hub and
  five three-level pages on foundations, addressing, connections, routing and
  name resolution, plus `paths/network-setup.html` with five checkpoints and a
  capstone. Completed 2026-11-21 (corpus 0.105.0).
- [x] **TCK30 (References and verification):** Seven glossary pages, RFC 6762
  and RFC 6763 bibliography entries, regenerated indexes, corpus/test/repo
  checks, `tam show`/`tam explain` spot checks, corpus 0.105.0 version and
  roadmap updates. Completed 2026-11-21.

### JavaScript and web development curriculum

Owning plan: [javascript-web-development-curriculum.md](docs/plans/javascript-web-development-curriculum.md)

- [x] **TCK31 (Web development curriculum):** Rollup for the zero-to-expert
  web series at corpus 0.123.0. Completed 2026-11-24 when every child below
  was complete.
- [x] **TCK31.1 (Web development pages):** Programming > Web development hub
  and ten three-level teaching pages from JavaScript history and HTML/CSS to
  browser engines, async APIs, Node, full-stack architecture, security,
  tooling, testing and deployment/Electron. Steps 1–10 declared in order.
- [x] **TCK31.2 (Web path and supporting corpus):** Five-stage goal path with
  checkpoints and a capstone, thirty glossary pages, five bibliography entries,
  programming hub link and `web.*` skill cells. Generated hubs rebuilt.
- [x] **TCK31.3 (Web bookkeeping and verification):** Corpus 0.123.0 version,
  roadmap, development contract, plan index and work record; corpus, repository
  and reader checks pass. Completed 2026-11-24.

### Welcome page and intro-first layout

Owning plan: [welcome-page-layout.md](docs/plans/welcome-page-layout.md)

- [x] **TCK32 (Intro-first corpus layout):** Rollup for the welcome rewrite and
  corpus-wide removal of duplicate body headings at corpus 0.124.0. Completed
  2026-11-24 when every child below was complete.
- [x] **TCK32.1 (Welcome page):** The requested breadcrumb, `Hello!`,
  performance sentence and `get started | get help` opening, with curated links
  to the subject hubs, paths, skills map, glossary, bibliography and help. No
  duplicate heading and no level sections.
- [x] **TCK32.2 (Corpus headings):** Removed the redundant `<h1>` from all
  1,442 non-root pages; the root index keeps its heading. The glossary hub
  generator now omits the heading so regenerated hubs follow the pattern.
- [x] **TCK32.3 (Standards and checks):** Templates, pedagogy and authoring
  rules, `build-index.py` docstring and `tests/guide.sh` updated; TCK32, corpus
  version, roadmap, development contract, plan index and work record written.
- [x] **TCK32.4 (Verification):** Corpus, repository and reader checks pass.
  Completed 2026-11-24.

### Email systems curriculum

Owning plan: [email-systems-curriculum.md](docs/plans/email-systems-curriculum.md)

- [x] **TCK34 (Email systems curriculum):** Rollup for the zero-to-expert email
  series at corpus 0.132.0. Every child below is complete and verified.
  Completed 2026-11-25.
- [x] **TCK34.1 (Email hub and teaching pages):** Networking > Email hub and
  ten three-level teaching pages from the client/server distinction to mail
  server roles, message format, SMTP, retrieval, DNS, identity, transport
  security, spam, clients and deliverability. Steps 1–10 declared in order.
- [x] **TCK34.2 (Email path and supporting corpus):** Five-stage goal path
  with checkpoints and a capstone, acronym and anti-spam glossary pages,
  bibliography entries, `mail.*` skill cells, hub links and contextual links
  from existing pages.
- [x] **TCK34.3 (Email bookkeeping and verification):** Corpus 0.132.0 version,
  roadmap, development contract, plan index and work record; corpus, repository
  and reader checks pass.
- [x] **TCK34.4 (Email glossary whatis discoverability):** Requested protocol,
  MX record and anti-spam names resolve in the live corpus while MX Linux stays
  reachable.

### Texting and mobile messaging curriculum

Owning plan: [texting-curriculum.md](docs/plans/texting-curriculum.md)

- [x] **TCK35 (Texting and mobile messaging curriculum):** Rollup for the
  zero-to-expert texting series at corpus 0.133.0. Every child below is complete
  and verified. Completed 2026-11-25.
- [x] **TCK35.1 (Mobile communications and texting teaching pages):** Systems >
  Mobile communications hub, texting subhub and twelve three-level teaching
  pages from what a text is to cellular radio, SIM identity, addressing, SMS,
  MMS, RCS/IMS, internet messaging apps, encryption, delivery, abuse and
  research frontiers. Steps 1–12 declared in order.
- [x] **TCK35.2 (Texting path and supporting corpus):** Six-stage goal path
  with checkpoints and a capstone, acronym and full practical glossary pages,
  bibliography entries, `texting.*` skill cells and contextual links from
  existing pages.
- [x] **TCK35.3 (Texting bookkeeping and verification):** Corpus 0.133.0
  version, roadmap, development contract, plan index and work record; corpus,
  repository and reader checks pass.
- [x] **TCK35.4 (Texting glossary whatis discoverability):** SMS, MMS, RCS and
  related names resolve in the live corpus to their own glossary pages.
