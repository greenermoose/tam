# Foundational glossary (corpus 0.44.0)

- **Date:** 2026-10-27
- **Time:** 09:42 to 09:49 -04:00
- **Stage:** Develop; corpus source
- **CLI Tool:** Codex CLI 0.164.0
- **Model:** GPT-6.1 Terra (`gpt-6.1-terra`)
- **Transcript:** Retained privately by the author.
- **Commits:** The commit that adds this record and the index follow-up.
- **Authorship:** Fred directed the priority; Codex wrote the corpus and record.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, local `bash(1)`, `hier(7)`, `proc(5)`, existing glossary and introductory pages.

## Guiding prompts

> Do a read-only audit to identify corrections and sources. Also suggest additional pages. The overall improvement I want to make is to introduce and define concepts more clearly. Too many pages just jump in assuming people understand a random bunch of tecnical terms. And we need to greatly expand the glossary and link to it from the Guide Index page. We have Guide topics but no glossary in the list of Appendices and More. Another thing that is hanging is the branch that was working on getting external links to work. Claude accidentally did an rm -r of a whole folder that wasn't backed up, so that stopped work on that branch. Hopefully we can finish that up without another mishap like that.

> agy is done. Pick your top priority and begin work on it now.

> Is there a folder of work plans and a ROADMAP for the tam repo? Please review them both and update them. If either one does not exist, please let me know and we can decide how to proceed.

## Key decisions

Start with common first-step vocabulary and link definitions at their first relevant use. Keep the existing three-level glossary format. Correct two overbroad filesystem statements while introducing the basics. Reconcile the roadmap, plan index and development contract, and bring the external-links handoff into the plan inventory without modifying or deleting its branch.

## Verification

`make corpus-and-test` passed: index generation, corpus lint, 52 unit tests and Guide acceptance checks. Inspected rendered Guide, terminal definition and shell introduction with the checkout corpus. Learner and packaged acceptance remain pending. Planning review compared the plan inventory and cached branch version files; documentation changes were checked for links and whitespace.
