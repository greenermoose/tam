# Refresh curated tldr pages (corpus 0.56.1)

- **Date:** 2026-11-01
- **Time:** 10:14 to 11:10 -05:00
- **Stage:** Develop, corpus 0.56.1
- **CLI Tool:** Cursor `3.27.22` (from `cursor --version`)
- **Model:** Composer (`composer`). This session identified itself as Auto, powered by Composer; no finer model id was logged by the CLI.
- **Transcript:** Retained privately by the author.
- **Commits:** (filled after commit)
- **Authorship:** Fred directed a refresh of the curated offline `tldr` pages, a check for new pages to add, fixes for two pre-existing `make check-repo` failures, verification, and the commit. Cursor curated and copied upstream pages, bumped the corpus, updated the roadmap, fixed the plan docs, and wrote this record.
- **Checked against:** `docs/agent-runbooks/refreshing-tldr-pages.md`, `knowledge/tldr/SOURCE.json`, upstream clone `tldr-pages/tldr` at `ba104601`, installed tools on the writing host, Tamlinux guide HTML references, `tamlinux/docs/plans/FILING-CARD.md`

## Guiding prompts

> I believe some tldr pages are installed in Tamlinux. Check if any of those pages have upstream updates and need to be update in the Tamlinux package. Also, explain to me how we package tldr pages for inclusion in Tamlinux?

> Refresh the tldr pages. Check to see if any new tldr pages should be added to our knowledge corpus.

> agy is done. Proceed.

> Fix the pre-existing issues. Verify the fix works. Then commit.

## Key decisions

- Refreshed all 711 previously curated English `linux`/`common` pages against upstream `ba104601`, then added 13 Tamlinux-relevant pages (installed tools and useful 1-hop See also targets): `notify-send`, `wireplumber`, `dua`, `firefox`, `code`, `trap` (common and linux), `pkgfile`, `ydotool`, `duf`, `unalias`, `fwupdtool`, `thunar`.
- Skipped new upstream niche pages (`flux-*`, `terraform-workspace`, `cdemu`, `grdctl`, and similar) and guide false positives (`as`, `open`, `ld`).
- Patch-bumped corpus `0.56.0` → `0.56.1` after agy finished Meta key work.
- For the broken historical link to pruned `tamlinux/docs/plans/installation-framework.md`, pointed readers at `FILING-CARD.md` instead of restoring the deleted plan.
- Added the missing Command `0.48.0` decision-record entry so `make check-repo` matches `src/version.h`.

## Verification

- `make corpus-and-test`: passed (corpus index, HTML lint, index/linter unit tests, `tests/guide.sh`).
- `make check-repo`: passed after the two plan-doc fixes.
- Spot-checked `tam tldr` rendering for `wireplumber`, `notify-send`, `dua`, `wpctl`, and `modprobe` against the refreshed corpus.
