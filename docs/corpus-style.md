# How the tam knowledge corpus reads

Read this with [`corpus-authoring.md`](corpus-authoring.md) and
[`corpus-pedagogy.md`](corpus-pedagogy.md). Authoring covers what must be
true. Pedagogy covers level anatomy, paths and quizzes. This file covers
voice, page kinds, one-page difficulty levels, learning paths, and how much
effort goes into static examples.

Approved for plan 26 W0 on 2026-11-04 (D225, D226). Do not change it without
the developer's word.

## Goal of the guide

A complete novice should be able to become an expert by reading this guide.
Write so each topic can carry a reader from a gentle first look through
practice to precise reference, and so links lead onward in an order that
builds skill and relevant knowledge.

## One page, three difficulty levels (D226)

Do **not** make separate novice, learner, and expert files for the same
topic. Put all three difficulty texts in **one** page file. The reader
chooses what to display: **all**, **novice**, **learner**, or **expert**.
The pager shows or hides the matching `data-level` sections.

Visible levels are **1: novice**, **2: learner**, **3: expert** (D221).

| Level | How that text reads | Examples in that text |
| --- | --- | --- |
| Novice (1) | Very simple conceptual intro. No goals, no quizzes, no practice. | **No worked examples.** No command-and-output walkthroughs. Keep the text light. |
| Learner (2) | The only level for `Goal:`, path links, `Try it`, and self-check. | **Worked examples live here** — a short captured command sequence with output, then a safe `Try it` when useful. |
| Expert (3) | Reference and advanced usage for people who already know the topic. | Illustrative advanced snippets for reference, not student exercises. |

Shared page chrome (title, one-line summary, **Where next**, **See also**,
revision line) is not level-tagged and appears at every lens. Do not burden
Novice sections with worked examples, option tables, or "try this now"
pressure.

## Learning paths through the material (D226)

Guide readers from page to page with **Where next** and **See also** links
in an order that builds skills and relevant knowledge. Prefer a path a
novice can follow toward expertise over a flat pile of unrelated pages.

Learner (2) text may also carry embedded path controls (`Start`, `Back`,
`Next` at `?level=2`) where a goal path includes the page; see pedagogy.
Those controls complement Where next / See also; they do not replace them.

## Voice

- Plain language. Prefer everyday words; define or link every technical term
  on first use (`ecosystem/terminology.html` or the glossary).
- Second person and active voice ("You open…", "Run…").
- Short sentences. One idea per sentence when teaching novices.
- At most one light touch of humour per page. Never mock the reader.
- Write for a high-school education and no specialized background on the
  first sentence of every page (see pedagogy's page openings).

## Page kinds

Use one kind per page or per clear section. Do not mix kinds in a way that
forces a novice through a tutorial.

| Kind | Job |
| --- | --- |
| Task | Get one job done, step by step |
| Concept | Explain what something is and why it exists |
| Reference | Precise names, options, tables; link man pages |
| Troubleshooting | Symptom → likely cause → fix → how to avoid it |
| Tutorial | Guided practice in Learner (2) sections only |

Kinds align with Diátaxis labels in page metadata (`how-to`, `explanation`,
`reference`, `tutorial`) where those are declared.

## Examples: keep them light until interactivity lands (D225)

Today's pager can only show fixed text. A static worked example is like a
textbook: useful, but not interactive. A future `tam` version will add
custom bash sandbox windows where the reader runs commands and sees live
results; the developer will specify that design when the time comes.

Until then:

- Put worked examples only in Learner (2) sections of the same page.
- Prefer one short, real capture over fading ladders, Parsons puzzles, or
  multi-stage example craft on every page.
- Capture output by running the command (never invent it). Prefer a scratch
  directory. Note when output varies by machine.
- Link the man page instead of copying long flag tables.
- Do not spend large effort polishing static example sequences that
  interactive sandboxes will replace.

Expert sections may still show advanced illustrative commands. Novice
sections must not.

## Learning devices (Learner sections only)

In Learner (2) sections, when they earn their place:

1. One-line **Goal:** ("You will be able to…").
2. A short **worked example** (captured; see above).
3. A safe **Try it** in a scratch or reversible setting, when practice helps.
4. A **Check yourself** question with a folded answer, when a check helps.
5. Path onward via **Where next** / **See also**, and embedded path links
   where a goal path includes the page.

Skip a device when it would be empty, false, or busywork. Never put these
devices in Novice (1) sections.

## Inputs to read, not copy

When shaping tone or structure, read Diátaxis, `man-pages(7)`, clig.dev, and
the Google and Microsoft developer style guides. Write in Tamlinux's own
words. Never paste third-party manual text into the corpus (authoring's
public-boundary and sourcing rules still apply; plan 26 forbids copying the
Omarchy manual or Arch Wiki prose).

## Related files

- [`corpus-authoring.md`](corpus-authoring.md) — sourcing, public boundary, terms
- [`corpus-pedagogy.md`](corpus-pedagogy.md) — level anatomy, paths, quizzes
- [`corpus-audit.md`](corpus-audit.md) — review criteria
- Templates under [`templates/`](templates/)
