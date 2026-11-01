# Nix and Home Manager guide page (corpus 0.53.0)

- **Date:** 2026-10-31
- **Time:** 22:57 to 23:16 -04:00
- **Stage:** Develop, corpus 0.53.0
- **CLI Tool:** OpenCode 1.19.4 (from `opencode --version`)
- **Model:** big-pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `247c3a7` adds the page, the ecosystem hub links, the corpus version bump and the roadmap update; this commit adds the record, the provenance rows and the refreshed indexes.
- **Authorship:** Fred asked for a detailed Nix and Home Manager section that takes a reader from no knowledge to reading the Nix source and improving the system. The AI tool gathered the public sources, captured read-only command output, wrote the page and hub links, and wrote this record and the provenance rows.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/templates/ladder.html`, `knowledge/ecosystem/index.html`, `knowledge/ecosystem/packaging.html`, `knowledge/learn/bibliography/deployment.html`; the public `tamlinux-packages` repository's `flake.nix`, `flake.lock`, `nix/packages/default.nix`, `nix/modules/shell.nix` and `docs/deployment.md`; live read-only output from `nix --version`, `readlink -f` on the profile link, `home-manager generations` and `nix eval .#packages.x86_64-linux.tam.version`; the Nix Reference Manual, Nixpkgs Manual and Home Manager manual.

## Guiding prompts

> Write up a section for the tam knowledge guide about how nix and Home Manager work. Assume no knowledge. Introduce every concept and explain it all in great detail so that by the end a reader goes from complete novice to expert developer who can understand the nix source code and improve the system.

> Continue if you have next steps, or stop and ask for clarification if you are unsure how to proceed.

## Key decisions

- The page lives at `ecosystem/nix.html` with track `learn`, parent `ecosystem/index.html` and prerequisite `packaging`. It belongs with the ecosystem choices because it explains the delivery system rather than a maintenance task.
- The ladder starts with the store, profiles and generations, then moves to flakes and derivations, then the package and module source, and ends with an improvement workflow. A beginner can follow the read-only commands before reading Nix code.
- Only captured output is presented as a session. The `nix flake metadata` block is a labeled schematic, not captured output, and the page says so. The beginner "Try it" commands are instructions without invented output.
- Terms are defined in place and linked to existing glossary entries. No new glossary pages were added.
- The Tamlinux boundary is stated explicitly: product software arrives through a recorded assembly, and activating an arbitrary generation from a checkout is a development action.
- The corpus version moves to 0.53.0 and both the ecosystem hub and the packaging page link to the new page.

## Verification

- `python3 tools/lint-corpus.py`: passes.
- `make corpus-and-test`: the corpus linter, 32 corpus-linter tests, 21 index tests and all guide checks pass.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/nix` renders the page without control sequences; `tam explain nix-home-manager` and `tam search nix` find it.
- `python3 tools/build-index.py` writes 140 entries; the parent, kind and skills indexes include the page.
