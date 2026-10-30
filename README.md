# tam

`tam` is a terminal command to help users learn about their Tamlinux system,
maintain it, and improve it.

The command includes host inspection and a built-in HTML parser and pager
to display and search the Tamlinux guide (knowledge corpus), man pages,
Info manuals, and tldr pages. It builds in C against the `libtam` library
and is delivered through Tamlinux package assemblies.

## Command overview

Bare `tam` prints a concise introduction and exits successfully without
launching a browser or writing state. `tam show` opens the guide: an
interactive pager in a terminal, or plain text when output is piped.
`tam hi` opens orientation and records successful display state.
`tam search <query>` searches the corpus with field-weighted ranking and
fuzzy suggestions; on a terminal the results open in the pager with followable
links; for use in scripts,  `--pipe` or `--json` stream them to stdout instead.
`tam explain <topic>` explains curated local concepts and tools, resolving
multi-word queries through search.

`tam inspect` inspects host facts, package tools, and supported profiles.

## Using tam

```sh
tam hi                # open the welcome orientation in the pager
tam hi --pipe         # print welcome text to stdout
tam show              # open the guide index in the pager
tam show welcome      # open a page by name
tam show --pipe       # print the guide as plain text (no escape sequences)
tam search "change file owner" # open matching guide pages in the pager
tam search --pipe permissions  # print search results as plain text
tam search --json permissions  # output search results as JSON
tam explain           # list available explanation topics
tam explain pipes     # explain a curated topic or shell concept
tam explain cpu load  # multi-word concepts resolve via search
tam explain '|'       # explain pipelines (quote shell metacharacters like '|')
tam explain browse    # browse is also an explain topic pointing to the guide
tam explain grep --pipe # print topic explanation as plain text
tam inspect           # inspect host facts and installation capabilities
tam install --help    # list the planned installation subcommands
tam upgrade           # compare the running command with all four source stages
tam upgrade --json    # print structured comparison results
tam switch test       # build and activate Test source after confirmation
tam switch --rollback # return to the previous local generation, offline
```

The pager is designed so pages can be read easily using arrow keys and the
space bar.  For keyboards without arrow keys, keys on the home row,
mirrored on each hand, can be used for navigation: `d`/`k` or Down moves
the attention line down a line and `s`/`l` or Up moves it up; `a`/`j` and
`f`/`;` move the attention column left and right from link to link along
that line. One row down, `x`/`.` or `Home` jumps to the top and `c`/`,`
or `End` to the bottom. `Space`/`b` or `PgDn`/`PgUp` scroll a screen,
`]` or `[` (or `Tab`/`Shift-Tab`) selects the next or previous link anywhere,
`/` opens `Search in page /` for text search (`n`/`N` navigates hits), 
`\` opens `Search in guide \` across the guide.

`g`, `Enter` or the right arrow follows the selected link (the right arrow
without a selected link proceeds forward along a visited path). `u` or the
left arrow goes back, or hides a shown answer on its `[-]` line (`u` means
hierarchy Up in Info). Backtick returns to the Guide. `q` or `Esc` quits.

`h` or `?` shows a help screen. `=` displays the current page address
(`Path = ...`). The status row reads
`<Page title> | L<line> <first>-<last>/<total> | <Level> text 0123 | h Help  Esc Quit  [] Select links  -> <selected link>`
with the version at the right edge, `v<command> (<corpus>)`, so a reader
always knows which build they are running (or `Path = <address>` when `=` is
active). Narrow terminals drop the title, then the link, then the key hints.

The pager uses terminfo capabilities for colors, cursor positioning and screen
updates. It falls back to reverse video on the Linux console and monochrome
terminals, and to plain text when stdout is not a usable terminal. On resize it
reflows the current document while retaining the reading position, selected
link and search state.

## Knowledge corpus

The guide is authored as semantic HTML under `knowledge/`; `tam` renders
HTML itself, so browsing needs no network and no external viewer. The corpus
is located via `$TAM_KNOWLEDGE_DIR`, then the XDG data directories, then
the compiled install prefix.

Man pages are rendered with groff once and cached in
`$XDG_CACHE_HOME/tamlinux/tam/man/` (default `~/.cache/tamlinux/tam/man/`),
one file per source page, stamped with the source's size and modification
time; a changed page is rendered again. Revisiting `man bash` takes about
50 ms instead of a second. Deleting the directory is always safe.

## Source stages and recovery

The `tam` command is written in C; the explicit source-upgrade helper
uses Python's standard library for strict metadata parsing and bounded
process control.  Command source, bundled guide pages, tests and component
documentation live in this repo. The `tamlinux` repository owns workstation
assembly and the installation framework.

`tam upgrade` checks the public Development (`develop`), Test (`test`), Run
(`main`) and Release (paired version tag) sources. A check is the only normal
command that contacts GitHub; it uses fixed HTTPS endpoints and needs `curl`.
Use `tam upgrade --apply --channel STAGE` to build a forward update, or
`tam switch STAGE` to select a stage or release intentionally. Release pins
name both command and corpus versions. A source build requires Git, curl,
`cc`, Make, pkg-config, Python 3, zlib, ncursesw and D-Bus development inputs.
The command previews the exact source revisions and runs their Makefiles only
after interactive confirmation. It does not install OS dependencies.

The build uses private snapshots and a private generation under
`~/.local/dev/opt/tam/`; it does not write to the working clone or shared
`libtam`. The exposed command and wrappers point to one atomic `current`
generation. `tam switch --rollback` uses the local receipt and works offline.
The helper prints a recovery launcher path so a previous source that lacks
`upgrade` and `switch` can still be moved forward or rolled back. After a
successful switch, run `hash -r` in the shell. This is a development override;
product delivery still requires the recorded `tamlinux-packages` assembly.
The first installation of these commands needs the component-development
bootstrap because an older tam cannot invoke commands it does not contain.

Read the [development plan](docs/plans/development.md) for current command
behavior, packaging and acceptance, and the
[knowledge-system plan](docs/plans/knowledge-system-0.31.0.md) for the completed
corpus change and its remaining packaged-review boundary.
The [offline-guide plan](docs/plans/offline-guide.md) is a historical design record.

## Layout

| Path | Purpose |
| --- | --- |
| `src/` | C command implementation. |
| `knowledge/` | Bundled offline HTML and generated metadata index; text derives at runtime. |
| `tests/` | Command behavior, offline guide rendering and package-data checks. |
| `docs/plans/` | Indexed current plans and historical designs with explicit dispositions. |
| `docs/ai/` | AI-assisted authorship and verification records. |

## Build, package, and develop

Tamlinux product software reaches a session only through a recorded
`tamlinux-packages` assembly and `tam-deploy`. A source checkout or direct
system installation is not a deployment route. The owning package repository
consumes this command and its knowledge corpus as pinned inputs.

### Editing and installing a development build

After editing `knowledge/` pages or `src/` files in this checkout, run the
interactive developer loop:

```sh
make dev
```

`make dev` collects uncommitted changes outside other sessions' active
`tam-work` claims, proposes the next version bump for each changed lane, runs
`make test`, commits the changes and any refreshed corpus indexes, installs the
development build under `~/.local/dev`, and asks before pushing `develop`. Use
`make dev DRY_RUN=1` to preview the change list and proposed versions without
writing, building or committing anything.

For a non-interactive build and development install:

```sh
make
make test PREFIX="$HOME/.local/dev"
make install PREFIX="$HOME/.local/dev"
```

`make` finds `libtam` 0.4.0 or later with `pkg-config libtam` and links it
statically, so install `libtam` first and make sure pkg-config can see it,
for example with
`PKG_CONFIG_PATH="$HOME/.local/dev/lib/pkgconfig"`. If it cannot, the build
stops and says what to do.

For an assembly or staged package build, set `PREFIX` and `DESTDIR` as required
by that packaging recipe. `DESTDIR` stages files under a temporary root; it
does not make that tree a deployment. Do not run the default `/usr/local`
install on a Tamlinux session.

### Runtime knowledge resolution

When `tam show`, `tam welcome`, or `tam explain` looks up knowledge pages, `tam` resolves the corpus root directory by searching the following paths in order:

1. **`$TAM_KNOWLEDGE_DIR`**: Direct override environment variable (used for development builds and automated test suites).
2. **Compiled-in default datadir (`TAM_DATADIR`)**: The corpus installed with this binary, built in at compile time via `CPPFLAGS += -DTAM_DATADIR=\"$(DATADIR)\"`. Checking it first means a development install under one prefix never renders pages from a different package location.
3. **`$XDG_DATA_HOME/tamlinux/knowledge`**: User data directory (defaults to `~/.local/share/tamlinux/knowledge` if `$XDG_DATA_HOME` is unset).
4. **`$XDG_DATA_DIRS/tamlinux/knowledge`**: System data directories checked in colon-separated order (e.g. `/usr/local/share/tamlinux/knowledge`, `/usr/share/tamlinux/knowledge`). These serve relocated (`DESTDIR`-staged) installs whose compiled path does not exist.

If none of these locations contain a valid guide corpus (`index.html`), `tam show` prints an informative diagnostic advising the user that the offline guide is not installed, while non-guide commands like `tam --help` and `tam --version` continue to function normally.

`tam` links against an **installed** `libtam`; it never builds or modifies the
library checkout. Set `LIBTAM_PREFIX` to name the library's install prefix
instead of asking pkg-config; package recipes do this. See [`docs/plans/development.md`](docs/plans/development.md)
for the command contract and the component development runbook for the
authorized local workflow.

## License and authorship

GPL-3.0-or-later; see [LICENSE](LICENSE). AI-assisted work is recorded in
[AI_PROVENANCE.md](AI_PROVENANCE.md).
