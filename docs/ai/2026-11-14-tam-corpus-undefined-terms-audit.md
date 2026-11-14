# Undefined Technical Terms Audit and Glossary Expansion (Corpus 0.89.0)

- **Date:** 2026-11-14
- **Time:** 07:17 to 10:25 -05:00
- **Stage:** Develop — corpus 0.88.3 → 0.89.0
- **CLI Tool:** Antigravity CLI `1.5.9`
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending commit on `develop`
- **Authorship:** Fred directed the corpus-wide audit for undefined technical terms, requested both individual and family glossary pages, standards in both glossary and bibliography, and a `Bibliography` link on `knowledge/index.html`; Antigravity CLI ran the audit, authored the new pages, and verified the build.
- **Checked against:** `knowledge/ecosystem/terminal-input/escape-sequences.html`, `knowledge/ecosystem/terminal-input/linux-console.html`, `knowledge/index.html`, `knowledge/learn/glossary/`, `knowledge/learn/bibliography/`, `src/views.c`, `knowledge/index.paths.tsv`, `tools/lint-corpus.py`, `tools/build-index.py`.

## Guiding prompts

> Page ecosystem/terminal-input/escape-sequences.html says, "The CSI form is defined by ECMA-48" but a search of the knowledge corpus for ECMA-48 finds nothing. This is unacceptable.
> Please do a complete audit of the tam knowledge corpus for undefined technical terms like this, then define every single one of them.

> 1 both. Have individual slugs but also provide a family page that lists all the members of the family and links to their individual slug pages
> 2 yes
> Add a link to Bibliography on the guide index page, i.e.
> Guide topics
> Learning paths
> Keyword index
> Glossary
> Bibliography
> The learning paths should be a launching pad to the first page for every learning path through the knowledge corpus. Knowledge paths are defined by meta tags in each page.

## Key decisions

- Added 404 new three-level glossary pages (`knowledge/learn/glossary/*.html`, bringing the glossary to 827 entries) covering every verified undefined standard, protocol, acronym, and technical term across all 220 content pages, plus family overview pages (`c0-control-codes`, `c-standards`, `aes-variants`, `sha-algorithms`, `displayport-link-rates`, `pam-modulation`, `cpu-caches`, `unix-signals`) linking all individual member slugs.
- Added 10 formal bibliography entries (`ecma-48`, `iso-8859-1`, `iso-iec-9899`, `iso-iec-21031`, `ieee-802-3an`, `rfc5280-x509`, `rfc5869-hkdf`, `rfc9106-argon2`, `cta-861`, `wcag`) in `knowledge/learn/bibliography/`.
- Added `Bibliography` to `knowledge/index.html` and verified `tam:paths` (`src/views.c` + `knowledge/index.paths.tsv`) already provides a direct `Start` link to the first step of all 25 meta-tag-defined learning paths.

## Verification

- `make corpus-and-test`: passed (1081 pages parsed and rendered, 85 unit tests OK, `check-repo.py` passed).
- `make command` + `env TAM_KNOWLEDGE_DIR=knowledge build/tam explain ECMA-48` / `search ECMA-48` / `show --pipe`: verified against checkout corpus.
