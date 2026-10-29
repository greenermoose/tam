# Standardize rootstock terminology (corpus 0.47.0)

- **Date:** 2026-10-29
- **Time:** 10:03 to 12:10 -04:00
- **Stage:** Develop; corpus 0.47.0
- **CLI Tool:** Antigravity CLI 1.4.10
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `15b9810`, and the commit that adds this record and refreshed index revisions
- **Authorship:** Fred Horch directed standardizing the term rootstock across the knowledge corpus for the operating system Tamlinux depends on. Antigravity authored the new glossary entry, renamed the rootstock glossary entry, updated the corpus pages, and regenerated index and coverage sidecars.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `ROADMAP.md`, `knowledge/ecosystem/distros.html`, `knowledge/ecosystem/terminology.html`

## Guiding prompts

> In the tam knowledge corpus, use the term "rootstock" consistently in every page written for Tamlinux. The term is spelled rootstock, not root-stock or root stock.
>
> You can explain it once per page, the first time the term is used, as being the kernel and core system software on which the Tamlinux cultivar is installed. The cultivar is all the software in the Tamlinux release that allows the computer to be used effectively and efficiently by both humans and AI agents.
>
> This project is to clean up and standardize the different terms in different places on different pages that we have used for the rootstock concept. We want to simplify and make our language consistent. And we want to make sure users know that rootstock is not a common term, but one that Fred made up to explain how Tamlinux works. Fred defines the rootstock to be the set of software that third-parties publish and is necessary to be installed so that the software we write can function properly.
>
> Our goal is to minimize the size of the rootstock and maximize its speed and efficiency. Currently we are using Arch Linux as our rootstock but we plan to migrate to Void Linux in future versions of Tamlinux. Beyond that we are considering a micro-kernel to further reduce the code size of the rootstock and improve its performance per watt.
>
> Make sure all of this is clear on every tam knowledge corpus page.

> Instead of "base OS" or "distro" the term rootstock should be used.

> When describing the operating systems that Tamlinux depends on.

> Distro can be defined in the glossary and used in other contexts freely, just not as a way to describe a rootstock for Tamlinux.

## Key decisions

- Standardized the spelling to `rootstock` without hyphens or spaces across all knowledge pages describing the underlying operating system Tamlinux depends on, replacing terms like "base OS", "base distribution", "base system", and "root-stock".
- Included the complete explanation on each page where the term is used: defined as the kernel and core system software on which the Tamlinux cultivar is installed, coined by Fred Horch, defined as third-party published software necessary for Tamlinux to function properly, with the goal to minimize size and maximize speed/efficiency, currently Arch Linux, migrating to Void Linux in future versions, and exploring microkernels beyond that.
- Added `learn/glossary/distro.html` as a standalone glossary term explaining distributions in general while clarifying that Tamlinux uses rootstock for the underlying system.
- Renamed `learn/glossary/root-stock.html` to `learn/glossary/rootstock.html` and updated all navigation and references accordingly.

## Verification

- `make corpus` built index and companion files without errors (130 entries).
- `make corpus-and-test` passed: `python3 tools/lint-corpus.py` (0 findings), 32 lint unittests, 21 build_index unittests, `tools/check-repo.py`, and `tests/guide.sh` (all tests passed).
- Rendered output checked via `build/tam show --pipe` for `learn/glossary/rootstock`, `learn/glossary/distro`, and `ecosystem/distros`.
