# Linux man pages and man-db

## What it is

- **Format:** roff source rendered by man-db or mandoc, with standard sections 1–8 for user commands, system calls, library calls, special files, file formats, games, overviews and system management; section 9 for kernel routines exists in man-db but is non-standard ([man-pages(7), full page read, accessed 2026-11-26](https://man7.org/linux/man-pages/man7/man-pages.7.html); [man(1), full page read, accessed 2026-11-26](https://man7.org/linux/man-pages/man1/man.1.html)).
- **Licence and scope:** the man-pages project documents kernel and C-library interfaces plus conventions; no licence statement was collected here. Debian Policy requires a man page for each program, utility or function ([Debian Policy §12.1, full page read, accessed 2026-11-26](https://www.debian.org/doc/debian-policy/ch-docs.html)).
- **Evidence:** man7.org pages and Debian Policy were read directly; the full man-db manual timed out. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: The `NAME` line feeds a precomputed one-line description index used by `whatis`, while exact name lookup with an optional section is the fast path before any search ([man-pages(7)](https://man7.org/linux/man-pages/man7/man-pages.7.html); [whatis(1), full page read, accessed 2026-11-26](https://man7.org/linux/man-pages/man1/whatis.1.html)).
  Tam application: store one canonical summary per page in metadata and let an exact hit bypass ranking.
- **adopt**: Cross-references are written as `name(section)` in a comma-separated `SEE ALSO` ordered by section then name, giving tools a stable link target ([man-pages(7)](https://man7.org/linux/man-pages/man7/man-pages.7.html)).
  Tam application: make link targets IDs rather than display strings, and resolve aliases to a canonical page.
- **adopt**: Fixed heading slots include `OPTIONS`, `EXIT STATUS`, `ENVIRONMENT`, `FILES`, `CAVEATS`, `EXAMPLES` and `SEE ALSO`, with `CAVEATS` describing typical misuse ([man-pages(7)](https://man7.org/linux/man-pages/man7/man-pages.7.html)).
  Tam application: reserve a safety slot for destructive operations rather than hiding warnings in prose.
- **adopt**: Semantic newlines and roughly 75-character source lines keep patches reviewable ([man-pages(7)](https://man7.org/linux/man-pages/man7/man-pages.7.html)).
  Tam application: prefer a source convention that produces small, readable diffs.
- **adopt**: Example programs should be complete, error-checking and warning-free, with shell sessions indenting typed input ([man-pages(7)](https://man7.org/linux/man-pages/man7/man-pages.7.html)).
  Tam application: make example hygiene part of corpus lint and review.
- **adopt**: `apropos` exits 16 when nothing is found, distinct from usage and operational errors ([apropos(1), full page read, accessed 2026-11-26](https://man7.org/linux/man-pages/man1/apropos.1.html)).
  Tam application: give "no match" its own documented exit status instead of treating it as a failure to run.
- **adopt**: man-db joins separate words to subcommand pages, so `man git diff` resolves to the hyphenated page ([man(1)](https://man7.org/linux/man-pages/man1/man.1.html)).
  Tam application: map spaced subcommand names to corpus pages without forcing users to know the file name.
- **consider**: mandoc's `apropos` queries semantic macro keys such as name, description, options, environment variables, files and cross-references, combined with Boolean operators ([mandoc apropos(1), full page read, accessed 2026-11-26](https://mandoc.bsd.lv/man/apropos.1.html)).
  Tam application: tag options, files and glossary terms in page metadata to allow fielded search without full-text scanning.
- **consider**: man honours terminal width while keeping redirected output stable ([man(1)](https://man7.org/linux/man-pages/man1/man.1.html)).
  Tam application: keep pager layout terminal-aware but never decorate machine-readable output.
- **reject**: Full-text search as the primary discovery path: `man -K` is explicitly brute-force over source and can hit comments or miss escaped hyphens ([man(1)](https://man7.org/linux/man-pages/man1/man.1.html)).
  Tam application: exact name and fielded keywords first; full text is a last resort if used at all.
- **reject**: Requiring documentation to be installed for the program to work, forbidden by Debian Policy ([Debian Policy §12](https://www.debian.org/doc/debian-policy/ch-docs.html)).
  Tam application: a missing page must degrade gracefully rather than break the command.
- **reject**: Shipping preformatted output as the only source, also forbidden by Debian Policy ([Debian Policy §12](https://www.debian.org/doc/debian-policy/ch-docs.html)).
  Tam application: keep source and generated indexes separate.

## Things to avoid

- Do not date claims in page prose: use a source or bug link instead, as ArchWiki's style rule also advises ([man-pages(7)](https://man7.org/linux/man-pages/man7/man-pages.7.html)).
- Do not explain general Unix concepts in every page; tldr's contributor rules exclude them and a glossary is the natural home ([tldr CONTRIBUTING.md, full page read, accessed 2026-11-26](https://raw.githubusercontent.com/tldr-pages/tldr/main/CONTRIBUTING.md)).
- Do not treat translated pages as current unless they are visibly marked when outdated ([Debian Policy §12.1](https://www.debian.org/doc/debian-policy/ch-docs.html)).
- Do not claim fuzzy or relevance-ranked search is a man-page convention: no primary source read here documents it.

## Where to look

- All URLs below were accessed 2026-11-26.
- [Full page read] [man-pages(7)](https://man7.org/linux/man-pages/man7/man-pages.7.html) — sections, headings, synopsis notation, links, newlines and examples.
- [Full page read] [man(1)](https://man7.org/linux/man-pages/man1/man.1.html) — lookup order, subpages, search modes, configuration and sandbox.
- [Full page read] [whatis(1)](https://man7.org/linux/man-pages/man1/whatis.1.html) and [apropos(1)](https://man7.org/linux/man-pages/man1/apropos.1.html) — index modes, filters and exit codes.
- [Full page read] [mandoc apropos(1)](https://mandoc.bsd.lv/man/apropos.1.html) — macro-key query language.
- [Full page read] [Debian Policy §12](https://www.debian.org/doc/debian-policy/ch-docs.html) — installation paths, compression, registration and the rule that programs must work without docs.
- [Gap] The full man-db manual at `https://www.nongnu.org/man-db/manual/man-db.html` timed out.
