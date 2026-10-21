# Learning levels, goal paths and self-quizzes

**Plan status:** In progress
**Reviewed:** 2026-10-21
**Scope:** Command and corpus. The first approved outcome is TCK20–TCK23:
the contract, a sticky level lens, folding and self-quizzes, and a pilot of
three pages and two goal paths. TCK24 (corpus scaffolding) and TCK25
(migration) follow under the same approval. Practice commands are deferred
(roadmap section 4). Versions are allocated from the current files when each
slice starts.
**Authority:** The developer's decisions of 2026-10-20 and 2026-10-21,
recorded as Tamlinux decision D184 (amends D174) and in the
[planning record](../ai/2026-10-21-tam-learning-levels-plan.md);
[plan maintenance procedure](../plan-maintenance.md).
**Next action:** TCK21 (sticky level lens). Claim the paths listed under it
and start from the baseline check.

## Problem and agreed outcome

The corpus has three levels of detail, but a reader cannot hold one. Keys
`0`–`3` reload the current page with `?level=`. The next link drops the
choice unless that link names a level itself. On 46 of the 70 guide pages a
level key only says "no X level on this page". Nothing on screen shows the
current level or that content was hidden. Level sections differ more in
length than in how they teach. The three level paths are the only reading
orders: none answers "I want to …". Self-checks print their hints beside the
questions, and the renderer cannot hide an answer.

The developer approved the following. Do not change it without asking him.

| Decision | Rule |
| --- | --- |
| Terms | **Level** is the reader's 0–3 lens: All, Beginner, Intermediate, Expert. A **path** is a reading order. "Depth" is retired from everything a reader sees. |
| Three levels | Keep beginner, intermediate and expert, with keys and numeric addresses `1`–`3` unchanged. Novices get an on-ramp instead of a fourth level: the First steps goal path, folded "New to this?" asides in beginner sections, and glossary links. Reopen only with learner-test evidence that novices still fail beginner sections. |
| Lens | `0`–`3` always set one lens. It stays in force across pages and persists between sessions. `TAM_LEVEL` overrides the saved value, and `--level` overrides both. A link with an explicit `?level=` sets the lens. |
| Default | All (0) until the reader chooses; the welcome page offers placement. |
| Filtering | A lens shows its own level plus untagged content. It is not cumulative. |
| Every page opens | A page without the lens level opens with a "Written for" banner and links to pages at the lens level. It never dead-ends with an error. |
| Display | Level in the status row, a level bar under the title, a one-line marker where a section is hidden, and generated level labels in the All view. Plain ASCII, never color alone. |
| Self-quizzes | `<details class="q">` folds in the pager. Enter toggles the selected answer, `r` reveals the next one, and `*` toggles all. Answers hide again when the reader leaves the page. `--pipe` collects them under "Answers" at the end of the section. Digit keys stay reserved for levels. |
| Paths | Two families share one format: the level paths (beginner, intermediate, expert) and goal paths organized by what the reader gains. Each goal path stage has a checkpoint quiz, and every goal path ends in a capstone. |
| First outcome | TCK20–TCK23, including folding and quizzes, so the pilot's answers are hidden. |

The teaching design (level definitions, personas, page anatomy, question
types, example kinds, the path format and the novice on-ramp) is the
contract in [`docs/corpus-pedagogy.md`](../corpus-pedagogy.md). Page skeletons
are in [`docs/templates/`](../templates/).

**Out of scope:** stateful progress tracking; `tam tutor`, `tam quiz` and
level-aware `tam ask` (roadmap section 4); a fourth level; keyboard layouts
other than US QWERTY; color as the only way to show anything.

## Baseline and dependencies

- Command 0.36.0 and corpus 0.37.0 on `develop` at `c9b6c5d`.
- 70 guide pages declare levels: 24 laddered, 19 beginner, 23 intermediate
  and 4 expert.
- 23 pages carry hand-typed "Beginner:", "Intermediate:" or "Expert:" heading
  prefixes.
- Depth filtering and the "Depth:" line are in `src/depth.c`
  (`depth_line()`, `tam_depth_apply_rev()`).
- The level keys are in `src/pager.c` (`switch_depth()`, the `case '0'`–`'3'`
  group in `handle_key_inner()`). The status row is built near the
  `show_address` branch of the frame painter.
- The corpus loader is `read_corpus_page()` in `src/tam.c`.
- The renderer handles `h1`–`h3`, `section`/`article`/`div` as block breaks,
  and no `details`, `summary`, `aside`, `dfn` or `cite`.
- The first-run marker already lives at
  `$XDG_STATE_HOME/tamlinux/welcome_seen` (fallback
  `~/.local/state/tamlinux/`). The lens file sits beside it.
- Interactive pager tests in `tests/run.sh` use `tests/fixtures/guide/`.
  The depth case reads the "Depth:" line and expects Back to restore a level.
  Both assertions change in TCK21.
- Free pager keys used here: `r` and `*`.

Before each slice, run `git log --oneline -3` and read `src/version.h` and
`knowledge/version`. Take the next minor or patch from the current files,
not from any number in this plan.

## Implementation and handoff

Follow `AGENTS.md` for claims, lanes, versions and provenance. Each slice
ends with `make test` (and `make corpus-and-test` when pages change), a
session record and a commit whose subject names the TCK.

### TCK20 (contract: plan, pedagogy guide, templates, terms)

Delivered with this plan:

- `docs/corpus-pedagogy.md`, and in `docs/templates/` the files
  `ladder.html`, `layered.html`, `single.html` and `path.html`.
- The Terms section of `docs/corpus-authoring.md`.
- The `CHECKLISTS.md` counter.
- The plan index, roadmap and development-contract entries.

No component version changes.

**Done when:** the files above exist and the entry points link this plan.

### TCK21 (sticky level lens; status row; level bar; banner)

Claim: `src/pager.c`, `src/depth.c`, `src/depth.h`, `src/tam.c`,
`src/version.h`, `man/tam.1`, `completions/`, `tests/run.sh`,
`tests/fixtures/guide/`, `tests/render/` if goldens change,
`docs/plans/development.md`, `ROADMAP.md`, this plan.

1. **Lens state.** Add `unsigned lens` to `pager_t`.
   - Set it at start from `--level`, then `TAM_LEVEL` (`0`–`3` or a level
     name), then the file `$XDG_STATE_HOME/tamlinux/tam_level`, else `0`.
   - Read and write the file the way `record_welcome_seen()` does: create
     the directory and replace the file atomically. A missing, unreadable
     or invalid file means `0`, silently.
   - Write the file only when the reader changes the lens. Skip the write
     when `TAM_LEVEL` is set, so the environment never overwrites the saved
     choice.
2. **Keys.** Replace the address-reload logic of `switch_depth()`:
   - `0`–`3` set the lens and re-render the current page in place. This
     does not push history and keeps the scroll position as near as the new
     layout allows.
   - On man, Info, tldr, `tam:` and standard-input pages the lens still
     changes, with the message `Level 2 Intermediate set; this page has one
     level` (wording to match the level).
   - Pressing the current lens reports `already at Level 2 Intermediate`.
3. **Loading.** When the pager loads a guide page (a corpus path or
   `page:` address) whose address has no `level=`, it appends the lens
   (`?level=` or `&level=`) when the lens is not 0.
   - Following a link that names `level=` sets the lens to that level.
   - History entries store the address without `level=`, so Back and
     Forward restore pages at the current lens.
   - `=` shows the address with the current lens.
4. **Every page opens.** In `tam_depth_apply_rev()`, stop returning NULL
   when the page lacks the requested level. Render the page and put a banner
   in place of the level line:

   ```
   Written for: Beginner. This page has no Intermediate version.
   At Intermediate: <link>, <link>
   ```

   - The links come from the index: pages in the same track that offer the
     lens level. Rank them by shared prerequisites, then by title, and show
     at most three.
   - With no match, link the level path (`beginner.html`,
     `intermediate.html`, `expert.html`).
   - `read_corpus_page()` already loads the index, so pass it what the
     banner needs.
   - `tam show --level` and `tam explain --level` on such a page now print
     the banner with exit status 0, where they used to fail. Update the
     command-line tests and the man page to match.
5. **Level bar.** Replace the "Depth:" line in `depth_line()`:
   - Laddered pages show
     `Level: All 0 | Beginner 1 | [Intermediate 2] | Expert 3   keys 0-3`.
     Offered levels are links; the current one is in brackets; a level the
     page lacks is plain text followed by ` -`.
   - Single-level pages show `Written for: Beginner`, or the banner from
     step 4 when the lens differs.
6. **Hidden-section markers.** Where a level section is skipped, emit one
   paragraph:
   `-- 1 Beginner: <section heading text> (hidden; press 1 or 0) --`.
7. **Status row.** Add the lens between the position and the actions:
   - `Level 2 Intermediate` when the row is 100 columns or wider;
   - `Lv2` from 60 to 99 columns;
   - `[2]` below 60.
   - Append ` (not on page)` when the page has no section at the current
     lens. Truncation order is otherwise unchanged.
8. **Messages.** After a change, show
   `[Level 3 Expert: 2 sections hidden]`.
9. **Other "Depth" text.** Rename the remaining reader-visible "Depth"
   strings (`src/tam.c` search and topic listings) to "Levels". Update the
   help overlay, the Guide index Quick keys table if it lists level keys,
   `man/tam.1` (the lens, `TAM_LEVEL`, the state file) and the completions.
10. **Version.** Command minor bump with the usual `tests/run.sh`
    assertion, decision-record entry and roadmap header and row.
11. **Tests** (fixtures only):
    - The lens persists across a link and across a pager restart (with a
      temporary `XDG_STATE_HOME`).
    - `TAM_LEVEL` overrides the file and is not written back.
    - A link with `?level=` sets the lens.
    - Back does not undo a lens change.
    - A single-level fixture page shows the banner.
    - Hidden-section markers appear.
    - Status row text at 40, 80 and 120 columns.
    - Man, Info and tldr pages show the lens message.
    - Rewrite the existing depth case's "Depth:" and Back-restores-level
      assertions to the new rules.

**Done when:** `make test` passes; the lens survives navigation and a
restart; no level key dead-ends on any page.

### TCK22 (folding, self-quizzes and block-level levels)

Claim: `src/render.c`, `src/render.h`, `src/depth.c`, a new
`src/fold.c`/`src/fold.h` if the transform is kept separate, `src/pager.c`,
`src/tam.c`, `Makefile` (new source), `src/version.h`, `man/tam.1`,
`tests/run.sh`, `tests/render/`, `tests/fixtures/`,
`tools/build-index.py`, `tools/lint-corpus.py`, the 23 pages with heading
prefixes, `knowledge/version`, the indexes, `docs/plans/development.md`,
`ROADMAP.md`, this plan. This slice spans both lanes.

1. **Fold transform.** Follow the depth layer's pattern: a pre-render pass
   over the page HTML, run before `tam_render_html()`.
   - Number every `<details>` on the page in order.
   - Closed: emit the `<summary>` content as a paragraph, then a link line
     `[+] Answer (Enter)` with address `tam:fold:<n>`. The label is
     `Model answer` for `details class="q open"`, `Solution` for
     `class="solution"`, `Hint` for `class="hint"` and `New to this?` for
     `aside class="first-time"`. Omit the body.
   - Open: the link reads `[-] Answer`, followed by the body.
   - **Pipe mode:** print each question in place. At the end of the
     enclosing `section` (or the page), collect the answer bodies under an
     `<h3>Answers</h3>`, numbered to match the questions.
2. **Fold state.** The pager keeps a set of open fold numbers for the
   current page only.
   - It clears on any page change, including Back and Forward, so returning
     to a page hides its answers again.
   - Pass the open set to the loader through an internal address parameter
     (for example `&fold=1.3`). History and `=` never show it.
   - Re-render in place on every toggle, keeping the scroll position.
   - `cmd_show` uses pipe mode whenever it is not paging, and only then.
3. **Keys.**
   - `tam:fold:<n>` links behave like links (selected with `]`, `[`, Tab,
     the attention column). Following one toggles the fold instead of
     navigating.
   - `r` opens the next closed fold at or below the cursor line, selects
     it, and scrolls it into view. With none left, it reports
     `all answers shown`.
   - `*` opens every fold, or closes all of them when all are open.
   - The status message reads `Quiz 2/5 revealed` when the page has
     `details.q`.
   - Add `r` and `*` to the help overlay and `man/tam.1`.
4. **Block-level levels.**
   - Apply level filtering to `data-level` on `aside`, `p`, `li`, `pre`,
     `details`, `div` and `table`, as well as `section`. An untagged block
     always shows.
   - In the All view, prefix each level `section`'s first heading with a
     generated label `[1 Beginner] `, and give its lines a gutter mark if
     the renderer supports one cheaply. If it does not, the label alone is
     enough; record which was done.
5. **Asides and risk labels.**
   - Render `aside class="note|tip|caution|tamlinux|why|peek"` as a block
     whose first line is the class word in bold (`Note:`, `Tip:`,
     `Caution:`, `On Tamlinux:`, `Why:`, `Peek under the hood:`).
   - Render `pre[data-risk]` with a label line above it: `[read-only]`,
     `[scratch directory]`, `[changes your files]`, `[changes the system]`
     or `[affects boot]`.
   - `dfn` renders in italics and `cite` as plain text; links inside them
     work as normal.
6. **Corpus.**
   - Remove the hand-typed level prefixes from the 23 pages' headings
     (`<h2>Beginner: Connecting…` becomes `<h2>Connecting…`).
   - Add lint errors for such prefixes; for `details.q` without a `summary`
     or with an empty body; and for privileged, package or bootloader
     commands (`sudo`, `pkexec`, `pacman -S`/`-R`/`-U`, `limine`,
     `mkinitcpio`, `efibootmgr`) in a `pre` without `data-risk="system"` or
     `"boot"`.
   - Keep the existing errors.
   - Corpus patch bump.
7. **Version.** Command minor bump, entries as in TCK21.
8. **Tests.**
   - A render golden for a fixture page with folds, asides and risk labels.
   - Pager cases for Enter, `r`, `*`, reset on Back, and the status
     message.
   - `--pipe` collects the answers.
   - Lint tests for the new checks.

**Done when:** `make test` and `make corpus-and-test` pass; answers are
hidden in the pager and collected under "Answers" in pipe output; no page
carries a hand-typed level prefix.

### TCK23 (pilot pages, goal-path hub, two goal paths)

Claim: `knowledge/shell/pipelines.html`,
`knowledge/filesystem/permissions.html`, `knowledge/maintain/updates.html`,
`knowledge/learn/paths/` (new), `knowledge/learn/index.html`,
`knowledge/welcome.html`, `knowledge/index.html` if its learning links
change, the indexes, `knowledge/version`, `ROADMAP.md`,
`docs/plans/development.md`, this plan. Corpus lane only.

1. **Rewrite the three pilot pages** to the full anatomy for each level in
   `docs/corpus-pedagogy.md`, starting from `docs/templates/ladder.html`.
   - Each beginner and intermediate section ends with a quiz.
   - Each beginner section has a folded "New to this?" aside.
   - Every example's output is captured by running it. Scratch-directory
     examples run in a `mktemp -d` directory. Examples that depend on the
     machine say "Your output will differ" and name the system and date
     they were captured on.
   - Replace the uncaptured `ps aux | grep firefox | wc -l` example. Its
     "3" was never run, and `grep` matches its own process.
   - Until the TCK24 glossary exists, "New words" is a short `<dl>` on the
     page.
2. **Create `learn/paths/index.html`.** It is the hub, listing paths by what
   the reader gains and then by starting level. It links the three level
   paths too.
3. **Create the two goal paths** from `docs/templates/path.html`:
   - `learn/paths/first-steps.html`: the novice on-ramp, from opening a
     terminal to returning to the Guide.
   - `learn/paths/keep-it-healthy.html`: updates, logs and rollback.
   - Each has a gain, "For you if", levels, 3–6 stages with checkpoint
     quizzes, a capstone with a folded worked solution, and next paths.
     Use only existing pages; mark a stage without one as planned.
4. **Welcome and Learning hub.**
   - The welcome page asks "What do you want to do?" (link to the paths
     hub) beside "Where are you starting?" (links to the three level paths
     until TCK24 adds placement).
   - `learn/index.html` links the hub.
5. **Close-out.** Corpus minor bump. Run `make corpus`, commit, run
   `make corpus` again and commit the index follow-up.
6. **Developer review.** The developer reads each pilot page at levels 0–3
   and walks First steps from the welcome page to its capstone. Record his
   adjustments to the anatomy or templates before TCK24 starts.

**Done when:** `make corpus-and-test` passes; each pilot section has its
anatomy and quiz; the two paths open from the welcome page; the developer
has reviewed the pilot.

### TCK24 (corpus scaffolding)

Approved direction. Write executor detail into this section before
starting, under the plan maintenance procedure.

- **Skills map** `learn/skills/`: the domains in the pedagogy guide × three
  levels of can-do cells.
- **Self-placement** `learn/placement.html`: links that set the lens.
- **Glossary** `learn/glossary/`: per-level definitions, absorbing the
  botanical terms.
- **Bibliography** `learn/bibliography/`: annotated, with stable IDs.
- **New metadata:** `<meta name="kind">` and `<meta name="skills">`. They go
  into `index.json` and new sidecar TSVs, with `index.tsv` unchanged (the
  `parent` precedent).
- **Build checks:** `dfn` → glossary, `cite data-ref` → bibliography,
  `kind` and skills IDs. Anatomy and quiz presence start as warnings.
- **Coverage and gap report:** cells and paths by level.
- **Pilot conversion:** pilot pages move to glossary links and citations.

### TCK25 (migration)

Approved direction. Plan each part before starting.

- Migrate one part at a time, threshold concepts first.
- Revise the level paths to the path format.
- Add the remaining goal paths in the pedagogy guide as their pages exist.
- When a part is done, its anatomy and quiz warnings become errors.

### Stop and ask the developer

- If a key named here turns out to be taken, or behaves differently under
  the home-row column.
- If the banner's related-page ranking gives poor suggestions on the real
  corpus.
- If a pilot page cannot meet its anatomy without inventing a fact.
- If anything appears to need a fourth level, color-only display or stored
  progress.

## Acceptance and limits

- A reader presses `2` on any page and stays at Intermediate across links,
  Back and a restart. The status row and the level bar say so. Hidden
  sections leave a visible marker.
- A single-level page opens at any lens with a truthful banner and useful
  links.
- In the pager, quiz answers stay hidden until the reader reveals them, and
  hide again on return. Pipe output prints them after the questions.
- The pilot pages and two goal paths follow the pedagogy contract. Their
  examples show captured output.
- Learner test (developer-paced, after TCK23): one novice on First steps
  and one experienced reader on the pilot pages, thinking aloud. The result
  settles the three-level reversal condition.
- Package Test, Run and Release remain developer-directed. Completing source
  work here accepts no packaged candidate.

## Disposition

TCK20 delivered on 2026-10-21 with this plan. TCK21–TCK25 open.
