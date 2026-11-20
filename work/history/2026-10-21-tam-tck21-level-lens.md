# Sticky level lens, TCK21 (command 0.37.0)

- **Date:** 2026-10-21
- **Time:** 05:56 to 06:25 -04:00
- **Stage:** Develop. Command 0.37.0 (minor); corpus unchanged.
- **CLI Tool:** Claude Code 2.1.306
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds this record
- **Authorship:** The developer asked to continue the learning levels plan.
  The AI tool implemented TCK21 as the plan specifies, wrote the tests,
  the man page, roadmap, decision record and plan updates, and stopped at
  the plan's banner-ranking question.
- **Checked against:** `docs/plans/learning-levels.md` (TCK21),
  `src/depth.c`, `src/pager.c`, `src/tam.c`, `src/render.h`, `src/index.h`,
  `knowledge/index.json` and `index.parents.tsv`, `knowledge/index.html`
  Quick keys table, `tests/run.sh`, `man/tam.1`, `completions/bash/tam`,
  libtam `tam_fs_read_file()`

## Guiding prompts

> Continue with the plan to improve learning levels in tam.

> I need to logout. Save a handoff, commit your work, release your work claims, and let me know when I can safely close this session.

## Key decisions

- The lens lives in `src/depth.c` (resolved once per process, saved
  through `tam_state_path()` in `src/tam.c`); the pager strips `level=`
  from every address it stores and appends the lens when it loads, so
  history, Back and Forward follow the current lens.
- Topic views (`tam:explain`, `tam:topics`, `tam:keywords`) take the lens
  because they already filtered by level; piped output never reads the
  saved lens, so scripts are reproducible.
- `tam_document_t` gained `levels` and `hidden` so the status row and
  messages know what the depth layer did.
- Stopped and asked: the plan's related-page rule gives poor banner links on
  the real corpus. The rule as written shipped; the question and a
  recommendation are recorded in the plan. The `tamlinux` repository's
  `welcome_seen` sentence is pending because that checkout was held.

## Verification

`make test`: command and corpus suites pass, including the new PTY cases
for persistence across links, Back and restart, `TAM_LEVEL` and `--level`
overrides, level links, banners, hidden-section markers and the status row
at 40, 80 and 120 columns. `make check-repo` passes.
