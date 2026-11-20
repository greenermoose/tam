# Local tam repository and command contract

- **Date:** 2026-10-08
- **Stage:** Develop — local scaffold and planning.
- **CLI Tool:** Codex CLI 0.162.0
- **Model:** `gpt-6.1-sol`
- **Transcript:** Retained privately by the author.
- **Commits:** Initial repository scaffold and approved development plan.
- **Authorship:** Fred supplied the goal and command behavior; Codex prepared
  the repository scaffold and documentation. No delegation.

## Guiding prompts

> Let's start work on the tam command. I think we should create a new repo called tam and put all of our work in that. Do you agree? If not, where should the development files be stored? How should we start? If you don't know what the tam command is for, read the repos and ask me questions.

> Bare tam should print a concise intro and exit. tam browse should open the guide.

> I approve the plan. Please commit your work and close the window showing the plan.

## Decisions and scope

- Prepare a separate local component repository for command source, bundled
  knowledge, tests and development documentation.
- Bare invocation prints concise orientation and exits 0; explicit `browse`
  opens the guide. Bare invocation does not write first-use state.
- Use C with the separate `libtam` dependency, following the selected language
  direction. This replaces the earlier Python plan.
- Product assembly and installer orchestration stay in the `tamlinux` product.
- The first milestone implements introduction/help/version; browsing,
  explicit welcome, curated explanation and read-only inspection follow.

## Verification

The scaffold is a documentation artifact; no executable has been implemented
or installed. Review checks cover whitespace, local Markdown links, command
contract consistency and public provenance privacy. Automated command tests
and performance measurements belong to the implementation milestones.

Fred approved the development plan on 2026-10-09. The plan was reread from
disk before committing; the Micro review window was closed normally.
