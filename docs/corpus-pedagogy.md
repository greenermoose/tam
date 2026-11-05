# Teaching with the tam knowledge corpus

Read this with [`corpus-authoring.md`](corpus-authoring.md) before you write or
restructure a page. That guide says what must be true and where facts come from.
This one says how a page teaches. The authoritative plan is
[learning levels, goal paths and self-quizzes](plans/learning-levels.md). It
records which parts the pager renders today and which arrive in later slices.

The goal is mastery. A reader should leave a page able to do something, and
come back later able to do more.

## Principles

Each rule below rests on established research in learning and documentation.
Authors do not cite these on pages; they are here so the rules make sense.

| Rule | Basis |
| --- | --- |
| Filter by level. Guidance that helps a newcomer slows an expert down. | Expertise reversal effect (Kalyuga et al. 2003) |
| Teach a beginner with a complete worked example, then fade the support: partial example, then blank task. | Worked-example and fading research (Sweller; Renkl; Atkinson) |
| Start from real tasks, act early, and help readers recognize and recover from errors. | Minimalism (Carroll, *The Nurnberg Funnel*, 1990) |
| Know which job a page does: tutorial, how-to, reference or explanation. That is separate from its level. | Diátaxis (Procida) |
| Define levels by what the reader can do. | Dreyfus model of skill acquisition (1980); CEFR can-do descriptors |
| End sections with questions the reader answers before seeing the answer. | Retrieval practice (Dunlosky et al. 2013); generation effect (Slamecka and Graf 1978) |
| Explain answers, including why wrong answers are wrong. | Formative feedback (Shute 2008) |
| Ask readers to explain why, and show a wrong way beside the right one. | Self-explanation (Chi 1989); contrasting cases (Schwartz and Bransford 1998) |
| Let experts watch an expert think: diagnosis narrated step by step. | Cognitive apprenticeship (Collins, Brown and Newman 1989) |
| Build paths around one whole, real task. | Four-component instructional design (van Merrienboer and Kirschner, *Ten Steps to Complex Learning*) |
| Give the hardest, most transforming ideas the deepest ladders. | Threshold concepts (Meyer and Land 2003) |
| Write beginner text in plain language: short sentences, every term defined at first use. | ISO 24495-1:2023; *Teaching Tech Together* (Wilson 2019) |

Two rules are Tamlinux's own:

- **Simplify, never misstate.** A beginner section may leave things out. It
  may not say anything false. The sourcing rule applies at every level.
  Where a simplification hides something important, add a "Peek under the
  hood" aside that links to the expert section.
- **Offline, cheap and plain.** Prefer what is installed (man, Info, tldr,
  `/usr/share/doc`). Mark online resources as online. Everything a page
  shows must work in plain ASCII on the Linux console.

## Terms

- **Level:** Novice (1), Learner (2) or Expert (3) (D221, amending D184).
  **All** (0) shows every level. The reader's chosen level is their *lens*.
  Pages declare levels in `<meta name="level">`. `beginner` and `intermediate`
  remain accepted backwards-compatible aliases for Novice and Learner.
- **Path:** a reading order through the material. Learning paths are embedded
  directly inside every topic page's Learner (2) text, showing the starting
  point (`Start`), previous stop (`Back`), and next stop (`Next`) with
  `?level=2`. The separate `Learning` (`learn/`) corpus track is eliminated in
  favor of discovery within each page.
- Do not call levels "depths" or "routes". Boot pages keep "fallback route"
  and "rescue route" for their own meaning.

## What each level means

Levels are **difficulty texts on the same page**, not separate files (D226).
The reader picks all, novice, learner, or expert; the pager shows or hides
matching sections. The guide's aim is that a complete novice can become an
expert by reading it, following **Where next** and **See also** (and
Learner path links) in an order that builds skill.

| | Novice (1) | Learner (2) | Expert (3) |
| --- | --- | --- | --- |
| Dreyfus stage | Complete newcomer | Competent practitioner | Proficient to expert |
| Reader intent | Gentle introduction; assumes no prior knowledge and no interest in practicing | Wants to practice, build skills, and learn by doing | Needs reference semantics, architecture, and advanced usage |
| Content role | Conceptual orientation: what it is, why it exists, everyday mental model, glossary definitions | The ONLY level with `Goal:` and `Try it` practice exercises, quizzes, and path links | Internals, precise semantics, edge cases, illustrative advanced examples (not student exercises) |
| Usual Diátaxis kind | Gentle explanation | Tutorial, hands-on how-to | Reference, architectural explanation |
| Examples | None — no worked examples at Novice (D225) | Short worked example and hands-on `Try it` when useful; keep static examples light until interactive sandboxes | Advanced illustrative configurations or script pipelines |
| Questions | None (no quiz pressure) | Check yourself self-quizzes with folded answers | Trade-offs; open prompts with model answers |
| Learning paths | Follow shared **Where next** / **See also** | Those links plus embedded `Start`, `Back`, and `Next` (`?level=2`) where a goal path includes the page | Follow shared **Where next** / **See also** |

### Personas

Write each section for one of these readers. Example accounts in commands
remain `alex`.

- **Sam** has come from macOS or Windows and has never used a terminal. Sam
  wants to get work done without breaking anything. Write beginner sections
  and First steps for Sam.
- **Priya** uses Linux servers at work and is new to Wayland, tiling and Nix.
  Priya wants to understand what is running. Write intermediate sections for
  Priya.
- **Kai** is a developer or administrator who wants to change Tamlinux, debug
  it and contribute. Write expert sections for Kai.

Readers are not one level everywhere. Sam may be an expert with spreadsheets
and a beginner at boot. The lens is one keypress away, and the skills map
lets readers place themselves domain by domain.

### The novice on-ramp

There are three levels, not four. Readers who have never used a terminal get
help in three places:

1. **The First steps goal path** starts before the beginner level. It covers
   opening a terminal, typing and pasting, Enter and Ctrl+C, reading a
   prompt, and getting back to the Guide.
2. **A folded "New to this?" aside** in each beginner section
   (`<aside class="first-time">`). It covers the steps a novice cannot
   infer: where to type, what `$` means, that silence often means success.
   Advanced beginners can leave it closed.
3. **Glossary links** on the first use of each term, to its beginner
   definition.

If learner tests show novices still failing beginner sections with all
three in place, the developer reconsiders a fourth level. Record the
evidence. Do not add one on your own.

## Page openings

After the navigation and level lines, start every page with a clear heading
and one ordinary sentence explaining what the page is about. Write for a
reader with a high school education and no specialized technical knowledge.
Define unfamiliar terms in the sentence or link to a definition; prefer
familiar words. Do not use labels, bold text, or other formatting on the
orientation sentence. Follow it with any necessary context, prerequisites,
and safety information in plain language. The heading and sentence appear
at every reading level.

## Page patterns

Choose one pattern per page. Do not pad a page into three sections to fill a
ladder.

| Pattern | Use for | Markup | Template |
| --- | --- | --- | --- |
| Ladder | Threshold concepts and core skills: streams, processes, permissions, ownership layers, the boot chain, compositor and client | One `<section data-level>` per declared level | [`ladder.html`](templates/ladder.html) |
| Layered | Mostly shared pages that need a beginner hint or an expert note | Untagged body; `data-level` on an `aside`, `p`, `li`, `pre`, `details`, `div` or `table` | [`layered.html`](templates/layered.html) |
| Single | Orientation (beginner), reference tables (expert), hubs and paths | `<meta name="level">` only | [`single.html`](templates/single.html) |
| Path | Level paths and goal paths | Single pattern with the path format below | [`path.html`](templates/path.html) |

Untagged content shows at every lens. Content is shared by every level only
outside the level sections: the title, one-line summary, "Where next", "See
also" and the revision line. The pager generates the level bar and level
labels. Never type "Beginner:" into a heading.

## Section anatomy

Use these blocks in this order. Leave out a block only when it would be
empty or false, never to save effort.

**Novice (1)**

Assumes no prior knowledge and no interest in practicing. A gentle introduction for someone who knows nothing about the topic.
1. **What this is:** One plain-language paragraph explaining the concept simply.
2. **Why it matters:** What problem it solves in everyday terms.
3. **How to think about it:** An everyday mental model or analogy.
4. **New words:** Plain-language definitions linking to glossary terms.
*(Novice sections never include Goal statements, Try it practice exercises, quizzes, or worked examples.)*

**Learner (2)**

The only level for active skill practice and hands-on learning.
1. **Goal:** One can-do sentence ("You will be able to…").
2. **Learning path:** Start, Back, and Next links (`?level=2`) for the topic's learning path.
3. **Worked example:** Step-by-step commands with captured output and explanation.
4. **Try it:** Practice exercises in a scratch directory for the reader to try.
5. **If it goes wrong:** Likely errors, what they mean, and recovery steps.
6. **On Tamlinux:** Specific Tamlinux defaults and conventions.
7. **Check yourself:** Self-quiz with folded questions and answers explaining why.

**Expert (3)**

Reference semantics, internals, and advanced usage.
1. **Reference:** Precise semantics, option, flag, and descriptor tables.
2. **How it works:** Internals, authoritative kernel/system interfaces, specs, and source paths.
3. **Advanced usage examples:** Complex commands and script patterns shown for reference and illustration (not framed as student practice exercises).
4. **Edge cases and failure modes:** Concurrency, performance, resource limits, and signal handling.
5. **Check yourself:** Deep trade-off and diagnostic questions with model reasoning.
6. **Sources & Improve it:** Primary references and public contribution links.

## Worked examples

Worked examples belong in **Learner (2) only** (D225). Novice text stays
simple and never carries command-and-output walkthroughs. Keep static
examples light: a future `tam` version will offer interactive bash sandbox
windows; do not invest heavily in textbook-style example craft that those
will replace. Full rules live in [`corpus-style.md`](corpus-style.md).

- **Capture every output by running the command.** Never write output from
  memory. Run a scratch example in `mktemp -d` and remove the directory at
  the end. When output depends on the machine, write "Your output will
  differ" and name the system and date you captured it on.
- **Annotate lightly:** a few numbered notes tied to the example's parts are
  enough; prefer one short capture over multi-stage fading sequences.
- **Contrasting pairs** (optional): put the tempting command beside the
  correct one and say why. The correct action comes first.
- **Think-aloud diagnosis (expert):** narrate how a practitioner narrows a
  real symptom when the page needs it. Show each piece of evidence and why
  a branch was taken.
- **Case studies** come only from public Tamlinux records. Keep the lesson:
  how the system is maintained.
- Fade ladders and Parsons problems are optional Learner devices, not a
  default for every page.

## Self-quizzes

Every beginner and intermediate section ends with a quiz. Expert sections
use open prompts. Write each question as a `<details class="q">` inside a
`<section class="quiz">`:

```html
<section class="quiz">
<h3>Check yourself</h3>
<p>Decide your answer before you reveal it.</p>
<details class="q">
<summary>What does <code>|</code> connect?</summary>
<p>The standard output of the command on its left to the standard input of
the command on its right. It passes a stream of bytes, not files or
arguments.</p>
</details>
</section>
```

- **Beginner question types:** recall, predict the output, "which key…?".
- **Intermediate question types:** scenario ("you see X; what do you check
  first?"), spot the bug, classify ("which layer owns this file?"), Parsons
  problems.
- **Expert question types:** trade-offs, "what breaks if…?", open prompts.
  Use `class="q open"`; the pager labels them "Model answer". Present one
  sound line of reasoning, not the only one.
- **Multiple choice:** list the options in the question. The answer names
  the right option and says why each other option is wrong.
- **Answer length:** keep answers under about 120 words. Each must teach
  something, not just say "B".
- **Other folds:** solutions use `<details class="solution">`, hints use
  `<details class="hint">`.

In the pager, answers stay hidden until revealed (Enter, `r`, `*`) and hide
again when the reader leaves. In pipe output they print under "Answers" at
the end of the section. Older readers show them expanded, which is
acceptable.

## Paths

Every path page uses [`templates/path.html`](templates/path.html):

1. **Opening:** a clear heading and an ordinary orientation sentence, as in
   Page openings above. Explain who the path helps and any prerequisites or
   safety limits in ordinary sentences when needed.
2. **Stages:** three to six. Each stage has one to four page links at stated
   levels (`?level=`, which sets the reader's lens) and a checkpoint quiz.
3. **Capstone:** one realistic whole task in a scratch or reversible setting,
   with a folded worked solution.
4. **Next paths.**

Use only pages that exist. Mark a stage without a page as planned.

**Goal-path catalog.** Add each path when its pages exist.

| Path | Gain | Levels |
| --- | --- | --- |
| First steps | Use the desktop and terminal with confidence from the first day | before beginner, then 1 |
| Settle in from macOS or Windows | Do everyday work here and know where things moved | 1 |
| Command-line fluency | Read, combine and adapt commands, then write small scripts | 1 to 3 |
| Make it yours | Customize keybindings, appearance and workflow safely | 1 to 2 |
| Keep it healthy | Update, read logs and roll back with confidence | 2 |
| Rescue | Recover from a failed update or a system that will not boot | 2 to 3 |
| Run lean | Use less energy and memory without losing capability | 2 to 3 |
| Work with AI locally | Run and tune local AI tools and know what they cost | 2 to 3 |
| Understand the design | Explain how Tamlinux is put together and why | 2 to 3 |
| Build and contribute | Set up a development toolchain and make a safe first change | 3 |
| Cables and protocols | Trace electricity and signaling from circuits to reliable links | 1 to 3 |
| From atoms to bits | Connect quantum and solid-state ideas to digital logic | 1 to 2 |
| From gates to machine code | Follow logic through architecture, the instruction cycle and assembly | 2 to 3 |
| Display and GPU | Explain pixels, GPUs and display cables to a blank-screen check | 2 to 3 |
| How software runs | Trace kernel, process, filesystem and init into a running program | 2 to 3 |
| Session and desktop | Follow login, compositor and input into a working desktop | 2 to 3 |
| Diagnose from first principles | Choose a failing layer with evidence from physics through packages | 3 |

## Corpus-wide structures

TCK24 provides these structures. The skills map reports the pilot's coverage;
unmapped domains still need TCK25 teaching migration. Anatomy/quiz warnings
are nonfatal during that migration. See the authoring guide for exact metadata
and citation markup, and `tools/report-coverage.py` for cells without pages or
paths. Source availability does not establish learner acceptance.

- **Skills map** (`learn/skills/`). Domains: Desktop, Shell, Files and
  permissions, Processes, Software and packages, Updates and rollback, Boot
  and recovery, Hardware and displays, Signals and cables, Networking,
  Troubleshooting, Customizing, Contributing. Each domain has can-do cells at
  the three levels. Pages and paths name their cells in `<meta name="skills">`.
- **Self-placement** (`learn/placement.html`). For each level, a "you will
  recognize yourself if…" description and folded questions. It ends in
  links that set the lens.
- **Glossary** (`learn/glossary/`). One definition per level, plain to
  precise to standard, with the botanical terms included. Mark a term's
  first use on a page with `<dfn>` or a glossary link.
- **Bibliography** (`learn/bibliography/`). Each entry has a stable ID and
  notes its level, cost, whether it is offline or online, and its license.
  Cite entries with `<cite data-ref="id">`.
- **Further structures, after the scaffolding:** a FAQ in each part (real
  questions only), a symptom index under `maintain/symptoms/`, recipes under
  `use/recipes/`, and reference cards generated from sources.

## Callouts and risk labels

- **Asides:** `<aside class="note|tip|caution|tamlinux|why|peek|first-time">`.
  `tamlinux` marks where Tamlinux differs from generic Linux. `peek` links a
  simplification to the expert section.
- **Risk labels:** put `data-risk` on every `<pre>` that holds commands.

  | Value | Meaning |
  | --- | --- |
  | `read-only` | Reads only |
  | `scratch` | Writes only in a temporary directory |
  | `user` | Changes your own files or settings |
  | `system` | Needs privilege or changes packages |
  | `boot` | Touches the bootloader, initramfs or firmware |

  The linter requires `system` or `boot` wherever a block uses `sudo`,
  `pkexec`, package installs or removals, or boot tools.
