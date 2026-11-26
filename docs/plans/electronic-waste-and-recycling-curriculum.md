# Electronic waste and recycling curriculum

**Plan status:** Completed
**Reviewed:** 2026-11-26
**Scope:** Corpus-only content ending at corpus 0.135.0: a Hardware > Lifecycle
and environment hub, eight three-level teaching pages, a five-stage
`hardware-lifecycle` goal path, a Computer materials page added as stage 4 of
From atoms to bits, 29 new glossary pages plus a revised ISO/IEC 21031 entry,
26 bibliography sources, `lifecycle.*` skill cells and contextual links from
existing Hardware and systems pages. Teaching exercises are decisions made on
paper; no device is opened, no battery is removed and no system setting is
changed. No command, install or stage change.
**Authority:** The developer asked: "Add to the from atoms to bits a discussion
of the materials used to build computers. Explain which metals, which types of
plastic, which chemicals are used for different display types, etc. Add a series
on electronic waste and recycling, explaining in detail how computers are
manufactured, how they can be repaired, and how they can be recycled at the end
of their life. Add a glossary page for RoHS and other standards designed to make
computers more environmentally friendly. Also discuss EPA's Energy Star and
other environmental standards computer hardware and software can achieve."
This authorizes Develop work only; Test, Run, Release, install and push remain
his word.
**Next action:** None. All four slices are complete and verified at corpus
0.135.0; no stage promotion, install or push is authorized.

Follow the [plan maintenance procedure](../plan-maintenance.md).

## Problem and agreed outcome

A reader who has followed matter, circuits and digital logic should be able to
name the materials that do each job in a computer, then follow one continuous
path from raw materials through manufacture, use, repair, reuse and recovery.
The series must distinguish repair from reuse, reuse from recycling, and
depollution from material recovery. It must explain how a device is built, what
fails and can be replaced, how data is handled before another owner receives a
device, and how collection, sorting and certified recyclers recover material.

The environmental standards need to be sorted by the question each answers.
RoHS restricts substances in products; WEEE covers collection and recovery;
REACH controls chemicals; ENERGY STAR certifies named efficient product
categories; EPEAT rates products; EU Ecodesign and the ESPR set product
requirements; ISO 14001 describes an organisational management system; ISO
14040/14044 describe how to perform a life-cycle assessment; ISO/IEC 21031
measures and discloses software environmental impact; R2v3 and e-Stewards
certify recycling facilities. Conflict-minerals due diligence, battery rules,
the right to repair and ecolabels such as TCO Certified and Blue Angel complete
the practical set. A badge is never presented as proof of an unrelated claim.

## Baseline and dependencies

Command 0.69.7 and corpus 0.134.3 are on `dev` at the start of this plan. The
texting and email curricula established the hub, goal-path, glossary,
bibliography and skills pattern this plan follows. New pages use
`docs/templates/ladder.html` and `docs/templates/path.html`, the subject-library
parent rules, and the sourcing and public-boundary rules in
`docs/corpus-authoring.md`. The corpus build enforces path-page and skill
contracts. Versions are independent: the command stays at 0.69.7 and only the
corpus minor version advances.

The developer's subject choice is Hardware, not Systems. The practical full
standards set is in scope. The Materials page belongs both to From atoms to bits
and to the lifecycle path, so its path steps are numbered `from-atoms-to-bits:4`
and `hardware-lifecycle:2`; the remaining lifecycle pages use steps 3–9. This
numbering satisfies the corpus path contract, which requires contiguous steps.

## Implementation and handoff

| Slice | Identifier | Work |
| :--- | :--- | :--- |
| 1 | `TCK36.1` | Add `knowledge/hardware/lifecycle/index.html`, eight three-level teaching pages, the five-stage `hardware-lifecycle` goal path and `knowledge/hardware/foundations/computer-materials.html` as stage 4 of From atoms to bits and step 2 of the lifecycle path. |
| 2 | `TCK36.2` | Add 29 glossary pages for materials, lifecycle stages and standards, and revise the existing ISO/IEC 21031 entry. |
| 3 | `TCK36.3` | Add 26 bibliography sources, register `lifecycle.*` skills, and link the series from Hardware, Physical foundations, displays, energy, operating systems and relevant existing pages. |
| 4 | `TCK36.4` | Record corpus 0.135.0 in the roadmap and development contract, close the work records, and verify corpus, repository, reader and `whatis` checks. |

Teaching pages declare `novice`, `learner` and `expert` sections. Novice text
assumes no prior knowledge. Learner sections give a goal, a worked example, a
short practice task and failure guidance. Expert sections give precise
architecture, standards, failure modes and source references. Every exercise is
a paper decision about a described device; the series contains no interactive
sandbox and performs no physical operation.

## Acceptance and limits

- [x] The Hardware hub links the Lifecycle and environment hub; that hub links
  the life-of-a-computer page and all eight teaching pages, and the goal path.
- [x] Every teaching page has all three levels and a unique
  `hardware-lifecycle:N` step; the Materials page also has stage 4 of From atoms
  to bits. The path page has five stages, checkpoint questions, a capstone and
  next paths.
- [x] The series covers metals, polymers, FR-4, display chemistries and flame
  retardants; wafer and chip fabrication, board and display assembly; fault
  diagnosis, ESD control and module replacement; backup and verified data
  sanitisation; WEEE collection and depollution; and magnetic, optical and
  chemical recovery with facility certifications.
- [x] RoHS, WEEE, REACH, ENERGY STAR, EPEAT, EU Ecodesign and ESPR, ISO 14001,
  ISO 14040/14044, ISO/IEC 21031, the EU Batteries Regulation, R2v3,
  e-Stewards, conflict minerals, TCO Certified, Blue Angel and the right to
  repair each have a glossary entry with the correct scope and limits. The
  existing ISO/IEC 21031 entry was revised in place.
- [x] Over-broad claims are avoided: ENERGY STAR certifies named
  product categories rather than arbitrary software, ISO/IEC 21031 has no
  threshold or pass/fail label, RoHS does not ban all brominated flame
  retardants, Regulation (EU) 2019/2022 covers dishwashers rather than
  computers, and no unverified grams-per-computer figure is given.
- [x] `lifecycle.*` is registered at all three levels and mapped from the skills
  table and teaching pages.
- [x] Corpus 0.135.0 is recorded in `knowledge/version`, the roadmap dev row and
  delivered history, and the development decision record.
- [x] `make corpus`, `make corpus-and-test`, `make check-repo`, the coverage
  report, reader spot checks and live `whatis` checks pass. No stage is
  promoted, no tag or release is created, nothing is installed or pushed.

## Verification

Corpus 0.135.0 on `dev`. This plan does not change the command; the separate
command-lane bump to 0.69.8 is recorded in the roadmap. `make corpus` reported 1,790
pages and current indexes; `make corpus-and-test` passed the corpus checker,
component-coverage check and 96 corpus tooling tests. The coverage report shows
`lifecycle.novice`, `lifecycle.learner` and `lifecycle.expert` with no gaps and
anatomy warnings of zero. Reader spot checks with `TAM_KNOWLEDGE_DIR=knowledge
build/tam show --pipe` rendered the Lifecycle and environment hub, Computer
materials, the goal path, Standards and labels, and a RoHS bibliography source.
Live `whatis` checks resolved RoHS, WEEE, REACH, ENERGY STAR, EPEAT, R2v3,
e-Stewards, Blue Angel, ISO 14001, Surface-mount technology, Right to repair,
Electronic waste and Life cycle assessment to their own glossary entries. No
stage, tag, release, install or push was performed.

## Disposition

Closed 2026-11-26 at corpus 0.135.0. The command is outside this plan; its
separate 0.69.8 bump is recorded in the roadmap. All four
TCK36 children are complete and verified on `dev`. Test, Run, Release, install
and push remain the developer's separate instructions.
