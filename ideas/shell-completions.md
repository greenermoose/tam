# Shell completions and CLI help conventions

## What it is

- **Format:** shell functions and command-keyed metadata rather than prose: Bash programmable completion uses `complete` and `compgen` with action, word-list, glob, function and subprocess sources, while fish exposes a `complete` command keyed by command name ([bash builtins reference, search-result snippet, accessed 2026-11-26](https://www.linux.org/docs/man1/builtin.html); [fish completions docs, search-result snippet, accessed 2026-11-26](https://fishshell.com/docs/current/completions.html)).
- **Scope and licence:** discovery outside manuals, including `--help`, `--version`, completion loading and terminal-aware output; no licence was checked. Bash-completion reads command-specific files on demand from a directory discovered through `pkg-config`, with user overrides in an XDG data directory ([bash-completion README, search-result snippet, accessed 2026-11-26](https://github.com/scop/bash-completion)).
- **Evidence:** Bash and fish details are search-result snippets, while the tldr client spec, GNU Coding Standards mirror and man pages were read directly. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: GNU Coding Standards require `--version` to print a parseable name and version and `--help` to print brief usage on stdout and exit without doing work ([GNU Coding Standards mirror, full page read, accessed 2026-11-26](https://benpfaff.org/writings/blp-stds/standards.html)).
  Tam application: help and version are modes of the command itself, not errors or corpus lookups.
- **adopt**: man-db follows the same convention and uses exit codes that distinguish usage, operational and not-found cases ([man(1), full page read, accessed 2026-11-26](https://man7.org/linux/man-pages/man1/man.1.html); [apropos(1), full page read, accessed 2026-11-26](https://man7.org/linux/man-pages/man1/apropos.1.html)).
  Tam application: document exit statuses once and keep them stable across subcommands.
- **adopt**: The tldr client specification requires plain output on non-TTY and one page name per line for list mode ([CLIENT-SPECIFICATION.md, full page read, accessed 2026-11-26](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md)).
  Tam application: pager-friendly output can still be script-friendly in explicit list modes.
- **adopt**: man's pager prompt includes page name, section and line number, and width follows terminal settings without changing redirected output ([man(1)](https://man7.org/linux/man-pages/man1/man.1.html)).
  Tam application: context in a prompt is cheap; decoration in piped output is not.
- **consider**: Completion data is effectively a tiny command-keyed metadata corpus with an established user override location ([bash-completion README, search-result snippet, accessed 2026-11-26](https://github.com/scop/bash-completion)).
  Tam application: a page-name and exact-alias list could feed completions if it is generated from the same catalog.
- **consider**: GNU guidance asks for long options equivalent to short ones and consistent names across programs ([GNU Coding Standards mirror](https://benpfaff.org/writings/blp-stds/standards.html)).
  Tam application: consistent option names reduce what the corpus has to repeat about flags.
- **consider**: Help text's trailing contact, home page and general-help links reduce what a separate corpus must carry ([GNU Coding Standards mirror](https://benpfaff.org/writings/blp-stds/standards.html)).
  Tam application: keep those links in `--help` and link from corpus pages rather than duplicating them.
- **reject**: Embedding documentation in shell completion scripts as a second corpus ([bash-completion README, search-result snippet, accessed 2026-11-26](https://github.com/scop/bash-completion)).
  Tam application: completions should point at identifiers that the catalog owns.
- **reject**: Attributing fuzzy or approximate completion to these systems: no primary source read here documents it ([gap, accessed 2026-11-26](https://fishshell.com/docs/current/completions.html)).
  Tam application: do not promise fuzzy matching based on this evidence.

## Things to avoid

- Do not make a completion list the only place a page name is defined: that creates two things to keep in sync.
- Do not decorate non-TTY output by default: tldr's specification explicitly forbids it ([CLIENT-SPECIFICATION.md](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md)).
- Do not treat Bash and fish details as full reads: both came from search-result snippets.
- Do not claim zsh completion behaviour was researched; its documentation was not read.

## Where to look

- All URLs below were accessed 2026-11-26; snippet-only sources are marked.
- [Search-result snippet] [bash builtins reference](https://www.linux.org/docs/man1/builtin.html) — `complete` and `compgen` sources and filters.
- [Search-result snippet] [fish completions](https://fishshell.com/docs/current/completions.html) — command-keyed completion pattern.
- [Search-result snippet] [bash-completion README](https://github.com/scop/bash-completion) — on-demand loading, XDG override path and symlinks.
- [Full page read] [GNU Coding Standards mirror, §4.7/§4.8](https://benpfaff.org/writings/blp-stds/standards.html) — `--help` and `--version` contract.
- [Full page read] [man(1)](https://man7.org/linux/man-pages/man1/man.1.html) and [apropos(1)](https://man7.org/linux/man-pages/man1/apropos.1.html) — help modes, exit codes and terminal width.
- [Full page read] [tldr CLI specification](https://raw.githubusercontent.com/tldr-pages/tldr/main/CLIENT-SPECIFICATION.md) — machine-readable output rules.
