# tam grounded ask — Milestone 9 plan

**Plan status:** Completed
**Reviewed:** 2026-10-18
**Scope:** Command 0.17.0 opt-in grounded ask.
**Authority:** [Current development contract](development.md) and the evidence below.
**Next action:** None for this closed plan; follow the current roadmap.
**Completion evidence:** Implementation `e929ee5`; [verification record](../ai/2026-10-14-tam-grounded-ask.md). The corpus-only grounding and opt-in boundary survive. System-manual AI retrieval is not part of the overhaul.

> Historical design snapshot. This plan is closed; its original work items do
> not assign further implementation. Current behavior belongs to the development
> contract and current scope to the [plan index](README.md).

## Original design record

**Component:** `tam` (this repository)  
**Target Version:** `0.17.0` (minor bump from `0.16.0`)  
**Authority:** [development plan](development.md), [ROADMAP.md](../../ROADMAP.md), and two private design reports from 2026-10-12 (local-model ask design; help and self-inspection requirements) retained by the developer  
**Author:** Antigravity (`agy`)

---

## 1. Executive Summary & Purpose

Milestone 9 introduces `tam ask <question>`: an opt-in, grounded question-answering CLI surface over the bundled Tamlinux documentation corpus.

The command answers questions using an already-running local inference server (e.g. Ollama) while maintaining `tam`'s core Unix identity: strictly offline by default, sub-5ms startup, zero background daemons, zero external runtimes bundled into the core binary, and rigorous non-interactive pipeline safety.

---

## 2. Core Architectural Invariants & Safety Envelopes

1. **Strictly Offline by Default (The `ask` Exception Boundary):**
   - The core `tam` binary remains 100% offline and dependency-free.
   - `tam ask` is the single deliberate, opt-in exception that communicates over local sockets/HTTP.
2. **Zero-Daemon Invariant (Virtue D53):**
   - `tam` **never** starts, restarts, or manages background daemons (no `systemctl start ollama`, no background polling).
   - `tam` connects strictly to an **already-running** local inference server.
3. **Retrieval-Augmented Grounding & Mandatory Citations (R-D1):**
   - Queries are grounded in the bundled corpus: the top relevant pages from the corpus index (`tam_index_search`) are extracted to build a bounded context (≤4 KB).
   - The model is instructed to answer strictly using the provided excerpts and must append mandatory citation lines:
     ```text
     Sources:
       - shell/pipelines.html (Shell pipelines and redirection)
     ```
   - **Grounding Miss Safety:** If the corpus contains no relevant documentation for the query, `tam ask` avoids hallucination by returning:
     ```text
     Not found in Tamlinux guide.
     Sources: (none)

     Try 'tam search <terms>' to find related topics.
     ```
4. **Non-Interactive Pipeline Safety & Exact Refusal (R-D3):**
   - When invoked non-interactively (under `--pipe`, redirected stdout, or non-TTY) and no local model or server is reachable, `tam ask` **must immediately exit with status 2** and output:
     ```text
     tam: ask requires a local model; run interactively to fetch or start Ollama
     ```
   - Never hangs waiting for input and never performs unattended network requests.
5. **Interactive Server-Down Refusal (R-D4):**
   - In an interactive terminal when the local server is down or unreachable:
     ```text
     tam: no local model available (Ollama not running). Start Ollama or run 'tam search <query>' instead.
     ```
     Exits with status 1 and prints top candidate topics from `tam search` as an immediate offline fallback.
6. **Interactive Consent for Model Fetch (R-D3):**
   - If connected to a running server but the requested model is not installed, `tam ask` never auto-pulls.
   - In an interactive TTY, it presents full disclosure before asking for confirmation:
     ```text
     Model 'qwen2.5-coder:1.5b' is not installed locally.
     URL: https://ollama.com/library/qwen2.5-coder:1.5b
     Estimated Size: ~986 MB
     Expected SHA256: [verified manifest hash]
     Storage Path: /var/lib/ollama/models
     Fetch model now? [y/N] 
     ```
   - Verifies SHA256 hash upon retrieval; deletes partial data on mismatch.
   - `$TAM_ASK_ALLOW_FETCH=false` disables network fetching unconditionally even in interactive mode.

---

## 3. Command-Line Interface Specification

```text
Usage:
  tam ask [OPTIONS] <question>
  tam ask [OPTIONS] -

Options:
  --model <name>     Model to query (default: $TAM_ASK_MODEL or 'llama3.2')
  --server <url>     Inference server endpoint (default: $TAM_ASK_SERVER or 'http://127.0.0.1:11434')
  --json             Emit structured machine-readable JSON output
  --pipe             Format plain text without terminal escapes or interactive pagination
  -h, --help         Show usage help for tam ask

Environment Variables:
  TAM_ASK_MODEL        Override default model name
  TAM_ASK_SERVER       Override default server base URL (e.g. http://127.0.0.1:11434)
  TAM_ASK_ALLOW_FETCH  Set to 'false' to disable model download prompts
```

### JSON Schema (`tam ask --json`)
```json
{
  "question": "how do I inspect system logs?",
  "grounded": true,
  "answer": "Use journalctl to inspect system logs. You can filter by unit with -u or priority with -p err..emerg.",
  "sources": [
    {
      "path": "maintain/logs.html",
      "title": "System Logging and Diagnostics"
    }
  ],
  "model": "qwen2.5-coder:1.5b",
  "server": "http://127.0.0.1:11434",
  "schema_version": "1.0.0"
}
```

---

## 4. Implementation Design & File Architecture

### A. New Modules
1. **`src/ask.h`**:
   - `int tam_cmd_ask(int argc, char **argv, const char *knowledge_dir);`
2. **`src/ask.c`**:
   - CLI option parsing (`--model`, `--server`, `--json`, `--pipe`).
   - Stdin reading when question argument is `-`.
   - Health probe (`GET /api/version` with ≤500ms timeout via POSIX sockets).
   - Corpus retrieval: calls `tam_index_search` on loaded index to retrieve top-k (k=3) pages.
   - Grounding context extraction: reads page text or summaries from corpus files via `tam_fs_read_file`, truncating total context to ≤4 KB.
   - Prompt construction with strict citation constraints.
   - JSON payload generation and POSIX HTTP/1.1 POST to `/api/generate` (or `/api/chat`).
   - Streaming/buffered response collector and JSON response parsing.
   - Source citation formatting.
   - Refusal handlers:
     - Non-interactive / pipe missing model -> exit code 2.
     - Interactive server unreachable -> exit code 1 + search fallback.
     - Grounding miss -> `Not found in Tamlinux guide.` + `Sources: (none)` + suggestions.

### B. Modified Modules
1. **`src/tam.c`**:
   - Register `ask` subcommand in argument dispatcher.
   - Update `tam --help` usage output to document `tam ask`.
   - Ensure state isolation: `tam ask` never modifies `$XDG_STATE_HOME/tamlinux/welcome_seen`.
2. **`src/version.h`**:
   - Bump `TAM_COMMAND_VERSION` from `0.16.0` to `0.17.0`.
3. **`Makefile`**:
   - Add `src/ask.c` to `SRC` and `src/ask.h` to `DEPS`.
4. **`completions/bash/tam`**:
   - Add `ask` to top-level subcommands and add options completion.
5. **`tests/run.sh`**:
   - Add comprehensive test assertions for Milestone 9.

---

## 5. Verification & Testing Strategy

Tests will be added to `tests/run.sh`:

1. **Version assertion:** Verify `tam --version` reports `0.17.0`.
2. **Usage & Argument Handling:**
   - Bare `tam ask` without question fails with usage error (exit 1).
   - `tam --help` includes `ask` subcommand description.
3. **Refusal Under Non-Interactive / Piped Invocation (Exit Code 2):**
   - Run `tam ask "what is a pipeline" --pipe` when no server is running.
   - Verify exit code is exactly `2`.
   - Verify stdout/stderr matches: `tam: ask requires a local model; run interactively to fetch or start Ollama`.
4. **Interactive Refusal When Server Inactive (Exit Code 1):**
   - Run in simulated TTY environment with server inactive.
   - Verify exit code is `1`.
   - Verify stderr outputs `tam: no local model available (Ollama not running)...` and prints `tam search` candidates.
5. **State Isolation:**
   - Ensure `tam ask` does not write `welcome_seen` state.
6. **End-to-End Grounded Retrieval with Mock Server:**
   - Spin up a lightweight local Python HTTP mock server on a dedicated loopback port (e.g. `127.0.0.1:18434`).
   - Configure `tam ask --server http://127.0.0.1:18434 ...`.
   - Verify:
     - Prompt received by mock server contains corpus snippets from `tam`'s indexed pages.
     - Mock server returns cited response; `tam ask` outputs answer and verifies `Sources:` line.
     - Test grounding miss: question with zero topic overlap returns `Not found in Tamlinux guide.` and `Sources: (none)`.
     - Test `--json` flag emits valid JSON matching the schema with `"grounded": true` and sources array.
7. **Clean Output:**
   - Verify `tam ask ... --pipe` emits zero ANSI terminal escape sequences.
8. **Shell Auto-Completion:**
   - Test `tam ask --` bash completion returns `--model`, `--server`, `--json`, `--pipe`.

---

## 6. Documentation, Provenance & Release Steps

1. **`docs/plans/development.md`**: Update Stage summary, mark Milestone 9 done, add decision record entry.
2. **`ROADMAP.md`**: Update version to `0.17.0`, record Milestone 9 under Section 2 and reference matrix.
3. **`AI_PROVENANCE.md`** & **`docs/ai/`**: Record public AI provenance record.
4. **Development Build Installation:** Run `make test` and `make install PREFIX=$HOME/.local/dev`.
