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

- **Level:** Beginner (1), Intermediate (2) or Expert (3). **All** (0) shows
  every level. The reader's chosen level is their *lens*. Pages declare levels
  in `<meta name="level">`.
- **Path:** a reading order. *Level paths* are `beginner.html`,
  `intermediate.html` and `expert.html`. *Goal paths* live in `learn/paths/`
  and are named by what the reader gains.
- Do not call levels "depths" or "routes". Boot pages keep "fallback route"
  and "rescue route" for their own meaning.

## What each level means

| | Beginner (1) | Intermediate (2) | Expert (3) |
| --- | --- | --- | --- |
| Dreyfus stage | Novice to advanced beginner | Competent | Proficient to expert |
| The reader asks | What do I type, and what should I see? | How does it work, and which option fits? | What exactly happens, why, and how do I change it? |
| Afterwards they can | Complete the task safely by following steps, recognize success, undo it | Adapt to a new case, diagnose common failures, choose between options | Explain the internals, handle failures nobody has seen before, extend the system, contribute |
| Usual Diátaxis kind | Tutorial, how-to | How-to, explanation | Reference, explanation |
| Examples | Complete worked example with captured output | Faded example, then a completion task, then an independent task | Precise semantics, edge cases, experiments |
| Questions | Recall; predict the output | Scenario; spot the bug; classify | Trade-offs; open prompts with a model answer |
| Safety | Scratch directory only; risky commands labeled | Risk labels; undo stated | Rollback point stated before any system change |

Intermediate is not "beginner with more words". Its center is transfer
(adapting to a new case) and troubleshooting.

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

**Beginner**

1. **Goal:** one can-do sentence ("You will be able to…").
2. **Why it matters:** one concrete paragraph.
3. **Before you start:** prerequisites, with links, and what the commands
   touch.
4. **New to this?:** a folded `aside class="first-time"`.
5. **Worked example:** exact commands, captured output, and a numbered
   "What just happened".
6. **Try it:** a close variation in a scratch directory.
7. **If it goes wrong:** likely messages, what they mean, how to recover.
8. **Check yourself:** a quiz of two to four questions.
9. **New words:** glossary links (a short `<dl>` until the glossary exists).
10. **Next step.**

**Intermediate**

1. **Goal.**
2. **Mental model:** how it works, with an ASCII diagram when one helps.
3. **Worked example, completion task, independent task,** with folded
   solutions.
4. **Choosing:** a decision table, "use X when…".
5. **Troubleshooting:** a table of symptom, likely cause, check, fix.
6. **On Tamlinux:** where Tamlinux differs from generic Arch Linux, sourced.
7. **Check yourself:** scenario questions.
8. **Further reading:** two to four annotated references.

**Expert**

1. **Reference:** precise semantics, option and descriptor tables.
2. **How it works:** internals, with the authoritative files, manual pages,
   specifications and public Tamlinux source paths.
3. **Why Tamlinux chose this:** the reasoning in words. Never cite internal
   decision identifiers.
4. **Edge cases and failure modes,** including performance and energy cost.
5. **Explore:** open-ended experiments, such as tracing a pipeline with
   `strace -f`.
6. **Case study,** when a public record supports one.
7. **Check yourself:** open prompts with model answers.
8. **Sources:** primary references.
9. **Improve it:** the public repository or plan where a reader can
   contribute.

## Worked examples

- **Capture every output by running the command.** Never write output from
  memory. Run a scratch example in `mktemp -d` and remove the directory at
  the end. When output depends on the machine, write "Your output will
  differ" and name the system and date you captured it on.
- **Annotate:** follow the example with numbered notes tied to its parts.
- **Contrasting pairs:** put the tempting command beside the correct one and
  say why. The correct action comes first, as in the pitfall-to-best-practice
  convention.
- **Fade:** set the same task three times with less help each time: full
  example, a blank to fill (`___`), then the goal alone.
- **Parsons problems:** give the lines of a pipeline or procedure out of
  order and ask for the order.
- **Think-aloud diagnosis (expert):** narrate how a practitioner narrows a
  real symptom. Show each piece of evidence, each branch point and why it
  was taken.
- **Case studies** come only from public Tamlinux records, such as a plugin
  ABI mismatch, or a library crash handled with a self-retiring local
  package. Keep the lesson: how the system is maintained.

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

1. **Gain:** one can-do sentence.
2. **For you if:** two or three lines a reader can recognize themselves in.
3. **Levels:** where the path starts and where it ends, and roughly how many
   reading sessions it takes. Never promise hours.
4. **Stages:** three to six. Each stage has one to four page links at stated
   levels (`?level=`, which sets the reader's lens) and a checkpoint quiz.
5. **Capstone:** one realistic whole task in a scratch or reversible setting,
   with a folded worked solution.
6. **Next paths.**

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

## Corpus-wide structures

TCK24 provides these structures. The skills map reports the pilot's coverage;
unmapped domains still need TCK25 teaching migration. Anatomy/quiz warnings
are nonfatal during that migration. See the authoring guide for exact metadata
and citation markup, and `tools/report-coverage.py` for cells without pages or
paths. Source availability does not establish learner acceptance.

- **Skills map** (`learn/skills/`). Domains: Desktop, Shell, Files and
  permissions, Processes, Software and packages, Updates and rollback, Boot
  and recovery, Hardware and displays, Networking, Troubleshooting,
  Customizing, Contributing. Each domain has can-do cells at the three
  levels. Pages and paths name their cells in `<meta name="skills">`.
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
