# Glossary acronym and portmanteau definitions (corpus 0.75.3)

- **Date:** 2026-11-06
- **Time:** 14:06 to 15:25 -05:00
- **Stage:** Develop — corpus 0.75.3 glossary acronym definitions and portmanteau explanations
- **CLI Tool:** Antigravity CLI 1.5.2, checked from the running binary (`agy --version`).
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `48c97de`, `5b2f620`, and the commit that records them
- **Authorship:** Fred Horch directed a review of all 397 glossary terms in the Tam knowledge corpus to include full definitions of acronyms in entries and headings, and explanations for portmanteaus in parentheses. The AI tool systematically analyzed all 397 terms, identified 105 acronyms and 20 portmanteaus/abbreviations, updated all page titles, `<h1>` headings, breadcrumbs, and index listings, added ARM naming history and ZRAM Ziv compression origin notes, bumped the patch version, updated `ROADMAP.md`, regenerated the indexes, and verified all tests pass without errors.
- **Checked against:**
  - `knowledge/learn/glossary/*.html` (all 397 glossary pages)
  - `knowledge/learn/glossary/index.html` (glossary master index)
  - POSIX, IEEE 802, IETF RFC, and VESA standards for technical acronym definitions
  - `src/index.c` (glossary candidate search scoring)
  - `tools/build-index.py` (title extraction and breadcrumb validation)
  - `tools/lint-corpus.py` (corpus HTML rules and linting)
  - `tests/guide.sh` (guide rendering and link verification)

## Guiding prompts

> In the tam knowledge corpus, include defitions of acronyms in entries and headings of Glossary pages.
> 
> E.g.
> 
> ABI
> 
> should be
> 
> ABI - Aplication Binary Interface
> 
> Do a thorough review of all terms in the glossary. If the term is an acronym, find out what it is and put it on the line that shows on the list of words used in the guide and in the title of the glossary page (e.g., learn/glossary/abi.html).
> 
> Ask if you have questions. There are dozens or more acronyms in the glossary.

> I misspelled Application

> 1 yes
> 2 yes, but note history in page text
> 3 yes
> 4 okay
> 5 explain the portmanteaus in parentheses, like ZRAM (Z in UNIX means compressed), and then explain in the ZRAM page that Z for compression comes from Jacob Ziv.

## Key decisions

- Formatted all 104 standard acronyms as `ACRONYM - Expansion` in `<title>`, `<h1>`, breadcrumbs, and `index.html`.
- Formatted portmanteaus, contractions, and compound terms in parentheses: `Term (explanation)`.
- Updated `CRC` to `CRC - Cyclic Redundancy Check` and re-alphabetized its position in `index.html` between `CPU` and `CRT`.
- In `arm.html`, expanded title to `ARM - Advanced RISC Machines` and added historical explanation of Acorn RISC Machine (1983), Advanced RISC Machines (1990), and modern styling Arm to the learner definition.
- In `zram.html`, titled as `ZRAM (Z in UNIX means compressed)` and explained in the novice definition that the letter Z signifies compression in Unix tradition in honor of Jacob Ziv (co-inventor of Lempel-Ziv algorithms).
- Expanded `UTF-8` as `UTF-8 - Unicode Transformation Format (8-bit)`.
- Expanded `UID`, `GID`, and `PID` using "Identifier" (`User Identifier`, `Group Identifier`, `Process Identifier`).
- Preserved existing keywords, summaries, and structural level markup across all 125 modified glossary files.

## Verification

- `make corpus`: generated `index.tsv` and companion outputs across all 582 corpus entries cleanly.
- `make corpus-and-test`: passed all checks:
  - Corpus HTML linter (`tools/lint-corpus.py`) clean.
  - Corpus linter unit tests (`test_lint_corpus.py`) passed (35 tests).
  - Corpus index unit tests (`test_build_index.py`) passed (22 tests).
  - Component coverage unit tests (`test_component_coverage.py`) passed (6 tests).
  - Component coverage gap check passed.
  - Guide verification (`tests/guide.sh`) passed all 12 assertions.
- `tam show --pipe`: verified visual text rendering of `learn/glossary/abi`, `learn/glossary/arm`, `learn/glossary/zram`, and `learn/glossary/crc`.
