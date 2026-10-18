# Session: 2026-10-18 — Source upgrades and stage switching (command 0.31.0, corpus 0.32.0)

- **CLI Tool**: Codex CLI 0.163.0
- **Model**: GPT-6.1 Sol (gpt-6.1-sol)
- **Implementation author**: Codex, directed by Fred
- **Implementation commits**: `f721120` (feature), `5a84294` (API root fix);
  `09101b1` records the handoff and acceptance state.
- **Transcript**: Retained privately by the author.
- **Prompt**:
  > Implement the accepted plan in tam/docs/plans/upgrade.md. Begin with tam/docs/handoffs/tam-upgrade-luna.md and complete TCK6–TCK8.
- **Implementation notes**:
  - Added four-stage metadata discovery and independent command/corpus SemVer comparison with bounded GitHub transport, strict JSON handling, exact release identity, and running source metadata.
  - Added pinned source-build metadata, audited compatibility pins for recorded legacy source commits, isolated source generations, owned endpoint links, durable receipts, activation verification and offline rollback through a retained recovery helper.
  - Added upgrade / switch dispatch, offline help, Bash completion, manual, README, corpus guidance and an explicit first-install bootstrap procedure.
  - Allocated command 0.31.0 and corpus 0.32.0. Existing selected source versions remain unchanged when rebuilt.
- **Verification**:
  - `python3 -B -m unittest discover -s tests -p 'test_upgrade.py' -q`: 12 tests passed after final ownership hardening.
  - `TAM_TEST_SKIP_OLLAMA=1 make test PREFIX=$HOME/.local/dev LIBTAM_PREFIX=$HOME/.local/dev`: passed the complete command and corpus suite before final helper ownership hardening; no install target was run.
  - Python compile, shell syntax checks, and `git diff --check` passed after final hardening.
  - The full automated suite exercises fake-HOME activation, failure preservation, link ownership, locking and rollback. No successful live metadata check, real source build, or development-prefix activation was completed.
- **Live acceptance attempt**: The fixed GitHub repository root returned HTTP 404. Pushes to `origin/develop` failed with SSH public-key denial; `gh auth status` reported the configured token invalid. No credential was changed or bypassed. The endpoint and source access need to be restored before the live check and isolated build can pass.
- **Outstanding acceptance**: The accepted plan requires one real source build in an isolated prefix. First activation at ~/.local/dev still requires Fred's explicit instruction. Package assembly and Test/Run/Release remain separate.
