# tam README installation paths and packaging status

- **Date:** 2026-10-09
- **Stage:** Develop — documentation update.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.2
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred requested explaining where the tam binary and knowledge pages are installed by default on a Tamlinux cultivar, and clarifying the status of installation and tamlinux-packages packaging. Antigravity updated `README.md`, verified tests, and recorded provenance. No delegation.

## Guiding prompts

> Improve the README for the tam command to explain where the tam binary and the tam knowledge pages are installed by default on a tamlinux cultivar.

> Why does the README say installation is not implemented yet? The tam command is installed on my system and should be part of tamlinux-packages now.

> Commit your work. Release any claims. Let me know when it's safe for me to start a new session.

## Decisions and scope

- Added **Installation on a Tamlinux cultivar** to `README.md`:
  - Documented default binary path (`/usr/local/bin/tam`, mode 755, in `$PATH`).
  - Documented default knowledge pages directory (`/usr/local/share/tamlinux/knowledge/`).
  - Documented sequential runtime knowledge lookup order (`$TAM_KNOWLEDGE_DIR` → `$XDG_DATA_HOME` → `$XDG_DATA_DIRS` → compiled-in `TAM_DATADIR`).
  - Documented staged packaging overrides via `PREFIX` and `DESTDIR`.
- Clarified status:
  - Documented that `tam` is part of `tamlinux-packages` (as `tam` and default flake output) and installed on the host system.
  - Clarified that only the future `tam install` CLI subcommand (milestone 4: installation inspection) is unimplemented, eliminating confusion with package/file installation.
- Updated **Build and install** section with `make install` details and paths.

## Verification

- `make test` executed; all 18 command and offline guide behavior checks pass cleanly.
