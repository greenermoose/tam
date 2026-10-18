# tam GNU Info Subsystem Plan

**Plan status:** Completed
**Reviewed:** 2026-10-18
**Scope:** Command 0.22.0 native Info reader.
**Authority:** [Current development contract](development.md) and the evidence below.
**Next action:** None for this closed plan; follow the current roadmap.
**Completion evidence:** Implementation `a5a3b4f`; [verification record](../ai/2026-10-15-tam-gnu-info-subsystem.md). Parsing and Info URIs survive; the [overhaul](knowledge-system-0.31.0.md) replaces the three-track presentation and adds common navigation/discovery. Original host catalog counts below are historical.

> Historical design snapshot. This plan is closed; its original work items do
> not assign further implementation. Current behavior belongs to the development
> contract and current scope to the [plan index](README.md).

## Original design record

**Owner:** Fred.  
**Target Version:** `tam 0.22.0`.  

---

## 1. Motivation and Problem Statement

`tam` currently provides two distinct documentation surfaces:
1. **The Bundled Offline Guide** (`tam show`, `knowledge/`): Curated, human-centered walkthroughs, conceptual models, and fading templates organized around the core triad: *Using*, *Maintaining*, and *Improving* the system.
2. **System Manual Pages** (`tam man [SECTION] PAGE`): Native dynamic rendering of system roff manual pages into semantic HTML with hyperlinked `SEE ALSO` cross-references (`man:name(sec)`), navigable within `tam`'s built-in ANSI terminal pager or as clean text when piped.

However, a fundamental gap exists in Linux system documentation: **GNU Info manuals are absent from `tam`**.

### The GNU vs. Unix Documentation Divide

On Unix and Linux systems, system documentation is historically and architecturally divided into two formats:
- **Unix Manual Pages (`man`)**: Originated in 1971 at AT&T Bell Labs. Man pages are concise, linear reference cards ("cheat sheets") designed for rapid syntax lookup when the user already understands the command.
- **GNU Info System (`info`)**: Created by Richard Stallman in 1986 for the GNU Project. The GNU Project explicitly rejected man pages as the primary documentation medium for GNU software. In GNU standards, man pages are considered secondary or legacy summaries (often auto-generated via `help2man`), while **GNU Texinfo/Info manuals are the canonical, complete, and authoritative documentation**.

Almost every core utility, toolchain component, and fundamental library man page on this system contains this explicit disclaimer:
> *"The full documentation for ... is maintained as a Texinfo manual. If the info and ... programs are properly installed at your site, the command 'info ...' should give you access to the complete manual."*

### The Wealth of Info Documentation on the Host

On this Tamlinux system, `/usr/share/info/` contains **163 info files** comprising **over 80 complete book-length reference manuals** and **2,584 individually cataloged menu index items** in `/usr/share/info/dir`. This includes:
- **Core Utilities & Shell:** Complete, exhaustive manuals for `bash` (formal grammar, expansion algorithms, programmable completions), `coreutils` (detailed invocation, flag rationale, and corner cases for 100+ utilities), `grep`, `sed`, `find`, `tar`, `gzip`, `diffutils`, `readline`, and `inetutils`.
- **Compilers & Development Toolchains:** Complete guides and internal architectures for `gcc`, `gccint` (compiler internals), `gdb` (debugger, machine interface, remote serial protocol), `cpp`, `as`, `ld` (linker scripts and ELF internals), `make` (rules, functions, parallelism), `binutils`, `bison`, `flex`, `m4`, `autoconf`, and `automake`.
- **C Library & Systems Programming:** The complete, 20-volume **GNU C Library Reference Manual (`libc`)**—a 1,000+ page textbook covering POSIX systems programming, memory allocation internals, signal handling, pthreads, IPC, sockets, and GNU extensions.
- **Security & Cryptography:** Comprehensive architecture manuals and protocols for `gnupg`, `gpgme`, `gnutls`, `gcrypt`, `nettle`, and `libtasn1`.

### The Problem for Tamlinux Users

Currently, `tam` cannot view these manuals. When a user reads a man page via `tam man` or a guide chapter via `tam show`, following cross-references to GNU manuals requires either running the standalone GNU `info` binary or reading raw files.

The standalone `info` reader has significant drawbacks:
1. **Unfamiliar and Arcane Keybindings:** Standalone GNU `info` uses GNU Emacs keybindings (`Ctrl-x Ctrl-c`, `m`, `f`, `l`, `r`, `t`) that clash with modern terminal ergonomics and `tam`'s streamlined navigation keys (`Tab`, `Shift-Tab`, `Enter`, `u`, `` ` ``, `/`).
2. **Disconnected Ecosystem:** Standalone `info` does not integrate with `tam`'s unified knowledge graph, does not link back to `tam man` pages or the Tamlinux User Guide, and does not support `tam`'s `--pipe` non-TTY clean text contract.
3. **Missing Uniform Entry Point:** Users must remember which commands have complete man pages versus which commands require `info`.

`tam` must unify these documentation layers, becoming the single, coherent entry point for all system documentation.

---

## 2. Architectural Design

```text
                     +---------------------------------------+
                     |           tam Entry Point             |
                     +---------------------------------------+
                                         |
         +-------------------------------+-------------------------------+
         |                               |                               |
         v                               v                               v
+------------------+           +-------------------+           +-------------------+
|     tam show     |           |      tam man      |           |     tam info      |
|  Curated Guide   |           |  Manual Reference |           |    GNU Library    |
| (knowledge/*.html|           |  (groff -Thtml)   |           | (/usr/share/info) |
+------------------+           +-------------------+           +-------------------+
         |                               |                               |
         +---------------+---------------+-------------------------------+
                         |
                         v
         +-------------------------------+
         |    tam Semantic HTML Engine   |
         |    (src/render.c, links, dl)  |
         +-------------------------------+
                         |
         +---------------+---------------+
         |                               |
         v                               v
+------------------+           +-------------------+
|  Interactive TTY |           |  Piped / Non-TTY  |
|  Built-in Pager  |           | Clean Plain Text  |
|   (src/pager.c)  |           |   (--pipe / >)    |
+------------------+           +-------------------+
```

### 2.1 The Tripartite Documentation Model

With `tam info`, `tam` offers a complete 3-tier offline documentation ladder:
1. **Curated Workstation Guide (`tam show`)**: The top-level mental model and practical orientation (The *Use*, *Maintain*, and *Improve* tracks).
2. **UNIX Manual Pages (`tam man`)**: Fast, command-line syntax sheets, option flags, and system call definitions.
3. **GNU Info Manuals (`tam info`)**: In-depth, encyclopedic manuals, language specifications, compiler internals, and systems programming textbooks.

### 2.2 Subcommand CLI Contract

| Command | Behavior |
| :--- | :--- |
| `tam info` | Opens the top-level Info Directory (`(dir)Top` from `/usr/share/info/dir`) in the built-in pager, presenting a categorized, hyperlinked directory of all installed manuals. |
| `tam info MANUAL` | Opens the `Top` node of the specified Info manual (e.g. `tam info coreutils`, `tam info bash`, `tam info libc`). |
| `tam info MANUAL NODE` | Jumps directly to a specific node within the manual (e.g. `tam info coreutils "head invocation"`, `tam info bash "Shell Syntax"`). |
| `tam show info:MANUAL` | Direct virtual URI navigation from within the Guide or man pages. |
| `tam show info:MANUAL/NODE` | Direct virtual URI navigation to a specific node. |
| `bin/tam-info` | Lightweight shell wrapper delegating to `tam info` (matching `bin/tam-man`). |

All invocations respect the standard `tam` output contract:
- **Interactive TTY:** Rendered in `tam`'s native terminal pager with hyperlinked menu items and cross-references.
- **Piped / Non-TTY (`--pipe`):** Emits clean, formatted plain text with an indexed link table and zero terminal escape sequences.

### 2.3 Info Document Format & Node Topology

An Info file consists of pre-compiled, structured hypertext nodes delimited by ASCII `\x1F` (Unit Separator, byte 31).

#### Node Header Structure:
```text
^_
File: coreutils.info,  Node: head invocation,  Next: tail invocation,  Prev: cat invocation,  Up: Output of entire files
```

Every node explicitly declares:
- `File`: Source info file.
- `Node`: Name of the current node.
- `Next`: Sibling node to follow forward.
- `Prev`: Sibling node to follow backward.
- `Up`: Parent node in the document hierarchy.

#### Tag Tables & O(1) Seeking:
Large Info manuals (such as `libc.info`, which spans 20 split files) conclude with an indirect file list and a **Tag Table**:
```text
Tag Table:
Node: Top 1472
Node: Overview 2688
Node: Invoking gzip 10509
...
End Tag Table
```
By parsing the tag table, `tam` can jump directly to any node in constant time $O(1)$ without scanning multi-megabyte files into memory.

#### Menus (Child Nodes):
Downward hierarchical branches are declared in menu blocks:
```text
* Menu:

* Invoking gzip::             How to run gzip.
* Advanced usage::            Concatenated files.
```
`tam` transforms these into semantic HTML unordered lists:
```html
<ul class="info-menu">
  <li><a href="info:gzip/Invoking gzip">Invoking gzip</a>: How to run gzip.</li>
  <li><a href="info:gzip/Advanced usage">Advanced usage</a>: Concatenated files.</li>
</ul>
```

#### Cross-References (Hyperlinks):
Inline cross-references within or across manuals use `*Note`:
- Intra-manual: `*Note Advanced usage::` $\rightarrow$ `<a href="info:gzip/Advanced usage">Advanced usage</a>`
- Renamed: `*Note Concatenation: Advanced usage.` $\rightarrow$ `<a href="info:gzip/Advanced usage">Concatenation</a>`
- Cross-manual: `*Note (coreutils)head invocation::` $\rightarrow$ `<a href="info:coreutils/head invocation">head invocation</a>`
- Manual page references: `(see man page bash(1))` $\rightarrow$ `<a href="man:bash(1)">bash(1)</a>`

---

## 3. Implementation Plan (Milestone 14)

### Phase 1: Info Parser & Loader (`src/info.c`, `src/info.h`)
- Implement `tam_info_locate(const char *manual, char *path_out, size_t path_cap)`:
  - Searches `$INFOPATH` followed by standard directories (`/usr/share/info`, `/usr/local/share/info`).
  - Checks for uncompressed (`.info`) and gzipped (`.info.gz`) files.
- Implement decompression reader for `.info.gz` files via zlib or pipeline streaming.
- Implement node parser:
  - Parses node delimiters (`\x1F`), extracting `Node`, `Next`, `Prev`, `Up`.
  - Parses Tag Table for fast node offset lookup.
  - Transforms node body into semantic HTML:
    - Code blocks (`<pre>`)
    - Menus (`<ul class="info-menu"><li><a href="...">`)
    - Cross-references (`*Note ...` $\rightarrow$ `<a href="info:...">`)
    - Man page references (`<b>name</b>(sec)` $\rightarrow$ `<a href="man:name(sec)">`)
- Special handling for `/usr/share/info/dir`:
  - Parses the global directory into categorized sections (Basics, Development, GNU C Library Functions) with live links to each manual.

### Phase 2: CLI Subcommand & Pager Integration (`src/tam.c`, `src/pager.c`)
- Wire `tam info [MANUAL [NODE]]` into `src/tam.c`.
- Wire `info:` URI resolution in `src/tam.c` (`cmd_show` and `corpus_load`):
  - When a user follows an `info:manual/node` link in the pager, `corpus_load` routes directly to `src/info.c`.
  - Pager history stack (`u` to go back, Right Arrow to go forward) seamlessly traverses between Guide pages, man pages, and info nodes!
- Bind dedicated Info navigation keys in `src/pager.c`:
  - `n`: Jump to `Next` node.
  - `p`: Jump to `Prev` node.
  - `u`: Jump to `Up` (parent) node.
  - `t`: Jump to `Top` node of current manual.
  - `d`: Jump to root `dir` (directory) catalog.
  - `` ` ``: Jump to Guide index (`index.html`).

### Phase 3: Shell Wrapper & Bash Completions (`bin/tam-info`, `completions/bash/tam`)
- Create `bin/tam-info`:
  ```bash
  #!/usr/bin/env bash
  exec tam info "$@"
  ```
- Install `bin/tam-info` to `$(PREFIX)/bin/tam-info`.
- Update Bash completions:
  - Complete `info` as top-level subcommand.
  - Complete available manual names from `/usr/share/info/` for `tam info <TAB>`.

### Phase 4: Guide Integration & Knowledge Cross-Linking
- Update `knowledge/index.html` (Table of Contents):
  - Add section in Part III: "System Manuals & GNU Info Documentation" linking to `tam:topics`, `man`, and `info`.
- Cross-link Guide pages to their authoritative GNU Info manuals:
  - `shell/pipelines.html` $\rightarrow$ `info:coreutils/Pipelines`
  - `shell/jobs.html` $\rightarrow$ `info:bash/Job Control`
  - `filesystem/permissions.html` $\rightarrow$ `info:coreutils/File permissions`
  - `improve/toolchains.html` $\rightarrow$ `info:make` and `info:gcc`

### Phase 5: Verification & Automated Test Suite
- Unit and integration tests in `tests/run.sh`:
  - `tam info` exits 0 and prints root `dir` catalog under `--pipe`.
  - `tam info coreutils` loads `coreutils.info.gz` Top node.
  - `tam info coreutils "head invocation"` loads exact node.
  - In-pager link following across `info:`, `man:`, and guide links.
  - Non-TTY piped output contains clean plain text and link index.
- Add golden snapshots in `tests/snapshots/`.

---

## 4. Acceptance Criteria

1. **Bare Directory Discovery:** `tam info` launches the interactive pager displaying the categorized `/usr/share/info/dir` menu with live hyperlinks to installed manuals. Under `--pipe`, it outputs clean, formatted plain text with an indexed link table.
2. **Targeted Node Lookup:** `tam info bash` opens the top node of the Bash manual; `tam info bash "Shell Syntax"` navigates directly to that specific node.
3. **Hypertext Traversal:** Menu entries and `*Note` cross-references render as live hyperlinks. Selecting a cross-reference opens the target node; selecting an inter-manual link (e.g. `*Note (libc)malloc::`) opens the target manual; selecting a man page link opens the corresponding `tam man` page.
4. **History & Directional Navigation:** `u` moves up to the parent node or returns back in history; `n` and `p` traverse sibling nodes; `` ` `` returns immediately to `index.html`.
5. **Zero External Reader Dependencies:** Does not execute or require `/usr/bin/info`. Parsing and rendering are performed natively within `tam` using `libtam`.
6. **Performance & Footprint:** Node lookup latency $\le 5\text{ ms}$; peak RSS memory consumption $< 4\text{ MB}$; zero persistent background daemons.
