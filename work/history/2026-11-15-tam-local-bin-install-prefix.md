# Default tam development installs to ~/.local/bin

- **Date:** 2026-11-15
- **Time:** 21:17 to 21:20 -05:00
- **Stage:** Develop; tooling only, no command or corpus version change
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen Medium (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending commit in this session
- **Authorship:** Fred Horch directed switching tam development install targets from `~/.local/dev/bin` to `~/.local/bin`; Antigravity updated `Makefile`, `tools/dev.py`, `README`, `tests/test_install_guard.py`, and `tests/test_dev.py`.
- **Checked against:** `Makefile`, `tools/dev.py`, `README`, `tests/test_install_guard.py`, `tests/test_dev.py`

## Guiding prompts

> When we develop tam, use ~/.local/bin as the target for installs, rather than ~/.local/dev/bin.

## Key decisions

Changed the default Nix/Tamlinux install `PREFIX` in `Makefile` and the default `make dev` install prefix in `tools/dev.py` from `~/.local/dev` to `~/.local` so binaries install into `~/.local/bin`, matching `PATH` and retiring `~/.local/dev/bin` for `tam` development installs. Updated `README` and the unit tests in `tests/test_install_guard.py` and `tests/test_dev.py` to match.

## Verification

- `python3 -B -m unittest -v tests/test_install_guard.py tests/test_dev.py`: 34 tests pass.
- `make -f Makefile -f - print-prefix`: resolves to `~/.local`.
- `make check-repo`: passes.
