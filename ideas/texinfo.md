# GNU Texinfo and Info

## What it is

- **Format:** Texinfo source generates Info hypertext with explicit nodes; an Info node advertises `Next`, `Previous` and `Up`, menus branch downward and inline `*Note label: node.` references jump anywhere ([Info: An Introduction, GNU manual mirror, full page read, accessed 2026-11-26](https://web.mit.edu/gnu/doc/html/info.html)).
- **Licence and scope:** GNU documentation system for large manuals that are looked up rather than read linearly; no licence statement was checked here, and the source read was an MIT mirror that may be older than the current release.
- **Evidence:** the Info manual mirror was read directly; a Texinfo cross-reference page was seen only as a search excerpt, and direct `gnu.org` fetches timed out. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Typed `Next`, `Previous`, `Up` and last-position links solve navigation that a flat list cannot, and a top-level Directory node makes every manual reachable ([Info manual mirror, full page read, accessed 2026-11-26](https://web.mit.edu/gnu/doc/html/info.html)).
  Tam application: a pager history plus "parent", "next" and "catalog" links is worth more than a full book graph.
- **adopt**: Menu items are designed for short unique abbreviations with Tab completion and numeric selection ([Info manual mirror, full page read, accessed 2026-11-26](https://web.mit.edu/gnu/doc/html/info.html)).
  Tam application: choose short, unique page IDs so command-line jumps need little typing.
- **adopt**: A tag table maps node names to byte offsets and must be regenerated when nodes move, while `Info-validate` checks pointers and cross-references ([Info manual mirror, full page read, accessed 2026-11-26](https://web.mit.edu/gnu/doc/html/info.html)).
  Tam application: generated indexes need a rebuild step and integrity checks or links silently rot.
- **adopt**: The Directory is simply a menu at a known node, and adding a document is a menu edit ([Info manual mirror, full page read, accessed 2026-11-26](https://web.mit.edu/gnu/doc/html/info.html)).
  Tam application: a single generated catalog is simpler than implicit discovery by directory scan.
- **consider**: Cross-reference commands produce node-based Info links and printable references because reference manuals are looked up rather than read linearly ([Texinfo: Cross References, search excerpt, accessed 2026-11-26](https://web.mit.edu/gnu/doc/html/texi_9.html)).
  Tam application: one link definition can serve pager, HTML and printable output if targets are stable.
- **consider**: Debian maintains the shared Info directory through `install-info` and package metadata, while packages must not ship the directory file ([Debian Policy §12.2, full page read, accessed 2026-11-26](https://www.debian.org/doc/debian-policy/ch-docs.html)).
  Tam application: generated catalogs should be owned by the build, not hand-edited by contributors.
- **reject**: The full Info node graph with menus, tag tables and pointer validation for a compact offline corpus ([Info manual mirror, full page read, accessed 2026-11-26](https://web.mit.edu/gnu/doc/html/info.html)).
  Tam application: take history, parent links and a catalog; skip the book machinery.
- **reject**: Treating an arbitrary directed graph as a healthy navigation model: the manual says unpointed substructures are effectively undiscoverable ([Info manual mirror, full page read, accessed 2026-11-26](https://web.mit.edu/gnu/doc/html/info.html)).
  Tam application: require at least one catalog route to every page.

## Things to avoid

- Do not quote current `info` default keybindings as verified: the source was an MIT mirror and the current binary was not inspected.
- Do not hand-maintain a directory of manuals as prose: Info's directory is generated from metadata ([Debian Policy §12.2](https://www.debian.org/doc/debian-policy/ch-docs.html)).
- Do not let links point only forward: `Up`, last-position and catalog routes are what make a large corpus navigable ([Info manual mirror](https://web.mit.edu/gnu/doc/html/info.html)).
- Do not assume the Texinfo cross-reference page was read in full; it is a search excerpt.

## Where to look

- All URLs below were accessed 2026-11-26; labels show evidence depth.
- [Full page read] [Info: An Introduction (GNU manual mirror)](https://web.mit.edu/gnu/doc/html/info.html) — nodes, keys, menus, cross-references, Directory, tags and validation.
- [Search excerpt] [Texinfo: Cross References](https://web.mit.edu/gnu/doc/html/texi_9.html) — `@xref`-style commands and rationale.
- [Full page read] [Debian Policy §12.2](https://www.debian.org/doc/debian-policy/ch-docs.html) — Info installation and directory registration.
- [Gap] Direct `gnu.org/software/texinfo` fetches timed out; current feature changes were not verified.
