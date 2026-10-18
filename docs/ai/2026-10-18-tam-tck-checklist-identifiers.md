# TCK checklist item identifiers

- **Date:** 2026-10-18
- **Time:** 10:28 to 10:50 -04:00
- **Stage:** Develop; documentation only, no version change
- **CLI Tool:** Claude Code 2.1.302
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** Fred Horch identified the identifier collision and chose
  the `TCK` prefix; Claude Code investigated the collision, proposed the
  rules and wrote the changes.
- **Checked against:** `docs/plans/knowledge-system-0.31.0.md`, `ROADMAP.md`,
  `docs/plans/README.md`, `docs/plans/development.md`,
  `docs/plan-maintenance.md`, and a search of every Tamlinux repository for
  existing `TCK` labels (none found)

## Guiding prompts

> It appears that we have two sets of K* checklist items. One is for the
> [redacted: private repository] repo and the other is for the tam repo. This is
> not going to work: the point of a unique checklist identifier is that it is
> unique. Please investigate and propose a remedy so that I an unambiguously
> refer to a checklist item.
>
> This problem surfaced as I am about to prompt an AI agent to work on
> tam/docs/plans/knowledge-system-0.31.0.md. The prompt code suggested refers
> to K1 through K5. Those conflict with K1 through K5 in
> [redacted: private repository path].

> Why not create a new prefix, such as TCK for tam command and corpus
> checklist items? Then we can use checklists in the tam repo and not have to
> use the wordier slice idea?

> I approve. Do all steps then report the results to me in this session.

## Key decisions

- The knowledge-system plan's slices `K1`-`K5` became `TCK1`-`TCK5`. Another
  Tamlinux checklist series already uses `K`, so a bare `K2` was ambiguous.
- Claude Code first proposed plain "Slice N" labels. The developer chose a
  repository-wide prefix instead, because a TCK identifier stands alone
  without naming its plan.
- `TCK` is one permanent series for the repository, never restarted per plan;
  [`CHECKLISTS.md`](../../CHECKLISTS.md) holds the rules and the counter.
- The 2026-10-18 knowledge-overhaul planning record keeps its original
  `K1`-`K5` wording; session records are not rewritten. `CHECKLISTS.md`
  notes the rename.

## Verification

`rg -n '\bK[0-9]+\b'` over the current plans, roadmap and development
contract finds no remaining `K` labels; the only matches are the two
intended mentions in `CHECKLISTS.md`. `make test` was not run: no test, tool
or build rule reads the changed files.
