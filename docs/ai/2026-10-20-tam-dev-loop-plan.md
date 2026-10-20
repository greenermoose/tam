# Developer loop plan: `make dev` and drift checks

- **Date:** 2026-10-20
- **Time:** 08:29 to 08:55 -04:00
- **Stage:** Develop; a Ready plan (TCK11, TCK12), no code or version change
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record and the plan
- **Authorship:** The developer described his editing workflow and asked for
  a deterministic `make` flow that an agent could implement. He chose the
  options below. The AI tool read the build and corpus tooling, proposed the
  design, and wrote the plan and its entry-point updates.
- **Checked against:** `Makefile`, `tools/build-index.py`
  (`git_page_revision`, `.build_id`), `AGENTS.md`, `src/version.h`,
  `knowledge/version`, `tests/run.sh`, `ROADMAP.md`,
  `docs/plans/development.md`, `docs/plan-maintenance.md`, `CHECKLISTS.md`,
  `tam-work status --json`, the `tamlinux-packages` tam recipe

## Guiding prompts

> I have agy working on tam right now, but it makes me realize that I don't want to rely on AI agents whenever I have an update to the tam knowledge corpus to make. But the reason I kicked off an agy session is that there are too many fiddly bits with trying to compile and install the tam command and corpus manually. I'd like you to ponder this and propose a fix to the makefile and associated scripts to make the deterministic make process work better for me. I'll explain my development process and what I want to happen. Your job is to plan a project that agy can implement that will make the deterministic scripts do what I want so I don't need to drag in an AI agents just to help me develop the tam knowledge corpus.
>
> I use vs code to edit files in the knowledge folder of the tam repo. I want to spend my time thinking about what I want to say and editing those files. Once I've done a bunch of edits and saved them, the repo will be in a dirty state. Using git, it should be possible for a script to easily see exactly which files I've edited and exactly which changes I've made since the last commit. I might also edit some files in the src folder to change how the tam command works. All of these changes should be easy for a script to see using git.
>
> After making the set of changes that I want to compile into a version of tam and test, I'd like to run a simple command, such as make. That command should handle the commit, compiling the code, and installing it for me. It can ask for things like the commit message and confirm the installation directory. But to kick off this commit, compile and install process I shouldn't have to do anything more than type make and then answer prompts.
>
> Do you understand the gist of this request? Any questions?

> What about editing other files like markdown? There are a bunch of things in the repo I'm not sure I fully understand and can start to drift.

> Keep the plan in the repo. Then tell me how I should prompt agy to build it.

## Key decisions

- The developer chose a new `make dev` target over changing plain `make`, so
  packaging and agents keep a non-interactive build.
- Paths under another session's active `tam-work` claim are excluded and
  listed, rather than refusing to run or picking files by hand.
- Versions are proposed with a default. The plan offers patch or minor but no
  "none" for a lane whose content changed, following the developer's
  2026-10-18 rule that changed code is never delivered under a reused version.
- Pushing `develop` is asked every time.
- Drift is caught by a checker of copied facts plus a repository map, not by
  scripts rewriting prose.

## Verification

Documentation only. Every relative link in the changed planning documents
resolves; `docs/plans/README.md` has one row per plan; the version statements
match `src/version.h` and `knowledge/version`; `git diff --check` is clean.
