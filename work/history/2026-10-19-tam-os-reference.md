# Operating Systems reference (corpus 0.33.0)

- **Date:** 2026-10-19
- **Time:** 20:50 to 21:20 -04:00
- **Stage:** Develop; twelve new corpus pages, corpus 0.32.4 to 0.33.0
- **CLI Tool:** Claude Code 2.1.304
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record, and the index commit after it
- **Authorship:** Fred asked for an offline operating-systems section and for
  a proposed organization; the AI tool proposed the layout, Fred chose among
  the offered options, and the AI tool wrote the pages.
- **Checked against:** `tamlinux` repository
  `docs/plans/operating-system-horizons.md` (written in the same session as
  the public source for roles and Tam 2.0) and
  `docs/plans/base-operating-system.md`; `docs/plans/knowledge-system-0.31.0.md`
  for part boundaries; `pacman -Si arch-wiki-docs arch-wiki-lite`;
  `tam --help`; the existing `ecosystem/` pages. Descriptions of each
  system's design are general knowledge.

## Guiding prompts

> In our knowledge corpus I'd like to add a section about operating systems, including all the Linux distros we're tracking for bug fixes and feature additions to Tamlinux and the micro-kernel operating systems we're considering as root stock for Tam 2.0. I'm not sure how to organize; I'm hoping you can ponder and propose a plan, then implement. I'm not sure whether this would be in maintaining or improving or would go in another section of the guide, perhaps an appendix or something. I want to be able to browse and read about operating systems offline without needing to do a Google search or have an Internet connection going.

> You can take the claim from the other claude session. I stopped it before it could release its work claims.

## Key decisions

- Placement: Learning > The Tamlinux Ecosystem and Its Choices, as the new
  hub `ecosystem/os/index.html`. The five-part design admits no appendix or
  sixth part, and Learning's boundary covers architecture and terminology.
  `ecosystem/distros.html` keeps its home as the base-choice page and links
  the reference.
- Fred chose: a public plan as the source for Tamlinux roles (the research
  catalog was private), background coverage of non-candidate microkernels,
  and one page per family rather than per system.
- Pages give lasting design and Tamlinux's role, not version numbers or
  release dates, which go stale offline.
- `docs/corpus-authoring.md` gains a sourcing row for operating-system roles.

## Verification

`make corpus-and-test PREFIX=$HOME/.local/dev LIBTAM_PREFIX=$HOME/.local/dev`:
corpus lint clean, 29 linter and 10 builder tests pass, all guide tests pass
(67 entries). Read the hub and microkernel pages with
`tam show --pipe`; `tam search` finds redox, microkernel, freebsd,
tumbleweed, alpine, qubes, haiku, void, debian and "tam 2.0" on the
intended pages.
