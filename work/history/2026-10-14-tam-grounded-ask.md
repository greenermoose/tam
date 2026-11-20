# tam opt-in grounded ask CLI surface (tam 0.17.0)

- **Date:** 2026-10-14
- **Stage:** Develop — Milestone 9: opt-in grounded ask CLI surface implemented; version bumped to 0.17.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.5
- **Correction (2026-10-15):** CLI version 1.3.5 is taken from the agy process log that served this session; this record originally said 1.3.4.
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested: "Let's work on milestone 9. Before you proceed with implementation, create a detailed work plan and let me review it. Remember to ask when I'm ready for micro-review. I need to make sure I'm not typing when you open a micro-review window with your plan for my review." and subsequently guided model selection and evaluation ("We may need to install a smaller local model. The ones we have now take a very long time to respond to prompts... llama 3.21b is fine... qwen2.5-coder:1.5b is good, too. See which one works better."). Antigravity authored the detailed implementation plan in `docs/plans/grounded-ask.md`, coordinated review via the micro-review workflow in `micro` text editor, benchmarked both `llama3.2:1b` and `qwen2.5-coder:1.5b` on local hardware, confirmed `qwen2.5-coder:1.5b` delivers superior technical accuracy and concise grounding, configured it as the primary default model with fallback to `llama3.2:1b`, claimed required paths via `tam-work`, and implemented the complete Milestone 9 specification in `src/ask.c`, `src/ask.h`, `src/tam.c`, `src/version.h`, `Makefile`, `completions/bash/tam`, `tests/run.sh`, `docs/plans/development.md`, and `ROADMAP.md`. Bumped `TAM_COMMAND_VERSION` to `0.17.0`, installed to `~/.local/dev`, and verified the entire test suite (`make test`).

## Guiding prompts

> Let's work on milestone 9. Before you proceed with implementation, create a detailed work plan and let me review it. Remember to ask when I'm ready for micro-review. I need to make sure I'm not typing when you open a micro-review window with your plan for my review.
> We may need to install a smaller local model. The ones we have now take a very long time to respond to prompts.
> Do some research to find a small model that is fine tuned for answering questions about linux systems, if you can.
> llama 3.21b is fine
> qwen2.5-coder:1.5b is good, too. See which one works better.

## Decisions and scope

- **Work Plan & Micro-Review Discipline:**
  - Authored comprehensive work plan in `docs/plans/grounded-ask.md` detailing the socket HTTP client architecture, refusal contracts, corpus grounding excerpt extraction, prompt design, and test matrix.
  - Adhered strictly to Fred's micro-review protocol (`micro-review show` / `micro-review close`) ensuring keyboard focus safety.
- **Local Model Optimization & Comparative Benchmarking:**
  - Benchmarked existing local models (`qwen3.5:9b`, `gemma4:12b`), diagnosing that deep reasoning tokens caused high latency on the integrated Vega 8 GPU (~6.6 tok/s).
  - Evaluated `llama3.2:1b` (1.3 GB) and `qwen2.5-coder:1.5b` (986 MB):
    - `qwen2.5-coder:1.5b` achieved 1.0s warm response times (~35 tok/s), 25% smaller disk footprint, zero permission hallucinations on octal modes (e.g. `chmod 755`), and clean extraction of Tamlinux guide excerpts without leaking prompt structures or local usernames.
    - `llama3.2:1b` achieved ~40 tok/s but hallucinated group/other write permissions under `chmod 755` and inserted redundant `[Source: ...]` headers into response text.
  - Set primary default model preference to `qwen2.5-coder:1.5b`, with automatic fallback to `llama3.2:1b` or any installed tag if unavailable (overridable via `--model` or `TAM_MODEL`).
- **Pure C POSIX HTTP Client (`src/ask.c`, `src/ask.h`):**
  - Implemented self-contained HTTP/1.1 socket client over non-blocking POSIX sockets with `poll()` timeouts, connecting to `http://127.0.0.1:11434` (or `--server`/`OLLAMA_HOST`). Zero external HTTP/cURL library dependencies.
  - Parses `/api/tags` to discover available models and `/api/generate` to stream/collect grounded responses.
- **Strict Refusal Contracts:**
  - *Non-interactive / pipe failure contract:* If stdout is not a TTY or `--pipe` is passed, and the Ollama server is unreachable or no supported model is available, exits immediately with code `2` and writes exact stderr:
    `tam: ask requires a local model; run interactively to fetch or start Ollama\n`
  - *Interactive refusal contract:* In interactive TTY mode without server/model, prints polite error, suggests starting Ollama or fetching the model, and executes fallback to `tam search` (exit code `1`).
  - *Interactive consent prompt:* Never downloads models automatically in the background; interactively prompts `Download llama3.2:1b (~1.3 GB)? [y/N]` before initiating fetch.
- **Corpus Grounding & Citation Guarantees:**
  - Leveraged `tam_index_search` to query the local offline corpus index for high-relevance topics.
  - Extracted bounded plain-text excerpts (≤4 KB total context) across matching topics.
  - Enforced strict prompt system instructions requiring answers to cite guide references or refuse if unsupported.
  - Always appends `Sources:` lines with relative guide paths (`e.g., Sources: shell/pipelines.html`).
  - *Grounding Miss contract:* If the corpus index yields zero matches or query cannot be answered from the guide, returns clean answer `Not found in Tamlinux guide.` with `Sources: (none)` (exit code 0).
- **Structured JSON & Pipe Support:**
  - `--json` emits `{ "query": "...", "answer": "...", "sources": [...], "model": "..." }`.
  - Accepts queries via argument vector or standard input (`tam ask -`).
  - Strips all ANSI escape sequences when piping.
- **CLI Surface & Completion:**
  - Subcommand `tam ask [options] <query...>` added to `src/tam.c` and help output.
  - Added options: `--model MODEL`, `--server URL`, `--json`, `--pipe`.
  - Updated `completions/bash/tam` with options and dynamic argument handling.
- **Version Bump:** Bumped `TAM_COMMAND_VERSION` in `src/version.h` to `0.17.0`.
- **Testing & Documentation:**
  - Added extensive test assertions to `tests/run.sh` covering usage, `--help`, non-interactive refusal exit 2, grounding miss, live local model inference, JSON output schema, stdin reading, and ANSI stripping under pipe.
  - Updated `ROADMAP.md` and `docs/plans/development.md`.
  - Installed development build to `~/.local/dev/bin/tam`.

## Verification

- `make test` runs both `tests/run.sh` and `tests/guide.sh` cleanly with zero failures.
- Live queries tested against local Ollama running `llama3.2:1b` with sub-second response times.
- Development build installed via `make install PREFIX=$HOME/.local/dev` and verified with `tam --version`.
