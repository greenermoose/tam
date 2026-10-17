# Linux Boot Fallbacks and Rescue Routes (corpus 0.29.0)

- **Date:** 2026-10-17
- **Time:** 07:45 to 07:49 -04:00
- **Stage:** Develop — new three-depth corpus guide and corpus version bump
- **CLI Tool:** Codex CLI (`codex`) `0.163.0`
- **Model:** GPT-6 Luna (`gpt-6-luna`)
- **Transcript:** Retained privately by the author.
- **Commits:** `b9c17ed` (guide and corpus bump), `9de0735` (generated index)
- **Authorship:** Fred asked for a distinct guide to Linux fallback and rescue routes, a corpus version bump, and a clean development build ID. Codex authored the guide, links, corpus metadata and supporting documentation, then rebuilt and checked the corpus.
- **Checked against:** Existing `knowledge/maintain/boot.html`, `recovery.html` and `rollback.html`; UEFI specification (removable-media boot behavior); Limine v12 configuration documentation; `mkinitcpio(8)`; SystemRescue booting manual; `tools/build-index.py`; `tools/lint-corpus.py`; `tests/guide.sh`.

## Guiding prompts

> We are working to get Tamlinux 0.4.4 ready to Run. Please explain in detail what is the Linux fallback / rescue route.

> Agy is working on the tam knowledge corpus right now, but make a plan to add a page or series of pages to the corpus that explains the concepts of Linux fallback and rescue route. I think this is different than the emergency topic we have in the corpus. When agy has released its work claims on the tam repo, you can have a go at it. You will bump the version of the corpus as part of your work, and make sure to build in a way that doesn't leave a dirty note on the dev of the corpus version. Ask if you have questions.

> You're on. Implement your plan.

## Key decisions

- Added one Maintain guide with beginner, intermediate and expert sections. It explains how to choose a route based on the last working boot stage and the components that each route still shares with normal boot.
- Kept emergency procedures in the existing emergency-recovery guide and linked the pages both ways. The new guide covers firmware fallback entries, alternate kernels and initramfs images, snapshot boot versus restoration, virtual consoles, live media, encryption dependencies and the separate role of backups.
- Bumped only the corpus version from 0.28.0 to 0.29.0. Committed the guide first, regenerated its search index from committed Git history, then refreshed the ignored development build ID from the clean corpus tree.

## Verification

- `python3 tools/build-index.py`: generated 49 entries; a subsequent clean-tree run left the tracked index unchanged and set `knowledge/.build_id` to `9de0735` with no dirty suffix.
- `make corpus`: up to date.
- `make test-corpus` using the installed development `libtam`: corpus lint, 29 unit tests and all guide tests passed.
- `TAM_KNOWLEDGE_DIR=knowledge ./build/tam explain 'Linux fallback' --pipe`, `... 'rescue route' --pipe` and `... 'fallback initramfs' --pipe`: each resolves to the new guide.
- `TAM_KNOWLEDGE_DIR=knowledge ./build/tam show --pipe maintain/boot-recovery`: rendered all three depths and revision metadata.
