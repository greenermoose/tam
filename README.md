# tam

`tam` is the terminal entry point to Tamlinux's offline guide. It helps users
learn about their system, maintain it, and discover ways to improve it.

**Stage:** Develop — repository scaffold and command contract. There is no
executable or installed command yet.

Bare `tam` prints a concise introduction and exits successfully. `tam browse`
explicitly opens the guide. Browsing uses Lynx when available, with readable
plain text when Lynx is absent or output is piped.

The implementation will use C with the separate `libtam` foundation library.
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

## License and authorship

GPL-3.0-or-later; see [LICENSE](LICENSE). AI-assisted work is recorded in
[AI_PROVENANCE.md](AI_PROVENANCE.md).
