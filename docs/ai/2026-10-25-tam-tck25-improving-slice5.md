# TCK25 Improving track migration slice 5 (corpus 0.43.7)

- **Date:** 2026-10-25
- **Time:** 19:42 to 20:30 -04:00
- **Stage:** Develop; two corpus pages migrated, command unchanged at 0.43.2.
- **CLI Tool:** Antigravity CLI 1.4.7
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** Implementation commit and the generated-index follow-up containing this line.
- **Authorship:** The developer directed continuing TCK25 Improving track migration with the next slice: `ecosystem/package-versions.html` and `ecosystem/virtualization.html`, claiming scopes first under `AGENTS.md`. Antigravity CLI claimed the scopes, migrated `ecosystem/package-versions.html` and `ecosystem/virtualization.html` to the full ladder anatomy with captured read-only examples, mapped skills (`software.beginner`, `software.intermediate`, `software.expert`, `contributing.beginner`, `contributing.intermediate`, `contributing.expert`), reduced corpus-wide anatomy warnings from 8 to 6, and updated plan and roadmap records.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, `tools/build-index.py`, `tools/lint-corpus.py`, `knowledge/learn/skills/skills.json`; live captured `tam --version`, `tam-deploy status`, `which -a tam`, `systemd-detect-virt`, `systemd-detect-virt --list`, `ls -l /dev/kvm`, `lscpu | grep -i Virtualization`, and `qemu-system-x86_64 -accel help` outputs; [systemd-detect-virt(1)](https://man7.org/linux/man-pages/man1/systemd-detect-virt.1.html), [qemu(1)](https://www.qemu.org/docs/master/), [nix3-flake(1)](https://nixos.org/manual/nix/stable/command-ref/new-cli/nix3-flake.html).

## Guiding prompts

> In the tam repo, we are completing checklist item TCK25. Improving track slices 1–4 landed in corpus 0.43.3, 0.43.4, 0.43.5, and 0.43.6. Continue TCK25 Improving track migration with the next slice: ecosystem/package-versions.html and ecosystem/virtualization.html. Follow tam/AGENTS.md and claim scopes before editing.

## Key decisions

- Use a corpus patch bump (0.43.7) because this slice restructures and migrates existing lessons without introducing new guide pages or command changes.
- `ecosystem/package-versions.html` was migrated to the full ladder anatomy with captured read-only `tam --version` and `tam-deploy status` inspection, cultivar vs component version distinctions, `tamlinux-packages` pinning architecture via Nix flakes and `flake.lock`, Home Manager store path linkages in `~/.local/bin`, development override prioritization in `PATH` (`~/.local/dev/bin`), four-phase lifecycle discipline, and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to `software.beginner`, `software.intermediate`, and `software.expert`.
- `ecosystem/virtualization.html` was migrated to the full ladder anatomy with captured read-only bare-metal detection (`systemd-detect-virt`), hardware virtualization inspection (`/dev/kvm`, `lscpu`), hypervisor accelerator queries (`qemu-system-x86_64 -accel help`), hypervisor vs container isolation trade-offs, headless virtual machine verification harnesses with serial console redirection (`console=ttyS0`), Virtio paravirtualization mechanisms, and folded self-quizzes across Beginner, Intermediate, and Expert levels. Mapped to `contributing.beginner`, `contributing.intermediate`, and `contributing.expert`.
- Whole-corpus anatomy warnings dropped from 8 to 6 (all 6 remaining warnings are in unmigrated lessons). Unclassified pages dropped from 39 to 37; unmapped pages dropped from 79 to 77.

## Verification

- `make corpus`: cleanly generated index and sidecars with 110 entries.
- `python3 tools/lint-corpus.py`: zero findings or errors.
- `python3 -m unittest tests/test_lint_corpus.py tests/test_build_index.py`: 52 tests passed (OK).
- `sh tests/guide.sh`: all 12 guide test assertions passed cleanly.
- `make check-repo`: passes cleanly with updated version header in `ROADMAP.md`.
- `coverage.json`: confirmed warnings dropped from 8 to 6, unclassified pages fell from 39 to 37, and unmapped pages fell from 79 to 77.
- All captured outputs verified against live host command outputs.

Pilot/learner review and physical Linux-console/Foot acceptance of an exact packaged candidate remain unverified. No package deployment or stage promotion was performed.
