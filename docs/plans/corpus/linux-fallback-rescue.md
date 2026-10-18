# Corpus note: Linux fallback and rescue routes

**Plan status:** Completed
**Reviewed:** 2026-10-18
**Scope:** Boot fallback/rescue guide in corpus 0.29.0.
**Authority:** [Current development contract](../development.md) and the evidence below.
**Next action:** None for this closed plan; follow the current roadmap.
**Completion evidence:** Implementation `b9c17ed`, index `9de0735`; [verification record](../../ai/2026-10-17-tam-linux-fallback-rescue.md). Keep the page in Maintaining under the [overhaul](../knowledge-system-0.31.0.md).

> Historical design snapshot. This plan is closed; its original work items do
> not assign further implementation. Current behavior belongs to the development
> contract and current scope to the [plan index](../README.md).

## Original design record

- **Date:** 2026-10-17
- **Asked for:** “Add a page or series of pages to the corpus that explains the concepts of Linux fallback and rescue route. This is different from the emergency topic already in the corpus.”
- **Failed queries:** `tam explain 'Linux fallback'` returned distro choice, filesystem hierarchy, beginner guide and virtual-console pages; it does not explain boot fallbacks. `tam explain 'rescue route'` returned only Emergency Disaster Recovery. `tam explain 'boot recovery'` returned the boot, emergency, logs and maintenance index pages without a guide to choosing recovery routes.
- **Proposed page:** `knowledge/maintain/boot-recovery.html`, one Maintain page linked from `maintain/index.html`, `knowledge/index.html`, `maintain/boot.html` and `maintain/recovery.html`. The boot page teaches boot stages and image generation; the emergency page teaches regaining control and live-USB repair. This page teaches how to reason about recovery paths and their dependencies.
- **Scope:**
  - Distinguish fallback boot entries, alternate kernels, snapshot boots, system rollback and external rescue media.
  - Trace each path through firmware, EFI loader, boot image, encrypted storage/root filesystem, userspace and graphical login; identify which components it shares with normal boot.
  - Match common failure boundaries to an appropriate next route, including when another menu entry cannot help because it shares the failed component.
  - Explain that booting a snapshot is different from restoring it, and that rollback can overwrite later writes or omit separately stored data.
  - Cross-link to boot mechanics, snapshot rollback and emergency recovery; keep machine-specific keys, paths and claims out of the general guide.
- **Sources to check:** `knowledge/maintain/boot.html`, `maintain/recovery.html`, `maintain/rollback.html`, product and distribution boot/recovery documentation for any Tamlinux-specific or command claims, Limine documentation for menu/EFI behavior, and official GParted Live or SystemRescue documentation for external rescue capabilities. Add an authoring-guide source row if the final page introduces a recurring source category not covered there.
- **Open questions:** None. One page covers the conceptual decision guide; detailed emergency procedures remain in their existing guide. Corpus 0.29.0 is a minor bump from 0.28.0. Regenerate and verify the index from committed corpus content, then refresh ignored `knowledge/.build_id` from the clean committed tree so the corpus reports its committed development revision without a `-dirty` suffix.
