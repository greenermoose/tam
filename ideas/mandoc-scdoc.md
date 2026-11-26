# mandoc and scdoc

## What it is

- **Format:** mandoc is a small ISO C, ISC-licensed suite that parses mdoc and man macro languages and renders terminals, HTML5 and Markdown; scdoc is a C99 generator whose Markdown-like source produces ordinary roff ([mandoc homepage, full page read, accessed 2026-11-26](https://mandoc.bsd.lv/); [scdoc(5), full page read, accessed 2026-11-26](https://man.archlinux.org/man/scdoc.5.en)).
- **Scope and size:** mandoc lists version 1.14.6 dated 2021-09-23 with a 2026 file timestamp and ships `man`, `apropos`, `makewhatis`, `man.cgi`, `mandocd` and `catman`; scdoc is listed at 1.11.5-1 and was under 1,000 lines of C99 in 2018 ([mandoc homepage](https://mandoc.bsd.lv/); [Introducing scdoc, historical 2018 post, accessed 2026-11-26](https://drewdevault.com/blog/scdoc)).
- **Evidence:** the mandoc homepage, scdoc man page and the author's 2018 rationale were read directly; `mandoc(1)` and `mdoc(7)` full texts were not. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: A friendly source format can generate a standard output format at build time, so readers need no special viewer ([scdoc(5)](https://man.archlinux.org/man/scdoc.5.en); [Introducing scdoc](https://drewdevault.com/blog/scdoc)).
  Tam application: keep readable sources and generate HTML and index data rather than maintaining both by hand.
- **adopt**: mandoc's `libmandoc` is a validating compiler, making validation part of authoring rather than an afterthought ([mandoc homepage](https://mandoc.bsd.lv/)).
  Tam application: a validating corpus lint should catch malformed links and headings before build.
- **adopt**: scdoc uses a simple preamble, `#`/`##` headings with blank lines, bold and underline, tab indentation, lists, tables, fenced literal blocks, comments and UTF-8 input ([scdoc(5)](https://man.archlinux.org/man/scdoc.5.en)).
  Tam application: a small, explicit source grammar is easier to lint than free-form HTML embedded in pages.
- **consider**: One mandoc source can target terminal, HTML5 and Markdown output ([mandoc homepage](https://mandoc.bsd.lv/)).
  Tam application: one page source could serve the pager, a generated HTML site and a search export.
- **consider**: mandoc's `makewhatis` builds a `mandoc.db` index, and third parties use mandoc for manual servers ([mandoc homepage](https://mandoc.bsd.lv/)).
  Tam application: build the search index alongside rendered pages and treat it as derived data.
- **consider**: scdoc's author cites full UTF-8 support as removing a blocker to localisation ([Introducing scdoc, historical](https://drewdevault.com/blog/scdoc)).
  Tam application: choose source and renderer rules that do not presume ASCII if translation is ever explored.
- **reject**: A proprietary runtime reader for the generated output: scdoc's output is ordinary man source ([scdoc(5)](https://man.archlinux.org/man/scdoc.5.en)).
  Tam application: standard, inspectable output beats a custom viewer format.
- **reject**: Hand-writing roff-like markup when a readable source can compile to it ([Introducing scdoc, historical](https://drewdevault.com/blog/scdoc)).
  Tam application: authoring friction should not discourage small corrections.

## Things to avoid

- Do not claim mandoc's current release is newer than the homepage's listed 1.14.6; a 2026 file timestamp is not a version.
- Do not quote automatic tagging or lint diagnostics in detail: `mandoc(1)` and `mdoc(7)` were not source-read.
- Do not present authoring-effort claims as measured: the notes call them inferences from format rules.
- Do not treat the 2018 scdoc rationale as current project status; it is explicitly historical.

## Where to look

- All URLs below were accessed 2026-11-26; the 2018 post is labelled historical.
- [Full page read] [mandoc homepage](https://mandoc.bsd.lv/) — formats, outputs, tooling, version and third-party users.
- [Full page read] [scdoc(5)](https://man.archlinux.org/man/scdoc.5.en) — source syntax and ordinary roff output.
- [Source repository] [scdoc repository](https://git.sr.ht/~sircmpwn/scdoc) — release activity.
- [Historical, 2018] [Introducing scdoc](https://drewdevault.com/blog/scdoc) — design rationale, size and UTF-8 motivation.
- [Gap] `mandoc(1)` and `mdoc(7)` full texts were not fetched.
