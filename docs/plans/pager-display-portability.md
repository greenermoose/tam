# Pager display across terminals

Status: proposed for review, 2026-10-17. Implementation has not started.
Investigated command 0.28.0 at commit `041f7163963e473280a55017047349cddcc4db2c`.

## Goal and visible behavior

Make the built-in pager comfortable in a Linux virtual console, Foot window,
other capable terminals, SSH sessions and terminal multiplexers, across short
and long pages and small and large viewports. The attention line must remain
visible on ordinary text, blank separators and linked or searched text.
Moving it should not flash the page. Resizing must preserve the reader's
place and reflow the content. Existing navigation semantics remain intact.

Use capabilities to adapt presentation. A terminal without cursor addressing
gets readable plain output; a monochrome terminal with cursor addressing can
still offer an interactive pager. Pixel-identical styling across unlike
terminals is neither necessary nor possible. Clear reading focus is necessary.

## Findings and strength of evidence

1. `src/pager.c` hard-codes `TAM_ATTENTION_BG` to `ESC[48;5;236m`.
   This is an indexed 256-color background, not true color. No terminal
   capability check precedes it. The existing attention test in `tests/run.sh`
   looks for that exact sequence and therefore rewards the nonportable behavior.
2. Local `infocmp -1 linux` reports eight colors; `infocmp -1 foot` reports
   256. Linux documents extended background colors as reduced to eight basic
   colors. Its console implementation converts index 236 to a dark gray whose
   RGB components all fall below the background-color threshold, producing
   black. On a default black console background the attention tint disappears.
   This explains the reported symptom if the affected TTY is a Linux console.
   The exact console/palette has not been inspected in this session.
   [Linux console manual](https://man7.org/linux/man-pages/man4/console_codes.4.html),
   [kernel implementation, `rgb_from_256` and `rgb_background`](https://github.com/torvalds/linux/blob/master/drivers/tty/vt/vt.c).
3. `draw()` sends `ESC[2J ESC[H` before every frame. The main loop calls it
   after every input event, including ignored keys and movement already clamped
   at a page boundary. `emit_line()` frequently calls `write()` once per byte
   and once per padding space. This exposes a blank screen and partial repaint
   between updates, a direct mechanism for the reported flicker. The helpers
   also discard short-write and error results.
4. A read-only PTY probe of the existing `build/tam`, using the four-line
   `tests/fixtures/nav-grid.html` page at 80 columns by 24 rows, produced this
   output for each of `TERM=foot`, `TERM=linux` and `TERM=dumb`:

   | Action | Output bytes | Whole-screen clears |
   | --- | ---: | ---: |
   | Initial entry | 412 | 1 |
   | Down arrow | 395 | 1 |
   | Up arrow | 398 | 1 |
   | Up again at the top | 398 | 1 |
   | Ignored `z` | 398 | 1 |

   All three terminal profiles emitted the same charcoal sequence. A one-row
   probe also entered the pager; the current layout assigns both content and
   status to row one. PTYs verify the byte stream, not physical console colors
   or perceived smoothness. Real console and Foot acceptance is still required.
5. `draw()` rereads dimensions, but does not reflow `tam_document_t`. Loaders
   in `src/tam.c`, `src/man.c` and `src/info.c` retain the original width in
   their contexts, so even later page loads can keep the stale width.
   Shrinking the height can leave the attention line outside the viewport.
6. `emit_line()` and `pager_build_links()` count bytes as columns. Status
   truncation counts code points, and the renderer's `dwidth()` also assumes
   one cell per code point. None handles wide or combining characters
   consistently. The renderer additionally imposes a 16-column minimum.
7. The search prompt writes directly to the terminal, without width-aware
   clipping or a resize path. Merely optimizing `draw()` would leave another
   source of wrapping, stale cells and visual inconsistency.

## Recommended design

Keep the C document and navigation model. Introduce a small terminal backend
using the ncurses **terminfo API**, plus a buffered renderer that compares the
previous and next visible rows. Use that backend for content, status and the
search prompt. Do not make a terminal library responsible for page navigation.

This uses established TUI techniques: ncurses compares its virtual and physical
screens and batches output; Ratatui constructs an in-memory frame and emits a
diff against the preceding frame. The useful principle is to compose first and
update the terminal once, with only changed content.
[ncurses refresh documentation](https://invisible-island.net/ncurses/man/curs_refresh.3x.html),
[Ratatui rendering design](https://ratatui.rs/concepts/rendering/under-the-hood/).

Terminfo can be used without starting a curses screen. Discover and expand
capabilities in-process with `setupterm`, `tigetnum`, `tigetstr`, `tparm` and
`tputs`; do not spawn `tput` for rendering. Route the `tputs` output callback
into the frame buffer and respect padding requirements. Resolve actual link
flags with pkg-config/the platform packaging rather than assuming every system
provides a separate `libtinfo`.
[terminfo API](https://invisible-island.net/ncurses/man/curs_terminfo.3x.html).

### Alternatives considered

| Approach | Assessment |
| --- | --- |
| Change the color and buffer the existing full repaint | Useful intermediate fix, but repeated clears still flash and size/layout faults remain. |
| Terminfo plus buffered row comparison | Recommended: fits the existing small pager and preserves its input/navigation behavior. |
| Full ncursesw screen and input backend | Mature capability and update handling; reasonable if the small backend becomes complex. Requires a broader rendering/input conversion and performance measurements. |
| Notcurses, a Rust/Go framework, or an external pager | Adds a larger dependency or changes the implementation direction. No demonstrated need for this reader. |
| Synchronized output alone | Helps supporting graphical terminals, but cannot solve console styling, unsupported terminals or excess repainting. |

Do not maintain two interactive backends. If the initial terminfo integration
reveals that full ncursesw is substantially simpler, bring the measured tradeoff
back for design review before broadening implementation. Measure startup and RSS
against the roadmap's existing budgets; no performance claim is assumed here.

### Capability and attention policy

Initialize terminal capabilities once, only for interactive output. Use the
terminal attached to stdout, independent of whether a graphical session exists.
Do not infer capabilities from `DISPLAY`, `WAYLAND_DISPLAY`, or the presence of
an SSH connection. Inside tmux/screen, use the multiplexer-facing capabilities.

| Capability profile | Attention presentation |
| --- | --- |
| 256 or more indexed colors | Retain the charcoal background; explicitly choose a readable pale foreground on that row so a light terminal's default dark text cannot disappear. |
| 8/16 colors or monochrome, with reverse/standout | Full-row reverse/standout using default foreground/background, avoiding extended-color assumptions. |
| Cursor addressing but no usable reverse/standout | Reserve a small ASCII focus gutter (`>` on the active row, spaces elsewhere) before layout. |
| `TERM=dumb`, missing/unknown terminal description, or no usable cursor addressing/reset | Plain readable output with no pager control sequences or raw-mode entry. |

Honor nonempty `NO_COLOR` by using non-color focus attributes or the gutter,
not by removing the attention indication. Empty `NO_COLOR` does not disable
color. A real monochrome VT-style terminal remains interactive if its
capabilities suffice. Never change the terminal palette to create a tint.
[NO_COLOR convention](https://no-color.org/).

Resolve semantic styles centrally: row focus, heading/code/link, selected link,
search match and current search match. Never toggle reverse twice and expect
the visual result to compose. Define a tested style table for each profile:
on a reverse attention row, selected links get an additional bold/standout
distinction rather than another reverse; search gets an additional available
emphasis. Honor `ncv` attribute/color conflicts. The Linux entry advertises
underline/dim conflicts with colors. If a combination cannot be rendered
distinctly, use monochrome emphasis and a gutter rather than unreadable colors.
[terminfo attribute conflicts](https://invisible-island.net/ncurses/man/terminfo.5.html).

For charcoal focus, suppress or remap semantic foreground colors that would
become unreadable on it; retain semantic emphasis. Test normal text, headings,
code, blank rows and selected/searched links together. Provide an ASCII gutter
escape hatch for unusual user palettes; settle its public option spelling at
review instead of adding a general theming framework.

### Rendering and event loop

Separate state changes, layout, frame composition and terminal emission:

```text
input or resize
  -> apply state changes in order
  -> normalize geometry and viewport
  -> compose visible rows with resolved styles
  -> compare with last successfully emitted rows
  -> buffer cursor moves, changed rows and status
  -> write the update completely
```

Start with row-level comparison; cell-level differencing is not required.
Each row must serialize deterministically from a known attribute state and
restore that state, so comparisons include both text and effective styles.
Moving attention between two plain unlinked lines with a fixed viewport changes
two body rows and the status row. Changes to a wrapped selected link or search
state may change additional rows; let the frame comparison discover them.

Do not clear the display during ordinary movement, page changes or search.
Overwrite changed rows in place, then erase only obsolete trailing cells with
the correct background. Handle `bce` explicitly: use erase-to-end with a styled
background only when supported; otherwise write appropriate padding. A full
cache invalidation repaints all visible rows without first exposing a blank
screen. Allow an explicit redraw/recovery action for externally corrupted output.

Use absolute positioning between rows, not a stream of CR/LF operations. Protect
the bottom-right cell from unintended wrap/scroll; keep the final status cell
unused when no safe terminal operation can fill it. On a one-row terminal,
erase-to-end with `bce` can fill the attention background without printing into
that cell. Test delayed wrap and both `bce` settings. Reset attributes before
painting unused rows or leaving the pager.

Replace `out()`/`outn()` with an update buffer and a checked `write_all` loop.
Handle short writes and `EINTR`; handle terminal loss/errors without spinning
or recording an uncompleted frame as the physical screen. One buffered update
is not an atomic screen presentation guarantee.
[write semantics](https://man7.org/linux/man-pages/man2/write.2.html).

Keep rendering event-driven. No timer should repaint an idle pager. Process
already-available complete key events in order and optionally combine their
visual updates within a bounded batch; do not discard navigation actions or
delay isolated keys. Preserve escape-sequence deadlines and keep Quit responsive.

Add synchronized output as a later enhancement. Foot documents private mode
2026. Where supported, wrap an already-built update in `CSI ? 2026 h` and
`CSI ? 2026 l`. Discover support with a bounded DECRQM query and consume its
reply in the input decoder, retaining interleaved user input. Render the first
frame immediately; lack of a reply disables the enhancement. Consume late
replies safely. Do not leave sync enabled while waiting for input, loading a
page or accepting a search query. Guarantee matching end/reset on normal and
recoverable error paths. Linux consoles must work with synchronization disabled.
[Foot control sequences](https://man.archlinux.org/man/foot-ctlseqs.7.en),
[synchronized-output protocol](https://github.com/contour-terminal/vt-extensions/blob/master/synchronized-output.md).

### Geometry, text width and reflow

Use one geometry calculation for drawing, movement, link visibility and paging.
For height >= 2, use `height - 1` content rows and one status/prompt row. At
height 1, show one navigable content row and omit persistent status; `/` may
temporarily occupy that row and restore content afterward. For zero/unknown
ioctl sizes, retain the last valid geometry or use the documented startup
fallback. Use checked allocation sizes for large dimensions.

Make the status line adaptive: retain essential position/quit information,
then add title, selected target/message and longer key hints as space permits.
Never let a long title push all essential controls out of a narrow display.
Render an empty document deliberately without reporting a nonexistent `L1/0`.

Introduce a shared UTF-8/display-cell iterator used by wrapping, clipping,
padding, status, search input and hyperlink column calculations. Keep byte
offsets for search ranges and logical source positions, and cell offsets for
terminal geometry. Decode complete characters and use `wcwidth` under an
initialized `LC_CTYPE`; account for combining marks and width-two characters.
Never split a UTF-8 character or leave half a wide character at the margin.
Invalid/nonprintable text needs a deterministic visible replacement. Provide
ASCII substitutions for interface symbols in non-UTF-8 locales. Width agreement
with every terminal's complex emoji segmentation is not guaranteed; test common
combining/CJK text and document that boundary rather than claiming perfection.
[wcwidth](https://man7.org/linux/man-pages/man3/wcwidth.3.html),
[multibyte decoding](https://man7.org/linux/man-pages/man3/mbrtowc.3.html).

Retain an opaque parsed layout backing object in `tam_document_t` instead of
discarding all unwrapped blocks after `tam_render_html()`. Add an atomic
`tam_document_reflow(doc, content_width)` operation. This supports stdin pages
without rereading exhausted input, and avoids rerunning man conversion just
because the window resized. Keep ownership explicit and free backing storage
with the document. Measure the memory cost on representative pages.

Emit a logical position with each visual row (block/run identity plus byte
offset). Save logical positions with history entries, alongside fallback visual
positions. Reflow restores the same reading text as closely as possible;
height-only changes need no relayout. Rebuild link geometry and search hits
after width changes, keeping selected link identity and current hit when valid.
Resize itself must not push history, reset depth or refetch content.

Change `tam_page_loader_t` to accept the current content width explicitly and
update its implementations/callers in corpus, generated topics, man and Info.
Remove stale width authority from loader contexts. Preserve initial stdin
documents independently of whether they have a page-navigation loader.

Remove the unconditional 16-column layout clamp for interactive narrow views.
Define long-token, preformatted and table behavior: wrap long tokens/preformatted
text at cell boundaries, and stack table cells/rows with labels when columns
cannot fit. Keep text reachable instead of silently clipping it away. Make
list indentation shrink safely when the viewport is narrower than its prefix.
Expected plain-output fixture changes must be reviewed explicitly.

## Implementation sequence and handoff

Another agent can implement the following milestones after design review.
Work in the existing clone, claim only the current milestone's paths, and
refresh the inspected baseline before editing. No implementation or deployment
is authorized by this proposal alone.

| Milestone | Work and likely paths | Completion evidence |
| --- | --- | --- |
| 1. Capability policy | New private `src/terminal.[ch]`; pager entry/fallback in `src/pager.[ch]` and shared view callers; Makefile linkage. Profile discovery, focus policy, basic cleanup. | Linux/8-color, Foot/256-color, monochrome and dumb profiles have the intended behavior; `NO_COLOR` retains focus. |
| 2. Buffered incremental painting | New private `src/pager_screen.[ch]` if useful; replace direct writes and `draw()` in `src/pager.c`; integrate status/search prompt. | No ordinary screen clear; two-row focus updates; no output for unchanged frames; short-write/error tests. |
| 3. Geometry and width | Shared display-width helper; `src/render.[ch]`, `src/pager.c`; narrow status/prompt and height-one policy. | Tiny, narrow, wide, Unicode and bottom-right cases pass; no status scroll. |
| 4. Resize and retained reading position | Retained parsed backing/layout mapping in renderer; width-aware loader signature in `src/render.h`, `src/tam.c`, `src/man.c`, `src/info.c`; history/reflow in pager. | Widen/shrink restores readable text, attention visibility, search/link state and back/forward position; stdin resize works. |
| 5. Synchronized output and input bursts | Backend capability probe/reply handling and bounded event batching. | Support, rejection, timeout, late/split reply and interleaved-key tests; matching begin/end; smooth operation with feature disabled. |
| 6. Integration and acceptance | New focused PTY/screen tests and fixture pages; `tests/run.sh`, renderer goldens where affected; README/man and version/development records. Packaging dependency changes in the owning delivery repo when preparing a candidate. | Automated matrix passes; actual Linux console and Foot review accepts focus, readability and held-arrow behavior. |

Each milestone should keep a runnable command. Avoid coupling unrelated corpus
content work into this change. Use a small line-indexed link/search lookup if
profiling shows whole-document rescans on every arrow; visible-frame storage
and terminal output should scale with viewport size, not page length.

## Verification and acceptance matrix

Add meaningful PTY integration tests with a screen-state interpreter capable of
cursor positioning, attributes, erase semantics and pending wrap. Captured ANSI
substring checks alone cannot establish that the right cells are highlighted.
Use an existing suitable test dependency if already available, or a documented
small test-only model covering the emitted subset. Do not pretend that setting
`TERM=linux` makes a PTY emulate the kernel console.

Run the cross product strategically, covering each boundary and interaction:

| Dimension | Required cases |
| --- | --- |
| Capabilities | Foot; xterm-256color; Linux eight-color; synthetic 16-color and zero-color profiles; monochrome reverse; gutter-only; dumb; unknown/missing terminfo; no alternate screen; `bce` off. |
| Transport | Direct terminal; tmux/screen; SSH with correct inner TERM; piped stdout; redirected stdout; piped stdin with a controlling TTY; no usable controlling TTY. |
| Geometry | Widths 1, 2, 8, 15, 16, 40, 80, 160; heights 1, 2, 3, 24, 60; zero-sized report; shrink/grow/rapid resize while reading and entering search. |
| Page length | Empty; one line; fewer than content rows; exactly content rows; one more row; hundreds and thousands of lines. |
| Content/state | Blank attention row; headings/code; long unbroken tokens; tables/preformatted text; wide/combining/malformed UTF-8; wrapped selected link; search match overlapping focus/selection; long title/target/query; active depth; history and failed page load. |
| Input/lifecycle | Single arrows; held arrows; top/bottom no-op; ignored key; Tab/Shift-Tab and a/s/d/f; page keys; search/cancel; EOF; Ctrl-C/SIGTERM; suspend/resume if supported; partial writes and terminal disconnect. |

Objective rendering assertions:

1. With a stationary viewport and no links/search, a one-line move repaints only
   the former/new attention rows and changed status. No whole-screen erase is
   sent after initialization during navigation. A no-op emits no visual update.
2. All changed rows, including every continuation of a selected wrapped link,
   reach the correct final text/attributes. Shorter replacement rows leave no
   stale text or tint. Blank focus rows remain visible.
3. For height >= 2 and a nonempty document,
   `top <= cursor_line < top + content_rows`, with valid document bounds.
   Short pages do not scroll; excess rows clear to normal background. Status
   and search never spill into content or scroll the screen accidentally.
4. Resizes preserve a logical reading location and valid selection/search.
   The same applies after a resize followed by back/forward or a new page load.
5. Plain-output and fallback paths emit no cursor/style/probe escapes and do
   not enter raw mode. Cursor-addressable monochrome remains interactive.
6. All controllable exits restore terminal attributes, cursor and modes actually
   changed, including synchronization and alternate screen where supported.
   Repeated pager invocation in one process must reset global flags. SIGKILL
   cannot be promised cleanup; do not make that an acceptance claim.
7. Record bytes written, write calls and arrow-to-update latency before/after
   on the same fixture/sizes. A plain focus move's output depends on changed
   row width, not terminal height or whole-document length. No idle repaint or
   periodic polling appears. Report startup/RSS changes with the baseline.

Run focused tests and `make test-command`; run `make test` with the repository's
required stable-tree/exclusive coordination before final integration. Refresh
renderer goldens only for intended changes and review their diffs. Use sanitizer
runs for the new retained-layout ownership, size arithmetic and input parser.

Finally, have the developer review an exact packaged candidate in a real Linux
console and Foot, including light/dark appearance where available, one-row/tiny
windows, short pages, long pages and held Up/Down at viewport boundaries.
Automated stream checks cannot replace that visual acceptance. Record the exact
candidate and result. Test/Run/Release and package delivery remain separate
developer-controlled stages; this work plan does not advance them.

## Review decisions and current limits

- Recommended default: retain charcoal on capable terminals, use full-row
  reverse on consoles/monochrome, with an ASCII gutter escape hatch. Confirm
  whether the developer prefers a gutter as the ordinary console presentation.
- Confirm that the reported text TTY is a Linux virtual console. The capability
  design remains appropriate for SSH/serial connections, but the exact failure
  explanation and manual matrix may need adjusting.
- Select the command version at implementation time against the current
  roadmap. Do not reserve a potentially conflicting number in this draft.
- This proposal changes no pager source, installed software or terminal palette.
  Source investigation and the PTY baseline are complete; physical-console
  appearance and the proposed implementation are not yet validated.
