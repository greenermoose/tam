# Comprehensive glossary expansion (corpus 0.71.0)

- **Date:** 2026-11-04
- **Time:** 12:04 to 12:40 -05:00 (opencode authoring); ~15:55 -05:00 (Antigravity completion)
- **Stage:** Develop. Corpus 0.70.1 to 0.71.0; command version unchanged at 0.48.2.
- **CLI Tool:** opencode 1.19.4 (content); Antigravity CLI 1.5.0 finished the commit after opencode hit its free usage limit
- **Model:** big-pickle (`big-pickle`); Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`) for handoff commit
- **Transcript:** Retained privately by the author; Antigravity takeover under Fred's direction 2026-11-04.
- **Commits:** This commit on `develop` (corpus 0.71.0).
- **Authorship:** Fred directed the glossary expansion, requesting the full technical set plus specialist and Tamlinux product terms for novices and computer learners, delivered as a single 0.71.0 corpus release. OpenCode authored the 319 term pages, rebuilt the glossary index, added cross-links in `New words` sections, updated the roadmap and version, and regenerated corpus indexes; Antigravity took over the claim under Fred's instruction, verified tests, recorded provenance, and committed.
- **Checked against:** `docs/corpus-pedagogy.md`, `docs/corpus-authoring.md`, `knowledge/learn/glossary/index.html`, `tools/build-index.py`, `tools/lint-corpus.py`, and existing glossary entries (e.g. `crc.html`).

## Guiding prompts

> Take a look at the glossary for the tam knowledge corpus. Look for technical terms used in the knowledge corpus but not defined in the glossary. Propose additions to the glossary. The goal is to define every technical term that a complete novice would not understand, and that anyone learning about computers would need to know.

> Full set plus the specialist and product terms.

> One large 0.71.0 glossary release.

> I accept the plan. Begin work on improving the glossary.

> Proceed.

> Continue if you have next steps, or stop and ask for clarification if you are unsure how to proceed.

> Take the work claims from opencode and finish up the four steps.

## Key decisions

- Expand the glossary comprehensively with 319 new terms (bringing total glossary entries from 78 to 397), covering digital logic, hardware components, boot & OS concepts, filesystems & storage, shell, C/development toolchains, networking, security, graphics, desktop, and Tamlinux-specific product terms (e.g. `assembly-tamlinux.html`, `store-path.html`, `rollback-point.html`, `carried-patch.html`).
- Delivered as a single unified corpus release `0.71.0`.
- All glossary term pages use the standard 3-tier pedagogical level format (`<p>`, detailed explanations, and navigation footers) without using unrelated ladder templates.
- Cross-linked unlinked terms in `New words` blocks across 46 tutorial and guide pages, carefully avoiding ambiguous or context-dependent mappings (such as Micro's internal command prompt vs. shell prompt, and audio input/output devices vs. standard streams).
- Fixed a private-path lint error in `home-directory.html` to ensure `tools/lint-corpus.py` passes cleanly.

## Verification

- `make corpus-and-test`: Corpus index generation (563 entries), HTML linter (`tools/lint-corpus.py`), linter unit tests (`tests/test_lint_corpus.py`), index unit tests (`tests/test_build_index.py`), and guide checks (`tests/guide.sh`) pass with no errors.
- `make test`: Full command test suite, upgrade tests, golden tests, and corpus tests pass cleanly in ~15s.
