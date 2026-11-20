# tam command foundation (0.0.1)

- **Date:** 2026-10-09
- **Stage:** Develop — first command slice implemented.
- **CLI Tool:** opencode 1.18.35
- **Model:** `big-pickle`
- **Transcript:** Retained privately by the author.
- **Commits:** Not yet committed; the work was prepared in the shared checkout
  at Fred's request.
- **Authorship:** Fred supplied the goal, the first-slice behavior and the
  version number; opencode wrote the C source, Makefile, test harness and the
  toolchain research report. No delegation.

## Guiding prompts

> Review the plan for the tam command. Then implement the first slice: create a
> tam executable that this system can run. Bare tam should print a concise
> introduction to the terminal and exit 0 without launching a browser or
> writing state. Also implement tam --help and tam --version. First version
> will be 0.0.1. You'll need to get a minimal libtam C library installed and
> use it for compiling tam on this system. Write the C code, the Makefile, then
> compile and link and install an executable that I can run on this system.

> As part of this project, do research on makefiles and the C compiler chain,
> decide what you will use for developing the tam command, and prepare a report
> on that for [redacted: private repository].

> instead of "Tamlinux product" call it "Tamlinux cultivar". In the future, we
> will give each version of Tamlinux as a whole a cultivar name as well as a
> version number. There is no cultivar name yet, so we just call it "Tamlinux
> cultivar". As an example of a future cultivar name, we might call version
> 1.0.0, "Tamlinux cultivar blue sparkler 1.0.0".

## Decisions and scope

- Bare `tam` prints a concise introduction and exits 0; it never launches a
  browser and writes no state. `-h`/`--help` and `-V`/`--version` are
  implemented; unrecognized arguments exit 2 and point at `--help`.
- The command version is `0.0.1` (`src/version.h`), separate from the `libtam`
  version (reported from `tam_version()`) and from the installed Tamlinux
  cultivar version, which is read from `$XDG_CONFIG_HOME/tamlinux/version`
  (falling back to `~/.config/tamlinux/version`), then `$TAMLINUX_VERSION`,
  and otherwise reported as unavailable. Fred terms the whole-release identity
  a **cultivar**; a cultivar *name* will join the version later
  (`Tamlinux cultivar <name> <version>`), but no name exists yet.
- The build uses GNU Make and `cc` (GCC) and links an **installed** `libtam`
  with an rpath for the `/usr/local` development install; the Makefile never
  builds the sibling library. The toolchain rationale and the packaging path
  for each cultivar are recorded in a private research report.

## Verification

- `libtam` built and its `-fsanitize=address,undefined` test suite passed;
  installed to `/usr/local` (`include/tam.h`, `lib/libtam.a`, `lib/libtam.so`).
- `tam` compiled with `-Wall -Wextra -Wpedantic -std=c11` with no warnings and
  installed to `/usr/local/bin/tam`; `readelf` confirms `NEEDED libtam.so` and
  `RUNPATH /usr/local/lib`.
- `make test` runs `tests/run.sh`, which checks: bare invocation exits 0 and
  writes no state under an isolated home; `--help`/`-h`; `--version`/`-V`;
  cultivar-version resolution (file beats env, env fallback, explicit
  unavailable); piped output contains no escape sequences; unknown arguments
  error with guidance. All checks passed.
- Manually verified on the installed command: bare introduction, help, version
  (`tam command 0.0.1`, `libtam 0.1.0`, `Tamlinux cultivar 0.4.0`), and a
  non-zero exit on an unknown argument.
