# Add `tam tldr` subsystem and curated upstream `tldr` pages (command 0.33.0 / corpus 0.35.0)

- **Date:** 2026-10-20
- **Time:** 10:17 to 13:04 -04:00
- **Stage:** Develop (`tam` command 0.33.0, knowledge corpus 0.35.0)
- **CLI Tool:** Antigravity CLI (`agy`) 1.4.3
- **Model:** Gemini 4 Argon (High) (`gemini-4-argon-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `49e27d7` and the commit that adds this record
- **Authorship:** Fred directed the requirements, answered the three design questions (AI-curated English subset refreshed on demand from the canonical upstream clone, preserving Arch's `tldr` client untouched, and rendering both short and long option forms), and approved the plan; Antigravity wrote the code, corpus updates, tests, and index builder.
- **Checked against:** `src/man.c`, `src/info.c`, `src/discovery.c`, `src/render.c`, `src/pager.c`, `src/tam.c`, `tools/build-index.py`, `knowledge/index.html`, `knowledge/learn/documentation.html`, `knowledge/man/index.html`, `knowledge/learn/tam.html`, `knowledge/tam.html`, and the upstream `tldr-pages/tldr` repository (`pages/linux/` and `pages/common/`, `STYLE-GUIDE.md`, `LICENSE.md`).

## Guiding prompts

> /plan Create a plan to have Tamlinux include man pages, info manuals, and tldr pages. We currently wrap man pages and info manuals and make it easy to get to them from the Tamlinux User Guide index page, but we do not do that for tldr pages. Plan a project that does the following:
>
> 1) Make sure we have downloaded and have locally available all the tldr pages from the ultimate upstream repo. I believe it is on GitHub but I could be wrong. We don't want to get a version packaged by anyone else; we want Tamlinux to get these pages straight from the horse's mouth.
>
> 2) Add a convenient way to find and display a tldr page. We probably want to provide our own index of all tldr pages if the tldr repo doesn't already provide a good program for reading tldr pages, including an index of all pages available.
>
> 3) Our pager should be able to show tldr pages just like it shows man pages. If possible, add hyperlinks. If not, skip that. Just make sure that the top bar allows a tam user to return to a helpful index page for tldr and also to return to the Tamlinux Guide index page.
>
> Ask if you have questions about this project. Your job now is to plan it, not build it yet. Do whatever research is necessary for your planning.

> /plan open the plan in a micro review window now

> /plan I'm answering the open questions. First, if we clone https://github.com/tldr-pages/tldr.git doesn't that obviate question 1? Explain why not.

> /plan Install just the English pages that are relevant to Timlinux. This may require an AI agent to do on demand. I think the best approach is to clone the repo, have an AI examine the current state of the Tamlinux project, and then examine the tldr pages from the clone and install the ones it thinks are currently relevant. The set of relevant pages depends on the state of the Tamlinux project and the state of the tldr project, so it is difficult to have this decided ahead of time deterministically. This is a good task for an AI agent to do on demand when Fred decides to refresh the tldr pages available in tam.

> /plan Explain the choices we have regarding the Arch tldr client package.

> /plan Of three options, I choose 1, keep Arch's tldr as is
>
> For question 3 in the plan: tam tldr should display both forms
>
> Any other questions?

> /plan close the review window, build the plan

> Looks good. Commit then recompile so the versions don't say dirty. After you're sure you're done and have cleaned up everything, then release your work claims.

## Key decisions

- **AI-curated English subset from upstream `tldr-pages/tldr`:** Rather than shipping all ~6,400 upstream pages across non-relevant operating systems or relying on a distro-packaged archive, `tam` ships an AI-curated subset of 711 English `linux` and `common` pages relevant to Tamlinux under `knowledge/tldr/pages/`, accompanied by `knowledge/tldr/SOURCE.json`, `knowledge/tldr/LICENSE.md` (CC BY 4.0), and a pre-extracted `knowledge/tldr/index.tsv` built by `tools/build-tldr-index.py`.
- **Standalone wrapper and coexistence:** Arch's `tldr` package remains untouched at `/usr/bin/tldr`; `tam` provides `tam tldr` and `bin/tam-tldr`.
- **Both option forms & tight example layout:** `{{[-s|--long]}}` placeholders expand to display both forms (`[-s|--long]`). Example descriptions render as bullet items (`B_DT`) immediately followed by indented `<pre><code>` blocks (`B_PRE`) with tight vertical spacing so quotes and placeholders (`"{{*.html}}"`) preserve exact character adjacency without breaking renderer goldens.
- **Hyperlinks and top bar breadcrumbs:** Inline backtick command references in `See also:`, alias lines, and `tldr <cmd>` tips resolve to live `tldr:` hyperlinks when present in the curated index, alongside footer links to the companion `man:` page and any alternate platform variant. `tam_document_ensure_location` prepends `Guide > tldr pages > <cmd>` so every `tldr` page links back to both `tam:tldr` (`tldr pages`) and `index.html` (`Guide`).

## Verification

- `make corpus`, `make test`, and `make check-repo`: all command tests (`tests/run.sh`), upgrade tests (`tests/test_upgrade.py`), 6 renderer golden fixtures (`tests/render.sh`), corpus lint/index/dev/repo-check unit tests, and live-corpus guide tests (`tests/guide.sh`) passed.
