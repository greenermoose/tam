# Writing the tam knowledge corpus

Read this before you add or change a page under `knowledge/`. It supplements
`AGENTS.md`, which covers claims, lanes, versions and provenance.

The subject library is defined by [the current organization plan](plans/subject-library.md).
Canonical subject tracks are `hardware`, `systems`, `terminal`, `networking`,
`security`, `programming`, `desktop`, and `tamlinux`. Shared tracks are `paths`,
`glossary`, `bibliography`, `skills`, and `help`. The root has an empty track.
Every other page declares its canonical `parent`; folder hubs form the parent
chain back to the root. Breadcrumbs match that chain. Older corpora retain
their historical tracks and directory fallback when read by the command.

The index column layout and optional parent, kind, skill, and path sidecars
remain compatible. New folders need a hub. Choose a subject home by what the
page principally teaches, then cross-link other subjects. Do not duplicate a
lesson to place it in a second section. Keep task and learning-path navigation
separate from canonical ownership.

Use Before this for actual prerequisites, Next for recommended continuation,
Related for other useful branches, and Sources and further reading for evidence
and deeper study. Keep prerequisite metadata and visible links in agreement.
Keywords include ordinary phrasing, acronyms, and synonyms; they power Search,
not a separate catalog. Glossary entries live in `glossary/<initial>/`, or in
`glossary/numeric/` when the title begins with a digit, and link back to
teaching pages. Bibliography entries retain stable citation IDs.

The corpus is public product documentation. Readers trust it to describe the
Tamlinux they are running, so the rules below are about one thing: every
statement about Tamlinux must be true of Tamlinux, and a reviewer must be able
to confirm it quickly.

## The sourcing rule

- **A statement about Tamlinux traces to a source:** a file in a product
  repository, the real output of a command, or a recorded decision in a
  public plan. This covers what Tamlinux ships, binds, runs, names, chose or
  rejected, and why.
- **General Linux knowledge needs no source,** such as what a TTY or a display
  manager is. Do not present it as a Tamlinux choice.
- **Anything not built yet is marked planned.** If a command's own help lists
  a verb under "Planned", the page says planned too. The same applies to the
  target base system and a future compositor.
- **Do not write history that no source records.** "Tamlinux evaluated and
  rejected X" needs a decision that says so.
- **When sources disagree, do not choose.** Write what is certain, and report
  the disagreement to the developer in your session record.
- **List what you checked** in the session record's `Checked against:` line,
  for example `bindings/tiling.lua, src/install.c, tam --help`. Sources do not
  appear on the page.

## Where facts live

| Claim | Authority | Not |
| --- | --- | --- |
| `tam` subcommands, options, aliases | `tam --help` from the dev build; `src/tam.c` | older pages, the README |
| Planned or stub `tam` verbs, `check` IDs | `src/install.c` and its help text | |
| Command and corpus versions | `src/version.h` (`TAM_COMMAND_VERSION`); `knowledge/version` | remembered numbers |
| Make targets, compiler flags | `Makefile` | |
| Example output | run the command and paste what it prints | output written from memory |
| Keybindings | `tamlinux` repository: `desktop/session/hypr/tamlinux/bindings/*.lua`; readers can open `tam-menu-keybindings` | the live desktop (personal overrides), Omarchy or Hyprland defaults |
| `tam-*` commands and their options | `tamlinux` repository: `commands/<name>`; standalone tools in `tamlinux-tools` | |
| Bootloader, initramfs and rescue-media behavior | The UEFI specification and the relevant bootloader, image-generator or live-media manual; use the product repository for Tamlinux-specific configuration | menu labels, another distribution's defaults |
| Login, session entry, SDDM | `tamlinux` repository: `desktop/native/integration/`, expected names in `desktop/tests/`; `tamlinux-packages` repository: `docs/sddm-cutover.md`, `docs/session-runtime.md` | |
| Tamlinux versions, candidate letters, stages | `tamlinux` repository: `VERSIONING.md`, `VERSION` | |
| Packaging, pins, deployment | `tamlinux-packages` repository: `flake.nix`, `flake.lock`, `nix/`, `docs/deployment.md` | |
| Installed package versions | `pacman -Q <package>`; say which system you checked | |
| Base-system and compositor direction | `tamlinux` repository: `plans/P3-base-operating-system.md` and `plans/P30-theme-system.md`; Hyprland is current, `tam-pane` is the planned own compositor, and Sway/wlroots are not planned | |
| Which other operating systems Tamlinux watches, their roles, and Tam 2.0 microkernel research | `tamlinux` repository: `docs/plans/operating-system-horizons.md`; always written as research or planned | a project's own claims about Tamlinux, remembered roles |
| Third-party tool flags | the tool's `--help` or manual page; if the tool is not installed, say it is optional and name its package | |
| How agents work on Tamlinux | the repository's `AGENTS.md`; `tam-work --help` | |

If a claim you need is not in this table, find its source the same way and
add a row here in the same change.

## The public boundary

Never put these in a page:

- home-directory paths or real account names; use `alex` and `/home/alex`;
- names of private repositories, or "the user's own configuration repository"
  described in a way that identifies it;
- conversation or session identifiers;
- internal decision or principle identifiers of the form `Dnnn` or `Vnn`; state
  the reasoning in words instead.

Naming the developer:

- His only title is **developer**. Never invent another, such as creator,
  maintainer or author.
- At the first mention on a page, give his full name with that title, for
  example "Fred Horch is the Tamlinux developer" or "Fred Horch develops
  Tamlinux for his personal and business work". Call him "Fred" afterwards.
- Where a page describes the release process, use the title as a role: "on
  the developer's Test instruction".
- Example accounts are never his: write `alex`.

## Terms

- A **level** is Novice (1), Learner (2) or Expert (3); All (0) shows
  every level. The reader's chosen level is their lens. Do not call levels
  "depths" or "routes", and do not use the superseded terms "beginner" or
  "intermediate". Every content page must provide `novice`, `learner`, and
  `expert` sections so a reader can progress from zero prior technical
  knowledge to expert mastery.
- A **path** is a topic-centered reading order defined by a hub page in
  `paths/<id>.html` (`kind="path"`) and `<meta name="paths" content="<id>:<step>, ...">`
  on its step pages. The pager builds path navigation dynamically from `index.paths.tsv`
  so a single page can belong to several paths. Boot pages keep "fallback route"
  and "rescue route" for their own, separate meaning.
- The desktop modifier is **Meta** (chords `Meta + K`; Hyprland tokens
  `META + …`). Do not write `SUPER + …` or “the SUPER key” in living pages.
  Historical Super/SUPER belongs only on `desktop/navigation/meta-key.html` and
  `glossary/m/meta.html` (D217). `lint-corpus.py` rule `SUPER-MOD`
  enforces this.

## How a page teaches

Every page starts after its navigation lines with one or two plain,
unformatted sentences introducing the page to a reader without specialized
technical knowledge. Do not add a heading that repeats the breadcrumb title;
the pager already shows it. Define or link unfamiliar terms. See
`corpus-pedagogy.md` for the complete opening rule.

Before you write or restructure a page, read
[`corpus-style.md`](corpus-style.md) (how it reads; example policy) and
[`corpus-pedagogy.md`](corpus-pedagogy.md) (level definitions, personas, the
novice on-ramp, page patterns, section anatomy, quizzes and paths). Start a
new page from the matching file in [`templates/`](templates/). The
authoritative plan is
[learning levels, goal paths and self-quizzes](plans/learning-levels.md).

### Learning metadata and references

Pages may declare `<meta name="kind">` as `tutorial`, `how-to`, `reference`,
`explanation`, `path`, `hub`, `glossary` or `bibliography`. Missing metadata
remains valid during migration. Supplied kinds must be valid. Declare skills
as comma-separated IDs in `<meta name="skills">`, for example
`shell.novice, shell.learner`. IDs come from
`knowledge/skills/skills.json`; a skill's level must be offered by the page.
Topic paths may name skills at the levels they visit; their level metadata
states the starting level. Declare path membership on step pages with
`<meta name="paths" content="<id>:<step>, ...">`, where `<id>` is the `topic`
of a `paths/*.html` page (`kind="path"`) and `<step>` is a unique 1-based
step number within that path. Do not write static `<nav class="learning-path">`
blocks; the pager synthesizes path navigation dynamically and preserves the
reader's active path across multi-path pages. The skills map publishes the same
can-do statements for readers.

A path page also declares `<meta name="path-category">` and
`<meta name="path-order">`. The category is one of `start-here`,
`understand-computer`, `keep-running`, `connect`, `stay-safe`, `write-software`
or `improve-work`; the order is a positive integer unique within that category.
Keep the page's opening **Outcome**, **Prerequisites** and **Time** paragraphs
in that order: the builder projects all three into `index.paths.tsv` after the
`skills` column, turning links to other path pages into `tam:path` links. The
reader uses these fields for the path menu, summary and detail view. When
adding a path, choose the category by the outcome a reader wants, not by the
track of its first step.

Every page declares `<meta name="topic">` and `<meta name="summary">`. The
topic is the page's permanent stable ID: it does not change when the page
moves or its title changes, and retiring an ID needs an explicit alias
decision. The summary is the canonical one-line description, written by the
author rather than copied from the opening paragraph. It is plain text on one
line, at most 160 characters, and the build rejects missing, empty, duplicate,
multi-line or overlong values. `tam show <topic>` resolves that stable ID
before guide-path suffixes.

Topic IDs are slugs: lowercase letters, digits, and hyphens only
(`^[a-z0-9][a-z0-9-]*$`). When a topic name contains `/`, replace each `/`
with `-` in the `<meta name="topic">` value and filename, collapse repeated
hyphens, and keep the `/` in visible titles and text. For example, X/Open uses
topic `glossary-x-open` and filename `knowledge/glossary/x/x-open.html`.
Apply the same slug treatment to any other character that cannot appear in a
topic slug.

Mark a term's first use with a glossary link, optionally inside `<dfn>`.
Every `<dfn>` must contain a working link under `glossary/`; definitions
themselves can use ordinary headings and paragraphs. A citation uses a stable
bibliography ID and its exact entry link:

```html
<dfn><a href="glossary/s/stream.html">stream</a></dfn>
<cite data-ref="bash"><a href="bibliography/bash.html">Bash manual</a></cite>
```

Bibliography entries use `<section class="reference" id="bash">` and annotate
level, availability, cost and license. The builder rejects missing glossary
targets, unknown citation IDs and absent bibliography links. Root-relative
corpus addresses retain the existing guide-link convention. Give each entry
its own page: current readers support same-page anchors but do not resolve
cross-page fragments. Glossary links likewise name a complete term page.

Search and navigation are derived from the pages at build time. The builder
reads body text, headings and explicit `id` / `name` anchors from each page's
HTML sections. `index.search.tsv` holds weighted single terms for the reader;
`index.anchors.tsv` holds the validated anchor graph. Term tokenisation is
CLI-aware: ordinary words are stemmed, while flags (`--pipe`, `-h`), dotted
IDs (`shell.novice`), versions (`0.71.0`), paths and hyphenated IDs keep their
punctuation. `tools/search_tokens.py` defines the rules that `src/index.c`
mirrors; change both together.

Use `<meta name="search-boost" content="...">` and
`<meta name="search-exclude" content="...">` with comma-separated single
terms when the derived weights need correction. Each value must be exactly one
CLI term already present on the page; a boost raises its weight, an exclude
removes it entirely, and a page may not exclude all of its terms. A boost is
not a way to add a word the page never says: edit the prose instead. Reader
navigation follows the generated parent/path graph and same-page anchors; a
cross-page `#fragment` is rejected because the reader does not resolve it.

`index.json` adds `kind`, `skills`, `paths` and a per-entry `search` block.
`index.kinds.tsv`, `index.skills.tsv`, `index.paths.tsv`, `index.search.tsv`
and `index.anchors.tsv` project them without altering the established
ten-column `index.tsv`.
These optional sidecars omit pages with no supplied kind/skills/paths; absence means
unclassified/unmapped, as the JSON and coverage report also show.
`coverage.json` reports every skill cell's page/path coverage, explicit gaps,
unclassified/unmapped pages and anatomy warnings. Read it with
`python3 tools/report-coverage.py --warnings` after `make corpus`. A mapped
page is a coverage claim, not learner validation or proof of mastery.

`component-coverage.json` maps each product component
(`tamlinux/commands/*`, `fred.*` plugins, binding `.lua` files, and `tamlinux-tools/src/*` tools) to corpus
pages that mention its exact name, or to an exemption in
`tools/component-coverage-exemptions.tsv`. Read or refresh with
`python3 tools/report-component-coverage.py` (and `--json` / `--check`).
`make corpus-and-test` runs `--check --quiet` automatically via `corpus-check`.
Style and teaching quality remain under [`corpus-style.md`](corpus-style.md).
The unmapped list includes navigation and reference pages that may not need
skill mappings; the per-cell page/path gaps identify missing teaching coverage.
Anatomy and quiz gaps are warnings during migration; reference, glossary,
hub, bibliography and path jobs are not forced into tutorial anatomy.
The skills registry's optional `enforce_tracks` list promotes instructional
section anatomy/quiz gaps to errors for completed parts. It currently includes
`use`; other parts retain warnings until their migration is complete.

## Markup the renderer can show

Readers see the text that `tam show` renders, not the HTML.

- Use only the named entities in `html_entities[]` in `src/html_entities.inc`
  (included by `src/render.c`). For anything else, write the character in UTF-8
  or use plain words.
- No LaTeX or `$...$` math. The renderer has no math support.
- Read your page as a reader will:

  ```sh
  make command
  env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/sddm
  ```

- Start a new page from a template in `docs/templates/`. `tools/build-index.py`
  requires a unique stable `topic` and a one-line `summary` of at most 160
  characters, and validates the `keywords`, `level`, `track` and `prereqs`
  metadata, the `data-level` sections and blocks, and the breadcrumb.
- In the All view the first heading of each tagged section receives its level
  label automatically. Do not hand-type `Novice:`, `Learner:`, `Beginner:`,
  `Intermediate:` or `Expert:` prefixes; the linter rejects them.
- `details.q` requires a `summary` question and a nonempty answer body. Answers
  start folded in the pager; pipe output collects them under `Answers` at the
  enclosing section's end. See the pedagogy contract for other fold classes.
- `pre[data-risk]` renders the risk labels from the pedagogy contract. The
  linter requires `system` or `boot` for privileged commands, package installs
  and removals, and boot tools. These labels describe examples; no command is
  executed by the renderer.
- Named asides render a bold label, `dfn` uses italics and `cite` is plain text.
  Links inside them behave like other links.

## Adding a page

1. Claim the page, its section `index.html`, `knowledge/index.html`,
   `knowledge/index.json`, `knowledge/index.tsv`, `knowledge/index.parents.tsv`,
   `knowledge/index.kinds.tsv`, `knowledge/index.skills.tsv`, `knowledge/index.paths.tsv`,
   `knowledge/index.search.tsv`, `knowledge/index.anchors.tsv`,
   `knowledge/coverage.json`, `knowledge/version`,
   your `work/history/` record, and `AI_PROVENANCE.md` while
   you edit it.
2. Write the page from its sources. Keep a list of what you checked.
3. Link it from its canonical subject hub and appropriate learning/task routes.
   The Guide root offers Browse subjects, Learn step by step, Search, and Sources; it does not list every chapter. Keep cross-links contextual without changing a page's home.
4. Run `make corpus` to regenerate the indexes and coverage report.
5. If a note in `docs/plans/corpus/` asked for the page, confirm each query it
   records now finds the intended page (`tam explain <query>`), and close the
   note with version/commit and verification evidence. Update the plan index,
   roadmap and development contract under the maintenance procedure.
6. Bump `knowledge/version` in the same commit: minor for a new page, patch
   for a correction.
7. Run `make corpus-and-test`. It rebuilds the corpus before running the corpus
   checker, `tools/lint-corpus.py`; fix every line it prints. Corpus pages
   have no snapshots: read your page with `tam show --pipe` instead.
8. Write the session record and its `AI_PROVENANCE.md` row and commit only your
   paths. Installation is a separately instructed workflow under `AGENTS.md`;
   a content change or closed plan does not authorize a session deployment.

## Rebuilding after commits

`make corpus` always checks every HTML page recursively and refreshes the
Git-derived page revisions, timestamps and ignored `knowledge/.build_id`.
It catches additions, deletions and renames, including changes whose file
timestamps did not advance. Unchanged outputs keep their contents and mtimes.

After committing page changes, run `make corpus` again and review and commit
any changes to `knowledge/index.tsv`, `knowledge/index.json`,
`knowledge/index.parents.tsv`, `knowledge/index.kinds.tsv`,
`knowledge/index.skills.tsv`, `knowledge/index.paths.tsv`,
`knowledge/index.search.tsv`, `knowledge/index.anchors.tsv` and
`knowledge/coverage.json`.
Rerun `make corpus` after that index commit to
refresh the build marker; a clean corpus/tooling tree reports no `-dirty`.
No separate Python invocation is needed. Version bumps remain manual.

Use `make corpus-and-test` to rebuild and test in one command. It also builds
the reader when necessary, so it needs an installed `libtam`; set
`LIBTAM_PREFIX` to its installation prefix if needed. Neither target installs
software or promotes a stage. Preview the checkout with
`TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe <page>`.

## Notes about missing pages

When the developer asks for a note rather than a page, copy
`docs/plans/corpus/TEMPLATE.md` to `docs/plans/corpus/<topic>.md` and fill it
in, in about twenty lines. Do not draft the page itself; the author outlines it
when writing it.
Add the note to `docs/plans/README`, with the status and next action the
developer's request supports. Do not leave an implemented note labeled Open.

## Audits

An independent agent reviews the corpus on a cadence; see
`docs/corpus-audit.md`. An audit finds what a checker cannot, so the sourcing
rule above is what keeps a page out of the next audit report.

## Optional host documentation

Use `data-doc-source="info"`, `"tldr"` or `"archwiki"` on an anchor to retain
its label without a link while that collection is absent. Put the fallback
text in `<span data-doc-missing="info"> (not installed)</span>` using the
same identifier. This works on every corpus page. Do not place host-dependent
messages in C code or bake a build machine's availability into HTML.
