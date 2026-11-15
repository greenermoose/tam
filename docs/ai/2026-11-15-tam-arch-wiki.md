# Offline Arch Wiki Integration (command 0.60.0 / corpus 0.93.6)

- **Date:** 2026-11-15
- **Time:** 12:42 to 12:50 -05:00
- **Stage:** Develop — installed `arch-wiki-docs`, added native `tam archwiki` / `archwiki:` / `tam:archwiki*` support (command 0.60.0), and linked Arch Wiki from the Guide index and documentation help (corpus 0.93.6).
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending commit on `develop`
- **Authorship:** Fred directed the installation of `arch-wiki-docs` and seamless integration into the `tam` guide; the AI tool implemented the subsystem, corpus links, tests, and provenance record.
- **Checked against:** `src/tldr.c`, `src/man.c`, `src/address.c`, `src/render.c`, `src/pager_search.c`, `src/tam.c`, `knowledge/index.html`, `knowledge/help/documentation.html`, `pacman -Si arch-wiki-docs`, and `agy --version`.

## Guiding prompts

> Install arch-wiki-docs and link to it from the index page of the tam knowledge corpus. It should be available in the Man pages, Info manuasl, and tldr pages section of the guide index. Make sure it is as seamlessly integrated into the tam guide as those other references are.

## Key decisions

- Read offline `arch-wiki-docs` HTML from `/usr/share/doc/arch-wiki/html/en` (with `$TAM_ARCHWIKI_DIR` override for testing), extracting the MediaWiki article body (`mw-content-ltr mw-parser-output`) while stripping sidebar TOC, buttons, styles, scripts, and category footers.
- Rewrite internal `../en/<Page>.html[#frag]` links to `archwiki:<Page>[#frag]` so wiki cross-references navigate inside `tam`'s pager with `Guide > Arch Wiki > <Title>` breadcrumbs.
- Provide `tam:archwiki` landing page, A–Z index (`tam:archwiki:list`), title/keyword search (`tam:archwiki:open?query=`, `tam:archwiki:keyword?query=`), interactive prompts (`tam:prompt:archwiki-open`, `tam:prompt:archwiki-keyword`), `tam archwiki` subcommand, and `bin/tam-archwiki` wrapper.

## Verification

- `make check-repo` passed.
- `make corpus-and-test` passed (1136 corpus pages, 92 tooling unit tests).
- `tests/guide.sh`, `tests/render.sh` (7 fixtures), and `tests/test_upgrade.py` (15 tests) passed.
- `tam archwiki` fixture integration checks passed (`bin/tam-archwiki`, `tam archwiki --pipe`, `tam archwiki --list`, `tam archwiki --search`, `tam show archwiki:...`, `tam show tam:archwiki`, `--help`).
- `tests/run.sh` in the sandbox only failed on `storage_btrfs` because the sandbox `/` is `tmpfs`.
