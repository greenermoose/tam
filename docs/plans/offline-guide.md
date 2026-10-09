# tam offline guide — slice 2 plan

**Stage:** Develop — **implemented 2026-10-09.** Milestone 2 of the
[development plan](development.md): the offline guide (`browse`, `--pipe`,
corpus lookup and the viewer).

**Decision (Fred, 2026-10-09):**

1. **Native renderer.** `tam` renders the guide itself natively, with no
   external browser dependency.
2. **Text is derived at runtime** from the same HTML by `tam`'s renderer;
   committed `.txt` files are golden test fixtures for review and drift
   detection, not a second authored corpus.
3. **File the plan, then implement.**

## 1. Why a native renderer rather than an external browser

- `tam` must work before the graphical desktop and on minimal / musl / legacy
  Void, without requiring any external browser package.
- We author the corpus, so we control the HTML subset: `tam` needs a parser for
  *our* tags, not a general browser.
- `libtam`'s anti-bloat premise favours a self-contained C renderer.
- foot (v1.28.0 here) renders SGR/truecolour and OSC 8 natively, so output can
  match Tamlinux theming better than a foreign `.lss`.
- `tam_exec` cannot drive an interactive browser anyway: it captures
  stdout/stderr and imposes a timeout (`libtam/src/tam_exec.c`), which the
  [development plan](development.md) already forbids for interactive browsing.
  The native in-process renderer and pager avoid subprocess execution entirely.

## 2. Content: one source of truth

- Canonical, hand-authored **semantic HTML5** in `tam/knowledge/`, installed to
  `<prefix>/share/tamlinux/knowledge/`.
- **Plain text is derived at runtime** by the renderer for `--pipe` and any
  non-TTY stdout; it contains no escape sequences. Committed `.txt` snapshots
  under `tests/snapshots/` are golden fixtures: a test regenerates them and
  fails on drift, so authors review the rendered text in a diff while the HTML
  stays the only thing they edit.

### Supported HTML subset

`<title>`, `<h1>`–`<h3>`, `<p>`, `<ul>`, `<ol>`, `<li>`, `<pre>`, `<code>`,
`<blockquote>`, `<hr>`, `<br>`, `<a href>`, `<em>`/`<i>`, `<strong>`/`<b>`,
entities (`&amp; &lt; &gt; &quot; &nbsp; &mdash; &ndash; &hellip;`, plus
numeric). Unknown tags are ignored; `<script>`/`<style>` contents are skipped;
nothing is executed. Links in text mode become a numbered `Links:` appendix;
external links are labelled `external` and are never fetched.

## 3. Display: one parse tree, two front ends

- **Text emitter** (`src/render.c`): the interactive pager wraps to the
  terminal width (`TIOCGWINSZ`, then `COLUMNS`, then 80); `--pipe` and any
  non-TTY stdout wrap to a fixed 80 columns so snapshots and downstream tools
  are deterministic. Renders lists/headings/quotes/pre and appends the link
  list.
- **Interactive pager** (`src/pager.c`): raw `termios`, alternate screen,
  scrolling (`j`/`k`/space/`b`/`g`/`G`/arrows/`Home`/`End`/`PgUp`/`PgDn`),
  link selection (`Tab`, digits) and follow (`Enter`, right arrow), history
  back (`u`, left arrow), quit (`q`, `Esc`). Local `href`s navigate
  in-process; external links are shown, never opened, keeping the guide
  offline. OSC 8 is reserved for external links a user may want to open
  elsewhere.
- The renderer stays in `tam/src/` for this slice; it moves to a `libtam`
  module only if the upgraded-`man` idea later needs to share it.

## 4. Corpus location and packaging

Resolution order (no fixed `/usr/share` assumption): `$TAM_KNOWLEDGE_DIR`
(development override) → `$XDG_DATA_HOME/tamlinux/knowledge` →
`$XDG_DATA_DIRS/tamlinux/knowledge` → compiled `<prefix>/share/tamlinux/knowledge`.
A missing corpus produces a truthful diagnostic while `--help` still works.
Relative `href`s resolve against the corpus root; `..` escapes, absolute paths
and network URLs are refused. `PREFIX`/`DESTDIR` staging must relocate cleanly.

## 5. Work items

1. ✅ Corpus + subset spec + first pages: `index`, `welcome`, `install/`,
   `shell/pipelines`, `man/`.
2. ✅ Corpus locator and diagnostics.
3. ✅ `render.c`: tokeniser, entity decoding, layout, text emitter.
4. ✅ `pager.c`: interactive viewer over the rendered lines.
5. ✅ `show` wiring (originally `browse`): `tam show [page]`; `--pipe` / non-TTY → text.
6. ✅ Tests: golden text snapshots, link-graph resolution, control-free pipes,
   relocated staged install; verified clean under ASan/UBSan.
7. ⛔ External browser accelerator dropped: native renderer and pager handle
   display completely self-sufficiently with zero external dependencies.
8. ✅ Docs: milestone 2 marked in `development.md`; `docs/ai/` record added.

## 6. Acceptance

- Piped output and `--pipe` are readable and free of escape sequences.
- Missing corpus / missing page produce useful diagnostics; `--help` unchanged.
- Relative guide links resolve after a staged install away from the source tree.
- Opening the guide fetches no network content.
- The renderer is clean under `-Wall -Wextra -Wpedantic` and ASan/UBSan.
- Startup, binary size and RSS are measured before any performance claim.
- The pager is verified in foot (SGR styling and link navigation).
