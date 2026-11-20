# Boot phases (corpus 0.46.0)

- **Date:** 2026-10-28
- **Time:** 08:03 to 08:05 -04:00
- **Stage:** Develop; corpus source
- **CLI Tool:** Codex CLI 0.165.0 (running session); the executable on PATH reports 0.164.0.
- **Model:** GPT-6.1 Terra (`gpt-6.1-terra`)
- **Transcript:** Retained privately by the author.
- **Commits:** The commit that adds this record and the index follow-up.
- **Authorship:** Fred named the term and requested a page; Codex wrote the page and record.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `knowledge/maintain/boot.html`, Tamlinux `docs/plans/console-first-startup.md`, the UEFI specification boot manager, Linux kernel initramfs documentation, and runit's upstream overview and runlevels documentation.

## Guiding prompts

> Let's call them boot phases. Make a note of that terminology for Tamlinux. Add a page to the tam knowledge corpus about this.

## Key decisions

Keep machine-specific menus out of the general page. Separate encrypted disk unlock from user login and hibernation resume from a fresh boot. Treat an optional graphical session as following boot, not a runlevel.

## Verification

`make corpus-and-test` passed: corpus lint, 52 unit tests and Guide acceptance checks. The new page was inspected with the checkout reader in pipe mode. Packaged and learner acceptance remain pending.
