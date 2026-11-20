# Restore current version history rows

- **Date:** 2026-11-07
- **Time:** 14:04 to 14:05 -05:00
- **Stage:** Develop; documentation correction for existing versions
- **CLI Tool:** Codex CLI `0.167.0`
- **Model:** `gpt-6.1-sol`
- **Transcript:** Retained privately by the author.
- **Authorship:** Fred requested completion of Tamlinux 0.6.15 packaging; Codex restored two missing roadmap history rows.
- **Checked against:** `src/version.h`, `knowledge/version`, `ROADMAP.md`, commits `b3516f8` and `48c97de`, and `tools/check-repo.py`.

## Guiding prompts

> Proceed. The goal is to package up and test Tamlinux 0.6.15. Continue working toward that goal. Ask me for help if you get stuck.

> Keep 0.6.15: authorize this documentation-only re-freeze

The selected source passed its functional checks but failed the repository
metadata gate because the delivered-history table omitted command 0.49.1 and
corpus 0.75.3. The correction records the work already delivered by those
versions. It changes no command code or corpus content and does not bump either
version. Fred explicitly authorized re-freezing the uninstalled candidate with
this documentation correction and rerunning the affected checks.

Verification: `python3 tools/check-repo.py` and `git diff --check` pass.
