# Kerberos glossary page (corpus 0.91.0)

- **Date:** 2026-11-14
- **Time:** 22:18 to 22:20 -05:00
- **Stage:** Develop — add `glossary/k/kerberos.html` and link it from `glossary/k/index.html` (`corpus 0.91.0`)
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** `704a8ac` and follow-up index/record commit
- **Authorship:** Fred requested a Kerberos glossary page; Antigravity CLI authored `knowledge/glossary/k/kerberos.html`, linked it in `knowledge/glossary/k/index.html`, bumped `knowledge/version` to `0.91.0`, rebuilt the corpus indexes, and updated `ROADMAP.md`.
- **Checked against:** `knowledge/glossary/k/kdc.html`, `knowledge/glossary/t/tgt.html`, `knowledge/glossary/c/cts-hmac.html`, `knowledge/glossary/n/ntp.html`, `knowledge/security/protocols/security-protocols.html`, `knowledge/glossary/k/index.html`, `knowledge/version`, `ROADMAP.md`

## Guiding prompts

> Write a page about kerberos for the tam knowledge corpus glossary. It is mentioned in passing but it's missing from the glossary.

## Key decisions

None beyond the prompts.

## Verification

- `make corpus-and-test`: 1132 pages parsed and rendered, component coverage check passed, 92 corpus tooling unit tests passed, and `tools/check-repo.py` passed.
- `env TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe glossary/k/kerberos` and `env TAM_KNOWLEDGE_DIR=knowledge build/tam explain kerberos`: rendered the new glossary page and verified topic lookup.
- `make check-repo`: passed.
