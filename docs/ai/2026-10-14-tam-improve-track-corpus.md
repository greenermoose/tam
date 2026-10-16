# tam "Improve" Track Knowledge Corpus Expansion (tam 0.21.0)

- **Date:** 2026-10-14
- **Stage:** Develop — Milestone 13: "Improve" Track Knowledge Corpus Expansion (`knowledge/improve/` chapters: `index.html`, `efficiency.html`, `zram.html`, `terminal.html`, `scripting.html`, `toolchains.html`, `ai-performance.html`), User Guide Part V hyperlinking, version bump to 0.21.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5
- **Correction (2026-10-15):** CLI version 1.3.5 is taken from the agy process log that served this session; this record originally said 1.3.4.
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred directed work on Milestone 13 for `tam` in the `tam` repository. Antigravity claimed the repository paths using `tam-work`, authored the full "Improve" track documentation suite with Beginner, Intermediate, and Expert fading templates across all subtopics, updated the root User Guide (`knowledge/index.html`) Part V links, updated `src/version.h` to `0.21.0`, updated `tests/run.sh` with version and topic assertions, updated `tests/guide.sh` with page and extensionless resolution checks, regenerated the corpus index (`tools/build-index.py`) and golden text snapshots, verified the full test suite (`make test`), and installed the development build under `PREFIX=$HOME/.local/dev`.

## Guiding prompt

> Please start work on milestone 13 for tam in the tam repo.

## Decisions and scope

- **"Improve" Track Hub Page (`knowledge/improve/index.html`):**
  - Authored the track portal connecting the six core improvement pillars: zero-polling architecture, ZRAM memory compression, terminal and editor ergonomics, shell scripting conventions, native C toolchains, and local AI generation physics.
  - Linked directly to system resources and performance (`hardware/resources.html`) and maintenance pillars (`maintain/index.html`).
- **Zero-Polling and Event-Driven Architecture (`knowledge/improve/efficiency.html`):**
  - Beginner: Identifying CPU wakeups and measuring idle efficiency with `vmstat 1 3`, explaining context switches, timer interrupts, and CPU package C-states.
  - Intermediate: Event-driven file watching with `inotifywait -m -e create,delete`, replacing naive `sleep 1` loops.
  - Expert: POSIX `epoll(7)` event multiplexing (`epoll_create1`, `epoll_ctl`, `epoll_wait`) and Wayland compositor UNIX socket dispatching (`wl_display_dispatch` over `$XDG_RUNTIME_DIR/wayland-0`), enforcing the Tamlinux zero-polling rule for desktop widgets and background agents.
- **Memory Compression with ZRAM (`knowledge/improve/zram.html`):**
  - Beginner: Inspecting active in-RAM swap with `zramctl`, explaining virtual disk size vs physical compressed RAM.
  - Intermediate: Calculating compression ratios ($Ratio = \text{DATA} / \text{COMPR}$) and memory savings, querying machine-readable byte metrics (`zramctl --bytes --output-all`).
  - Expert: Linux kernel swap writeback path, `zsmalloc` object slab compaction, and algorithmic tradeoffs: LZ4 (ultra-fast >4 GB/s decompression for low latency) vs zstd (higher compression ratio ~2.5x-3x).
- **Terminal and Editor Ergonomics (`knowledge/improve/terminal.html`):**
  - Beginner: Intuitive editing with Micro (Virtue V11), universal keybindings (`Ctrl+S`, `Ctrl+Q`, `Ctrl+Z`, `Ctrl+C`/`Ctrl+V`), launching with `SUPER + Shift + W`.
  - Intermediate: Live disk autoreload during multi-agent pair programming: Micro detects on-disk inode changes via `stat(2)` when AI agents or Git modify files and synchronizes buffer state seamlessly without cursor loss.
  - Expert: Sub-10ms shell startup latency budget (`time bash -i -c exit`), pruning `.bashrc` of dynamic subshells, and `foot` Wayland terminal client architecture (shared memory, FreeType/HarfBuzz glyph caching, precise damage regions).
- **Shell Scripting and Tool Conventions (`knowledge/improve/scripting.html`):**
  - Beginner: Canonical Bash script template with `set -euo pipefail` (errexit, nounset, pipefail) and safe variable quoting.
  - Intermediate: Safe argument forwarding with `"$@"`, filename whitespace handling, and guaranteed cleanup handlers via `trap cleanup EXIT INT TERM`.
  - Expert: Tamlinux command namespace separation (Decision D131): portable system-wide `tam-*` tools vs personal machine-specific `fred-*` scripts; POSIX exit code contracts (0, 1, 2).
- **Native Toolchains and Multi-Agent Workflows (`knowledge/improve/toolchains.html`):**
  - Beginner: Compiling C tools with GNU Make and GCC/Clang, verifying sub-5ms startup times and `libtam` linking.
  - Intermediate: Managing concurrent feature branches with isolated Git worktrees (`git worktree add`, `git worktree list`, `git worktree remove`).
  - Expert: Host microarchitecture optimization flags (`-O2`, `-march=native`, `-pipe`), dynamic runpaths (`-Wl,-rpath`), and `tam-work` scope claims and repository lanes (`command` vs `corpus`).
- **Physics of Local AI Generation (`knowledge/improve/ai-performance.html`):**
  - Beginner: Running private local inference with Ollama and `tam ask`, adhering to the offline-by-default boundary.
  - Intermediate: The memory bandwidth law of autoregressive generation: $Tokens/s \approx Bandwidth (GB/s) / ModelSize (GB) \times Efficiency$. Calculating throughput on DDR5 dual-channel memory.
  - Expert: Quantization formats and weight precision (FP16 vs Q8_0 vs Q4_K_M vs Q2_K), KV cache memory scaling ($2 \times layers \times heads \times dim \times context$), and CPU SIMD vectorization (AVX2, AVX-512, ARM NEON).
- **Integration, Versioning, and Testing:**
  - Updated `knowledge/index.html` Part V to hyperlink the newly authored improve pages.
  - Bumped `TAM_COMMAND_VERSION` to `0.21.0` in `src/version.h`.
  - Regenerated corpus index (`knowledge/index.tsv` and `knowledge/index.json`, expanding from 36 to 43 entries).
  - Updated `tests/guide.sh` with the 7 new pages and extensionless `tam show improve` test.
  - Updated `tests/run.sh` with version `0.21.0` assertion and topic resolution checks for `improve`, `efficiency`, `zram`, `terminal`, `scripting`, `toolchains`, and `ai-performance`.
  - Regenerated golden snapshots (`UPDATE=1 tests/guide.sh build/tam`).
  - Verified full test suite passes with `make test`.
  - Installed development build under `PREFIX=$HOME/.local/dev`.

## Verification

- `make corpus` generated `knowledge/index.tsv` and `knowledge/index.json` with 43 entries and zero validation errors.
- `make all` compiled `build/tam` with zero compiler warnings.
- `tests/guide.sh` passed 100% across all 43 corpus pages.
- `tests/run.sh` passed 100% across all command behavioral tests.
- `make test` ran fully green.
- Development install verified at `~/.local/dev/bin/tam` with `tam --version`, `tam explain improve --pipe`, and `tam explain topics --pipe`.
