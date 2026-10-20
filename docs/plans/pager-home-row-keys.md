# Home-row pager keys and attention column

**Plan status:** Completed
**Reviewed:** 2026-10-20
**Scope:** Command 0.32.0 (pager keys, attention column, help texts, man
page) and corpus 0.33.5 (welcome page). One checklist item, TCK10.
**Authority:** The developer's decision to build it, 2026-10-20, recorded as
Tamlinux decision D175; [plan maintenance procedure](../plan-maintenance.md).
**Next action:** None in source. Interactive review by the developer and
packaged acceptance follow the Tamlinux lifecycle on his word.

## Problem and agreed outcome

The pager's letter keys had grown by accretion. `j`/`k` moved the attention
line (vim), `a`/`f` selected the previous/next link anywhere on the page,
`s`/`d` jumped to links on other lines, and `g`/`G` went to the top and
bottom. There was no consistent rule a reader could learn once.

The developer's design (D175) puts navigation on the home row, mirrored on
each hand, so the same finger does the same thing on both hands:

| Finger | Left hand | Right hand | Action |
| --- | --- | --- | --- |
| Little | `a` | `;` | column outward (`a` left, `;` right) |
| Ring | `s` | `l` | attention line up one row |
| Middle | `d` | `k` | attention line down one row |
| Index | `f` | `j` | column inward (`f` right, `j` left) |
| Ring, one row down | `x` | `.` | top of page |
| Middle, one row down | `c` | `,` | bottom of page |

`g` follows the selected link and `h` toggles help (`?` stays). `G` is
retired. The right hand is vim's `h j k l` moved one key right; `j` = left
and `k` = down deliberately differ from `less`/`man`/vim, and the docs say so.

The **attention column** is the horizontal partner of the attention line.
Left and right keys raise it on the attention line's caught link and move it
from link to link along that line, wrapping at either end. It is drawn as a
band as wide as the caught link on every content row. The selected link is
where the column crosses the line. The column clears when the attention line
moves, the selection or page changes, or `g`/Enter is pressed.

Unchanged: arrows, Home/End, PgUp/PgDn, Space/`b`, Tab/Shift-Tab, Enter,
Left Arrow, `u`, `` ` ``, `n`/`N`/`p`/`t`/`i`, `/`, `0`-`3`, `q`, Esc.

## Baseline and dependencies

Command 0.31.5 and corpus 0.33.4 at `c649aad`. Builds on the attention line
(command 0.25.0), breadcrumb navigation (0.31.1, 0.31.4) and the portable
display profiles (0.29.0).

## Implementation and handoff

**TCK10 (home-row pager keys)**, done when the keys behave as above, the
column is visible in the screen model and clears as specified, and the help
overlay, status line, `tam info --help`, man page, README and welcome page
describe the new keys.

- `src/pager.c`: `move_line_up()`/`move_line_down()` share the arrows'
  breadcrumb behavior; `move_column()` moves along the attention line or the
  breadcrumb row; `handle_key()` wraps the key dispatch and clears
  `column_active`. `column_band()` and `emit_band_padding()` draw the band in
  `emit_line()` and on rows past the end of the page. `select_vertical_link()`
  is removed.
- Tests: the nav-grid case in `tests/run.sh` covers every new key, wrapping,
  lines without links, retired `G` and `g` following; `tests/pager_screen.py`
  checks the band's cells and its clearing. Existing tests send `k`/`l` where
  they sent `j`/`k`.

## Acceptance and limits

`make test` passes (see the session record). The band uses the attention
tint on 256-color terminals and reverse video on the Linux console. On a
terminal with neither, the band is not drawn; the selected-link emphasis
still shows the catch. For a link that wraps onto a second line, the band
covers its first-line portion.

Not yet done, and only on the developer's direction: his own interactive use,
and review of an exact packaged candidate.

## Disposition

Completed in command 0.32.0 / corpus 0.33.5. Evidence: the
[session record](../ai/2026-10-20-tam-pager-home-row-keys.md) and its commits.
