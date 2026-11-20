# TCK25 Learning and Installing work (corpus 0.45.0)

- **Date:** 2026-10-27
- **Time:** 10:00 to 14:28 -04:00
- **Stage:** Develop; corpus source
- **CLI Tool:** Codex CLI 0.164.0
- **Model:** GPT-6.1 Terra (`gpt-6.1-terra`)
- **Transcript:** Retained privately by the author.
- **Commits:** The commit that adds this record and the index follow-up.
- **Authorship:** Fred requested completion; Codex wrote and checked this slice.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, `tools/build-index.py`, `src/install.c`, `src/ask.c`, checkout `tam install --help` and `tam ask --help`, the Tamlinux text-startup and base-system plans, native-session login documentation, user-defaults Bash sources, local `systemctl(1)`, `tty(1)`, C compiler, local Ollama and the actual example outputs.

## Guiding prompt

> Finish up TCK25.

> Continue

## Key decisions

Distinguish the existing graphical-login route from text-first startup and avoid fixed VT assumptions. Keep installation inspection distinct from an installer. Classify Learning and Installing pages, tag instructional lessons and enable the existing tagged-section checks for all parts. Add six catalog paths, including a captured local inference example and a checked scratch build. The paths guide bounded practice and do not establish learner acceptance, a particular power saving or an upstream contribution. The anatomy checker skips untagged material, including the placement assessment, so a zero warning count is not proof of teaching quality. Corrected misleading resource, privacy and shell-safety claims and removed recursive cleanup from the affected exercises. The client accepts non-loopback endpoints and strips an HTTPS prefix without TLS despite its local-only help; the command defect remains open. TCK25 source implementation is complete; pilot, learner and packaged acceptance remain pending.

## Verification

`make corpus-and-test` and the final `make test` passed, including live Ollama checks. Ran the scratch builds and scripting examples, captured a real inotify event and a local grounded answer, and inspected rendered pages. A prior full run found a live-corpus assertion tied to the AI lesson title; preserving the title fixed it.
