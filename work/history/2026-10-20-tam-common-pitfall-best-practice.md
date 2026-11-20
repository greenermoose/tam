# Common pitfall sections become Best practice (corpus 0.35.2)

- **Date:** 2026-10-20
- **Time:** 17:12 to 17:36 -04:00 (OpenCode); completed 17:55 to 18:10 -04:00 (Claude Code)
- **Stage:** Develop; corpus 0.35.1 to 0.35.2
- **CLI Tool:** OpenCode 1.19.0; Claude Code 2.1.304 for the completion
- **Model:** big-pickle (`opencode/big-pickle`); Claude Opus 5.5 (`claude-opus-5-5`) for the completion
- **Transcript:** Retained privately by the author.
- **Commits:** `91f44a2` (style-audit SOP and audit report), `56cc928` (plan),
  `ae1fa41` (16 pages), all by OpenCode without AI trailers; `ca25c8f` (index
  follow-up) and the commit that adds this record, by Claude Code
- **Authorship:** Fred asked for the audit, the plan and the page changes.
  OpenCode wrote `docs/style-audit.md`, the audit report, the plan and the page
  edits, then installed a development build from an uncommitted index. Claude
  Code added the missing index commit, the plan's required header and index
  row, the corpus 0.35.2 bump and this record.
- **Checked against:** the OpenCode session store (prompts, model, times),
  `git show` of the three OpenCode commits, `grep` over `knowledge/`,
  `docs/plan-maintenance.md`, `docs/plans/README.md`

## Guiding prompts

To OpenCode:

> In our tam knowledge corpus, several pages have a section called "Common pitfall" that says something like "Never do X." first and then second explains what to do. This is an anti-pattern. This section should instead be "Best practice" and explain what you SHOULD do first, and then explain what bad things happen if you do it another way. Do an audit for pages with this Common pitfall section and prepare a report so that a future agent can go through and fix all these pages. Ask if you have questions. There should be an SOP for how to do an audit and where to file your report. But if there is not a SOP for doing this kind of audit, let me know so I can create that SOP for you.
>
> Draft both the style-audit SOP and the report. Where are you planning to file both of those?
>
> Have you committed your work and released your work claims?
>
> Commit your work.
>
> I see the audit. Is there a work plan for fixing the pages found by the audit?
>
> Create a work plan for fixing these pages.
>
> Do the work in the plan.
>
> Build and install the new version of the tam knowledge corpus. What is the version number now?
>
> Are you done?

To Claude Code, after its report of what OpenCode left unfinished:

> Yes, fix all three and then reinstall

## Key decisions

- Corrections take the corpus patch bump, so 0.35.2; OpenCode had left the
  changed pages at 0.35.1.
- OpenCode's commits are not amended to add trailers; this record and the
  ledger carry the attribution. Its audit record keeps its original header.

## Verification

`grep -rn "<h3>Common pitfall" knowledge/` finds nothing and 16 pages carry
`<h3>Best practice</h3>`. `make test` passes, including `check-repo`. The
development install reports corpus 0.35.2 with a clean build id.
