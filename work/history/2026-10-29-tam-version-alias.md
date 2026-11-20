# Short version option (command 0.45.0)

- **Date:** 2026-10-29
- **Time:** 06:08 to 06:10 -04:00
- **Stage:** Develop
- **CLI Tool:** Codex CLI 0.165.0
- **Model:** `gpt-6.1-terra`
- **Transcript:** Retained privately by the author.
- **Commits:** The commit that adds this record.
- **Authorship:** Fred requested the alias and asked about help; Codex implemented and verified it.
- **Checked against:** `src/tam.c`, `tests/run.sh`, `completions/bash/tam`, `docs/plans/development.md`.

## Guiding prompts

> Make tam -v be a synonym for tam --version. Does tam -h work for --help?

> install that version of tam so I can test

## Key decisions

Retain the existing `-V` alias and add `-v` to help and completion. `-h` already aliases `--help`.

## Verification

`timeout 300 make test-command` passed, including 15 upgrade tests and renderer goldens. Live Ollama checks skipped because no server was available. The full `make test` found a decision-record format error, corrected in a follow-up commit; `make check-repo` passed afterward.
