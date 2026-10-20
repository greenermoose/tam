# Writing the tam knowledge corpus

Read this before you add or change a page under `knowledge/`. It supplements
`AGENTS.md`, which covers claims, lanes, versions and provenance.

The current five-part organization and metadata contract were delivered in
corpus 0.31.0 under the [authoritative knowledge-system plan](plans/knowledge-system-0.31.0.md).
The plan records its page mapping, hierarchy rules and verification. Preserve
those contracts when adding content. Plan/note status and closure follow
[plan maintenance](plan-maintenance.md).

The new track values are `install`, `learn`, `use`, `maintain` and `improve`.
Every non-root page in corpus 0.31.0 declares `<meta name="parent" ...>` with
its canonical parent path relative to `knowledge/`, including `.html`. The
guide root has no parent and has an empty track because it is shared
navigation. Parent chains must reach `index.html` without cycles, breadcrumbs
must list the declared ancestors, and each page's track must match its
top-level part. Older corpora may omit `parent`; `tools/build-index.py` then
uses the historical directory hierarchy so corpus 0.30.0 remains readable.

The generated `index.tsv` keeps its established ten-column layout so readers
from earlier command versions continue to load it without changing `updated`.
The JSON entry gains an additive `parent` field, and `index.parents.tsv`
projects each page path to its parent for command readers that support logical
hierarchy. Older corpora without that sidecar use the directory fallback.

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
| Base-system and compositor direction | `tamlinux` repository: `docs/plans/base-operating-system.md`, `sway-integration.md`, `desktop-decoupling.md`; always written as planned | |
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

## Markup the renderer can show

Readers see the text that `tam show` renders, not the HTML.

- Use only the named entities in `entities[]` in `src/render.c`. For anything
  else, write the character in UTF-8 or use plain words.
- No LaTeX or `$...$` math. The renderer has no math support.
- Read your page as a reader will:

  ```sh
  make command
  env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/sddm
  ```

- Start a new page from the head of an existing one. `tools/build-index.py`
  validates the `topic`, `summary`, `keywords`, `level`, `track` and
  `prereqs` metadata, the `data-level` sections and the breadcrumb.

## Adding a page

1. Claim the page, its section `index.html`, `knowledge/index.html`,
   `knowledge/index.json`, `knowledge/index.tsv`, `knowledge/index.parents.tsv`,
   `knowledge/version`,
   your `docs/ai/` record, and `AI_PROVENANCE.md` while
   you edit it.
2. Write the page from its sources. Keep a list of what you checked.
3. Link it from its canonical part hub and appropriate learning/task routes.
   The Guide root links the five part hubs and shared utilities; it does not list
   every chapter. Keep cross-links contextual without changing a page's home.
4. Run `make corpus` to regenerate and validate all three indexes.
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
any changes to `knowledge/index.tsv`, `knowledge/index.json` and
`knowledge/index.parents.tsv`. Rerun `make corpus` after that index commit to
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
Add the note to `docs/plans/README.md`, with the status and next action the
developer's request supports. Do not leave an implemented note labeled Open.

## Audits

An independent agent reviews the corpus on a cadence; see
`docs/corpus-audit.md`. An audit finds what a checker cannot, so the sourcing
rule above is what keeps a page out of the next audit report.
