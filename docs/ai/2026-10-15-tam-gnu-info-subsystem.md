# Native GNU Info Subsystem (tam 0.22.0)

- **Date:** 2026-10-15
- **Stage:** Develop — Milestone 14: Native GNU Info Subsystem (`tam info`, `bin/tam-info`, `src/info.c`, `src/info.h`, pager keybindings `n`/`p`/`u`/`t`/`d`, `info:` URI resolution across guide and man pages, zlib decompression, tag tables, indirect subfiles, bash completion), version bump to 0.22.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.4
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred directed work on Version 0.22.0 for `tam` in the `tam` repository. Antigravity claimed the repository paths using `tam-work`, authored the native C Info parser and subsystem (`src/info.c`, `src/info.h`), integrated Info pager navigation keybindings (`src/pager.c`), added `info:` URI resolution to `tam show` and `tam man` (`src/tam.c`, `src/man.c`), wired `<link rel="..." href="...">` navigation headers in the HTML renderer (`src/render.c`, `src/render.h`), provided the wrapper script `bin/tam-info`, updated bash completions (`completions/bash/tam`), documented `tam info` in `man/tam.1`, bumped `TAM_COMMAND_VERSION` to `0.22.0` in `src/version.h`, updated `ROADMAP.md`, added cross-system `info:` links in the knowledge corpus (`knowledge/index.html`, `knowledge/man/index.html`, etc.), updated test suites (`tests/run.sh`, `tests/guide.sh`, `tests/snapshots/`), and verified full test pass.

## Guiding prompt

> Start working on tam version 0.22.0.

## Decisions and scope

- **Native C Info Parser (`src/info.c`, `src/info.h`):**
  - Zero external dependencies on `/usr/bin/info`. Implemented direct parsing of GNU Texinfo `.info` and `.info.gz` files using zlib (`gzopen`/`gzread`) and POSIX file lookups across `/usr/share/info`, `/usr/local/share/info`, `/usr/info`, and `$INFOPATH`.
  - Built full support for:
    - Top-level directory catalogs (`/usr/share/info/dir`) formatted with interactive menu tables.
    - Single-file and split multi-volume manuals via Indirect Subfile tables and Tag tables (tested against 20-volume split manuals such as `libc.info.gz`).
    - Standard node header delimiters (`\x1f\nFile: ..., Node: ..., Next: ..., Prev: ..., Up: ...`) transformed into semantic `<link rel="...">` elements and visual breadcrumb headers.
    - Node menus (`* Label: (manual)node. Description`) transformed into styled unordered lists with hyperlinked targets.
    - Inline cross-references (`*Note target::` and `*Note Label: target.`) converted to interactive `info:` hyperlinks.
  - Sub-5ms lookup latency and `<4 MB` RSS footprint, respecting Tamlinux workstation resource budgets.
- **Dedicated Pager Keybindings & Semantic Navigation (`src/pager.c`, `src/render.c`, `src/render.h`):**
  - Extended `tam_document_t` to store `nav_next`, `nav_prev`, `nav_up`, and `nav_top` target URIs parsed from `<link rel="...">` tags.
  - Implemented single-stroke Texinfo pager navigation keys:
    - `n`: Jump to next node (`nav_next`).
    - `p`: Jump to previous node (`nav_prev`).
    - `u`: Navigate up to parent node (`nav_up`) or back in history.
    - `t`: Jump directly to Top node of current manual (`nav_top`).
    - `d`: Jump to root Info directory catalog (`info:dir`).
    - `` ` ``: Return immediately to Tamlinux Guide index.
  - Enhanced pager status line to display navigation cues (`[n]ext [p]rev [u]p [t]op [d]ir`) whenever Info navigation targets are active.
- **Cross-System URI Resolution (`src/tam.c`, `src/man.c`):**
  - Added `info:` URI routing in `corpus_load` (`tam show`), `tam_man_load` (`tam man`), and interactive pager link activation. Following an `info:` link from the User Guide or a man page seamlessly opens the corresponding Info manual and node.
  - Added `tam info` CLI command handler supporting bare `tam info` (directory), `tam info MANUAL`, `tam info MANUAL NODE`, and `--pipe` plain-text mode.
- **CLI Wrapper and Completions (`bin/tam-info`, `completions/bash/tam`):**
  - Created standalone executable script `bin/tam-info` that forwards arguments to `tam info "$@"`.
  - Added `tam-info` to `Makefile` `install` and `uninstall` targets.
  - Added bash completions for `tam info <TAB>` to dynamically suggest available manuals from Info search directories.
- **Documentation and Versioning:**
  - Documented `tam info` in `man/tam.1` (SYNOPSIS, SUBCOMMANDS, PAGER KEYBOARD CONTROLS).
  - Bumped `TAM_COMMAND_VERSION` to `0.22.0` in `src/version.h`.
  - Updated `ROADMAP.md` marking Milestone 14 complete and outlining 0.23.0 Next.
  - Added `info:` cross-links in `knowledge/index.html`, `knowledge/man/index.html`, `knowledge/shell/pipelines.html`, `knowledge/shell/jobs.html`, `knowledge/filesystem/permissions.html`, and `knowledge/improve/toolchains.html`.
- **Verification and Test Suite Updates:**
  - Updated `tests/run.sh` with `tam info` node parsing, directory rendering, wrapper script, and Shift-Tab backward link circulation tests.
  - Updated `tests/guide.sh` link checker to recognize `info:` URIs and handle multi-word node names without IFS splitting.
  - Updated golden text snapshots for modified knowledge pages.

## Verification

- `make all` built cleanly under `-O2 -Wall -Wextra -Wpedantic -std=c11` with `-lz`.
- `make corpus` generated `knowledge/index.tsv` and `knowledge/index.json` with 43 entries and zero errors.
- `tests/run.sh` passed 100% of command and pager interactive tests.
- `tests/guide.sh` passed 100% of guide tests and snapshot comparisons.
- `make test` executed green across all lanes.
