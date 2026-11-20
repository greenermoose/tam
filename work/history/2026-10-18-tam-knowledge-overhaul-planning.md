# Knowledge-system overhaul and plan maintenance (planning)

- **Date:** 2026-10-18
- **Time:** 08:09 to 10:23 -04:00
- **Stage:** Develop — planning and repository documentation only; no component version change.
- **CLI Tool:** Codex CLI `0.163.0`, checked from the running binary and session metadata.
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`), checked against the active turn metadata.
- **Transcript:** Retained privately by the author.
- **Commits:** The commit adding this record contains the developer-reviewed plan, maintenance procedure and reconciled documentation.
- **Authorship:** Fred specified the five-part spine, approved shared-page learning routes and arbitrary manual lookup inside the guide, and requested a complete planning reconciliation. Codex inspected the corpus/reader, proposed the design, and wrote the plan, procedure, dispositions and handoff. No product implementation is claimed.
- **Checked against:** `knowledge/version`, all 50 HTML pages' metadata and the root/hubs/paths/manual lesson; `tools/build-index.py`, `src/version.h`, `src/tam.c`, `src/index.c`, `src/depth.c`, `src/pager.c`, `src/man.c`, `src/info.c`, `Makefile`, `AGENTS.md`, every existing plan in `docs/plans/`, and the implementation/verification records cited in their new preambles.

## Guiding prompts

> Our tam knowledge corpus needs a complete overhaul to align with the idea of a five-part spine:
>
> 1) installing
> 2) learning
> 3) using
> 4) maintaining
> 5) improving
>
> Examine how corpus version 0.30.0 organizes our knowledge now. Ponder a better organization that does a better job helping users find relevant knowledge and progress from beginner to intermediate to expert.  Also consider a better way to integrate man pages and info pages. When I'm browsing the GNU Info Directory it's not clear how to return to the Tamlinux User Guide (the top level breadcrumb row is not displayed and the bottom row shows the GNU Info Directory commands, but ` still returns you to the Tamlinux User Guide). Also, it's not clear how to start using man from the guide.  Once you have thought about all that, share your ideas with me so we can go back and forth and make sure we're on the same page about what to do to improve the overall organization and usability of Tamlinux's knowledge system. Then update the plans and ROADMAP for tam so another agent can do the work to implement your plan and won't be confused by past thinking that we are reconsidering and replacing now.

> 1 yes
> 2 yes
>
> This is excellent. Please proceed. The goals of this session are to
>
> a) make one authoritative work plan to overhaul the knowledge corpus to version 0.31.0
> b) reconcile ROADMAP and the development plan
> c) propose and implement a procedure to supersede and retire plans when they change or are achieved (plan maintenance procedure)
> d) apply that plan maintenance procedure to the tam repo
> e) create an unambiguous handoff for an agent that can implement your work plan for corpus 0.31.0
>
> Ask if you have questions

> this looks good. Pleaes commit. Close the review window. Finish up any loose tasks. I want to close this session and handoff implementation of the work plan to a new session. Please advise on how to do that and let me know when I can close this session.

## Key decisions

- One authoritative corpus 0.31.0 plan includes the next-agent handoff, all 50 existing page dispositions, required new pages, and K1–K5 dependencies/acceptance. Companion command target 0.30.0 supplies navigation and in-reader discovery. Versions remain independent and unchanged during planning.
- Preserve public page/plan addresses where practical. Logical parent metadata supports the five parts without a filesystem rename. Retain show/explain separation, native Info traversal and portable rendering.
- Retire the seven-part/three-track organization and obsolete scaling/scheduling proposals as authorities. Parser, installer, stateful progression and wider speculative work remain deferred without reserved versions.
- Implement plan maintenance through a repository procedure, explicit statuses/evidence/replacements, a complete index, templates and agent/authoring entry points. Preserve original design bodies as labeled history.
- Keep portable-pager physical acceptance open: `581822a` and its tests landed, but the cited record leaves actual packaged Linux-console/Foot review pending. Snapper is already corpus 0.30.0 (`d13c1f6`); close its formerly open note using the commit's reported checks, without claiming a fresh test run.
- The developer reviewed the plan in Micro and approved committing the documents. The review window was closed normally, and the plan was reread from disk before finalizing the change. Product builds, installs, deployments and promotions remain outside this documentation task.

## Verification

- Documentation consistency check: all 18 plans/templates have required metadata and exactly one matching index row (11 Completed, 2 Superseded, 1 Active, 1 In progress, 2 Template, 1 Ready). Local Markdown file links resolve across changed planning/routing documents and the provenance record.
- Independent page-set comparison: all 50 current HTML pages occur exactly once in the disposition map (1 root, 20 Learning, 1 Installing, 5 Using, 11 Maintaining, 12 Improving).
- Completion/disposition commit references resolve with `git cat-file -e`; new files pass `git diff --no-index --check`, and tracked changes pass `git diff --check`.
- Current version headers match command 0.29.3 / corpus 0.30.0. No source, corpus, test, tool, Makefile or component-version file changed. Product tests were not rerun for this documentation-only change.
- Public-boundary scan of changed/new files passed; the session mapping was recorded privately. The developer approved the reviewed documents for commit; no push, install or deployment was requested or performed.
