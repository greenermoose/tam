# Refactoring steps 2 to 5 (commands 0.40.1 to 0.40.4)

- **Date:** 2026-10-22
- **Time:** 07:03 to 11:22 -04:00
- **Stage:** Develop: step 2 of the refactoring sequence (0.40.1); man page
  lookups and rendering run through libtam 0.4.0's `tam_exec`, groff's
  scratch files leave `/tmp`, and libtam links statically. Step 3 (0.40.2):
  one HTML escaper and page builder, and libtam's path, JSON and state
  helpers in place of `tam`'s copies. Step 4 (0.40.3): one address module
  and loader. Step 5 (0.40.4): one HTML tag scanner
- **CLI Tool:** Claude Code 2.1.307
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `15cd730` (0.40.1), `634c955` (0.40.2), `0192688` (0.40.3)
  and the 0.40.4 commit that updates this record; with libtam `a051ebf` (0.4.0, step 1)
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

> Proceed to step 3.

> Proceed to step 4.

> Proceed to step 5

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

- Step 3: the page builder takes raw titles and labels and escapes them,
  so the man keyword title is now built from the raw query instead of a
  pre-escaped one. Index fields are plain text (the index builder decodes
  entities), so escaping them in the topics view is correct. `install.c`
  keeps its bounded JSON escaper: it fills fixed 512-byte evidence fields
  and must not split an escape when it truncates.

- Step 4: module-owned sub-routes (`tam:man:keyword?`, `tam:tldr:list`, ...),
  the pager's action names and the breadcrumb builder keep their own string
  checks; only scheme routing moved. The three command loaders read guide
  pages raw and joined `root/href` unchecked, so sharing the corpus loader
  is a behaviour fix, not only a deduplication.

- Step 5: `fold.c`'s scanner, which already parsed attribute lists, is the
  model. `depth.c` keeps its three helper signatures, now thin wrappers, so
  its twenty call sites are unchanged. A hyphen continues an element name,
  as in HTML custom elements; no page uses one, and it keeps `depth.c`'s
  stricter reading. The breadcrumb extractor was a fourth scanner and moved
  too. The review's estimate of 400 to 600 lines saved over steps 2, 3 and
  5 was high: step 5 is about line-neutral.

## Verification

- `make test`: both suites pass, including a new test that a man page
  renders identically from plain, gzip, bzip2, xz and zstd sources and leaves
  no scratch files.
- `tam man` for ls, bash, passwd(5), tam, groff, printf(1) and printf(3),
  `tam tldr` for tar, ls and git, and the `tam:man?q=copy` and
  `tam:man?section=8` pages match the 0.40.0 build byte for byte.
- Step 3: 36 generated pages and commands (topics, keywords, topic guides,
  search, man lookup and keyword pages including `&` and `<` queries, man
  pages, tldr pages and lists, the Info directory and nodes, guide pages at
  several levels) and the Top node of all 83 installed Info manuals render
  byte-identically to 0.40.1; `tam inspect` and `--json` match; `..`,
  absolute and URL page paths are refused; `make test` passes.
- Step 4: the 36 generated pages and commands match 0.40.2 byte for byte;
  `make test` passes, with new checks that `man:` opens the Man pages view,
  `--level` reaches `tam:keywords` and `tam:explain`, and `../` guide
  addresses are refused.
- Step 5: every guide page at levels 0 to 3, the fixture and golden pages
  through `tam show -`, the 36 generated pages, 25 man pages and the Top
  node of all 83 Info manuals match 0.40.3 byte for byte; render goldens
  match; a new test covers `data-href`, `href=` inside a value and `<abbr>`
  in a breadcrumb, all three of which 0.40.3 misread.
- `readelf -d build/tam` shows no RUNPATH; `make LIBTAM_PREFIX=/nonexistent`
  stops with the libtam 0.4.0 instructions.
