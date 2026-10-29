# Voice dictation and shell voice commands knowledge pages (corpus 0.48.0)

- **Date:** 2026-10-29
- **Time:** 19:29 to 19:42 -04:00
- **Stage:** Develop; corpus 0.48.0
- **CLI Tool:** Antigravity CLI 1.4.10
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `591b255`, and the commit that adds this record and refreshed index revisions
- **Authorship:** Fred Horch directed explaining how voice technology works under Tamlinux covering both F9 and F10, detailing commands invoked, where recordings go, processing via Whisper, and window injection mechanisms. Antigravity authored the technical documentation, drafted and presented the review artifact in Micro, and integrated two new knowledge pages with navigation updates.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `ROADMAP.md`, `knowledge/desktop/index.html`, `desktop/session/hypr/tamlinux/bindings/voxtype.lua`, `config-fred-tamlinux/config/bash/voice-command.bash`, `config-fred-tamlinux/bin/fred-voice-command`

## Guiding prompts

> I want you to explain how the technology works and provide unitied explanation covering both F9 and F10 on Tamlinux. Tam readers should know what commands those keybindings invoke and how the commands work. I especially want readers to know where the recordings go and how they get processed and turned into text, and then how that text gets injected into the focused window.
>
> Do this as a standalone topic page and a next section. You'll probably need to write several pages to do the subject justice.
>
> Yes, once I'm satisfied with the draft in the artifact, then claim the corpus lane in tam and integrate it into the repo.

## Key decisions

- Authored `knowledge/desktop/voice.html` (Beginner how-to): introduces desktop push-to-talk (`F9`) and toggle (`SUPER+CTRL+X`) dictation with `voxtype` and `wtype`.
- Authored `knowledge/desktop/voice-commands.html` (Intermediate how-to): explains safe terminal shell voice commands via `F10` (Readline `\e[21~`), temporary file isolation, allowlist validation against approved tools, and prompt injection without automatic execution.
- Updated `knowledge/desktop/index.html` and `knowledge/use/index.html` to link and index the new voice documentation.
- Bumped corpus version to 0.48.0 in `knowledge/version` and `ROADMAP.md`.

## Verification

- `make corpus` built index and companion files without errors (132 entries).
- `make corpus-and-test` passed: lint-corpus (0 findings), unittests, and repository integrity checks.
- Rendered output checked via `build/tam show --pipe` for `desktop/voice` and `desktop/voice-commands`.
