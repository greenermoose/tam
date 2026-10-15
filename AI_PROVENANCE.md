# AI collaboration and provenance

Fred directs this project. AI-assisted work records the tool and verified
version, model, guiding prompts, authorship boundaries and verification.
Session transcripts are retained privately by the author; public records
carry no transcript identifiers or store paths.

AI-assisted commits carry `Co-authored-by`, `AI-Tool` and `AI-Model` trailers.
Human-only commits retain their original authorship.

| Date | Work | Tool and model | Evidence |
| --- | --- | --- | --- |
| 2026-10-08 | Local repository scaffold and command contract | Codex CLI 0.162.0; `gpt-6.1-sol` | [Session record](docs/ai/2026-10-08-tam-repository-and-command-contract.md) |
| 2026-10-09 | Command foundation 0.0.1 (intro, `--help`, `--version`) | opencode 1.18.35; `big-pickle` | [Session record](docs/ai/2026-10-09-tam-command-foundation.md) |
| 2026-10-09 | Offline guide (milestone 2: renderer, pager, `browse`, corpus) | opencode 1.18.35; `big-pickle` | [Session record](docs/ai/2026-10-09-tam-offline-guide.md) |
| 2026-10-09 | `tam help` alias of `--help` (0.3.0) | opencode 1.18.35; `big-pickle` | [Session record](docs/ai/2026-10-09-tam-help-alias.md) |
| 2026-10-09 | Orientation and curated explanation 0.4.0 (`welcome`, `welcome_seen`, `explain`) | Antigravity CLI 1.3.2; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-09-tam-orientation-and-explanation.md) |
| 2026-10-09 | README cultivar installation paths and packaging status | Antigravity CLI 1.3.2; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-09-tam-readme-installation-paths.md) |
| 2026-10-09 | Installation inspection 0.5.0 (`install`, `install inspect`) | Antigravity CLI 1.3.2; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-09-tam-installation-inspection.md) |
| 2026-10-09 | Explain topic listing and scaling roadmap 0.6.0 (`explain`) | Antigravity CLI 1.3.2; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-09-tam-explain-topic-listing.md) |
| 2026-10-09 | `tam show` subcommand and explain pipe quoting 0.7.0 | Antigravity CLI 1.3.2; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-09-tam-show-and-explain-pipe-quoting.md) |
| 2026-10-09 | Remove Lynx references from plans and documentation | Antigravity CLI 1.3.2; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-09-tam-remove-lynx-references.md) |
| 2026-10-12 | Plan revision: milestones 6–9 (corpus index/search, depth ladder, read-only inspection, opt-in `ask`) | opencode 1.19.0; `big-pickle` | [Session record](docs/ai/2026-10-12-tam-plan-revision.md) |
| 2026-10-13 | Corpus index and search 0.10.0 (`search`, `--json`, index, multi-word explain) | Antigravity CLI 1.3.3; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-corpus-index-and-search.md) |
| 2026-10-13 | Backward link circulation 0.11.0 (`Shift-Tab`, status line, UTF-8 safety) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-link-navigation-backward.md) |
| 2026-10-13 | Forward path navigation 0.12.0 (right arrow forward, referring page memory) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-right-arrow-forward.md) |
| 2026-10-13 | Hyperlink display fixes 0.13.0 (no leading-space underline, multiline wrapped continuation highlight) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-hyperlink-display-fixes.md) |
| 2026-10-13 | Command and repository roadmap (next steps 0.14.0/0.15.0 through future 1.3.0) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-command-roadmap.md) |
| 2026-10-13 | Ecosystem chapters in the guide (`knowledge/ecosystem/`); private references removed from public docs | Claude Code 2.1.296; `claude-opus-5-5` | [Session record](docs/ai/2026-10-13-tam-ecosystem-chapters-and-public-cleanup.md) |
| 2026-10-13 | In-pager text search, reading telemetry and Bash completions 0.14.0 (`/`, `n`/`N`, status, static bash completion) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-in-pager-search-and-completions.md) |
| 2026-10-14 | Content-depth ladder and fading templates 0.15.0 (`explain --level`, `depth.c`, metadata projection) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-content-depth-ladder.md) |
| 2026-10-14 | Command/corpus lane build split (`make command`, `make corpus`) | opencode 1.19.0; `big-pickle` | [Session record](docs/ai/2026-10-14-tam-command-corpus-lanes.md) |
| 2026-10-14 | Lua scripting topic in the ecosystem chapters | opencode 1.19.0; `big-pickle` | [Session record](docs/ai/2026-10-14-tam-lua-topic.md) |
| 2026-10-14 | Read-only self-inspection findings and check contract 0.16.0 (`install inspect`, `check`, JSON contract) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-self-inspection-findings.md) |
| 2026-10-14 | Guide breadcrumb trails, beginner/intermediate/expert reading paths, and welcome-page refinement (corpus lane) | opencode 1.19.0; `big-pickle` | [Session record](docs/ai/2026-10-14-tam-corpus-breadcrumbs.md) |
| 2026-10-14 | Opt-in grounded ask CLI surface 0.17.0 (`ask`, `--json`, `qwen2.5-coder:1.5b`, bounded excerpt grounding, local Ollama client) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-grounded-ask.md) |
| 2026-10-14 | Navigation and output ergonomics 0.18.0 (`` ` `` guide index return, Goldilocks screen budget, interactive `explain topics`, `--terse`/`--verbose`) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-navigation-and-output-ergonomics.md) |
| 2026-10-14 | Maintain track corpus expansion 0.19.0 (`maintain/` chapters, updates/rollback/logs/power/recovery, two-pass alias resolution) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-maintain-track-corpus-expansion.md) |
| 2026-10-14 | User Guide TOC, botanical analogy, and tam(1) man page 0.20.0 (`knowledge/index.html`, `ecosystem/terminology.html`, `man/tam.1`, `show topics`) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-user-guide-and-man-page.md) |
| 2026-10-14 | "Improve" track knowledge corpus expansion 0.21.0 (`knowledge/improve/`, efficiency, zram, terminal, scripting, toolchains, ai-performance) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-improve-track-corpus.md) |
| 2026-10-15 | Native GNU Info Subsystem 0.22.0 (`tam info`, `src/info.c`, pager keys `n`/`p`/`u`/`t`/`d`, cross-system `info:` URIs, indirect tags, completions) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-15-tam-gnu-info-subsystem.md) |
| 2026-10-15 | Pager hyperlink navigation ergonomics (`a`/`s`/`d`/`f` 2D navigation, first-visible-link selection, Info dir rebind `i`) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-15-tam-hyperlink-navigation-keys.md) |



