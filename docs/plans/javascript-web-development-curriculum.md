# JavaScript and web development curriculum

**Plan status:** Completed
**Reviewed:** 2026-11-24
**Scope:** Corpus-only content for corpus 0.123.0: a Programming > Web
development hub, ten three-level teaching pages, a five-stage `web-development`
goal path, thirty glossary pages, five bibliography entries and the `web.*`
skill cells. No command, install or stage change.
**Authority:** The developer asked for a JavaScript and web development series
that takes a reader with no background to an expert full-stack web developer,
covering the language and its history, browser interpreters, browser
programming, Node, Electron and a complete web-development overview. The agreed
shape was ten level-tagged pages, a hub and a goal path, with all supporting
corpus bookkeeping on `dev`.
**Next action:** None. Closed 2026-11-24 at corpus 0.123.0; learner review,
installation and packaged acceptance remain developer-directed.

## Problem and agreed outcome

The programming subject had no connected route into web development. A reader
could find language and shell material, but the chain from JavaScript's origin
through browser engines, the DOM, asynchronous APIs, Node, server architecture,
security, tooling, verification and delivery was not written as one course. The
glossary lacked common web terms, the bibliography lacked the standards and
documentation those pages rely on, and the skills map had no web domain.

A reader who starts with no web background can now follow one path through ten
pages. Each page has a novice, learner and expert lens, a learner worked example
and a check question; the path gives each stage a lens, a checkpoint and one
reversible capstone. The final page reaches both deployed web applications and
Electron desktop packaging.

## Baseline and dependencies

Baseline on `dev`: command 0.69.1, corpus 0.122.3, three-level teaching pages
and goal paths with `meta paths` on step pages, the `skills.json` registry, and
the authoring rules in `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`
and `docs/corpus-style.md`. The `paths/web-development.html` page follows
`docs/templates/path.html` and the `validate_path_pages` contract in
`tools/build-index.py`. No command change is needed.

Examples were captured on 2026-11-24 from a Tamlinux workstation: Node.js
24.5.0, npm 12.2.1 and Python 3.14.8. Output in the worked examples is copied
from those runs; snippets marked illustrative are not presented as captured
output. Source facts were checked against MDN Web Docs, the Node.js and Electron
documentation, ECMA-262, WHATWG HTML, and encyclopedia summaries of the
language's history.

## Implementation and handoff

1. **Series (TCK31.1):** Add `programming/web-development/index.html` and ten
   teaching pages: JavaScript language and history, HTML/CSS, browser engines
   and the DOM, browser APIs and async, Node/npm, full-stack architecture, web
   security, frameworks and tooling, testing/debugging/performance, and
   deployment/Electron. Declare path steps 1–10 in order.
2. **Supporting corpus (TCK31.2):** Add thirty three-level glossary pages and
   regenerate the letter hubs. Add five bibliography pages with stable citation
   IDs and place them in every index view. Link the hub from the programming
   hub and add `web.novice`, `web.learner` and `web.expert` to the registry and
   skills map. Add `paths/web-development.html` with five stages, checkpoints
   and a capstone.
3. **Bookkeeping and verification (TCK31.3):** Bump `knowledge/version` to
   0.123.0, close this plan, update the plan index, roadmap and development
   contract, and write the work record. Regenerate indexes and run the corpus,
   repository and reader checks.

## Acceptance and limits

- `make corpus-and-test` passes with no new learning warning; the nine existing
  warnings are in the algorithms and computer-timescales pages.
- Every internal link, glossary target, citation ID and path step resolves. The
  path has five numbered stages with checkpoints, one capstone solution, next
  paths, and contiguous steps 1–10.
- Each teaching page declares novice, learner and expert sections. Novice text
  carries no goal, try-it or quiz; learner text carries a worked example, goal,
  try-it, failure note and quiz. Captured results name the host and date.
- `tam explain` finds the intended page for JavaScript, ECMAScript, DOM, Node,
  npm, Electron, REST, CORS, XSS and CSRF.
- No stage was pushed, tagged, installed or released. Learner review and
  packaged acceptance are separate developer-directed actions.

## Disposition

Delivered 2026-11-24 at corpus 0.123.0. The hub, ten pages, goal path,
glossary pages, sources and skills are corpus content only. Command 0.69.1 is
unchanged; stage promotion, installation and release need Fred's word.
