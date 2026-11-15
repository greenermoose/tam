# Convert definition and example lists to tables (corpus 0.93.3)

- **Date:** 2026-11-15
- **Time:** 12:22 to 12:27 -05:00
- **Stage:** Develop — corpus 0.93.3 definition and example list conversion
- **CLI Tool:** Antigravity CLI `1.5.9`
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** recorded in `AI_PROVENANCE.md` and git history on `develop`.
- **Authorship:** Fred Horch directed converting the `<ul>` list of other microkernel systems on `systems/operating-systems/microkernels.html` into a two-column `<table>` without a header row or trailing colons, and finding and converting other pages in the `tam` knowledge corpus with lists of definitions or examples. Antigravity updated `microkernels.html` (moving `id` attributes from `<li>` to `<tr>`), surveyed the corpus, converted 59 additional definition/example `<ul>` lists across 38 pages into two-column `<table>` elements, bumped `knowledge/version` to `0.93.3`, and updated `ROADMAP.md` and provenance records.
- **Checked against:** `src/render.c`, `knowledge/systems/operating-systems/microkernels.html`, `knowledge/tamlinux/about.html`, `knowledge/version`, `ROADMAP.md`, `agy --version`, `build/tam show --pipe`.

## Guiding prompts

> In the tam knowledge corpus on page systems/operating-systems/microkernels.html, turn this <ul> list:
>
> Genode and Sculpt OS: a framework for building systems from small components with explicit capabilities. It runs on several kernels,
>   including seL4 and NOVA. Sculpt OS is a general-purpose desktop built with Genode and used daily by its developers. Website:
>   genode.org
> - GNU Hurd: the GNU Project's kernel, a set of servers on the GNU Mach microkernel. Debian maintains an unofficial port. Decades of   
>   development show how hard hardware support and performance are for a small community. Website: gnu.org/software/hurd
> - HelenOS: a multiserver microkernel system from Charles University in Prague, portable to many processor architectures and used for
>   research and teaching. Website: helenos.org
> - MINIX 3: Andrew Tanenbaum's teaching system, redesigned for reliability: its reincarnation server restarts drivers that fail.
>   Development has been largely inactive for years. Website: minix3.org
> - QNX: a proprietary real-time microkernel system widely used in cars and industrial equipment. It shows a microkernel can be fast and
>   dependable in production, but it is not free software and cannot be a Tamlinux base. Website: blackberry.qnx.com
> - seL4: a member of the L4 family with a machine-checked mathematical proof that its implementation matches its specification. It is a
>   kernel, not a complete operating system; systems are built on it with frameworks such as CAmkES and the seL4 Microkit. Website:
>   sel4.systems
>
> into a table that renders
>
> Genode and Sculpt OS  a framework for building systems from small components with explicit capabilities.
>                       It runs on several kernels, including seL4 and NOVA. Sculpt OS is a general-purpose
>                       desktop built with Genode and used daily by its developers. Website: genode.org
> GNU Hurd              the GNU Project's kernel
>
> etc. The exact wrapping will depend on terminal width.
>
> The project is to turn a list of definitions or examples from a <ul> to a <table>. Find other pages that do the same thing, and conver them, as well.

## Key decisions

- Used headerless two-column `<table>` elements (`<tr><td>...</td><td>...</td></tr>`), matching `tamlinux/about.html`, and stripped trailing punctuation (`:`, `.`, `&mdash;`) from the first column so `tam` renders two clean columns separated by whitespace.
- Preserved `<li id="...">` anchors as `<tr id="...">` on `systems/operating-systems/microkernels.html` so in-page Contents links continue to resolve.
- Left navigation link lists, OS attribute sheets (`Website:`, `Lineage:`, etc.), duplicate-key pros/cons lists, and long-heading edge-case/pitfall lists as `<ul>` lists.

## Verification

- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe systems/operating-systems/microkernels`: verified the two-column rendering matches the requested layout.
- `make corpus-and-test`: passed (1136 pages parsed and rendered; 92 unit tests passed).
- `make check-repo`: passed.
