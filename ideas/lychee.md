# Lychee

## What it is

- **Format:** a Rust link checker for broken URLs and mail addresses in Markdown, HTML, reStructuredText and websites, available as a CLI, library and GitHub Action ([Lychee repository, full page read, accessed 2026-11-26](https://github.com/lycheeverse/lychee)).
- **Scope and licence:** async, stream-based checking with a documented feature matrix; licence and size were not checked in this study.
- **Evidence:** the repository README and feature matrix were read directly. No source was read for code-sample execution in documentation CI. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Validate links offline where possible: MkDocs rewrites relative `.md` links and generates heading anchors, so those derived targets are exactly where breakage starts ([MkDocs writing docs, full page read, accessed 2026-11-26](https://www.mkdocs.org/user-guide/writing-your-docs/)).
  Tam application: check relative page links, generated anchors and media targets in the normal corpus build.
- **adopt**: The documented feature model includes file globbing, include and exclude patterns, scheme filtering, config files and JSON output ([Lychee repository](https://github.com/lycheeverse/lychee)).
  Tam application: reuse the feature list as a checklist for a local validator, not its network stack.
- **adopt**: Lychee checks URL fragments and text fragments and resolves relative URLs ([Lychee repository](https://github.com/lycheeverse/lychee)).
  Tam application: anchor validity matters as much as page existence in a pager with deep links.
- **consider**: Library use and pre-commit hook support show that link checking can sit at several points in a workflow ([Lychee repository](https://github.com/lycheeverse/lychee)).
  Tam application: a corpus check is the first place to add it; an editor hook is optional.
- **reject**: Network-dependent checks in the regular test suite: throttling, retries, custom headers and status-code filtering exist because remote checks are flaky ([Lychee repository](https://github.com/lycheeverse/lychee)).
  Tam application: make external URL checks an explicit opt-in, separate from offline validation.
- **reject**: Importing an async HTTP stack into a documentation tool for links that are all local ([Lychee repository](https://github.com/lycheeverse/lychee)).
  Tam application: a small local checker is proportionate.

## Things to avoid

- Do not validate only HTTP URLs: MkDocs explicitly notes that links inside raw HTML escape Markdown parsing ([MkDocs writing docs](https://www.mkdocs.org/user-guide/writing-your-docs/)).
- Do not claim Lychee executes documentation code samples: no source was read for that.
- Do not treat comparisons with other checkers as evidence of relative performance.
- Do not make remote link checks block an offline build.

## Where to look

- All URLs below were accessed 2026-11-26.
- [Full page read] [Lychee repository](https://github.com/lycheeverse/lychee) — feature matrix, config, JSON output and fragment checks.
- [Full page read] [MkDocs writing docs](https://www.mkdocs.org/user-guide/writing-your-docs/) — link rewriting, heading IDs and raw-HTML limitation.
- [Gap] No source-read documentation of platform code-sample execution in CI was found.
