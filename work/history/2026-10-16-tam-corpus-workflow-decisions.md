# Corpus workflow review and decisions

- **Date:** 2026-10-16
- **Time:** 11:50 to 12:40 -04:00
- **Stage:** Develop. Documentation only; no command or corpus version change.
- **CLI Tool:** Claude Code (`claude`) 2.1.301
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `8be8912`, `94ce109`, `68c2097`, `7dff288`, and the commit
  that adds this record. The same retitling was committed to
  `ACKNOWLEDGEMENTS.md` in the `tamlinux` (`3bc614a`), `libtam` (`9b61f1a`)
  and `ecosystem-fred-tamlinux` (`94033ee`) repositories.
- **Authorship:** Fred Horch, the Tamlinux developer, asked for a review of how agents had worked on the
  corpus and ways to speed it up, then made eight decisions one at a time.
  Claude Code proposed the options and wrote the documents.
- **Checked against:** `src/render.c`, `src/tam.c`, `src/install.c`,
  `tools/build-index.py`, `tests/guide.sh`, `Makefile`, the corpus audit
  record of this date, the corpus session records since 2026-10-09;
  `tamlinux` repository `desktop/session/hypr/tamlinux/bindings/`,
  `desktop/native/integration/tamlinux.desktop`,
  `desktop/tests/test_native_login.py`, `commands/`, `VERSIONING.md`,
  `docs/plans/`; `tamlinux-packages` repository `docs/deployment.md`,
  `nix/packages/default.nix`; the installed `/usr/bin/tam-session`.

## Guiding prompts

> Review the effort agy has put into working on the tam knowledge corpus. Look for ways to speed that up. Are there better skills, guides, runbooks, or memories that will help agents be faster and more productive when working on tam or our knowledge corpus? Share your ideas with me and let's discuss.

> Let's go through the decisions you want me to make one by one. Carefully think about what you want me to decide, then give me my options, and then let's discuss each one so I can decide.

> A, with sources in the provenance record only

> B, write why from [a recorded decision]'s reasoning

> A put timing in the public record

> A, yes start W3 now

> C. Call me Fred Horch, the developer of Tamlinux. I should always be called the developer, not creator, not maintainer, when first introduced on a page. Then afterwards I can be referred to as Fred.

> Yes, change acknowledgements. I've changed my mind and now want to be known as the developer of Tamlinux. You can say, Fred develops Tamlinux for to do personal and business work. Or you can say, Fred is the Tamlinux developer. Just be consistent in calling me the "developer" rather than making up a different title for me.

The other decisions were single-letter answers to the options; the decision
record in `docs/plans/development.md` lists all eight.

## Key decisions

- Most review cost came from claims about Tamlinux written from general or
  Omarchy knowledge, so the main fix is a sourcing rule and a map of where
  each kind of fact lives, not more review.
- Audit finding F4 was wrong: candidate 0.4.4c renamed the session launchers
  to `tam-session` and `tam-hyprland-session`, which the SDDM page already
  used. It is withdrawn rather than fixed.
- The earlier suggestion to change when versions are bumped was withdrawn
  during the discussion: the existing rule had worked as intended.

## Verification

The new and changed files were searched for session IDs, store and home
paths, private repository names and internal decision identifiers; none
remain. `docs/corpus-authoring.md` paths were checked to exist in the named
repositories.
