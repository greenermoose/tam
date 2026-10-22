# Subprocesses without a shell (command 0.40.1)

- **Date:** 2026-10-22
- **Time:** 07:03 to 07:08 -04:00
- **Stage:** Develop: step 2 of the refactoring sequence; man page lookups
  and rendering run through libtam 0.4.0's `tam_exec`, groff's scratch
  files leave `/tmp`, and libtam links statically
- **CLI Tool:** Claude Code 2.1.307
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record; with libtam `a051ebf`
  (0.4.0, step 1)
- **Authorship:** Fred Horch approved the seven-step sequence from Claude's
  architecture review and asked for step 2. Claude wrote the code, tests and
  documentation.
- **Checked against:** `src/man.c`, `src/info.c`, `src/tldr.c`,
  `Makefile`, `bin/tam-upgrade`, `source-build.json`, libtam
  `include/tam.h`, and the 0.40.0 development build's output

## Guiding prompts

> I approve the sequence. What do you recommend regarding static linking?
> Add a man-page cache. In general, we want to use disk space to free up CPU
> cycles and RAM.

> I accept your recommendation. Start step 1 on that basis.

> Proceed to step 2 of the refactor.

## Key decisions

- `run_lookup()` stays as a thin adapter over `tam_exec_opts` so its
  callers keep their result fields; the `env` prefix carries
  `LC_ALL=C PAGER=cat MANPAGER=cat`, so libtam needed no environment option.
- gzip man pages, the common case on Arch, decompress in process with zlib
  through Info's reader, now shared as `tam_read_gzip_file`; this saves a
  process per page. bzip2, xz and zstd still run their decompressor.
- groff output is used when it is non-empty, even if groff exits non-zero
  after a warning, as the shell pipeline did.
- Scratch directory: `$XDG_RUNTIME_DIR/tamlinux/tam/groff-XXXXXX`, else the
  cache directory, else `/tmp`; it receives groff's `TMPDIR` and grohtml's
  images and is removed after each page.
- `source-build.json` pins libtam 0.4.0 for source upgrades and keeps
  `dbus-1`: building libtam compiles `libtam-portal` too. A core-only
  libtam build target would drop that dependency later.

## Verification

- `make test`: both suites pass, including a new test that a man page
  renders identically from plain, gzip, bzip2, xz and zstd sources and leaves
  no scratch files.
- `tam man` for ls, bash, passwd(5), tam, groff, printf(1) and printf(3),
  `tam tldr` for tar, ls and git, and the `tam:man?q=copy` and
  `tam:man?section=8` pages match the 0.40.0 build byte for byte.
- `readelf -d build/tam` shows no RUNPATH; `make LIBTAM_PREFIX=/nonexistent`
  stops with the libtam 0.4.0 instructions.
