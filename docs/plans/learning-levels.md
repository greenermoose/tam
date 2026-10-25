# Learning levels, goal paths and self-quizzes

**Plan status:** In progress
**Reviewed:** 2026-10-22
**Scope:** Command and corpus. The first approved outcome is TCK20–TCK23:
the contract, a sticky level lens, folding and self-quizzes, and a pilot of
three pages and two goal paths. TCK24 (corpus scaffolding) and TCK25
(migration) follow under the same approval. Practice commands are deferred
(roadmap section 4). Versions are allocated from the current files when each
slice starts.
**Authority:** The developer's decisions of 2026-10-20 and 2026-10-21,
recorded as Tamlinux decisions D184 (amends D174) and D185 (state
directory), and in the
[planning record](../ai/2026-10-21-tam-learning-levels-plan.md);
[plan maintenance procedure](../plan-maintenance.md).
**Next action:** Continue TCK25 Improving track migration with the remaining lessons (zram, toolchains, ai-performance, patches, package-versions, virtualization, tam.html), then enable Improving track build enforcement. Pilot and learner review remain unverified.

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

- Command 0.36.0 and corpus 0.37.0 on `develop` at `c9b6c5d` (before TCK21).
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
- `tam` keeps its state in its own subdirectory of the Tamlinux state
  directory, `$XDG_STATE_HOME/tamlinux/tam/` (fallback
  `~/.local/state/tamlinux/tam/`), as decided on 2026-10-21 (Tamlinux
  decision D185). The first-run marker is still written to
  `tamlinux/welcome_seen` at the top level, and nothing reads it; TCK21
  moves it.
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
`docs/plans/development.md`, `ROADMAP.md`, this plan; and in the `tamlinux`
repository, `docs/plans/tamlinux-command.md`.

1. **State directory.** Add one helper in `src/tam.c` that returns
   `$XDG_STATE_HOME/tamlinux/tam`, or `~/.local/state/tamlinux/tam` when
   `XDG_STATE_HOME` is unset or empty, and creates it on write.
   - `record_welcome_seen()` writes `tamlinux/tam/welcome_seen` through it.
   - Do not read, migrate or delete the old top-level
     `tamlinux/welcome_seen`. Nothing has ever read it, so a stale copy is
     harmless.
   - Update the welcome and state-isolation cases in `tests/run.sh` to the
     new path, and the `welcome_seen` sentence in the `tamlinux`
     repository's `docs/plans/tamlinux-command.md`.
2. **Lens state.** Add `unsigned lens` to `pager_t`.
   - Set it at start from `--level`, then `TAM_LEVEL` (`0`–`3` or a level
     name), then the file `tamlinux/tam/level` in the state directory, else
     `0`.
   - Write the file through the step 1 helper, replacing it atomically. A
     missing, unreadable or invalid file means `0`, silently.
   - Write the file only when the reader changes the lens. Skip the write
     when `TAM_LEVEL` is set, so the environment never overwrites the saved
     choice.
3. **Keys.** Replace the address-reload logic of `switch_depth()`:
   - `0`–`3` set the lens and re-render the current page in place. This
     does not push history and keeps the scroll position as near as the new
     layout allows.
   - On man, Info, tldr, `tam:` and standard-input pages the lens still
     changes, with the message `Level 2 Intermediate set; this page has one
     level` (wording to match the level).
   - Pressing the current lens reports `already at Level 2 Intermediate`.
4. **Loading.** When the pager loads a guide page (a corpus path or
   `page:` address) whose address has no `level=`, it appends the lens
   (`?level=` or `&level=`) when the lens is not 0.
   - Following a link that names `level=` sets the lens to that level.
   - History entries store the address without `level=`, so Back and
     Forward restore pages at the current lens.
   - `=` shows the address with the current lens.
5. **Every page opens.** In `tam_depth_apply_rev()`, stop returning NULL
   when the page lacks the requested level. Render the page and put a banner
   in place of the level line:

   ```
   Written for: Beginner. This page has no Intermediate version.
   At Intermediate: <link>, <link>
   ```

   - The links come from the index's parent links: the page's children,
     then its siblings, then its parent, each offering the lens level and
     ranked by shared prerequisites, then by title; show at most three.
     (Revised in command 0.37.1; the first rule used the same track.)
   - With no such relative, link only the level path (`beginner.html`,
     `intermediate.html`, `expert.html`).
   - `read_corpus_page()` already loads the index, so pass it what the
     banner needs.
   - `tam show --level` and `tam explain --level` on such a page now print
     the banner with exit status 0, where they used to fail. Update the
     command-line tests and the man page to match.
6. **Level bar.** Replace the "Depth:" line in `depth_line()`:
   - Laddered pages show
     `Level: All 0 | Beginner 1 | [Intermediate 2] | Expert 3   keys 0-3`.
     Offered levels are links; the current one is in brackets; a level the
     page lacks is plain text followed by ` -`.
   - Single-level pages show `Written for: Beginner`, or the banner from
     step 5 when the lens differs.
7. **Hidden-section markers.** Where a level section is skipped, emit one
   paragraph:
   `-- 1 Beginner: <section heading text> (hidden; press 1 or 0) --`.
8. **Status row.** Add the lens between the position and the actions:
   - `Level 2 Intermediate` when the row is 100 columns or wider;
   - `Lv2` from 60 to 99 columns;
   - `[2]` below 60.
   - Append ` (not on page)` when the page has no section at the current
     lens. Truncation order is otherwise unchanged.
9. **Messages.** After a change, show
   `[Level 3 Expert: 2 sections hidden]`.
10. **Other "Depth" text.** Rename the remaining reader-visible "Depth"
   strings (`src/tam.c` search and topic listings) to "Levels". Update the
   help overlay, the Guide index Quick keys table if it lists level keys,
   `man/tam.1` (the lens, `TAM_LEVEL`, the state file) and the completions.
11. **Version.** Command minor bump with the usual `tests/run.sh`
    assertion, decision-record entry and roadmap header and row.
12. **Tests** (fixtures only):
    - `tam welcome` writes `tamlinux/tam/welcome_seen`, and the
      state-isolation case still finds nothing written by other commands.
    - The lens persists across a link and across a pager restart (with a
      temporary `XDG_STATE_HOME`), stored in `tamlinux/tam/level`.
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
restart; no level key dead-ends on any page; all of `tam`'s state is under
`tamlinux/tam/`.

**Delivered (2026-10-21, command 0.37.0).** Choices made where this section
was silent, for the developer's review:

- The generated topic views (`tam:explain`, `tam:topics`, `tam:keywords`)
  take the lens like guide pages, because they already filtered by level.
  A topic with no page at the lens lists every level under a banner.
- Piped output ignores the saved lens and `TAM_LEVEL`; it shows the level
  that `--level` or the address names, else all levels.
- A laddered page without the lens level is shown whole: its level bar
  marks the lens as `[Expert 3] -`, followed by the banner.
- Every guide page declares levels, hubs included, so the Guide index and
  section hubs show `Written for: Beginner` and, at another lens, the
  banner.
- The level bar's three-space gap before `keys 0-3` renders as four after
  a linked level.
- The Guide index Quick keys table lists no level keys, so the corpus is
  unchanged.

**Resolved (2026-10-21, command 0.37.1).** The banner's first rule (same
track, shared prerequisites, title) gave poor picks on the real corpus: the
welcome page at Intermediate suggested "AI Tooling and Human Agency" and
"Base Distributions". The developer accepted the recommendation to rank by
the index's `parent` links (children, then siblings, then the parent, each
offering the lens level, then shared prerequisites and title) and to link
only the level path when the page has no such relative. The welcome page at
Intermediate now links its sibling "Intermediate path".

The `welcome_seen` sentence in the `tamlinux` repository's
`docs/plans/tamlinux-command.md` now names `tamlinux/tam/welcome_seen`.

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

**Delivered (2026-10-21, command 0.38.0 / corpus 0.37.1).** Fold numbering
runs before level filtering; the fold transform then receives only the visible
blocks. This keeps question identities stable through lens changes and keeps
filtered answers out of pipe output. The pager owns its open set; loader-only
`fold=` parameters never reach history or address display. Standard-input
HTML supports the same folds. All-view level identification uses generated
heading labels alone, without another gutter. The 23 prefixed pages now omit
those prefixes; existing privileged examples have risk labels. Implementation
and verification: [TCK22 record](../ai/2026-10-21-tam-folding.md).

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

**Source delivered (2026-10-21, corpus 0.38.0; developer review pending).**
The three pilot pages now use worked examples captured in temporary
directories or read-only native package queries, novice asides, faded tasks,
troubleshooting, vocabulary and folded quizzes. The paths hub and First steps
and Keep it healthy paths are linked from Welcome and the Learning hub.
Keep it healthy prepares an update and recovery decision without performing
a transaction or deployment. The source baseline was command 0.38.0 / corpus
0.37.1 at `56a2866`; command version is unchanged. Verification and review
findings: [pilot record](../ai/2026-10-21-tam-learning-pilot.md).

TCK23 remains open for its developer review criterion. The developer's
subsequent "Continue" directs TCK24 work without asserting that the pilot
review occurred. Learner tests remain developer-paced and separate from
automated source checks; record later anatomy/template adjustments explicitly.

### TCK24 (corpus scaffolding)

Approved direction. Write executor detail into this section before
starting, under the plan maintenance procedure.

**Execution contract (2026-10-21 continuation).** Baseline command 0.38.0 /
corpus 0.38.0 at `8859559`. Corpus minor 0.39.0; no command version change.
Claim the new learning structures, three pilot pages, Learning/Welcome hubs,
terminology page, corpus authoring/templates, index builder and tests, coverage
report tool, generated outputs, Makefile/dev output lists and status/provenance.

1. Define 36 stable skill IDs (`domain.level`) in
   `learn/skills/skills.json`: the 12 domains below, each at all three levels,
   with a can-do description. Publish the same cells in the skills-map page.
   IDs identify coverage, not learner scores or stored progress.
2. Add self-placement questions with folded explanations and explicit
   `?level=` links. Add per-term laddered glossary pages, including the six
   botanical terms; retain the terminology address as an orientation link.
   Keep conceptual/planned architecture distinct from built behavior.
3. Add annotated bibliography entries with stable IDs, level, online/offline
   availability, cost and license. Offline man/Info references remain primary;
   web entries are labeled online. Pilot citations use `<cite data-ref="id">`
   with a bibliography link, and `<dfn>` contains a glossary link.
   Each glossary term and bibliography entry uses a complete page address:
   current readers cannot load cross-page fragment addresses. This was found
   and corrected during real link verification, without changing the command.
4. Accept optional `kind` metadata with values tutorial, how-to, reference,
   explanation, path, hub, glossary and bibliography. `skills` is a list of
   registered IDs. Preserve missing metadata on legacy pages; invalid supplied
   values/IDs are errors. Add JSON fields and `index.kinds.tsv` and
   `index.skills.tsv`; preserve the ten-column `index.tsv` exactly.
5. Validate glossary links inside `dfn` and bibliography IDs in `cite`,
   including target existence. Emit nonfatal anatomy/quiz warnings for authored
   teaching sections, with glossary/reference/hub/path jobs handled separately.
6. Build `coverage.json` with page and path coverage for every skill cell and
   explicit gaps. A repository tool prints the report; no runtime command or
   learner-state store is added. Add focused fixture tests for dangling
   references, metadata compatibility, warnings and truthful coverage.
7. Convert the pilot's inline vocabulary and references, update metadata and
   hubs/templates, run the full suites, and commit page/index history separately.
   Pilot/learner review and package Test/Run/Release remain unverified.

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

**Source implemented (2026-10-21, corpus 0.39.0).** The execution contract
above is implemented: 36 skill cells, placement, 12 glossary terms, seven
bibliography entries, optional metadata and additive projections, teaching
reference validation, nonfatal anatomy warnings and a coverage report. The
terminology topic retains its address and links the absorbed definitions;
unsupported claims of a delivered signed nut are removed. Pilot metadata,
vocabulary and citations are converted. The TCK24 baseline had 24 cells without
mapped pages and 30 without paths; 40 legacy level sections warn about
anatomy. These are migration gaps, not new breakages or accepted teaching.
Verification: [session record](../ai/2026-10-21-tam-learning-pilot.md).

### TCK25 (migration)

Approved direction. Plan each part before starting.

**First part: Using (2026-10-21).** The developer requested continuation;
the agent offered Using versus Maintaining as migration priorities and uses
Using as the recommended default while no different priority is supplied.
Baseline `91b46d0`, command 0.38.0 / corpus 0.39.0. Allocate corpus 0.40.0
because this slice adds the Command-line fluency goal path. Command is unchanged.

Claim `shell/jobs.html`, `desktop/index.html`, `desktop/workspaces.html`,
`use/index.html`, the three level paths, the new goal path, skills registry,
builder/tests and generated outputs, status/provenance. Existing pilots are
already migrated. Work in this order:

1. Rewrite jobs with the full ladder anatomy and captured examples that
   start, wait for and signal only their own short-lived child process.
   Remove name-wide `pkill` exercises and unconditional promises that
   disown/nohup preserve a workload after terminal closure.
2. Rewrite desktop orientation (Beginner) and workspaces (Intermediate)
   as reversible practice with empty windows, sourced from the product's
   current Hyprland binding files. Supply folded novice help/checkpoints,
   choices, trouble checks and undo steps. Do not claim GUI acceptance.
3. Classify/map the Using hub and all six logical Using pages. Add
   `enforce_tracks: ["use"]` to the skill registry; validate the policy
   and promote anatomy/quiz warnings in that completed part to build errors.
   Other parts keep warning-only behavior.
4. Add the Command-line fluency goal path from the migrated threshold
   pages, with checkpoints and a captured scratch-directory capstone.
   Rewrite the three existing level paths to the agreed path format.
   A path's skills may span visited levels while its level metadata says
   where it starts; ordinary pages may map only levels they offer.
5. Run meaningful enforcement/compatibility tests, the full source/corpus
   suites, capture checks and pager navigation. Update all entry points and
   commit page/index histories separately. Report remaining parts and
   learner/packaged acceptance honestly.

- Migrate one part at a time, threshold concepts first.
- Revise the level paths to the path format.
- Add the remaining goal paths in the pedagogy guide as their pages exist.
- When a part is done, its anatomy and quiz warnings become errors.

**Using source implemented (2026-10-21, corpus 0.40.0).** All six logical
Using pages are classified; instructional pages map skill cells. Its
anatomy/quiz gaps now fail the build through validated `enforce_tracks`.
The jobs ladder uses captured owned-child examples. Desktop/workspace tasks
are sourced, reversible human practice, without a GUI acceptance claim.
Command-line fluency is added; all three level paths follow the stage,
checkpoint and capstone format. The current report has 19 page gaps, 21
path gaps and 38 warning sections in other parts. Maintaining, Learning,
Installing and Improving migration remain. Verification and limits:
[session record](../ai/2026-10-21-tam-learning-pilot.md).

**Maintaining recovery execution plan (2026-10-21).** The developer
asked to inspect the last session and continue the next chunk. Baseline
`476b499`, command 0.38.0 / corpus 0.40.0. Continue the recorded recovery
priority without changing the approved teaching design. Allocate corpus
0.41.0 for three new glossary pages; command stays unchanged.

1. Migrate `maintain/rollback.html` and `maintain/recovery.html` to the full
   ladder anatomy, with skill mappings and folded quizzes. Replace invented
   output, direct generation activation, assumed disk layouts and broad
   process termination with scratch practice or read-only inspection.
2. Teach recovery boundaries: retained recorded Tamlinux assembly/generation,
   separately mutable state, native boot state and external backups. Describe
   SysRq enablement and limitations without invoking it, and rescue planning
   without mounting, rebooting or repairing this workstation.
3. Add Snapshot, Generation and Virtual console glossary definitions. Update
   Keep it healthy to use the migrated Intermediate recovery lesson without
   changing the reader's lens to Beginner.
4. Capture and rerun displayed sessions, inspect all levels and folded answers,
   run source/corpus checks under an exclusive claim, and update status and
   provenance. Rebuild indexes after the page commit in a separate commit.

This is a partial Maintaining migration: its other pages, the Rescue goal
path and part-wide enforcement wait for subsequent slices. Learner review
and packaged acceptance remain unverified.

**Recovery slice source implemented (2026-10-21, corpus 0.41.0).** Both
ladders now carry skill mappings, complete anatomy and folded quizzes.
Captured exercises use scratch copies, terminal identity, mount queries and
SysRq policy reads; no recovery operation is run. Three glossary pages are
added and Keep it healthy stays at Intermediate for recovery planning.
Anatomy warnings fall from 38 to 34; page gaps from 19 to 15, with 21 path
gaps unchanged. These counts measure metadata, not learning effectiveness.
At that slice's completion, Maintaining still needed boot, boot-fallback,
Snapper, logs, power, resources, packaging and hub migration and its Rescue
path before part-wide enforcement. See the
[verification record](../ai/2026-10-21-tam-recovery-learning.md).

**Boot and Rescue execution plan (2026-10-21).** The developer explicitly
requested boot, boot-fallback and Snapper lessons, then Rescue. Baseline
`124ee80`, command 0.38.0 / corpus 0.41.0. The existing fallback page is
`maintain/boot-recovery.html`; preserve its address and topic. Allocate corpus
0.42.0 for the new Rescue goal path; command stays unchanged.

1. Migrate the boot chain, fallback dependencies and Snapper pages as ladders
   under the existing teaching contract. Capture only read-only host queries
   and scratch models. Separate evidence from inference; do not regenerate
   images, alter firmware, mount storage, create/delete snapshots or reboot.
2. Correct Snapper option/type examples against its native manual and help.
   Distinguish snapshot creation, menu synchronization, snapshot boot and
   persistent restoration; none proves the next. Keep current Arch provider
   details scoped to that provider, and qualify UKI update guarantees.
3. Add Rescue at Intermediate through Expert with staged links, checkpoints
   and a scratch recovery-decision capstone. Link it from goal/Learning hubs
   and Keep it healthy. Completion prepares a bounded recovery procedure;
   real machine repair and its acceptance remain separate.
4. Check examples against captured stdout, render every level, exercise the
   Rescue navigation/folds and run corpus checks under an exclusive claim.
   Update status/provenance and rebuild Git-derived indexes after committing.
   Remaining Maintaining pages and enforcement stay pending.

**Boot and Rescue source implemented (2026-10-21, corpus 0.42.0).** The
three ladders now declare skills and complete teaching anatomy, with folded
quizzes and captured read-only/scratch examples. Snapper descriptions, types,
comparison endpoints and global/creation flags are distinguished accurately.
Rescue links the recovery lessons through four stages, Intermediate to Expert,
with a scratch dependency-filter capstone and explicit later-write checks.
No image generation, firmware change, snapshot creation/deletion, mounting or
repair was performed. Anatomy warnings fall from 34 to 28; page gaps remain
15 and path gaps fall from 21 to 19. These are metadata counts. Remaining
Maintaining pages (logs, power, resources, packaging and hub), enforcement,
pilot/learner and packaged acceptance remain open. See the
[verification record](../ai/2026-10-21-tam-boot-rescue-learning.md).

**Logs/power slice (2026-10-22, corpus 0.42.3).** The developer said
"Proceed" to the proposed two-page slice. Baseline command 0.43.1 / corpus
0.42.2 at `235c42e`; this corrects and migrates existing pages, so it takes a
corpus patch version. Command behavior is unchanged.

The slice uses captured read-only journal/sysfs examples, full teaching
anatomy, folded quizzes and skill mappings. Logs teach bounded selection and
competing explanations; power separates blanking, sleep capability, mode
selection and observed completion. No sleep cycle, sysfs write, swap change,
journal cleanup or session installation is part of the exercises. Metadata
coverage is not learner validation. Resources, packaging and the hub remain;
Maintaining enforcement stays off. Verification and limitations:
[session record](../ai/2026-10-22-tam-logs-power-learning.md).

**Maintaining completion slice (2026-10-25, corpus 0.43.2).** The developer
directed: "Finish the remaining Maintaining pages under TCK25." Baseline command
0.43.2 / corpus 0.43.1 at `71bb7b6`. This slice migrates `hardware/resources.html`,
`ecosystem/packaging.html`, and `maintain/index.html`, and enables part-wide
build enforcement for the Maintaining track in `skills.json`. It takes a corpus
patch bump (0.43.2); command version is unchanged.

1. `hardware/resources.html` is migrated to the full ladder anatomy with captured
   read-only `free -h`, `df -h /`, `nproc`, and `uptime` examples, memory vs buffer
   distinctions, PSI metrics, and folded self-quizzes across Beginner, Intermediate,
   and Expert levels. Mapped to `hardware.beginner`, `hardware.intermediate`,
   `hardware.expert`.
2. `ecosystem/packaging.html` is migrated to the full ladder anatomy with captured
   read-only `pacman -Q bash`, `pacman -Qo /usr/bin/git`, and Home Manager generation
   inspection, distinguishing base OS packages from declarative workstation assemblies
   and user runtimes. Mapped to `software.beginner`, `software.intermediate`,
   `software.expert`.
3. `maintain/index.html` is classified as `kind="hub"` with links to all 11 Maintain
   pages and the Keep it healthy and Rescue goal paths.
4. `learn/skills/skills.json` enables `"enforce_tracks": ["use", "maintain"]`,
   promoting anatomy and quiz warnings across both Using and Maintaining to fatal
   build errors. Anatomy warnings drop from 24 to 22 (remaining warnings are in
   unmigrated parts). Maintaining has zero warnings and zero errors.

See the [verification record](../ai/2026-10-25-tam-tck25-maintaining-completion.md).

**Improving track slice 1 (2026-10-25, corpus 0.43.3).** The developer
directed: "Check the last agy session to see what we were doing in the tam repo. Then continue that work. We are completing checklists."
Baseline command 0.43.2 / corpus 0.43.2 at `2561305`. This slice begins the
Improving track migration under TCK25, migrating `improve/index.html` (hub),
`improve/scripting.html`, and `ecosystem/lua.html`, and resolving gaps for the
`customizing` domain skills. It takes a corpus patch bump (0.43.3); command
version is unchanged.

1. `improve/index.html` is classified as `kind="hub"` with links to all 11 Improving
   guides and goal paths.
2. `improve/scripting.html` is migrated to the full ladder anatomy with captured
   read-only Bash examples, parameter expansion defaults, `set -u` unbound error
   aborts, `set -o pipefail` pipeline guarantees, and `trap ... EXIT` cleanup handlers.
   Mapped to `customizing.beginner`, `customizing.intermediate`, and `customizing.expert`.
3. `ecosystem/lua.html` is migrated to the full ladder anatomy with captured
   read-only `lua -e` examples, table indexing rules, `luac -p` syntax verification,
   `hyprctl reload` compositor workflows, and host C API stack semantics. Mapped to
   `customizing.beginner`, `customizing.intermediate`, and `customizing.expert`.
4. Anatomy warnings drop from 22 to 18 across the corpus. Gaps for `customizing.beginner`,
   `customizing.intermediate`, and `customizing.expert` are resolved.

See the [verification record](../ai/2026-10-25-tam-tck25-improving-slice1.md).

**Improving track slice 2 (2026-10-25, corpus 0.43.4).** The developer directed:
"In the tam repo, we are completing checklist item TCK25. The Using and Maintaining tracks are fully migrated and enforced, and the Improving track hub, scripting, and lua lessons landed in corpus 0.43.3. Continue TCK25 Improving track migration with the next slice: improve/terminal.html and improve/efficiency.html. Follow tam/AGENTS.md and claim scopes before editing."
Baseline command 0.43.2 / corpus 0.43.3 at `85f6766`. This slice migrates
`improve/terminal.html` and `improve/efficiency.html` to full ladder anatomy with
captured read-only examples, mapped skills, and folded quizzes. It resolves the
page gap for `desktop.expert`. It takes a corpus patch bump (0.43.4); command
version is unchanged.

1. `improve/terminal.html` is migrated to the full ladder anatomy with captured
   read-only Micro scratch editing examples, live disk reload configuration
   (`reload auto`), Wayland client/compositor boundaries (`wl_shm`, `wl_surface.damage_buffer`,
   `WAYLAND_DEBUG=1`), sub-10ms shell startup latency benchmarking (`time bash --norc -i -c exit`),
   and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to
   `customizing.beginner`, `customizing.intermediate`, and `desktop.expert`.
2. `improve/efficiency.html` is migrated to the full ladder anatomy with captured
   read-only `vmstat 1 3` processor wakeup and context switch measurements,
   event-driven kernel filesystem monitoring with `inotifywait -e modify`,
   $O(1)$ `epoll(7)` multiplexing, Wayland socket event dispatching, and folded
   self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to
   `hardware.beginner`, `hardware.intermediate`, and `hardware.expert`.
3. Whole-corpus anatomy warnings drop from 18 to 14 (all 14 remaining warnings
   are in unmigrated lessons). Unclassified pages drop from 45 to 43; unmapped
   pages drop from 87 to 83. The page gap for `desktop.expert` is resolved.

See the [verification record](../ai/2026-10-25-tam-tck25-improving-slice2.md).

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

TCK20 delivered on 2026-10-21 with this plan. TCK21 delivered on
2026-10-21 as command 0.37.0, with the banner ranking revised in 0.37.1;
see its delivery note. TCK22 delivered as command 0.38.0 / corpus 0.37.1;
see its delivery note and verification record. TCK23 source is delivered as
corpus 0.38.0, with developer review pending. TCK24 source is implemented as
corpus 0.39.0 on the developer's "Continue". TCK25 Using migration and level
paths are implemented as corpus 0.40.0. Corpus 0.41.0 implements the first
two Maintaining recovery ladders. Corpus 0.42.0 adds the boot, boot-fallback
and Snapper migrations and Rescue path. Corpus 0.42.3 migrates logs and power. Corpus 0.43.2 completes Maintaining with resources, packaging, and the Maintaining hub, and enables track-wide build enforcement for Maintaining. Corpus 0.43.3 begins Improving track migration with the hub, scripting, and lua lessons. Corpus 0.43.4 continues Improving track migration with terminal and efficiency lessons, resolving the desktop.expert page gap. Remaining Improving lessons, Learning, and Installing migration remain open.
