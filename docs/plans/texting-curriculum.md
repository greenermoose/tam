# Texting and mobile messaging curriculum

**Plan status:** Completed
**Reviewed:** 2026-11-25
**Scope:** Corpus-only content ending at corpus 0.133.0: a Systems > Mobile
communications hub, a texting series hub, twelve three-level teaching pages, a
six-stage `texting` goal path, glossary entries for SMS, MMS, RCS and the
practical full set of related mobile and internet messaging standards,
bibliography sources, `texting.*` skill cells and contextual links from existing
pages. Teaching examples use labelled sandboxes with sample or simulated output;
no phone service, messaging app, SIM or network setting is configured. No
command, install or stage change.
**Authority:** The developer asked: "Write a series of pages about texting.
Include glossary pages for SMS and MMS and RCS. Make sure that the glossary page
titles adhere to the format Acronym (What is stands for). The series should take
a reader from no prior knowledge to a PhD-level understanding of how cell phone
technology and all instant texting works." When asked about the two design
choices, he chose Systems > Mobile communications as the subject home and the
full practical messaging set as the scope. This authorizes Develop work only;
Test, Run, Release, install and push remain his word.
**Next action:** None. All four slices are complete and verified at corpus
0.133.0; no stage promotion, install or push is authorized.

Follow the [plan maintenance procedure](../plan-maintenance.md).

## Problem and agreed outcome

A reader who has never used a phone should be able to enter the guide and follow
one continuous path to research-level understanding. The path must explain what
a text is, how a phone reaches a cell, how the SIM identifies a subscriber, how
addresses and numbers route a message, how SMS and MMS store and forward it,
how RCS and IMS add rich chat, how internet messaging apps differ, what
encryption and key management actually protect, why delivery is not certainty,
how abuse is controlled, and which questions remain open in research.

The requested glossary entries use the house acronym form: **SMS (Short Message
Service)**, **MMS (Multimedia Messaging Service)** and **RCS (Rich Communication
Services)**. Every acronym title spells out its letters in the title and explains
them in the body. The full practical set also covers radio, core-network,
identity, addressing, transport, encryption and anti-abuse terms. Product names
are taught where useful, but no proprietary app is presented as the whole
system.

## Baseline and dependencies

Command 0.69.7 and corpus 0.132.0 are on `dev` at the start of this plan. The
email curriculum established the ladder, hub, goal-path, glossary, bibliography
and skills pattern this plan follows. New pages use `docs/templates/ladder.html`
and `docs/templates/path.html`, the subject-library parent rules, and the
sourcing and public-boundary rules in `docs/corpus-authoring.md`. The corpus
build enforces path-page and skill contracts. Versions are independent: the
command stays at 0.69.7 and only the corpus minor version advances.

## Implementation and handoff

| Slice | Identifier | Work |
| :--- | :--- | :--- |
| 1 | `TCK35.1` | Add `knowledge/systems/mobile-communications/index.html`, its texting hub, twelve three-level teaching pages and the six-stage `texting` goal path with steps 1–12. |
| 2 | `TCK35.2` | Add the requested acronym glossary pages and the practical full set of related mobile, addressing, encryption and anti-abuse entries. |
| 3 | `TCK35.3` | Add bibliography sources, register `texting.*` skills, and link the series from Systems, Networking, Security and relevant glossary pages. |
| 4 | `TCK35.4` | Record corpus 0.133.0 in the roadmap and development contract, close the work records, and verify corpus, repository, reader and `whatis` checks. |

Teaching pages declare `novice`, `learner` and `expert` sections. Novice text
assumes no prior knowledge and contains no exercises. Learner sections contain a
goal, a labelled teaching sandbox, a short practice task, failure guidance and a
folded quiz. Expert sections contain precise architecture, standards, failure
modes and open questions. Every sandbox is a teaching sample; nothing in the
series sends a real text, changes a SIM or installs a messaging application.

## Acceptance and limits

- [x] The Systems hub links the Mobile communications hub; that hub links the
  texting hub; the texting hub links all twelve pages and the goal path.
- [x] Every teaching page has all three levels, a unique `texting:N` step and a
  labelled sandbox. The `texting` path page has six stages, checkpoints, a
  folded capstone solution and next paths.
- [x] The requested glossary titles and breadcrumbs use the acronym expansion
  form; the bodies explain the letters. `tam whatis` resolves SMS, MMS, RCS and
  the other requested names to their own glossary pages.
- [x] The series covers radio access and core networks, SIM/eSIM and subscriber
  identity, numbering and routing, SMS, MMS, RCS/IMS, internet messaging apps,
  encryption and key management, delivery and reliability, spam and smishing,
  and research frontiers.
- [x] `mail.*` remains the email domain; `texting.*` is registered at all three
  levels and mapped from the skills table and teaching pages.
- [x] Corpus 0.133.0 is recorded in `knowledge/version`, the roadmap dev row and
  delivered history, and the development decision record.
- [x] `make corpus`, `make corpus-and-test`, `make check-repo`, the coverage
  report, reader spot checks and live `whatis` checks pass. No stage is promoted,
  no tag or release is created, nothing is installed or pushed.

## Verification

Corpus 0.133.0 on `dev`, command 0.69.7 unchanged. `make corpus` rebuilt 1,714
pages; `make corpus-and-test` passed the corpus checker, component-coverage
check, 96 corpus tooling tests and the repository consistency check. The
coverage report shows `texting.*` with twelve pages and one path per level and
no anatomy warnings. Reader spot checks with `TAM_KNOWLEDGE_DIR=knowledge
build/tam show --pipe` rendered the series hub, a teaching page, the goal path,
the RCS glossary entry and an RCS bibliography source. Live `whatis` checks
resolved SMS, MMS, RCS, IMS, IMSI, SIM, eSIM, LTE, NR, 2FA, OTP, smishing, SIM
swap, MLS, Signal Protocol and RCS Universal Profile to their own glossary
pages. No stage, tag, release, install or push was performed.

## Disposition

Closed 2026-11-25 at corpus 0.133.0 with command 0.69.7 unchanged. All four
TCK35 children are complete and verified on `dev`. Test, Run, Release, install
and push remain the developer's separate instructions.
