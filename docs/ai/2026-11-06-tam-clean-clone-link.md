# Full link to the tamlinux filing card

- **Date:** 2026-11-06
- **Time:** 13:32 -05:00
- **Stage:** Develop; one documentation link
- **CLI Tool:** claude 2.1.318
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `c17ea33` and the commit that adds this record
- **Authorship:** Fred asked for test suite leftovers to be finished; Claude Code wrote the change.
- **Checked against:** `tools/check-repo.py`, `make check` from a clean clone

A relative link into the sibling `tamlinux` repository broke on GitHub and made
`make check` fail from a clean clone, which is how candidates now run it. It is
now a full link. `make check` passes from a clean clone.
