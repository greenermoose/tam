# IETF and RFC corpus pages (corpus 0.92.0)

- **Date:** 2026-11-14
- **Time:** 22:27 to 22:32 -05:00
- **Stage:** Develop — added `knowledge/glossary/i/ietf.html`, `knowledge/glossary/r/rfc.html`, and `knowledge/networking/ietf-and-rfcs.html`, linked them from `glossary/i/index.html`, `glossary/r/index.html`, and `networking/index.html`, and bumped corpus version to `0.92.0`.
- **CLI Tool:** Antigravity CLI `1.5.9`
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending commit on `develop`
- **Authorship:** Fred requested pages about the IETF and how RFCs work; Antigravity CLI drafted the glossary entries, explanation page, hub links, and provenance record.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-style.md`, `docs/corpus-pedagogy.md`, `knowledge/glossary/k/kerberos.html`, `knowledge/glossary/r/rfc-5280.html`, `knowledge/networking/index.html`, `knowledge/skills/skills.json`, `agy --version`, `make corpus-and-test`, `make check-repo`.

## Guiding prompts

> Write pages about the IETF (Internet Engineering Task Force) and how RFCs work. These are mentioned in the tam knowledge corpus glossary but never defined.

## Key decisions

Added both glossary entries (`glossary/i/ietf.html`, `glossary/r/rfc.html`) and a three-level explanation page (`networking/ietf-and-rfcs.html`) so readers looking up either term in the glossary get concise definitions and a link to the full explanation of how Internet-Drafts, working groups, RFC streams, and Standards Track maturity work.

## Verification

- `make corpus-and-test`: corpus checks, component coverage check, 92 corpus tooling unit tests, and `check-repo.py` all pass.
- `python3 tools/report-coverage.py --warnings`: 0 anatomy warnings.
