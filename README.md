# tam

`tam` is the terminal entry point to Tamlinux's offline guide. It helps users
learn about their system, maintain it, and discover ways to improve it.

**Stage:** Develop — command foundation (milestone 1) and offline guide
(milestone 2) implemented. Bare introduction, `--help`, `--version`, argument
errors, and the `browse` guide all build in C against the separate `libtam`
library and install as `tam`. Orientation and installation are not implemented
yet.

Bare `tam` prints a concise introduction and exits successfully without
launching a browser or writing state. `tam browse` opens the bundled offline
guide: an interactive pager in a terminal, or plain text when output is piped.
The guide is authored as semantic HTML under `knowledge/`; `tam` renders it
itself, so browsing needs no network and no external viewer.

## Using the guide

```sh
tam browse            # open the guide index in the pager
tam browse welcome    # open a page by name
tam browse --pipe     # print the guide as plain text (no escape sequences)
```

In the pager: `j`/`k` or arrows scroll a line, `Space`/`b` or `PgDn`/`PgUp`
scroll a screen, `g`/`G` or `Home`/`End` jump to the top/bottom, `Tab` selects
a link, `Enter` or the right arrow follows it, `u` or the left arrow goes back,
and `q` or `Esc` quits. External links are shown but never fetched, so the guide
stays offline. The corpus is located via `$TAM_KNOWLEDGE_DIR`, then the XDG data
directories, then the compiled install prefix.

The implementation uses C with the separate `libtam` foundation library.
Command source, bundled guide pages, tests and component documentation live
here. The `tamlinux` product repository owns workstation assembly and the
installation framework.

Read the [development plan](docs/plans/development.md) for the milestones,
command behavior, packaging contract and acceptance checks, and the
[offline-guide plan](docs/plans/offline-guide.md) for the guide's design.

## Layout

| Path | Purpose |
| --- | --- |
| `src/` | C command implementation. |
| `knowledge/` | Bundled offline HTML and matching text guide. |
| `tests/` | Command behavior, browser integration and package-data checks. |
| `docs/plans/` | Development scope and acceptance evidence. |
| `docs/ai/` | AI-assisted authorship and verification records. |

## Build and run

`tam` links against an **installed** `libtam`; it never builds or modifies the
library checkout. Build and install `libtam` first, then:

```sh
make            # build build/tam
make test       # run the behavior checks
sudo make install
```

Override `LIBTAM_PREFIX` if the library lives outside `PREFIX`; `PREFIX` and
`DESTDIR` support staged installation. See
[`docs/plans/development.md`](docs/plans/development.md) for the command
contract and
[`../improvements-fred-tamlinux/reports/2026-10-09-tam-c-toolchain-and-build.md`](../improvements-fred-tamlinux/reports/2026-10-09-tam-c-toolchain-and-build.md)
for the toolchain rationale.

## License and authorship

GPL-3.0-or-later; see [LICENSE](LICENSE). AI-assisted work is recorded in
[AI_PROVENANCE.md](AI_PROVENANCE.md).
