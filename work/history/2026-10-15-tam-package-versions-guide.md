# Package Versions Specification Guide and Corpus Bump (tam 0.25.0)

- **Date:** 2026-10-15
- **Stage:** Develop — Knowledge Corpus Expansion (`ecosystem/package-versions.html`) and Knowledge Corpus Version Bump to 0.25.0.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.6
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested installing `tam` version 0.25.0 and making it ready for Tamlinux 0.4.4, noting that he did not fully understand how package versions are specified for each Tamlinux release, and asked for an investigation and a dedicated page in the `tam` knowledge corpus alongside a corpus version bump. Antigravity claimed the affected paths under `tam-work`, researched the packaging and assembly architecture across `tamlinux-packages`, authored `knowledge/ecosystem/package-versions.html`, bumped the corpus version to 0.25.0, updated the corpus index, guide tests, and golden snapshots, and verified the test suite.

## Guiding prompt

> Please install tam version 0.25.0 and make that version ready for Tamlinux 0.4.4. I don't fully understand how we specify which version of which packages are included in each version of Tamlinux. Please investigate that and add a page to the tam knowledge corpus. Bump the version number of the corpus when you add that page. Ask if you have any questions.

## Decisions and scope

- **New Knowledge Corpus Page (`knowledge/ecosystem/package-versions.html`):**
  - Explains the architectural separation between the top-level Tamlinux product version (cultivar SemVer and candidate letters) and independent component versions (`tam`, `libtam`, `tamlinux-tools`, `tamlinux`).
  - Details the role of `tamlinux-packages` as the central assembly repository and single source of truth for the system bill of materials (BOM).
  - Documents how `flake.nix` declares inputs for Nixpkgs (locking base tools and libraries), Home Manager, and component source repositories.
  - Documents how `flake.lock` provides cryptographic, tamper-evident byte reproducibility (`narHash`).
  - Details the packaging recipes in `nix/packages/default.nix`, closed runtime `PATH` closures, and the Home Manager deployment linkage.
  - Provides a 3-depth ladder (`beginner`, `intermediate`, `expert`):
    - *Beginner:* Product versions vs component versions, inspecting running versions via `tam --version` and `tam-deploy status`.
    - *Intermediate:* The assembly repository, flake pinning architecture, derivation layers, and workstation deployment.
    - *Expert:* The full lifecycle from component development and `~/.local/dev` overrides to commit, pinning in `tamlinux-packages`, `nix flake lock`, candidate testing, Run endorsement, and dev override cleanup.
- **Corpus Version Bump (0.25.0):**
  - Updated `knowledge/version` from `0.24.0` to `0.25.0`.
  - Updated `TAM_DEFAULT_CORPUS_VERSION` in `src/version.h` to `"0.25.0"`.
- **Navigation & Tests:**
  - Added links to the new page in `knowledge/ecosystem/index.html` and `knowledge/index.html`.
  - Rebuilt index with `tools/build-index.py` (45 topics).
  - Added `ecosystem/package-versions` to `tests/guide.sh` and updated corpus version test in `tests/run.sh`.
  - Regenerated golden text snapshots for `index`, `ecosystem-index`, and new `ecosystem-package-versions`.

## Verification

- `python3 tools/build-index.py`: Validated metadata and breadcrumbs; generated 45 entries.
- `make test`: All command tests and guide tests passed cleanly.
- `UPDATE=1 tests/guide.sh`: Golden text snapshots match.
