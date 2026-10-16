# Build-time Git derivation of page revisions and timestamps (command 0.28.0, corpus 0.28.0)

- **Date:** 2026-10-16
- **Time:** 14:15 to 14:32 -04:00
- **Stage:** Develop — `tam` command 0.28.0 and corpus 0.28.0 (`tools/build-index.py`, `knowledge/index.tsv`, `knowledge/index.json`, `knowledge/version`, `src/index.h`, `src/index.c`, `src/depth.h`, `src/depth.c`, `src/tam.c`, `src/version.h`, `tests/run.sh`, `tests/guide.sh`, `ROADMAP.md`, `docs/plans/development.md`)
- **CLI Tool:** Antigravity CLI 1.4.0 (`agy 1.4.0`)
- **Model:** Gemini 4 Argon (`gemini-4-argon`)
- **Transcript:** Retained privately by the author.
- **Commits:** Recorded in git history on `develop`.
- **Authorship:** Fred directed deriving `Rev <N>` and `Last updated <YYYY-MM-DD HH:MM>` deterministically from Git history at build time so corpus authors and agents never have to manually edit revision strings inside raw `.html` files; Antigravity CLI implemented the build-time derivation in `tools/build-index.py`, the index parser and HTML injection in `src/`, and the test assertions.
- **Checked against:** `tools/build-index.py`, `knowledge/index.tsv`, `knowledge/index.json`, `src/index.h`, `src/index.c`, `src/depth.h`, `src/depth.c`, `src/tam.c`, `tests/guide.sh`, `tests/run.sh`, `tests/render.sh`.

## Guiding prompts

> Showing Rev <N> Last updated <YYYY-MM-DD HH:MM> to the reader is great for transparency. However, if humans or agents have to manually type and increment that string inside every .html file, it will cause constant merge conflicts, golden-snapshot churn, and forgotten updates. Instead, if we have tools/build-index.py (or the corpus build step) derive Rev <N> (git rev-list --count HEAD -- <page>) and Last updated (git log -1 --format=%cd -- <page>) automatically from Git history at build time, every page gets an exact, truthful revision and timestamp with zero manual maintenance. Please implement build-time Git derivation using a deterministic script.

## Key decisions

- **Timezone-invariant commit timestamp formatting (`--date=format:%Y-%m-%d %H:%M`):** `git_page_revision()` in `tools/build-index.py` uses `git rev-list --count HEAD -- knowledge/<rel_path>` for `rev` and `git log -1 --format=%cd --date=format:%Y-%m-%d %H:%M -- knowledge/<rel_path>` for `updated`. Because `--date=format:...` (as opposed to `--date=format-local:...`) formats the timestamp in the timezone recorded inside the Git commit object itself, `tools/build-index.py` produces identical `knowledge/index.tsv` and `knowledge/index.json` artifacts regardless of the build host's `TZ` environment variable or file `mtime`. Uncommitted new files with zero commits in `HEAD` fall back cleanly to `rev = 1` and UTC `mtime`.
- **Index-backed footer injection (`tam_depth_apply_rev()`):** `read_corpus_page()` looks up the requested page in `knowledge/index.tsv` via `tam_index_lookup_path()` and passes `rev` and `updated` to `tam_depth_apply_rev()`, which injects `<p>Rev <N> Last updated <YYYY-MM-DD HH:MM></p>` as a document footer immediately before the closing `</body>` tag (leaving page titles, breadcrumbs, and depth options at the top undisturbed). Standalone renderer fixtures in `tests/render/` and `tests/fixtures/` (which have no `index.tsv`) receive `rev = 0`, preserving all golden layout snapshots untouched.

## Verification

- `make test` (with `PREFIX` and `LIBTAM_PREFIX` set to the local development prefix): `tests/run.sh`, `tests/render.sh` (6 fixtures), `tools/lint-corpus.py`, `tests/test_lint_corpus.py` (29 tests), and `tests/guide.sh` (verifying `^Rev [1-9][0-9]* Last updated [0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}$` on every guide page) all pass.
- Installed development build via `make install`.
