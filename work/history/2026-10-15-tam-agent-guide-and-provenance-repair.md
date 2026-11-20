# Agent guide, lane-safe releases (0.25.1) and provenance repair

- **Date:** 2026-10-15
- **Stage:** Develop — command 0.25.1 and corpus 0.25.1.
- **CLI Tool:** Claude Code (`claude`) 2.1.301
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `77f1800`, `739b508`, `19225c0`, `2fdfd75`, and the commit that adds this record.
- **Authorship:** Fred asked for a review of how Antigravity and OpenCode had developed tam, then approved four follow-up changes. Claude Code read their session histories and made the changes.

## Guiding prompts

> Take a look at agy's and opencode's session history to learn how tam development has been done. I want to make sure we can continue to develop tam and our knowledge corpus with opencode and agy.

> I find worktrees a waste of time and tokens. What seems to work much better is coordinating among agents in the same repo.

> Do all four

## What the review found

- Agents worked well in the one shared clone with `tam-work` path claims;
  each session rediscovered the conventions because the repository had no
  agent guide.
- A corpus release had to edit command-lane and shared files
  (`src/version.h`, `tests/run.sh`, `tests/guide.sh`).
- `make test` aborted wherever no local Ollama server was running.
- Several sessions left no record or no commit trailers, and some records
  gave the wrong agy version. One record held a full conversation ID, and
  public plans, records and corpus pages named a private repository or
  showed home paths and the host name.

## Changes

- `AGENTS.md` (`19225c0`): one page for agents, covering plans, shared-clone
  claims instead of worktrees, lanes, versions, build, test, development
  install, branches and provenance.
- Command 0.25.1 (`77f1800`): the tests read the corpus version from
  `knowledge/version` and discover pages; the fallback corpus label is
  `unversioned`; the live `tam ask` checks skip visibly without a server.
- Corpus 0.25.1 (`739b508`, `2fdfd75`): the package-versions page names
  `knowledge/version` as the only corpus version source; examples use
  generic paths and host names.
- Provenance: removed the private references (`2fdfd75`); corrected nine
  records' agy versions from the agy process logs; added the five missing
  session records and their `AI_PROVENANCE.md` rows.

## Verification

`make test` passes both suites, with the live `tam ask` checks reported as
skipped because no Ollama server was running. The public provenance check
from the author's guide finds no IDs, store paths, home paths or private
repository names in `AI_PROVENANCE.md` or `docs/ai/`.
