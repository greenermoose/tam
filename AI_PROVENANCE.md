# AI collaboration and provenance

Fred directs this project. AI-assisted work records the tool and verified
version, model, guiding prompts, authorship boundaries and verification.
Session transcripts are retained privately by the author; public records
carry no transcript identifiers or store paths.

AI-assisted commits carry `Co-authored-by`, `AI-Tool` and `AI-Model` trailers.
Human-only commits retain their original authorship.

Pushed commits are never rewritten, so some trailers stay as committed.
These AI-assisted commits carry no AI trailers: `b55522c`, `cacdd78`,
`020ea30`, `326063d`, `7f5b579`, `1e88746`, `2921c85`, `b58c01c`,
`3a7e577`, `9464b0f` and `698756c`. Several Antigravity commits from
2026-10-14 and 2026-10-15 name an earlier `agy` version than the one that
ran. The table and session records give the verified tool, version and
model for each session.

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
| 2026-10-09 | Use track corpus, table and definition-list rendering, and the `tam man` subsystem (0.8.0, 0.9.0) | Antigravity CLI 1.3.2; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-09-tam-use-track-and-man-subsystem.md) |
| 2026-10-12 | Plan revision: milestones 6–9 (corpus index/search, depth ladder, read-only inspection, opt-in `ask`) | opencode 1.19.0; `big-pickle` | [Session record](docs/ai/2026-10-12-tam-plan-revision.md) |
| 2026-10-13 | Corpus index and search 0.10.0 (`search`, `--json`, index, multi-word explain) | Antigravity CLI 1.3.3; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-corpus-index-and-search.md) |
| 2026-10-13 | Backward link circulation 0.11.0 (`Shift-Tab`, status line, UTF-8 safety) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-link-navigation-backward.md) |
| 2026-10-13 | Forward path navigation 0.12.0 (right arrow forward, referring page memory) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-right-arrow-forward.md) |
| 2026-10-13 | Hyperlink display fixes 0.13.0 (no leading-space underline, multiline wrapped continuation highlight) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-hyperlink-display-fixes.md) |
| 2026-10-13 | Command and repository roadmap (next steps 0.14.0/0.15.0 through future 1.3.0) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-command-roadmap.md) |
| 2026-10-13 | Ecosystem chapters in the guide (`knowledge/ecosystem/`); private references removed from public docs | Claude Code 2.1.296; `claude-opus-5-5` | [Session record](docs/ai/2026-10-13-tam-ecosystem-chapters-and-public-cleanup.md) |
| 2026-10-13 | In-pager text search, reading telemetry and Bash completions 0.14.0 (`/`, `n`/`N`, status, static bash completion) | Antigravity CLI 1.3.4; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-13-tam-in-pager-search-and-completions.md) |
| 2026-10-14 | Content-depth ladder and fading templates 0.15.0 (`explain --level`, `depth.c`, metadata projection) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-content-depth-ladder.md) |
| 2026-10-14 | Command/corpus lane build split (`make command`, `make corpus`) | opencode 1.19.0; `big-pickle` | [Session record](docs/ai/2026-10-14-tam-command-corpus-lanes.md) |
| 2026-10-14 | Lua scripting topic in the ecosystem chapters | opencode 1.19.0; `big-pickle` | [Session record](docs/ai/2026-10-14-tam-lua-topic.md) |
| 2026-10-14 | Plan ideas for milestone 10: Guide index key and a one-screen output budget | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-plan-guide-index-key-and-output-budget.md) |
| 2026-10-14 | Read-only self-inspection findings and check contract 0.16.0 (`install inspect`, `check`, JSON contract) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-self-inspection-findings.md) |
| 2026-10-14 | Guide breadcrumb trails, beginner/intermediate/expert reading paths, and welcome-page refinement (corpus lane) | opencode 1.19.0; `big-pickle` | [Session record](docs/ai/2026-10-14-tam-corpus-breadcrumbs.md) |
| 2026-10-14 | Opt-in grounded ask CLI surface 0.17.0 (`ask`, `--json`, `qwen2.5-coder:1.5b`, bounded excerpt grounding, local Ollama client) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-grounded-ask.md) |
| 2026-10-14 | Navigation and output ergonomics 0.18.0 (`` ` `` guide index return, Goldilocks screen budget, interactive `explain topics`, `--terse`/`--verbose`) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-navigation-and-output-ergonomics.md) |
| 2026-10-14 | Maintain track corpus expansion 0.19.0 (`maintain/` chapters, updates/rollback/logs/power/recovery, two-pass alias resolution) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-maintain-track-corpus-expansion.md) |
| 2026-10-14 | User Guide TOC, botanical analogy, and tam(1) man page 0.20.0 (`knowledge/index.html`, `ecosystem/terminology.html`, `man/tam.1`, `show topics`) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-user-guide-and-man-page.md) |
| 2026-10-14 | GNU Info subsystem plan (milestone 14) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-info-subsystem-plan.md) |
| 2026-10-14 | Help message tiers; `tam inspect` and `tam check` promoted to the top level | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-help-message-redesign.md) |
| 2026-10-14 | "Improve" track knowledge corpus expansion 0.21.0 (`knowledge/improve/`, efficiency, zram, terminal, scripting, toolchains, ai-performance) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-14-tam-improve-track-corpus.md) |
| 2026-10-15 | Native GNU Info Subsystem 0.22.0 (`tam info`, `src/info.c`, pager keys `n`/`p`/`u`/`t`/`d`, cross-system `info:` URIs, indirect tags, completions) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-15-tam-gnu-info-subsystem.md) |
| 2026-10-15 | Pager hyperlink navigation ergonomics (`a`/`s`/`d`/`f` 2D navigation, first-visible-link selection, Info dir rebind `i`) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-15-tam-hyperlink-navigation-keys.md) |
| 2026-10-15 | Dynamic Topic Guides, tam explain topic keywords, and tam command page 0.23.0 (Milestone 15: topic guides, keywords catalog, pager integration, tam.html, Makefile targets, build chain, develop->test->run->release cycle) | Antigravity CLI 1.3.5; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-15-tam-topic-guides-and-tam-page.md) |
| 2026-10-15 | Decoupled Versioning and Knowledge Corpus Alignment 0.24.0 (Milestone 16: decoupled command/corpus versioning, dynamic corpus discovery, tam inspect/check corpus alignment, tam hello/hi, botanical Fruit layer, 6 guiding principles) | Antigravity CLI 1.3.6; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-15-tam-decoupled-versioning-and-corpus-alignment.md) |
| 2026-10-15 | Attention line and anchored link navigation 0.25.0 | Antigravity CLI 1.3.6; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-15-tam-attention-line.md) |
| 2026-10-15 | Package version specification guide and corpus bump 0.25.0 (`ecosystem/package-versions.html`, assembly BOM, flake pinning, lifecycle) | Antigravity CLI 1.3.6; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-15-tam-package-versions-guide.md) |
| 2026-10-15 | `AGENTS.md`, lane-safe releases (command and corpus 0.25.1), and provenance and public-boundary repair | Claude Code 2.1.301; `claude-opus-5-5` | [Session record](docs/ai/2026-10-15-tam-agent-guide-and-provenance-repair.md) |
| 2026-10-16 | SDDM display manager guide and corpus bump (0.26.0 and 0.26.1: `ecosystem/sddm.html`, greeter architecture, autologin, session selection, PAM, recovery) | Antigravity CLI 1.4.0; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-16-tam-sddm-guide.md) |
| 2026-10-16 | TTY and Virtual Terminal (VT) knowledge corpus plan and note | Antigravity CLI 1.4.0; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-16-tam-tty-and-vt-corpus-plan.md) |
| 2026-10-16 | TTYs, virtual consoles, and terminal architecture guide and corpus bump 0.27.0 (`ecosystem/ttys-and-virtual-consoles.html`, VTs, PTYs, line discipline, seat handover) | Antigravity CLI 1.4.0; `gemini-3.8-flash-high` | [Session record](docs/ai/2026-10-16-tam-ttys-and-virtual-consoles.md) |
