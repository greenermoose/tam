# Pager quick keys, address display and status row

**Plan status:** Ready
**Reviewed:** 2026-10-20
**Scope:** Command 0.34.0 (expected; pager keys, status row, `=` address,
`page:` addresses, action links, help texts, man page) and corpus 0.35.1
(expected; Guide index and welcome page). Five checklist items, TCK15–TCK19.
**Authority:** The developer's decisions of 2026-10-20 and his instruction to
write this plan for another agent to build, recorded as Tamlinux decision
D176; [plan maintenance procedure](../plan-maintenance.md).
**Next action:** Build TCK15, then each later slice in order. Commit after
each slice.

## Problem and agreed outcome

Several small gaps in the pager add up. Selecting links needs Tab and
Shift-Tab, and terminals send Shift-Tab in at least five different ways. The
reader cannot see the address of the page they are on, so they cannot reopen
it later or hand it to someone. The status row says `L65/79 (Top)` and
leaves out which lines are on screen. Searching the whole Guide takes a trip
through the index, and the two search prompts don't say which search they are.

The developer decided the following. Do not change it without asking him.

| Key or element | Today | After this plan |
| --- | --- | --- |
| `]` / `[` | unused | next / previous link anywhere on the page (wraps) |
| Tab / Shift-Tab | next / previous link | unchanged; documented as aliases for `]` / `[` |
| `N` | previous search hit, or previous link when no search is active | previous search hit only; without a search it does nothing |
| `=` | unused | toggles `Path = <address>` in the status row |
| `\` | unused | search the Guide; the prompt reads `Search in guide \` |
| `/` | search this page; the prompt reads `/` | same search; the prompt reads `Search in page /` |
| Status row position | `L65/79 (Top)`, `L75/79`, `L79/79 (Bot)`, `L3/4 (All)` | `L65 3-78/79`: line 65; lines 3 to 78 of 79 on screen |
| `tam show <address>` | guide paths and some `tam:`/`man:`/`info:`/`tldr:` addresses | also `page:<path>`, `tam:explain…` and `tam:keywords…` |
| Guide index | "Topics" and "Search" items; a one-line key hint | "Guide topics"; no Search item; a Quick keys table |

Rules for `=`:

- **Guide pages** show `page:` plus the page's path, for example
  `page:shell/pipelines.html` or `page:learn/index.html?level=beginner`.
  Addresses beginning `man:`, `info:`, `tldr:` or `tam:` are shown unchanged.
  Standard input has no address, and shows `Path = (standard input has no
  address)`.
- **Every address `=` shows opens the same page** when passed to
  `tam show <address>`.
- **Dismissing:** Esc, a second `=`, or moving to another page (a link, back,
  forward or a depth change) dismisses it. Moving within the page does not.
  This is the planner's default; the developer may change it.
- **Esc order:** a bare Esc first closes help, then the address, then the
  search highlights. With none of those showing, it quits, as before.
- **Quoting:** Man page addresses contain parentheses (`man:ls(1)`), so in a
  shell they need quotes: `tam show 'man:ls(1)'`. Keep the address format
  and document the quoting.

The Guide index Quick keys table has the developer's four rows:

| Key | Row | Link |
| --- | --- | --- |
| `/` | Search in page | `tam:prompt:page-search` (opens the `/` prompt) |
| `\` | Search in guide | `tam:prompt:guide-search` (opens the Guide search prompt) |
| `?` | Help | `tam:help` (opens the help overlay) |
| `` ` `` (backtick) | Return to this index | none; you are already there |

`tam:help` and `tam:prompt:page-search` are pager actions, not pages: they
change no history and `=` never shows them.

Out of scope: any other key, the status-row hints (`/ n/N asdf jkl; g u ``
` ``), bare `tam <address>` without `show`, and keyboard layouts other than
US QWERTY.

## Baseline and dependencies

Command 0.33.0 and corpus 0.35.0 on `develop` at `52b38fe`. Builds on the
[home-row keys](pager-home-row-keys.md) (TCK10, D175), the
[developer loop](dev-loop.md) fixtures (TCK13) and `tam tldr` (command
0.33.0). Before starting, run `git log --oneline -3` and read `src/version.h`
and `knowledge/version`. If either version has moved past this baseline,
take the next minor (command) and next patch (corpus) from the current
files, not the numbers here.

Line numbers below are from `52b38fe`; find the code by the names given.

## Implementation and handoff

Read `AGENTS.md` first. In particular: claim paths with `tam-work`, commit
only your paths, and never switch branches, stash, reset, rebase or amend.
Each slice lists the paths to claim. Its "Done when" line is the completion
evidence. Run `make test` before each commit. Commit trailers follow
`AI_PROVENANCE.md`.

### TCK15 (link keys `]` and `[`; `N` search-only; command version)

Claim: `src/pager.c`, `src/version.h`, `tests/run.sh`,
`docs/plans/development.md`, `ROADMAP.md`.

1. In `src/pager.c` `handle_key_inner()` (~line 1838), next to `case '\t':`,
   add `case ']':` calling `select_next_link(st, 1, 1)` and `case '[':`
   calling `select_next_link(st, -1, 1)`. Leave Tab and every Shift-Tab
   encoding as they are, in the bare-Esc branch (`c1 == '\t'`) and the CSI
   branch (`Z`, `1;2Z`, `9;2u`, `27;2;9~`).
2. Change `case 'N':` (~line 1910) so it calls `pager_next_hit(st, -1)` only
   when `st->search_query[0] != '\0'`, and otherwise does nothing.
3. Bump the command minor version. Change `TAM_COMMAND_VERSION` in
   `src/version.h` (expected `0.34.0`) and the `--version` assertion in
   `tests/run.sh` (~line 90). Add an entry at the **top** of the Decision record
   in `docs/plans/development.md` (newest entries are first, just under the
   heading). Change the `ROADMAP.md` version header and section 3 `develop`
   row. The help overlay changes in TCK18, not here.
4. Tests in `tests/run.sh`, in the link-navigation case (~lines 222–283, fixture
   `tests/fixtures/link-cycle/`):
   - `]` from no selection selects the same first link as Tab.
   - `[` from the first link wraps to the last link, as Shift-Tab does.
   - Replace the `N` fallback assertion (`t8`). `N` with no search active
     leaves the selected link unchanged.
   - The in-page search case (~line 688) still shows `N` stepping back
     through hits.

**Done when:** `make test` passes; `]`/`[` move through links exactly as
Tab/Shift-Tab do; `N` without a search changes nothing.

### TCK16 (status row `L65 3-78/79`)

Claim: `src/pager.c`, `tests/run.sh`, `tests/pager_screen.py`.

1. In `src/pager.c`, where the status row is built (~lines 1046–1075),
   replace the `essential` and `view` strings with one string
   `L<cursor> <first>-<last>/<total>`:
   - `cursor` = `st->cursor_line + 1`
   - `total` = `st->doc->nlines`
   - `first` = `st->top + 1`, or `0` when the page has no lines
   - `last` = the smaller of `st->top + usable` and `total`

   Remove `(All)`, `(Top)`, `(Bot)` and the code that measures and appends
   them. Keep the existing width tiers.
2. Update the tests that read the old format:
   - `tests/run.sh`: the `startswith("L1/")` checks (~lines 357–363), the
     telemetry check (~line 665, which also looks for `(Top)`/`(All)`), and the
     nav-grid assertions (~lines 878–973). The nav-grid page fits on screen,
     so `L3/4` becomes `L3 1-4/4`.
   - `tests/pager_screen.py` ~lines 325 and 330. A one-row terminal still shows
     no status row. The empty page shows `L1 0-0/0` and no longer
     `L1/0`; keep asserting that the status row stays off the content rows.
3. Add a scrolling case to `tests/pager_screen.py`. Use an existing fixture
   longer than a short terminal, choosing the height so the page overflows.
   - At the top, `first` is 1.
   - After `c` (bottom), `last` equals `total`.
   - After Space (page down) from the top, `first` is greater than 1.
   - In every state, `last - first + 1` equals the number of content rows.

**Done when:** `make test` passes, and the status row reads
`L<n> <a>-<b>/<N>` at the top, middle and bottom of a long page and on a page
shorter than the screen.

### TCK17 (`=` address display; `page:` addresses in `tam show`)

Claim: `src/pager.c`, `src/tam.c`, `tests/run.sh`, `man/tam.1`.

1. In `src/pager.c`:
   - Add `int show_address;` to `pager_t`.
   - Add `static void pager_address(const pager_t *st, char *out, size_t cap)`
     that follows the rules for `=` above. Read `st->cur_href`. Standard input
     arrives as `<stdin>`, or `-`; `switch_depth()` (~line 1381) already
     tests for both.
   - `case '=':` toggles `st->show_address`.
2. While `show_address` is set, the status row is `Path = <address>` in place
   of its usual contents. Truncate it to the width with
   `status_byte_limit()`, as the rest of the row does.
3. In the bare-Esc branch of `handle_key_inner()` (the `read_byte(..., 60,
   ...) != 1` case), clear `show_address` after the `help_open` check and
   before the search check.
4. In `handle_key()` (~line 2020), after `handle_key_inner()` returns, clear
   `st->show_address` when `st->doc != doc`. That one check covers links,
   back, forward and depth changes.
5. In `src/tam.c` `cmd_show()` (~line 1052):
   - After the argument loop, if `page` starts with `page:`, skip the five
     characters and continue as for a bare guide path. If nothing follows
     `page:`, report `missing guide page after 'page:'` with
     `TAM_EXIT_USAGE`.
   - Add `tam:explain` and `tam:keywords` to the branch that already opens
     `tam:man`, `tam:tldr` and `tam:search` through `corpus_load()` (~line
     1150).
6. In `man/tam.1`, under `show`, document `page:` addresses and the quoting
   of man addresses. In the pager key list (~line 262), document `=`.
7. Tests in `tests/run.sh`. Use `TAM_KNOWLEDGE_DIR` set to
   `tests/fixtures/guide`; never use `knowledge/` (`make check-repo`
   enforces this).
   - Non-interactive: the output of `show page:shell/pipelines --pipe`
     equals that of `show shell/pipelines --pipe`;
     `show tam:keywords --pipe` and `show 'tam:explain?topic=pipelines' --pipe`
     exit 0; `show page:` exits with the usage status.
   - Interactive, on `show shell/pipelines`:
     - `=` shows `Path = page:shell/pipelines.html`.
     - A line-down key keeps it.
     - Esc removes it and the position appears again.
     - `=` twice shows and then hides it.
     - `=` and then following a link removes it on the new page.

**Done when:** `make test` passes. Every address `=` shows on the fixture
pages opens with `tam show <address>`.

### TCK18 (`\` Guide search; labelled prompts; `tam:help` and `tam:prompt:page-search`; help texts)

Claim: `src/pager.c`, `src/man.c`, `src/info.c`, `tests/run.sh`,
`tests/fixtures/guide/quick-keys.html`.

1. In `src/pager.c`:
   - Add `case '\\':` calling `follow(st, "tam:prompt:guide-search")`.
   - In `pager_search_prompt()` (~line 723), change the label `"/"` to
     `"Search in page /"`.
   - In `pager_prompt_action()` (~line 762), change `"Guide search: "` to
     `"Search in guide \\"`. There is no trailing space; the cursor sits
     after the slash.
2. In `follow()` (~line 1292), before the `tam:prompt:` branch:
   - `tam:help` sets `st->help_open = 1` and returns.
   - `tam:prompt:page-search` calls `pager_search_prompt(st)` and returns.

   Neither pushes history.
3. Replace the help overlay text (~line 1016) with these lines, in this
   order:

   ```
   Navigation help
   Back: Left Arrow   Guide: `   Help: h or ?   Quit: q
   Line: d/k down, s/l up   Top: x or .   Bottom: c or ,
   Links on the line: a/j left, f/; right   Any link: ] next, [ previous
   Follow: g, Enter or Right Arrow   Tab / Shift-Tab: same as ] / [
   Search: / this page, \ the guide   Next/previous hit: n/N   Address: =
   Info Up: u   Next/Previous node: n/p
   Close help with h, ? or Esc. Focus, position and history stay put.
   ```

4. Make the other key hints match:
   - `src/man.c` (~line 871): "Press ] / [ (or Tab / Shift-Tab) to select
     links and Enter to follow them."
   - `src/info.c` (~line 1234): name `] / [` before Tab / Shift-Tab.
5. Add `tests/fixtures/guide/quick-keys.html`, a small page with three links:
   - "Search in page" → `tam:prompt:page-search`
   - "Search in guide" → `tam:prompt:guide-search`
   - "Help" → `tam:help`

   Do not link it from the fixture `index.html`, so link-order tests there
   are unaffected.
6. Tests in `tests/run.sh`, with `TAM_KNOWLEDGE_DIR=tests/fixtures/guide`:
   - `\` shows `Search in guide \`, and typing `pipe` and Enter shows
     `Results for` and `Fixture pipelines`. The existing prompt test
     (~line 1706) shows how.
   - `/` shows `Search in page /`.
   - On `show quick-keys`, following each link opens, in turn, the
     `Search in page /` prompt (Esc cancels), the `Search in guide \` prompt
     (Esc cancels) and the help overlay (`Navigation help`; Esc closes it).

**Done when:** `make test` passes; both prompts show their labels; the three
action links work; the overlay text matches item 3.

### TCK19 (Guide index, welcome page and README; corpus version; close-out)

Claim: `knowledge/index.html`, `knowledge/welcome.html`, `knowledge/version`,
`tests/guide.sh`, `README.md`, `ROADMAP.md`, `docs/plans/pager-quick-keys.md`,
`docs/plans/README.md`, `docs/plans/development.md`, `AGENTS.md`,
`AI_PROVENANCE.md` and your new `docs/ai/` record.

1. In `knowledge/index.html`, replace everything from
   `<p>Appendices and More</p>` through the closing `Press ...` paragraph with:

   ```html
   <p>Appendices and More</p>

   <ul>
   <li><a href="tam:topics">Guide topics</a></li>
   <li><a href="tam:man">Man pages</a></li>
   <li><a href="info:dir">Info manuals</a></li>
   <li><a href="tam:tldr">tldr pages</a></li>
   </ul>

   <p><a href="learn/documentation.html">Learn more</a></p>

   <p>Quick keys</p>

   <table>
   <tr><td><code>/</code></td><td><a href="tam:prompt:page-search">Search in page</a></td></tr>
   <tr><td><code>\</code></td><td><a href="tam:prompt:guide-search">Search in guide</a></td></tr>
   <tr><td><code>?</code></td><td><a href="tam:help">Help</a></td></tr>
   <tr><td><code>`</code> (backtick)</td><td>Return to this index</td></tr>
   </table>
   ```

   Open it with `TAM_KNOWLEDGE_DIR=$PWD/knowledge build/tam show --pipe` and
   read the table. If the renderer cannot lay it out, stop and ask.
2. In `tests/guide.sh` (~line 48), the index-link list expects `tam:search`,
   which the index no longer links. Replace it with `tam:prompt:guide-search`
   and add `tam:prompt:page-search` and `tam:help`.
3. In `knowledge/welcome.html` (~line 46), describe `]`/`[` (Tab/Shift-Tab
   also work), `=`, `\` and the labelled searches in the existing style.
   Follow `docs/corpus-authoring.md`.
4. In `README.md` (~line 63), make the same changes to the key paragraph and
   describe the new status row.
5. Bump the corpus patch version in `knowledge/version` (expected `0.35.1`).
   Run `make corpus`, commit any changed indexes, and run `make corpus` once
   more, as `AGENTS.md` describes.
6. Close the plan:
   - Mark this plan Completed: check each acceptance criterion and cite
     commits and the session record.
   - Update its row in `docs/plans/README.md`, `ROADMAP.md` section 3, the
     `development.md` header and Current implementation section, and the
     start-here paragraph of `AGENTS.md`.
   - Write one `docs/ai/` record from `docs/ai/TEMPLATE.md` and add its row to
     `AI_PROVENANCE.md`. The record's Key decisions section states adherence
     for each slice: followed as written, deviated (how and why), or needed
     rework (what).

**Done when:** `make test` passes; the index shows the new list and the Quick
keys table, and each link works; the plan, index, roadmap, contract and
`AGENTS.md` agree.

### Stop and ask the developer

- Before changing any key, label or behavior not described here, including
  the status-row hints.
- If a test outside the slice's files fails for reasons this change did not
  cause.
- If a slice needs a file outside its claim list, other than your own
  `docs/ai/` record.
- If the table in TCK19 does not render readably.
- If another session holds a claim on a path you need. Wait or ask; do not
  edit around it.
- Before installing, pushing, or touching `test` or `main`. `make dev` is the
  developer's loop; agents do not run it.

## Acceptance and limits

- `make test` passes after each slice and at the end.
- In a real terminal, on the Guide index:
  - `]` and `[` (and Tab and Shift-Tab) move through every link.
  - `=` shows `Path = page:index.html`.
  - `\` and `/` show their labelled prompts.
  - Each Quick keys link works.
  - The status row reads `L1 1-<n>/<total>`.
- `tam show "$(the address = showed)"` reopens the page for a guide page, a
  topic guide, a man page (quoted) and a tldr page.

Not done by this plan, and only on the developer's direction: his own
interactive use through `make dev`, a development install, and review of an
exact packaged candidate.

## Disposition

Open. On completion, record the commits, the session record and any
deviations here.
