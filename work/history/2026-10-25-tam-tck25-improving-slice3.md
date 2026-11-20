# TCK25 Improving track migration slice 3 (corpus 0.43.5)

- **Date:** 2026-10-25
- **Time:** 10:06 to 17:08 -04:00
- **Stage:** Develop; two corpus pages migrated, contributing page gaps resolved, command unchanged at 0.43.2.
- **CLI Tool:** Antigravity CLI 1.4.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** Implementation commit and the generated-index follow-up containing this line.
- **Authorship:** The developer directed continuing TCK25 Improving track migration with the next slice: `improve/zram.html` and `improve/toolchains.html`, claiming scopes first under `AGENTS.md`. Antigravity CLI claimed the scopes, migrated `improve/zram.html` and `improve/toolchains.html` to the full ladder anatomy with captured read-only examples, mapped skills (`hardware.beginner`, `hardware.intermediate`, `hardware.expert`, `contributing.beginner`, `contributing.intermediate`, `contributing.expert`), resolved all page gaps for the `contributing` domain skills, reduced corpus-wide anatomy warnings from 14 to 10, and updated plan and roadmap records.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, `tools/build-index.py`, `tools/lint-corpus.py`, `knowledge/learn/skills/skills.json`; live captured `zramctl` queries, raw byte counter metrics (`zramctl --bytes`), `swapon --show` priority orders, `/etc/systemd/zram-generator.conf` declarative settings, `/sys/block/zram0/` sysfs attributes (`comp_algorithm`, `mm_stat`), native C compilation and verification (`make test-command`), standalone version output inspection, `tam-work` path claims and commit discipline in a shared clone, host architecture compiler tuning (`-march=native`, `-pipe`, `-Wl,-rpath`), independent development lanes (`command` vs `corpus`), and dynamic library dependency checks via `readelf -d`; [zramctl(8)](https://man7.org/linux/man-pages/man8/zramctl.8.html), [swapon(8)](https://man7.org/linux/man-pages/man8/swapon.8.html), [zram-generator(8)](https://man7.org/linux/man-pages/man8/zram-generator.8.html), [zram-generator.conf(5)](https://man7.org/linux/man-pages/man5/zram-generator.conf.5.html), [make(1)](https://man7.org/linux/man-pages/man1/make.1.html), [gcc(1)](https://man7.org/linux/man-pages/man1/gcc.1.html), [readelf(1)](https://man7.org/linux/man-pages/man1/readelf.1.html).

## Guiding prompts

> In the tam repo, we are completing checklist item TCK25. Improving track slices 1 and 2 landed in corpus 0.43.3 and 0.43.4. Continue TCK25 Improving track migration with the next slice: improve/zram.html and improve/toolchains.html. Follow tam/AGENTS.md and claim scopes before editing.

## Key decisions

- Use a corpus patch bump (0.43.5) because this slice restructures and migrates existing lessons without introducing new guide pages or command changes.
- `improve/zram.html` was migrated to the full ladder anatomy with captured read-only `zramctl` queries, uncompressed vs compressed metrics (`DATA`, `COMPR`, `TOTAL`), exact compression ratio and net byte savings calculations (`zramctl --bytes`), systemd zram generator swap priority rules (`/etc/systemd/zram-generator.conf`), `zsmalloc` object slab allocator packing mechanisms, and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to `hardware.beginner`, `hardware.intermediate`, and `hardware.expert`.
- `improve/toolchains.html` was migrated to the full ladder anatomy with captured read-only native C compilation and verification (`make test-command`), standalone version output inspection, `tam-work` path claims and commit discipline in a shared clone, host architecture compiler tuning (`-march=native`, `-pipe`, `-Wl,-rpath`), independent development lanes (`command` vs `corpus`), and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to `contributing.beginner`, `contributing.intermediate`, and `contributing.expert`.
- Whole-corpus anatomy warnings dropped from 14 to 10 (all 10 remaining warnings are in unmigrated lessons). Unclassified pages dropped from 43 to 41; unmapped pages dropped from 83 to 81. All page gaps for `contributing` skills were resolved.

## Verification

- `make corpus`: cleanly generated index and sidecars with 110 entries.
- `python3 tools/lint-corpus.py`: zero findings or errors.
- `python3 -m unittest tests/test_lint_corpus.py tests/test_build_index.py`: 52 tests passed (OK).
- `sh tests/guide.sh`: all 12 guide test assertions passed cleanly.
- `make check-repo`: passes cleanly with updated version header in `ROADMAP.md`.
- `coverage.json`: confirmed warnings dropped from 14 to 10, unclassified pages fell from 43 to 41, unmapped pages fell from 83 to 81, and `contributing.beginner`, `contributing.intermediate`, and `contributing.expert` show `page_gap: false`.
- All captured outputs verified against live host command outputs.

Pilot/learner review and physical Linux-console/Foot acceptance of an exact packaged candidate remain unverified. No package deployment or stage promotion was performed.
