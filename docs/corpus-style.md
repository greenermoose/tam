# How the tam knowledge corpus reads

Read this with [`corpus-authoring.md`](corpus-authoring.md) and
[`corpus-pedagogy.md`](corpus-pedagogy.md). Authoring covers what must be
true. Pedagogy covers level anatomy, paths and quizzes. This file covers
voice, page kinds, one-page difficulty levels, learning paths, and how
interactive sandboxes replace static examples.

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

Line Lb shows the reader lens as `0 Showing all text`, `1 Showing novice
text`, `2 Showing learner text`, `3 Showing expert text` (D221).

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
  on first use (`tamlinux/design/terminology.html` or the glossary).
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

## Examples: interactive sandboxes (D225)

The pager can run a simulated shell in a `<tam-sandbox>`. Use it wherever a
page shows how to use the computer. The reader types commands and sees
authored output or works on the sandbox's own sample files. A static
command-and-output transcript is no longer the teaching device.

- Put every sandbox in the Learner (2) section of the same page. Pages with
  a sandbox declare novice, learner and expert sections.
- Give the sandbox one clear goal, short progressive hints and a fallback.
  Make the task a small discovery, prediction or diagnosis, not busywork.
- Prefer built-in commands that really operate on sandbox files: `ls`, `cd`,
  `cat`, `grep`, `wc`, `sort`, `uniq`, `head`, `tail`, `find`, `tree`, pipes
  and redirection. Scripted rules cover tools the sandbox cannot simulate.
- Label scripted output as a sample. Do not write that it was captured on a
  machine unless it really was; do not leave "Captured on..." prose beside
  a sandbox.
- Link the man page instead of copying long flag tables.
- Keep `<tam-init>` short: it is the only sandbox text visible in pipe
  rendering.

Expert sections may still show advanced illustrative commands. Novice
sections must not contain sandboxes, goals or quizzes.

## Learning devices (Learner sections only)

In Learner (2) sections, when they earn their place:

1. One-line **Goal:** ("You will be able to…").
2. A short **interactive sandbox** that replaces a captured worked example.
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
