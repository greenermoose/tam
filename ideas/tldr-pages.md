# tldr-pages

## What it is

- **Format:** Markdown pages with a command name, two description lines, an upstream link and roughly five concrete examples, with a hard maximum of eight ([CONTRIBUTING.md, full page read, accessed 2026-11-26](https://raw.githubusercontent.com/tldr-pages/tldr/main/CONTRIBUTING.md); [style guide, full page read, accessed 2026-11-26](https://raw.githubusercontent.com/tldr-pages/tldr/main/contributing-guides/style-guide.md)).
- **Licence and size:** no licence statement was collected; the README shows 64.3k stars, 5.6k forks, about 24,886 commits and roughly 45 `pages.*` locale directories ([tldr README, full page read, accessed 2026-11-26](https://github.com/tldr-pages/tldr)).
- **Evidence:** repository files were read directly; the community clients wiki was not. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Keep a small, fixed page shape with simplest examples first and an explicit hand-off to full manuals ([CONTRIBUTING.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CONTRIBUTING.md)).
  Tam application: a compact default view can share one source with a fuller reference view.
- **adopt**: Irreversible operations use placeholder values such as a device token instead of a literal disk name, so examples cannot be pasted unchanged ([style guide](https://raw.githubusercontent.com/tldr-pages/tldr/main/contributing-guides/style-guide.md)).
  Tam application: make placeholder syntax part of the corpus format and render it distinctly.
- **adopt**: Placeholders, option variants, alternatives, ranges and keypress tokens have one documented syntax, and prose styling is reserved for client highlighting ([style guide](https://raw.githubusercontent.com/tldr-pages/tldr/main/contributing-guides/style-guide.md)).
  Tam application: a small token grammar keeps examples consistent across pages and renderers.
- **adopt**: Aliases point to a canonical command, disambiguation pages handle colliding names, and subcommands map spaces to dashes ([style guide](https://raw.githubusercontent.com/tldr-pages/tldr/main/contributing-guides/style-guide.md)).
  Tam application: an alias and disambiguation table in the catalog can do this without stub files.
- **adopt**: The client specification requires version and spec-version reporting, plain output without decoration on non-TTY, one page name per line for list mode, and a nonzero exit for a missing page ([CLIENT-SPECIFICATION.md, full page read, accessed 2026-11-26](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md)).
  Tam application: document a stable CLI contract so scripts can rely on it.
- **adopt**: `tldr-lint` runs on every pull request and can be run locally ([CONTRIBUTING.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CONTRIBUTING.md)).
  Tam application: lint page shape, links and tokens in the normal corpus check.
- **consider**: Lookup order is deterministic: lowercasing and space-to-dash, then preferred platform, `common`, other platforms with a warning; language falls back from `LANGUAGE` and `LANG` to English ([CLIENT-SPECIFICATION.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md)).
  Tam application: copy the principle of explicit fallback order, not the whole platform matrix.
- **consider**: Clients cache a whole corpus ZIP and expose an explicit update action ([CLIENT-SPECIFICATION.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md)).
  Tam application: an offline corpus needs an opt-in refresh story if it is ever distributed separately.
- **consider**: Translation policy requires native speakers, current English sources and dashboards for stale translations ([CONTRIBUTING.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CONTRIBUTING.md)).
  Tam application: one canonical definition per term in a glossary avoids one-file-per-language drift.
- **reject**: Generative-AI-authored pages and unreviewed machine translation, which the project discourages and may close ([CONTRIBUTING.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CONTRIBUTING.md)).
  Tam application: AI assistance in drafting is fine; publication still needs human verification.
- **reject**: Online-only fetching: the specification's cache and update rules assume offline use ([CLIENT-SPECIFICATION.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md)).
  Tam application: any update mechanism must be separate from reading.

## Things to avoid

- Do not re-explain paths, globbing, escaping or return values in every page: tldr excludes general Unix concepts ([CONTRIBUTING.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CONTRIBUTING.md)).
- Do not put help and version examples first: the style guide puts them last ([style guide](https://raw.githubusercontent.com/tldr-pages/tldr/main/contributing-guides/style-guide.md)).
- Do not cite the old cache asset URL from before its removal on 2026-01-20 as current ([CLIENT-SPECIFICATION.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md)).
- Do not call the Node client the leading implementation: the README says it has fallen behind in updates ([tldr README](https://github.com/tldr-pages/tldr)).
- Do not claim offline-versus-online usage rates; no data was collected.

## Where to look

- All URLs below were accessed 2026-11-26.
- [Full page read] [tldr README](https://github.com/tldr-pages/tldr) — purpose, clients, scale and release assets.
- [Full page read] [CONTRIBUTING.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CONTRIBUTING.md) — example counts, progression, exclusions, lint and translation policy.
- [Full page read] [style guide](https://raw.githubusercontent.com/tldr-pages/tldr/main/contributing-guides/style-guide.md) — template, tokens, aliases, disambiguation and safety placeholders.
- [Full page read] [CLIENT-SPECIFICATION.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md) — CLI contract, lookup order, caching and exit codes.
- [Gap] The community clients wiki at `https://github.com/tldr-pages/tldr/wiki/Clients` was not source-read.
