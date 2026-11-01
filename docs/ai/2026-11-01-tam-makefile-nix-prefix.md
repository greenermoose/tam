# Default make install PREFIX under Nix

- **Date:** 2026-11-01
- **Time:** 06:18 to 06:45 -05:00
- **Stage:** Develop; Makefile install target
- **CLI Tool:** Antigravity CLI 1.4.12
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit and follow-up
- **Authorship:** Fred Horch requested fixing the `make install` target when installing without specifying PREFIX on systems using Nix. Antigravity updated `Makefile` to detect Nix and default `PREFIX` to `$(HOME)/.local/dev`, and added unit tests in `tests/test_install_guard.py`.
- **Checked against:** `Makefile`, `tests/test_install_guard.py`, `agent/docs/agent-runbooks/developing-tamlinux-components.md`.

## Guiding prompts

> Fix the make install target for tam so I can install the new version of tam. Here is how it fails now. It doesn't know about nix.
>
> make install
>
> ==> [start] Corpus index generation (build-index.py): python3 tools/build-index.py
> Generated index.tsv and five companion outputs with 142 entries.
> ==> [done]  Corpus index generation (build-index.py) (3.32s)
>
> install -d /usr/local/bin
> install -m 755 build/tam /usr/local/bin/tam
> install: cannot create regular file '/usr/local/bin/tam': Permission denied
> make: *** [Makefile:182: install] Error 1

## Key decisions

- On Tamlinux and systems where Nix/Home Manager is present, `/usr/local` is retired and root-owned, while user-facing development builds belong under `$(HOME)/.local/dev` to override store-linked commands on PATH.
- `Makefile` checks `ifndef PREFIX`: when `NIX_PROFILES` or `$(HOME)/.nix-profile` is detected, `PREFIX` defaults to `$(HOME)/.local/dev`. Otherwise, it falls back to `/usr/local`. Explicit `PREFIX` overrides on the command line continue to take precedence.

## Verification

- `python3 -m unittest -q tests/test_install_guard.py`: all 11 unit tests pass, verifying default prefix detection with Nix, without Nix, and with explicit overrides.
- `make -f Makefile -f - print-prefix`: resolves to `~/.local/dev`.
- `make -n install`: confirms installation targets `~/.local/dev/bin`, `~/.local/dev/share/tamlinux/knowledge`, and `~/.local/dev/share/man`.
