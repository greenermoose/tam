# Logs and power learning ladders (corpus 0.42.3)

- **Date:** 2026-10-22
- **Time:** 14:03 to 14:20 -04:00
- **Stage:** Develop; two existing corpus lessons migrated, command unchanged at 0.43.1.
- **CLI Tool:** Codex CLI 0.163.0 (live runtime detector).
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`).
- **Transcript:** Retained privately by the author.
- **Commits:** `daeecdd` and the generated-index follow-up containing this line.
- **Authorship:** The developer requested the next step and approved the proposed logs/power slice. Codex wrote the lesson, index, planning and provenance changes.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/plans/learning-levels.md`, `tools/build-index.py`, `knowledge/learn/skills/skills.json`; captured `journalctl` and `/sys/power/` queries; the public `tamlinux` repository's `desktop/README.md`, `desktop/plugins/fred.workspaces/Workspaces.qml` and `desktop/native/defaults/root/usr/lib/systemd/system-sleep/tam-hibernate-swap`; [journalctl](https://www.freedesktop.org/software/systemd/man/latest/journalctl.html), [journald.conf](https://www.freedesktop.org/software/systemd/man/latest/journald.conf.html), [kernel sleep states](https://docs.kernel.org/admin-guide/pm/sleep-states.html), [hibernation](https://docs.kernel.org/power/swsusp.html) and [sysfs power ABI](https://github.com/torvalds/linux/blob/master/Documentation/ABI/testing/sysfs-power).

## Guiding prompts

> What is the next step for tam command and corpus?

> Proceed.

The proposed next slice was logs and power, following the approved TCK25 migration. Resources, packaging and the Maintaining hub remain before enforcement.

## Key decisions

Use a corpus patch version because the slice corrects and restructures existing pages and adds no new page or command capability. Examples read logs/sysfs and require no sleep cycle or system change. Kernel capability and mode selection are distinct from successful sleep/resume and measured energy; structural journal verification is distinct from authenticity and causal evidence. Updated cached branch rows from the actual local remote-tracking refs; those rows do not claim package installation.

## Verification

- `make corpus-and-test`: passed; corpus lint, 32 linter tests, 20 index-builder tests and every guide contract.
- Read both lessons rendered with the checkout corpus at levels 0–3; eight pipe previews contain quizzes and end-of-section answers without terminal control sequences. Beginner and Intermediate layout and tables reviewed.
- Coverage changes: page gaps 15 to 10; anatomy warning sections 28 to 24; path gaps remain 19. Both migrated pages have no anatomy warnings. These are metadata counts, not evidence of learner mastery. Maintaining enforcement remains off.
- `make check-repo`: passed. All four captured example outputs matched their commands. `git diff --check` and the public-boundary scan passed before committing.

Pilot/learner review and physical Linux-console/Foot acceptance of an exact packaged candidate remain unverified. No development installation or package-stage promotion was performed.
