# Code taken from other projects

This file lists every piece of code or documentation `tam` takes from
another project. An idea taken without its source goes in `ideas/`, not
here.

## tldr-pages

- **Project:** tldr-pages (`https://github.com/tldr-pages/tldr`)
- **License:** Creative Commons Attribution 4.0 International (`CC-BY-4.0`);
  see `knowledge/tldr/LICENSE.md`
- **Source:** `pages/linux/` and `pages/common/`, commit
  `ba104601bfa537bf9ce54d898bd25ea3c42d93b2` (2026-11-01, recorded in
  `knowledge/tldr/SOURCE.json`)
- **Here:** `knowledge/tldr/pages/` (724 curated English pages) and
  `knowledge/tldr/index.tsv`
- **Changes:** Selected English pages relevant to Tamlinux and indexed
  for offline rendering by `tam tldr`.
- **Taken:** 2026-11-01 (most recent refresh; see `knowledge/tldr/SOURCE.json`)

## How to record a piece

One section per piece, newest last:

    ## <what it is>

    - **Project:** name and URL
    - **License:** the project's license
    - **Source:** file path in the project, and the commit it came from
    - **Here:** the file or files it now lives in
    - **Changes:** how it was rewritten or reshaped, and why
    - **Taken:** YYYY-MM-DD

Add the project's copyright notice to `COPYRIGHT` in the same commit.
When upstream fixes a bug in code we took, check whether our copy has
the same bug, and record what we did in `bugs/`.
