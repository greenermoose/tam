# Session: 2026-10-13 — Ecosystem chapters in the guide; public-content cleanup

- **CLI tool:** Claude Code `2.1.296`
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Authorship:** AI-written pages, tests and documentation edits, directed and
  reviewed by Fred. No delegation.

## User direction

Fred's guiding prompts, verbatim:

> I think we are building a knowledge corpus for the tam command in two places: 1) the tam repo, and 2) the ecosystem-fred-tamlinux repo. Please look into that and give me your thoughts about what should be done.

> agy was working in the ecosystem repo but it should be done now. If you're able to, please claim the paths and do steps 1, 2 and 3.

> Make sure you don't bleed private ideas and plans into any public repos.

> Remove any private info that agy inadvertently put in a public repo. You don't have to do surgery to remove history, but make sure the latest version of each public repo is clean.

> agy is done so you should be able to work on any file you need.

## Decisions and implementation

1. **One corpus.** `tam` reads a single knowledge root, so the ecosystem
   chapters drafted in `ecosystem-fred-tamlinux` could only replace this
   guide. They now live in `knowledge/ecosystem/` (12 pages, Improve and
   Maintain tracks) with short topic names (`compositors`, `init`, `libc`,
   …), linked from the guide index (commit `a067971`). The ecosystem
   repository keeps the registry and links here.
2. **Public sources only.** The pages were rewritten from the public
   `tamlinux` README and plans and from upstream documentation. Undecided
   survey recommendations are labelled as recommendations.
3. **Renderer subset.** Root-relative links and only supported entities
   (`&larr;` and `&times;` rendered literally and were removed).
4. **Private references removed.** Links to a private report, decision-record
   numbers, private-repository paths and a machine-local path were removed
   from `README.md`, `ROADMAP.md`, `docs/plans/` and two earlier session
   records (commit `a0f0015` and the commit adding this record). Quoted
   prompts are redacted, not paraphrased.

## Verification

- `make test`: all command and guide tests pass; `guide.sh` renders,
  link-checks and snapshots every new page; `run.sh` adds search checks for
  `sway`, `runit` and `carried patch` and updates the pager Shift-Tab
  sequence for the two new index links.
- A repository-wide search finds no private-repository references, decision
  numbers or machine-local paths.
