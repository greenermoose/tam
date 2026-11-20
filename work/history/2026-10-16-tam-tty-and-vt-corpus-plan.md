# TTY and Virtual Terminal (VT) Knowledge Corpus Plan and Note

- **Date:** 2026-10-16
- **Stage:** Develop — Planning and Documentation Backlog (`docs/plans/corpus-ttys-and-vts.md`).
- **CLI Tool:** Antigravity CLI (`agy`) 1.4.0
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** (Pending)
- **Authorship:** Fred noted running into issues with TTYs and VTs on the system, finding that `tam explain VT` yielded no topic and requesting to add a note to the `tam` repository that a page about TTY and a page about VT (or a combined page if recommended) is needed for the knowledge corpus without writing the page itself yet. Antigravity claimed the planning and provenance paths under `tam-work`, evaluated single vs. two-page options, authored the planning document `docs/plans/corpus-ttys-and-vts.md` recommending a unified page, recorded the upcoming corpus addition in `docs/plans/development.md`, and updated AI provenance records.

## Guiding prompt

> As we're working on the system I'm running into issues with TTYs and VTs. I tried tam explain VT and came up with nothing. Please add a note that we need to write a page about TTY and a page about VT (or a combined page if you think that's a better idea) for our knowedge corpus. Don't actually write the page, just make a note and file it in the tam repo. We'll do it later.

## Decisions and scope

- **Evaluation of Combined Page vs. Separate Pages:**
  - Recommended a single combined page (`knowledge/ecosystem/ttys-and-virtual-consoles.html`) covering the TTY subsystem, Virtual Terminals (VTs), and Pseudo-Terminals (PTYs).
  - Rationale: A Linux VT is an instance of a kernel-managed TTY device (`/dev/ttyN`), and user confusion frequently stems from conflating the abstract TTY device model, hardware-backed kernel virtual consoles, and GUI pseudo-terminals (`/dev/pts/*`). Placing them in one cohesive reference enables side-by-side contrast and unified search/explainer discovery for both `tam explain tty` and `tam explain vt`.
- **Repository Plan Document (`docs/plans/corpus-ttys-and-vts.md`):**
  - Outlined motivation, technical rationale, metadata keywords, 3-level depth ladder (Beginner, Intermediate, Expert), practical CLI inspection tools, DRM/KMS master lease mechanics, and common failure modes.
- **Development Plan Reference (`docs/plans/development.md`):**
  - Added an entry under Excluded and deferred work linking to `corpus-ttys-and-vts.md`.
- **Corpus and Binary Preserved:**
  - Strictly planning work; no HTML corpus page or command source code was modified.
