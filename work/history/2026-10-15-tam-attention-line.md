# Attention line and anchored link navigation (0.25.0)

- **Date:** 2026-10-15
- **Stage:** Develop — tam 0.25.0; the corpus stayed at 0.24.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.6, from the agy process log that served this session.
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commit:** `698756c`, which carries no AI trailers.
- **Record:** Written on 2026-10-15 by Claude Code from the session transcript; the session wrote no record of its own.
- **Authorship:** Fred specified the behaviour and its reading-order model. Antigravity discussed it before planning, renumbered the later roadmap versions, and implemented it in `src/pager.c`.

## Guiding prompts

Session-management questions are omitted.

> Please insert a new tam version 0.25.0 for the tam command and increment all existing version numbers in the milestone to follow.
>
> We will not update the knowledge corpus, so that can stay at version 0.24.0.
>
> What I want is the notion of a line cursor. When you use any pager key (including ones that scroll and ones that select a hyperlink), you might change the linenumber of the cursor. The point of this is that this anchors the selection for hyperlinks so you can follow them more easily and naturally.
>
> For example, if you were tabbing through a page and then scrolled off the bottom of the page, you might leave your hyperlink behind. Then when you press enter while looking at a link in the visible page you might end up going who knows where.
>
> Maybe this better described as an attention line. There should be one line in the file that is subtly indicated to have your attention, either because you have scrolled to it or selected a hyperlink in it.
>
> Let's discuss this concept before you plan how to implement. I want to make sure you grasp it and we're on the same page (metaphorically speaking).

> 1 B
> 2 Enter should only active if the attention line contains a link OR if there is a selected link visible on the page. You should be able to move the attention line up and down on the page to "catch" a hyperlink that stays selected until you catch another one. If you sweep down, you "catch" the first hyperlink on the line, if you sweep up, you "catch" the last hyperlink on the line. You can always use a s d f or Tabs to jump to the next hyperlink.
> 3 yes, but see my note above. Which link gets selected depends on the direction of entry: going down, you get the first link. Going up, you get the last link.
>
> I'm thinking of the way you read English text, left to right, then top to bottom. When you get to the end of a line, you wrap your attention to the beginnnig of the next line. When you are going back from one line to the one above it, you go from the left of the line below to the right of the line above.

> Yes, let's go. We may need to adjust the background tint to ensure readability, but your suggestion seems reasonable. If you are able to get data on best practices for highlight values to minimize distraction and maintain readability but be noticeable, that would be great.

## Decisions and scope

- One attention line, tinted with a dark background (`48;5;236`), moves with
  scrolling and link keys; the status line reports `L<line>/<lines>`.
- Sweeping down catches the first link on a line, sweeping up the last; a
  caught link stays selected across lines without links while it is visible.
- `Tab`, `Shift-Tab` and `a`/`s`/`d`/`f` move relative to the attention line;
  `Enter` follows only a link on the attention line or a selected link still
  on screen.

## Verification

The session added terminal-driven pager tests to `tests/run.sh`. Two
existing pager navigation tests failed during the work; the session then
edited `tests/run.sh`, and the final `tests/run.sh` run passed. The guide suite was not
run in this session. No `AI_PROVENANCE.md` row or decision-record line was
written; both were added on 2026-10-15.
