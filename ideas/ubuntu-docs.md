# Ubuntu docs

## What it is

- **Format:** tutorial guides are single Discourse topics with a key/value metadata table, categories, 1–5 difficulty and `## Step title` followed by `Duration: MM:SS`; the tutorial template uses admonitions, language-hinted code fences and an image syntax ([How to write an Ubuntu guide, search-result snippet, accessed 2026-11-26](https://discourse.ubuntu.com/t/how-to-write-a-tutorial/14000)).
- **Scope and style:** Canonical's style guide prescribes a four-way Diátaxis structure, sentence-case headings, no skipped heading levels, copyable code and screenshots only when no better way exists ([Documentation Style Guide, full page read, accessed 2026-11-26](https://documentation.ubuntu.com/style-guide/)).
- **Licence:** not checked in this study; evidence below mixes full page reads with search-result snippets, marked per item. All verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Every guide opens with an Overview containing a summary, "What you'll learn" and "What you'll need", with prior knowledge stated and linked ([Ubuntu tutorials guidelines, full page read, accessed 2026-11-26](https://ubuntu.com/tutorials/guidelines)).
  Tam application: use the same three fields on each path landing page and point "What you'll need" at prerequisite pages or glossary entries.
- **adopt**: Guides are capped at 60 minutes, steered to 15–30 minutes, with roughly 5–10 minutes per step and user-visible progress at each step ([Ubuntu tutorials guidelines, full page read, accessed 2026-11-26](https://ubuntu.com/tutorials/guidelines)).
  Tam application: show a rough time per checkpoint and keep each checkpoint to one observable result.
- **adopt**: Code checkpoints tell a reader where they should be and offer a way to restore that state ([Ubuntu tutorials guidelines, full page read, accessed 2026-11-26](https://ubuntu.com/tutorials/guidelines)).
  Tam application: store expected sandbox state with each checkpoint so a learner can resume or repeat it.
- **adopt**: Setup is written once in beginner guides, and advanced guides link back as prerequisites rather than repeating it ([Ubuntu tutorials guidelines, full page read, accessed 2026-11-26](https://ubuntu.com/tutorials/guidelines)).
  Tam application: share preparation checkpoints between paths and branch only where commands genuinely differ.
- **adopt**: A product should have one tested "true path", possibly split into parts or variants, rather than several near-duplicate routes ([The idea of a tutorial, full page read, accessed 2026-11-26](https://ubuntu.com/blog/the-idea-of-a-tutorial)).
  Tam application: keep one canonical route per goal and express role differences as variants.
- **adopt**: The closing page reminds the reader of the result and offers "Next steps" and "Further reading" ([Ubuntu tutorials guidelines, full page read, accessed 2026-11-26](https://ubuntu.com/tutorials/guidelines)).
  Tam application: end every path with explicit onward links instead of a bare completion message.
- **consider**: Difficulty is publication metadata on a 1–5 scale, with level 1 assuming only that the reader can open a terminal ([Ubuntu tutorials guidelines, full page read, accessed 2026-11-26](https://ubuntu.com/tutorials/guidelines)).
  Tam application: expose it as a filter label, not as a promise that content adapts itself.
- **consider**: Deliberate mistakes are walked into, analysed and fixed in the guide ([Ubuntu tutorials guidelines, full page read, accessed 2026-11-26](https://ubuntu.com/tutorials/guidelines)).
  Tam application: add a "if this went wrong" branch on checkpoints that touch the real system, not on every step.
- **consider**: A lint pass over navigation generated from source tables would catch small errors before publication ([How-to maintain documentation on Discourse, search-result snippet, accessed 2026-11-26](https://discourse.ubuntu.com/t/how-to-maintain-documentation-on-discourse/28916)).
  Tam application: validate generated navigation and links as part of the corpus build.
- **reject**: A Discourse forum as the primary corpus ([How to write an Ubuntu guide, search-result snippet, accessed 2026-11-26](https://discourse.ubuntu.com/t/how-to-write-a-tutorial/14000)).
  Tam application: forum categories, metadata tables and a protected navigation topic would add a platform dependency to a local command.
- **reject**: A locked forum post as the source of published navigation ([How-to maintain documentation on Discourse, search-result snippet, accessed 2026-11-26](https://discourse.ubuntu.com/t/how-to-maintain-documentation-on-discourse/28916)).
  Tam application: navigation belongs in version-controlled source that can be reviewed and tested.
- **reject**: The audience survey widget with an analytics backend ([Ubuntu tutorials guidelines, full page read, accessed 2026-11-26](https://ubuntu.com/tutorials/guidelines)).
  Tam application: its read-versus-do and self-rated level questions are useful prompts, but not telemetry.

## Things to avoid

- Do not separate exercises from answers: Ubuntu explicitly says tutorials are not a class or a test ([Ubuntu tutorials guidelines](https://ubuntu.com/tutorials/guidelines)).
- Do not scatter external links through the middle of a guide: Ubuntu restricts them to the first and last steps, except checkpoint source links ([Ubuntu tutorials guidelines](https://ubuntu.com/tutorials/guidelines)).
- Do not copy the UK/US spelling conflict as settled guidance: Discourse contribution guidance says British English while the current Canonical style guide says US English ([Contributing to Discourse documentation, search-result snippet, accessed 2026-11-26](https://discourse.ubuntu.com/t/contributing-to-discourse-documentation/42391); [Documentation Style Guide, full page read, accessed 2026-11-26](https://documentation.ubuntu.com/style-guide/)).
- Do not force storytelling and cliffhangers into a terse CLI tool: the tangible result matters more than narrative entertainment ([Ubuntu tutorials guidelines](https://ubuntu.com/tutorials/guidelines); [Diátaxis tutorials, full page read, accessed 2026-11-26](https://diataxis.fr/tutorials)).

## Where to look

- All URLs below were accessed 2026-11-26; snippet-only pages are marked.
- [Full page read] [Ubuntu tutorials guidelines](https://ubuntu.com/tutorials/guidelines) — overview contract, time boxes, checkpoints, mistakes and difficulty scale.
- [Full page read] [Canonical Documentation Style Guide](https://documentation.ubuntu.com/style-guide/) — Diátaxis, headings, code blocks and language rule.
- [Full page read] [The idea of a tutorial](https://ubuntu.com/blog/the-idea-of-a-tutorial) — one true path and tutorial-versus-how-to distinction.
- [Full page read] [Diátaxis: tutorials](https://diataxis.fr/tutorials) — reliability and visible results.
- [Search-result snippet] [How to write an Ubuntu guide](https://discourse.ubuntu.com/t/how-to-write-a-tutorial/14000) and [How to write a tutorial](https://discourse.ubuntu.com/t/how-to-write-a-tutorial/14000) — forum schema and review route.
- [Search-result snippet] [How-to maintain documentation on Discourse](https://discourse.ubuntu.com/t/how-to-maintain-documentation-on-discourse/28916) — fragile navigation table.
- [Gap] A direct fetch of the tutorial-writing Discourse topic returned no output, and rendered tutorial output was not inspected.
