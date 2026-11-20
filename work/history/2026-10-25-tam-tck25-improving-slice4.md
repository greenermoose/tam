# TCK25 Improving track migration slice 4 (corpus 0.43.6)

- **Date:** 2026-10-25
- **Time:** 17:25 to 19:25 -04:00
- **Stage:** Develop; two corpus pages migrated and Glossary alphabetized, command unchanged at 0.43.2.
- **CLI Tool:** Antigravity CLI 1.4.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** Implementation commit and the generated-index follow-up containing this line.
- **Authorship:** The developer directed continuing TCK25 Improving track migration with the next slice: `improve/ai-performance.html` and `ecosystem/patches.html`, claiming scopes first under `AGENTS.md`, and later requested alphabetizing the Glossary terms. Antigravity CLI claimed the scopes, migrated `improve/ai-performance.html` and `ecosystem/patches.html` to the full ladder anatomy with captured read-only examples, mapped skills (`hardware.beginner`, `hardware.intermediate`, `hardware.expert`, `customizing.beginner`, `customizing.intermediate`, `customizing.expert`), alphabetized all 23 terms in the Glossary hub (`learn/glossary/index.html`), reduced corpus-wide anatomy warnings from 10 to 8, and updated plan and roadmap records.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, `tools/build-index.py`, `tools/lint-corpus.py`, `knowledge/learn/skills/skills.json`; live captured `free -h` metrics, `tam ask --help` and fallback output, `/proc/cpuinfo` AVX/AVX2/FMA vector flags, `/etc/pacman.conf` repository precedence rules, `/var/lib/local-patches-upstream/status` verification verdicts, `pacman -Q` and `pacman -Qi` package queries, `vercmp` version comparisons, unprivileged `makepkg -Cf` build rules, public fork branch conventions, and `50-local-patches-upstream.hook` retirement detection; [free(1)](https://man7.org/linux/man-pages/man1/free.1.html), [pacman(8)](https://man7.org/linux/man-pages/man8/pacman.8.html), [pacman.conf(5)](https://man7.org/linux/man-pages/man5/pacman.conf.5.html), [vercmp(8)](https://man7.org/linux/man-pages/man8/vercmp.8.html), [makepkg(8)](https://man7.org/linux/man-pages/man8/makepkg.8.html), [PKGBUILD(5)](https://man7.org/linux/man-pages/man5/PKGBUILD.5.html).

## Guiding prompts

> In the tam repo, we are completing checklist item TCK25. Improving track slices 1–3 landed in corpus 0.43.3, 0.43.4, and 0.43.5. Continue TCK25 Improving track migration with the next slice: improve/ai-performance.html and ecosystem/patches.html. Follow tam/AGENTS.md and claim scopes before editing.

> While you're making improvements, alphabetize the list of terms in the Glossary (Path = page:learn/glossary/index.html).

## Key decisions

- Use a corpus patch bump (0.43.6) because this slice restructures and migrates existing lessons and alphabetizes the glossary without introducing new guide pages or command changes.
- `improve/ai-performance.html` was migrated to the full ladder anatomy with captured read-only `free -h` and `tam ask` queries, memory bus bandwidth limits (`Tokens/sec &asymp; Bandwidth / Model_Size`), quantization tradeoffs (FP16, Q8_0, Q4_K_M, Q2_K), KV cache scaling calculations, CPU vector register SIMD utilization (AVX2, AVX-512), and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to `hardware.beginner`, `hardware.intermediate`, and `hardware.expert`.
- `ecosystem/patches.html` was migrated to the full ladder anatomy with captured read-only local patch status inspection (`/var/lib/local-patches-upstream/status`), repository precedence rules in `pacman.conf`, patch export workflows (`git format-patch`), clean unprivileged `makepkg -Cf` builds with local `-debug` symbols, automated retirement detection via `50-local-patches-upstream.hook` and `vercmp`, public fork conventions (`greenermoose/<Upstream>`), and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to `customizing.beginner`, `customizing.intermediate`, and `customizing.expert`.
- `learn/glossary/index.html` was updated to sort all 23 defined terms in strict alphabetical order.
- Whole-corpus anatomy warnings dropped from 10 to 8 (all 8 remaining warnings are in unmigrated lessons). Unclassified pages dropped from 41 to 39; unmapped pages dropped from 83 to 79.

## Verification

- `make corpus`: cleanly generated index and sidecars with 110 entries.
- `python3 tools/lint-corpus.py`: zero findings or errors.
- `python3 -m unittest tests/test_lint_corpus.py tests/test_build_index.py`: 52 tests passed (OK).
- `sh tests/guide.sh`: all 12 guide test assertions passed cleanly.
- `make check-repo`: passes cleanly with updated version header in `ROADMAP.md`.
- `coverage.json`: confirmed warnings dropped from 10 to 8, unclassified pages fell from 41 to 39, and unmapped pages fell from 83 to 79.
- All captured outputs verified against live host command outputs.

Pilot/learner review and physical Linux-console/Foot acceptance of an exact packaged candidate remain unverified. No package deployment or stage promotion was performed.
