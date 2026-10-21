# Boot, Snapper and Rescue learning (corpus 0.42.0)

- **Date:** 2026-10-21
- **Time:** 14:20 to 14:34, -04:00
- **Stage:** Develop; TCK25 Maintaining second slice
- **CLI Tool:** Codex CLI 0.163.0
- **Model:** GPT-6.1 Sol (`gpt-6.1-sol`)
- **Transcript:** Retained privately by the author.
- **Commits:** `dda1535` and the index-history follow-up updating this record.
- **Authorship:** The developer requested the next boot/Snapper/Rescue slice; Codex planned, wrote and checked the content.
- **Checked against:** Baseline `124ee80`; `ROADMAP.md`, `docs/plans/learning-levels.md`, `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/templates/ladder.html`, `docs/templates/path.html`, `src/pager.c`, `src/version.h`, `knowledge/version`, `tools/build-index.py`, `knowledge/learn/skills/skills.json`; product `commands/tam-update/tam-update`, `docs/plans/0.4.4-boot-recovery.md`; assembly `docs/plans/0.4.4-boot-providers.md`; local Snapper 0.13.2 help/manual, findmnt output and the primary references below.

## Guiding prompt

> Keep working on the learning project in the tam repo. Next: boot, boot-fallback and Snapper lessons, then the Rescue path.

## Decisions and limits

Follow the approved ladder/path design. The existing fallback page keeps
`maintain/boot-recovery.html` and its topic. Adding Rescue takes a corpus minor
bump to 0.42.0; command remains 0.38.0. Boot and fallback map boot skills;
Snapper maps update/recovery skills. No new renderer capability is needed.

Each ladder has novice help, worked practice, faded/completion and independent
tasks, troubleshooting/choice tables, folded quizzes and expert failure
analysis. Read-only host captures and explicit fictional scratch inventories
avoid requiring a failed installation for practice. A missing fact remains
unknown; models and menu labels do not establish recovery readiness.

Correct previous Snapper examples: creation defaults to type single and
read-only; pre/post pairs need explicit types and actual pre-number linkage;
comparisons need verified endpoints; global `-r` selects target root rather
than snapshot writability. The native updater's “pre:” description does not
make its checkpoint a pre/post pair. Direct mode bypasses D-Bus, not permissions,
and direct modifications can bypass daemon coordination. Generic default-
subvolume rollback is not a universal explicit-root boot restore recipe.

UKIs group boot components but do not make the entire software/root update
atomic. The lessons distinguish boot-image, root, product-generation and
mutable-data recovery boundaries. Rescue links four stages from Intermediate
to Expert and ends in a scratch recovery-decision capstone, with later-write
preservation and stop conditions. This prepares a machine-specific recovery
procedure; no actual repair, reboot, mount, firmware change, image generation,
snapshot creation/deletion or deployment was performed.

Warnings fall from 34 to 28; page gaps remain 15 and path gaps fall from 21
to 19. These are metadata counts, not learner effectiveness. Logs, power,
resources, packaging and the Maintaining hub still need migration before
Maintaining enforcement. Pilot/learner review, physical boot/rescue and
packaged acceptance remain unverified.

## Primary references

- [UEFI 2.10 boot manager](https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html): boot entry pointers, firmware recovery and default EFI names.
- [UKI specification](https://uapi-group.org/specifications/specs/unified_kernel_image/): component and file semantics; does not claim whole-system transactional updates.
- [mkinitcpio manual](https://man.archlinux.org/man/mkinitcpio.8.en): configuration/presets and image generation; the effective installed provider remains authoritative.
- [Limine v12 configuration](https://github.com/Limine-Bootloader/Limine/blob/v12.x/CONFIG.md): configuration search and entry paths.
- [SystemRescue boot manual](https://www.system-rescue.org/manual/Booting_SystemRescue/): external environment startup requirements.
- Native `snapper(8)` and `snapper --help`: installed flags, default snapshot properties, comparison and classic rollback semantics. The online Arch manual was also consulted; actual option review used the installed 0.13.2 manual.

## Verification

- `make corpus-and-test LIBTAM_PREFIX=<development prefix>`: 32 lint tests,
  20 index-builder tests, and all 108 guide pages render/link/relocation checks pass.
- Eight displayed command sessions rerun directly from authored HTML with exact
  stdout matches. Scratch setup, expanded-pair practice and cleanup pass.
- All 28 page/level combinations for changed/new lessons and hubs render without
  control sequences or stderr. Folded answers remain available in pipe output.
- Six real lesson/level PTY views exercise quiz reveal/collapse and addresses.
  A Rescue PTY journey follows actual Intermediate and Expert links, verifies
  the sticky lens and reset of quiz state on Back. Terminal state is temporary.
- `make check-repo`, all 216 relative Markdown links in changed documents,
  public-boundary scan and `git diff --check`: passed. Git-derived corpus
  revisions are rebuilt after the page commit and recorded separately.
