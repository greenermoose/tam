# Snapper corpus page work plan

- **Date:** 2026-10-18
- **Time:** 07:07 to 07:12 -04:00
- **Stage:** Develop. Documentation note only; no command or corpus version change.
- **CLI Tool:** Antigravity CLI (`agy`) 1.4.2
- **Model:** Gemini 4 Argon (`gemini-4-argon`)
- **Transcript:** Retained privately by the author.
- **Commits:** the commit that adds `docs/plans/corpus/snapper.md` and this record
- **Authorship:** Fred Horch, the Tamlinux developer, asked for a work plan to add a knowledge corpus page for `snapper` and to decide which section it belongs in without creating the page yet. Antigravity inspected the existing corpus structure and Tamlinux's `snapper` integration, chose the Maintain (`maintain/`) section, and wrote `docs/plans/corpus/snapper.md`.
- **Checked against:** `docs/corpus-authoring.md`, `docs/plans/corpus/TEMPLATE.md`, `docs/plans/corpus/linux-fallback-rescue.md`, `knowledge/index.html`, `knowledge/maintain/index.html`, `knowledge/maintain/rollback.html`, `knowledge/maintain/boot.html`, `knowledge/maintain/boot-recovery.html`, `tam explain snapper --pipe`, `snapper --help`, `pacman -Q snapper`, `tamlinux` repository `commands/tam-update/tam-update` and `docs/plans/0.4.4-boot-recovery.md`, `tamlinux-packages` repository `contracts/arch-hyprland.json` and `contracts/providers/boot-selection.json`.

## Guiding prompts

> tam explain snapper gives no results. Create a work plan to add a knowledge corpus page for snapper. Decide which section it belongs in. Do not create the page, just create the work plan for creating the page.

## Key decisions

- Placed the planned page at `knowledge/maintain/snapper.html` in the **Maintain** (`maintain/`) section because `snapper` on Tamlinux is an operational maintenance and recovery checkpoint tool (`tam-update` pre-update root snapshots, numbered Btrfs snapshot management under `/.snapshots`, and `limine-snapper-sync` boot menu synchronization), complementing `maintain/rollback.html`, `maintain/boot.html`, and `maintain/boot-recovery.html`.

## Verification

- Verified `docs/plans/corpus/snapper.md` follows `docs/plans/corpus/TEMPLATE.md` and `docs/corpus-authoring.md` public-boundary rules (no session IDs, home paths, or private repository names).
