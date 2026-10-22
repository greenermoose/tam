# Pager level line and status row (commands 0.39.0 to 0.40.0)

- **Date:** 2026-10-21
- **Time:** 22:30 to 22:51 -04:00
- **Stage:** Develop; command 0.39.0 replaces the level bar and the "Written
  for" banner with a right-aligned level line above the page heading; 0.39.1
  letters the lines above the title and shows the lens as `All text 0123`;
  0.39.2 makes the title the top of the page for the top-of-page keys;
  0.40.0 rebuilds the status row with the version at its right edge.
  A development installation of 0.39.0 was made at the developer's request.
- **CLI Tool:** Claude Code 2.1.307
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `959535f` (0.39.0), `dd013ea` (0.39.1), `7f66ebf` (0.39.2)
  and the 0.40.0 commit that updates this record
- **Authorship:** Fred Horch wrote the layout note (the line's form, its
  position, alignment and examples) and decided the level bar's and related
  links' placement. Claude proposed the relatives line's wording, dimming
  and narrow-width wrapping, and wrote the code, tests and documentation.
- **Checked against:** `src/depth.c`, `src/render.c`, `src/pager.c`,
  `src/tam.c`, `tests/run.sh`, `tests/pager_screen.py`, `man/tam.1`,
  `infocmp -1 xterm-256color`, and pages in `knowledge/`

## Guiding prompts

> Read my pager layout for tam idea in the [private ideas repository] then
> implement in the tam repo.

> I don't like separating the related page links from the level bar. Let's
> make that a third line, above the page heading. So it would go
>
> Breadcrumb (aligned left)
>     Level bar (aligned right)
>     Related page links (optional, aligned right)
> Page Heading
>
> Does this seem like a good UI/UX design choice?
>
> Push back if you think there is a better way. I do not like the current
> page layout. The level stuff and the related pages are not done well. We
> need to improve that. Either my idea or something else that is better than
> what we have now.

> Build it with refinements 1 to 3. I'm not sure should about the clickable
> way back. What is your recommendation on that.

> Install 0.39.0 to test

> For 0.39.1, change how the line numbering works. The level line should not
> be L1; it should be Lb. The page title should be L1. The breadcrumb line
> should be La. When the page opens, L1 should be active. See screen shot:
> when the active line is on Welcome to Tamlinux, the footer should read L1
> 1-32/49. If I up arrow to the Beginner text line, the footer should read Lb
> 1-32/49 and if I up arrow again to the breadcrumb line, the footer should
> read La 1-32/49.
>
> Also, in the footer, instead of showing Level 0 All, show All text 0123,
> with 0 underlined because it is currently selected. If I press 1, the
> footer should show Beginner text 0123 with 1 underlined.

> I think x/. keys should go to L1, not Lb. I consider L1 to be the top of
> the page. The level line is not part of page content; it is a page display
> control line.

> Let's add a v X.Y.Z (P.D.Q) aligned right in the footer to show me the
> version of the tam command (corpus). That way I'll know which version I'm
> testing when I run it.
>
> Also, let's remove some confusing key binding "help". Here's what I want
> the footer to show:
>
> <Page Title> | L<attention line> <Visible range>/<Page length> | <Level>
> text 0123 | h Help  Esc Quit  [] Select links  -> <Selected link>
> v0.0.0 (0.0.0)
>
> Ask if you have questions.

## Key decisions

- The developer's note gives the form
  `<Level> text [on <Level> path] [(no <Level> text)]`, right-aligned below
  the breadcrumb and above the heading, with the path linked. "All" names a
  page showing several levels.
- The developer chose to replace the level bar rather than keep it under the
  heading, and to put the related pages on a third right-aligned line rather
  than drop them or separate them from the level line.
- Refinements the developer accepted: the relatives line reads
  `<Level> text: A | B | C`; both lines are dimmed but keep link
  highlighting; a line too long for the terminal wraps left-aligned.
- Found while checking real pages: titles contain commas, so links are
  separated by bars, and the level path page is left out of the relatives
  because the line above already links it.
- Claude recommended against a clickable level chooser to replace the bar's
  links (keys `0`-`3` and the help overlay cover switching); none was built.
- 0.39.1: the developer's example kept the 0.39.0 range and total
  (`1-32/49`). Since the level line is no longer numbered, the range and
  total count from the title, so the same screen reads `L1 1-31/48`. A
  page with two level lines (a wrapped line, or the relatives line) letters
  them `Lb` and `Lc`. For the narrower status rows the developer did not
  specify, the lens reads `All 0123` (60 to 99 columns) and `0123` below
  that; a terminal without underline brackets the lens digit.
- 0.39.2: Home, an empty `#` link and `` ` `` on the Guide index are the
  same "top of page" jump as `x` and `.`, so all of them go to `L1`. Each
  scrolls to the very top, so the level lines stay in view above the title.
- 0.40.0 defaults the developer left open, stated to him before building:
  messages take the link's place while they show; narrow rows drop the
  title first (the breadcrumb repeats it), then the link, then the key
  hints, keeping the line, range, lens and version; the version is plain
  numbers. `q` still quits though only `Esc Quit` is shown.
- The fixture `shell/pipelines.html` gains one `?level=beginner` link, so the
  test that a level-naming link sets the lens still has a link to follow.

## Verification

- `make test-command`: all command tests pass, including the PTY level-lens
  tests and the refreshed `pedagogy` render golden (`UPDATE=1
  tests/render.sh build/tam`, diff read).
- `make test`: both suites pass.
- 0.39.1: `make test`, both suites pass, including new PTY assertions that
  the page opens on `L1` and Up/Down step through `Lb` and `La`; PTY drives
  at 120, 80 and 44 columns with the tests' screen model confirm the labels,
  `Lb`/`Lc` on a page with two level lines, and the underlined lens digit.
- 0.39.2: `make test`, both suites pass, including a PTY assertion that `x`
  and `.` go to `L1` from the bottom; a PTY drive of the Welcome page shows
  `x`, `.` and Home returning from `L52` to `L1` with "Beginner text" in view.
- 0.40.0: `make test`, both suites pass; tests that read the selected link
  now stop at the padding before the version, and the search test checks
  the new key list and the version. PTY drives at 160, 120, 80 and 44
  columns show the layout and the drop order.
- PTY renders at 100 and 44 columns with the tests' screen model: right-aligned
  at 100 columns, left-aligned wrap at 44, and `dim` (`ESC [2m`) emitted
  off the attention line.
