# tam

`tam` is the terminal entry point to Tamlinux's offline guide. It helps users
learn about their system, maintain it, and discover ways to improve it.

**Stage:** Develop — command 0.29.3 and knowledge corpus 0.30.0. The command
includes the offline guide, native man and Info readers, search and explanation,
host inspection, and a portable built-in pager. It builds in C against the
separate `libtam` library and installs as `tam`. The command is part of
`tamlinux-packages` and installed on the system. See the
[development plan](docs/plans/development.md) and
[roadmap](ROADMAP.md) for current milestones.

The next approved milestone is the [five-part knowledge-system overhaul](docs/plans/knowledge-system-0.31.0.md),
targeting corpus 0.31.0 and companion command 0.30.0. It has not been implemented.
Start there for its handoff; the [plan index](docs/plans/README.md) distinguishes
current work from historical plans. Follow [plan maintenance](docs/plan-maintenance.md)
when changing scope or closing work.

Bare `tam` prints a concise introduction and exits successfully without
launching a browser or writing state. `tam show` opens the bundled offline
guide: an interactive pager in a terminal, or plain text when output is piped.
`tam welcome` opens orientation and records successful display state.
`tam search <query>` searches the corpus index with field-weighted ranking,
fuzzy suggestions, and optional `--json` output.
`tam explain <topic>` explains curated local concepts and tools, resolving
multi-word queries through search.
`tam inspect` inspects host facts, package tools, and supported profiles.
The guide is authored as semantic HTML under `knowledge/`; `tam` renders it
itself, so browsing needs no network and no external viewer.

## Using tam

```sh
tam welcome           # open the welcome orientation in the pager
tam welcome --pipe    # print welcome text to stdout
tam show              # open the guide index in the pager
tam show welcome      # open a page by name
tam show --pipe       # print the guide as plain text (no escape sequences)
tam search "change file owner" # search the guide for topics or phrases
tam search --json permissions  # output search results as JSON
tam explain           # list available explanation topics
tam explain pipes     # explain a curated topic or shell concept
tam explain cpu load  # multi-word concepts resolve via search
tam explain '|'       # explain pipelines (quote shell metacharacters like '|')
tam explain browse    # browse is also an explain topic pointing to the guide
tam explain grep --pipe # print topic explanation as plain text
tam inspect           # inspect host facts and installation capabilities
tam install --help    # list the planned installation subcommands
```

In the pager: `j`/`k` or arrows scroll a line, `Space`/`b` or `PgDn`/`PgUp`
scroll a screen, `g`/`G` or `Home`/`End` jump to the top/bottom, `Tab`/`Shift-Tab`
selects next/previous link, `Enter` or the right arrow follows it (or the right
arrow without a selected link proceeds forward along a visited path), `u` or the
left arrow goes back (`u` means hierarchy Up in Info), backtick returns to the
Guide, and `q` or `Esc` quits. External links are shown but never
fetched, so the guide stays offline. The corpus is located via
`$TAM_KNOWLEDGE_DIR`, then the XDG data directories, then the compiled install
prefix.

The pager uses terminfo capabilities for colors, cursor positioning and screen
updates. It falls back to reverse video on the Linux console and monochrome
terminals, and to plain text when stdout is not a usable terminal. On resize it
reflows the current document while retaining the reading position, selected
link and search state.

The implementation uses C with the separate `libtam` foundation library.
Command source, bundled guide pages, tests and component documentation live
here. The `tamlinux` product repository owns workstation assembly and the
installation framework.

Read the [development plan](docs/plans/development.md) for current command
behavior, packaging and acceptance, and the
[knowledge-system plan](docs/plans/knowledge-system-0.31.0.md) for the next work.
The [offline-guide plan](docs/plans/offline-guide.md) is a historical design record.

## Layout

| Path | Purpose |
| --- | --- |
| `src/` | C command implementation. |
| `knowledge/` | Bundled offline HTML and generated metadata index; text derives at runtime. |
| `tests/` | Command behavior, offline guide rendering and package-data checks. |
| `docs/plans/` | Indexed current plans and historical designs with explicit dispositions. |
| `docs/ai/` | AI-assisted authorship and verification records. |

## Installation on a Tamlinux cultivar

By default on a Tamlinux cultivar (`PREFIX ?= /usr/local`):

- **`tam` binary:** Installed to `/usr/local/bin/tam` (`$(PREFIX)/bin/tam`, mode 755). `/usr/local/bin` is in `$PATH` across the system, allowing users and scripts to invoke `tam` directly from any terminal.
- **`tam` knowledge pages:** Installed to `/usr/local/share/tamlinux/knowledge/` (`$(PREFIX)/share/tamlinux/knowledge/`). This directory contains the complete offline semantic HTML guide corpus (`index.html`, `welcome.html`, `install/index.html`, `shell/pipelines.html`, `man/index.html`).

### Runtime knowledge resolution

When `tam show`, `tam welcome`, or `tam explain` looks up knowledge pages, `tam` resolves the corpus root directory by searching the following paths in order:

1. **`$TAM_KNOWLEDGE_DIR`**: Direct override environment variable (used for development builds and automated test suites).
2. **Compiled-in default datadir (`TAM_DATADIR`)**: The corpus installed with this binary, built in at compile time via `CPPFLAGS += -DTAM_DATADIR=\"$(DATADIR)\"` (default `/usr/local/share/tamlinux/knowledge`). Checking it first means a development install under one prefix (for example `~/.local/dev`) never renders the pages of a packaged install under another.
3. **`$XDG_DATA_HOME/tamlinux/knowledge`**: User data directory (defaults to `~/.local/share/tamlinux/knowledge` if `$XDG_DATA_HOME` is unset).
4. **`$XDG_DATA_DIRS/tamlinux/knowledge`**: System data directories checked in colon-separated order (e.g. `/usr/local/share/tamlinux/knowledge`, `/usr/share/tamlinux/knowledge`). These serve relocated (`DESTDIR`-staged) installs whose compiled path does not exist.

If none of these locations contain a valid guide corpus (`index.html`), `tam show` prints an informative diagnostic advising the user that the offline guide is not installed, while non-guide commands like `tam --help` and `tam --version` continue to function normally.

### Staged and packaged installation

`tam` is packaged for Tamlinux delivery in [`tamlinux-packages`](https://github.com/greenermoose/tamlinux-packages) (as `packages.x86_64-linux.tam` and the default flake output).

For other package managers (such as Arch `PKGBUILD` or Void `xbps-src`) or relocatable testing, `PREFIX` and `DESTDIR` can be overridden:

```sh
make install PREFIX=/usr DESTDIR=/path/to/stage
```

This installs the binary to `$(DESTDIR)$(PREFIX)/bin/tam` and the knowledge pages to `$(DESTDIR)$(PREFIX)/share/tamlinux/knowledge/`.

## Build and install

`tam` links against an **installed** `libtam`; it never builds or modifies the
library checkout. Build and install `libtam` first, then:

```sh
make            # build build/tam
make test       # run the behavior checks
sudo make install  # or: pkexec make install
```

`make install` installs the `tam` binary to `$(DESTDIR)$(BINDIR)/tam` (`/usr/local/bin/tam` by default) and copies the knowledge pages to `$(DESTDIR)$(DATADIR)/` (`/usr/local/share/tamlinux/knowledge/` by default).

Override `LIBTAM_PREFIX` if the library lives outside `PREFIX`; `PREFIX` and
`DESTDIR` support staged installation. See
[`docs/plans/development.md`](docs/plans/development.md) for the command
contract.

## License and authorship

GPL-3.0-or-later; see [LICENSE](LICENSE). AI-assisted work is recorded in
[AI_PROVENANCE.md](AI_PROVENANCE.md).
