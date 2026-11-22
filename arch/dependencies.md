# Dependencies

This describes command 0.67.8 at `4acc95a`. The Makefile accepts
`libtam` 0.8.0 or later. The source-upgrade manifest still pins 0.4.0, an
incompatibility tracked in [B1](../bugs/B1-source-upgrade-libtam-pin.md).

## Build and runtime

| Dependency | Purpose |
| --- | --- |
| C compiler, Make and pkg-config | Compile the command, track include dependencies and discover `libtam` and terminal flags. An explicit prefix or compiler and link flags also work. |
| libc and Linux interfaces | Allocation, files, sockets, process control, polling, POSIX termios and terminal size. |
| `libtam.a` | Static core foundation. The command carries the objects it uses from the library it was built against. |
| ncursesw/terminfo | Terminal capabilities for cursor movement, erasure, color and attributes, alternate screen and cursor shape. The Makefile falls back to `-ltinfo` when pkg-config supplies no flags. |
| zlib | In-process gzip handling for compressed reference material. |
| Python 3 and Git | Corpus metadata, linting, build identity and the source-upgrade helper. The helper uses curl for HTTPS and builds the sources it selects. |

Optional features find installed documentation and tools at runtime.
Manual pages use `man`, `apropos` and groff, with external decompressors
for formats outside zlib. `tam where` queries pacman or xbps when it
has no cultivar match. Inspection uses host files and bounded subprocesses.
The pager discovers an external opener when the reader follows a web link.
`tam ask` uses HTTP to contact an inference server. The source-upgrade
workflow currently calls libtam's full test and install targets, which
explain its D-Bus build dependency. The `tam` executable links the core
archive without the portal or display adapters.
[Makefile](../Makefile) · [Source-upgrade helper](../bin/tam-upgrade)

## libtam use and fit

The baseline is the published 0.8.0 default branch at `4b10a2e`
and development branch at `725dd1f`. Both report 0.8.0. The
development revision narrows the cultivar helpers to the current
installation layout. The review build used that revision.
[Default branch](https://github.com/greenermoose/libtam/commit/4b10a2ef74cc1adcb32f975b83841470c6f180dc)
· [Development branch](https://github.com/greenermoose/libtam/commit/725dd1f848e4717d18db3e6f228e2a75cb9d21bb)

| API | Current use and fit |
| --- | --- |
| `tam_str` | Builds generated HTML, text and JSON string values. Use allocation-safe append patterns when revisiting these paths. An append can return NULL while the previous allocation remains owned by the caller. |
| `tam_fs`, `tam_path`, `tam_xdg_path` | Read files, join paths, write lens and cache state atomically, and find per-command cache, state and scratch locations. `tam_path_join_under` validates path components lexically. The guide still owns its discovery precedence and its symlink policy. |
| `tam_exec`, `tam_exec_opts` | Run bounded host probes, package queries and manual tools. Manual lookup already lowers the output cap when appropriate. The terminal handoff needs different process semantics, including foreground process groups and inherited terminal I/O. |
| `tam_install_pending`, `tam_cultivar_read_version` | Check the startup barrier and label a file attributed to the cultivar. Identity reporting also resolves configured, executable-relative and per-user contexts. A shared reader is suitable once those roots and validation semantics are explicit. A probe error and a pending transaction currently receive the same CLI diagnostic. |
| `tam_utf8_decode`, `tam_utf8_encode` | Decode manual input and HTML character references and handle text around manual links. `tam_display_*` calculates cells with locale-sensitive decoding and explicit replacement behavior. Share UTF-8 decoding first, with tests for invalid bytes, control characters, combining marks, wide characters and the C locale. |
| `tam_key` | A candidate for the normal and sandbox key decoders. An adapter needs to preserve the pager's handling of terminal replies, Escape timing, navigation bindings and prompt input. |
| `tam_tty` | Provides ANSI/xterm terminal operations. Keep the terminfo renderer and the Linux console behavior of leaving the last page visible. Evaluate narrowly scoped input or raw-mode reuse against existing signal, resume and plain-output behavior. |

The main opportunity is focused reuse of byte-level key and Unicode
primitives. The address grammar, guide render model, navigation, screen
policy and subprocess handoff belong to `tam`. `tam_rec` has a
different record grammar from the existing TSV projections and is
unsuitable as a direct replacement for that reader.

## Pager helpers under development

The local 0.9.0 work in progress includes OSC 8 writers, styling and
footer helpers, URI classification and resolution, slugification and
detached `xdg-open` dispatch. Re-evaluate once the API is published.
The OSC 8 helpers are the smallest initial candidate if their length and
validation behavior matches the pager. Footer fields and attention
styling depend on the project's terminfo and layout choices. Generic
local resolution allows absolute filesystem paths, while guide lookups
are corpus-relative. The new URI classifier lacks `tam:`, `man:` and
the other application schemes. The detached opener also differs from
`tam`'s configurable opener, two-press confirmation and console browser
handoff. Keep those behaviors when considering shared lower-level helpers.
