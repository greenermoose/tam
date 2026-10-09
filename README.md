# tam

`tam` is the terminal entry point to Tamlinux's offline guide. It helps users
learn about their system, maintain it, and discover ways to improve it.

**Stage:** Develop — command foundation implemented (milestone 1). The
first slice — bare introduction, `--help`, `--version` and argument errors —
builds in C against the separate `libtam` library and installs as `tam`.
Browsing, orientation and installation are not implemented yet.

Bare `tam` prints a concise introduction and exits successfully without
launching a browser or writing state. `tam browse` will explicitly open the
guide; browsing is not implemented yet.

The implementation uses C with the separate `libtam` foundation library.
Command source, bundled guide pages, tests and component documentation live
here. The `tamlinux` product repository owns workstation assembly and the
installation framework.

Read the [development plan](docs/plans/development.md) for the first milestone,
command behavior, packaging contract and acceptance checks.

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
