# Decouple Command Build ID from Corpus Edits and Accelerate Dev Install (tooling only)

- **Date:** 2026-11-10
- **Time:** 20:07 to 21:10 -05:00
- **Stage:** Develop; repository build system and developer loop tooling
- **CLI Tool:** agy 1.5.5
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** `ab77e14`
- **Authorship:** The developer asked why there was a lag at install during `make dev` for corpus edits and why a Git commit was embedded into the binary, then directed a plan to resolve the lag using a hybrid of lane-scoped build revisions and targeted install. The AI tool updated `Makefile` to scope command build IDs to command sources, added an `install-corpus` target, updated `tools/dev.py` to skip command compilation and phony index regeneration during corpus-only installs, and added unit tests in `tests/test_dev.py`.
- **Checked against:** `Makefile`, `tools/dev.py`, `tests/test_dev.py`, `tests/test_install_guard.py`, `tests/test_check_repo.py`, `docs/plans/development.md`, `docs/plans/upgrade.md`.

## Guiding prompts

> why is there a lag at install during make dev for the tam corpus?
> Why are we embedding a git commit into the binary?
> /plan Plan how to achive the goals of the project better. Think deeply and suggest some alternatives. Share them with me so I can choose which direction to take to make this work better.
> /plan Show me the plan in micro.
> /plan ready
> /plan What is your recommendation? Option 1 or a hybrid of 1 plus 3?
> /plan I agree. Do a hybrid of option 1 + option 3. Now, what about the revision semantics for tam --version?
> /plan This sounds good. Just to make sure I'm clear, this is the source of the version numbers:
>
> tam command 0.52.1 (dev ef8fc12) = built into the tam binary when it is compiled. Never changes until that binary is recompiled.
> knowledge corpus 0.78.11 (dev a1b2c3d) = read by the tam command from a corpus file. Stored in plain text in the knowledge corpus. Changed when any file in the knowledge corpus is changed. Completely separate from the tam binary.
> libtam 0.5.1 = built into the library when it is compiled. Never changes until that library is recompiled. The tam command can read this when it is being compiled against the library.
> Tamlinux cultivar KBN WB 0.6.22 = read by the tam command from the cultivar somehow (probably a text file or perhaps a special command that reports cultivar number). Not built into the tam command.
>
> Is this how it will work?
> /plan Proceed

## Key decisions

- In `Makefile`, scoped `TAM_BUILD_ID` and `TAM_SOURCE_REVISION` to `git log -n 1 -- $(TAM_SOURCES)`, falling back to `git describe`/`rev-parse HEAD` when no prior command commits exist. This fulfills the Makefile's documented invariant ("a corpus-only edit must not change the command's embedded build id") and prevents `build/config` from invalidating all 23 C object files on corpus edits.
- Added an `install-corpus` target in `Makefile` that runs `check-clean-tree` and syncs `knowledge/.` to `$(DATADIR)` without re-running phony index generation.
- In `tools/dev.py`, updated `step_install(root, changes=changes)` to check `is_corpus_only_change(changes)`. For corpus-only changes, it skips `make command` and installs via `make install-corpus` (falling back to `install` if unavailable), completing installation in under 0.05 seconds with zero silent lag. For command changes, it streams compiler output via `stream=True`.
- Added `test_step_install_lane_targeting` to `tests/test_dev.py` to verify that corpus-only changes skip `make command` and invoke `install-corpus`.

## Verification

- `python3 -m unittest discover -s tests -p 'test_dev.py' -q`: all 22 tests pass.
- `python3 -m unittest discover -s tests -p 'test_install_guard.py' -q`: all 12 tests pass.
- `python3 -m unittest discover -s tests -p 'test_check_repo.py' -q`: all 11 tests pass.
- `python3 tools/check-repo.py`: passes with 0 findings.
- `make -C /home/fred/Code/tamlinux/tam test-command`: all command integration, upgrade, renderer golden, and consistency checks pass.
- `make -C /home/fred/Code/tamlinux/tam corpus-and-test SKIP_COVERAGE_CHECK=1 SKIP_UNIT_TESTS=1`: live corpus checks pass in 2.01s.
