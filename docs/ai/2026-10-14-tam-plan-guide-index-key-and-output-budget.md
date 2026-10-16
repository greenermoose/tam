# Plan ideas: Guide index key and a screen budget for output

- **Date:** 2026-10-14
- **Stage:** Develop — plan updates for milestone 10; no code.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5, from the agy process log that served this session.
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `2921c85`, `b58c01c`, `3a7e577`. None carries AI trailers.
- **Record:** Written on 2026-10-15 by Claude Code from the session transcript; the session wrote no record of its own.
- **Authorship:** Fred supplied the ideas and answered the design questions. Antigravity recorded them in `ROADMAP.md` and `docs/plans/development.md` as milestone 10 work.

## Guiding prompts

Session-management questions are omitted.

> I have some ideas for improving the tam command. Can you claim whatever is necessary so you can record these ideas and add them to the tam plan?

> Bind ` to return to the Guide index in the tam command, so no matter where you are you can always quickly return to the Guide index page.

> As a general rule, when you type something like tam explain and get a usage guide, it should not spew out so much detail that the critical info scrolls off the screen. For tam explain, the usage section is good, but the Available topics should not appear. Something like tam explain topics can show them all if you want them. Noodle on this and examine other output to make sure that we are keeping our output reasonable and useful. If you need more guidance on this to get to the core principle I'm trying to convey, just ask. I can try out my tam to see what it does if you're curious about various options and whether they are too verbose. You may also consider a --verbose or --terse flag. We can assume a "goldilocks" just right and offer --terse and --verbose so the tam command provides less or more detail for a piping situation.

> Record this in the plan. Those are all the ideas I have now. Please record them so we can work on them in a future milestone. Update plans for tam so these ideas are included. Any more questions?

> 1 open inside tam's biult-in pager, and each topic can then be hyperlinked so the user can easily go to the topic they want. tam explain by itself should let users know that there is a special topics page available that lists all the topics.
> 2 one slug per line

## Decisions and scope

- A backtick key returns to the Guide index from anywhere in the pager.
- Default output fits one screen ("Goldilocks"); `--terse` and `--verbose`
  give less or more. `tam explain` alone shows usage and points to a topics
  page; `tam explain topics` opens that page in the pager with each topic
  linked, and the terse listing is one slug per line.

## Verification

Plan text only; no build or test was needed. Milestone 10 implemented these
ideas in tam 0.18.0.
