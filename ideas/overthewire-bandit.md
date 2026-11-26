# OverTheWire Bandit

## What it is

- **Format:** a browser-delivered wargame with numbered levels, one short page per level and a progression gate: finishing a level yields the information needed to start the next ([Bandit wargame landing page, full page read, accessed 2026-11-26](https://overthewire.org/wargames/bandit/)).
- **Scope and licence:** aimed at absolute beginners learning command-line basics; no licence or level count was verified.
- **Evidence:** the landing page was read directly; individual level pages were not fetched. Verdicts are research inferences, not approved work.

## Ideas worth taking

- **adopt**: Progression is enforced by an observable goal rather than by a quiz or score ([Bandit](https://overthewire.org/wargames/bandit/)).
  Tam application: unlock the next checkpoint when the sandbox reaches the expected state, not when the reader clicks "done".
- **adopt**: The beginner note lists recovery routes in order — manual pages, built-in help, a search engine and chat — before giving answers ([Bandit](https://overthewire.org/wargames/bandit/)).
  Tam application: add a "what to try when stuck" branch to checkpoints that fail verification.
- **adopt**: Each level page contains only enough information to start, not the full solution ([Bandit](https://overthewire.org/wargames/bandit/)).
  Tam application: show expected output and the goal, but leave the next command to the learner.
- **consider**: An explicit environment caveat offers a concrete fix for a known SSH failure under NAT ([Bandit](https://overthewire.org/wargames/bandit/)).
  Tam application: attach environment-specific failure notes to checkpoints that touch network or terminal setup.
- **consider**: An external "introduction to user commands" manual is linked for readers new to the command line ([Bandit](https://overthewire.org/wargames/bandit/)).
  Tam application: link to a glossary or beginner page from the first path, not from every step.
- **reject**: A separate quiz UI as the verification mechanism ([Bandit](https://overthewire.org/wargames/bandit/)).
  Tam application: a state check in a sandbox gives stronger evidence that the learner completed the task.
- **reject**: Treating the wargame structure as a general content model: it is one game with a single linear gate ([gap, accessed 2026-11-26](https://overthewire.org/wargames/bandit/)).
  Tam application: paths with branches and resumable state need a richer model than levels.

## Things to avoid

- Do not publish full solutions on a level page: Bandit's design deliberately withholds them ([Bandit](https://overthewire.org/wargames/bandit/)).
- Do not claim individual level wording or newer hint behaviour was checked; those pages were not fetched.
- Do not attribute quizzes or scoring to Bandit's landing page: none is described there ([Bandit](https://overthewire.org/wargames/bandit/)).
- Do not treat the SSH caveat as universal guidance; it is environment-specific.

## Where to look

- URL accessed 2026-11-26.
- [Full page read] [Bandit wargame](https://overthewire.org/wargames/bandit/) — level gate, beginner recovery routes and environment caveat.
- [Gap] Individual level pages were not fetched; "Linux Journey" was attempted but resolved to an unrelated host with an error, so no claims are made about it.
