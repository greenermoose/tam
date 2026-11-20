# tam command and repository roadmap

- **Date:** 2026-10-13
- **Stage:** Plan & Reference — comprehensive development roadmap established for the `tam` command and offline knowledge base.
- **CLI Tool:** Antigravity CLI (`agy`) 1.3.4
- **Model:** `gemini-3.8-flash-high`
- **Transcript:** Retained privately by the author.
- **Commit:** Pending commit.
- **Authorship:** Fred requested analyzing everything done in the `tam` repository and all plans for the `tam` command, then creating `ROADMAP.md` setting out what needs to be done next followed by future work, with future version numbers corresponding to discrete feature sets. Fred steered the sequencing to prioritize in-pager search at version 0.14.0 before the content-depth ladder at 0.15.0, and confirmed `tam why` as a dedicated top-level subcommand for architectural rationale. Antigravity authored `ROADMAP.md`, updated `AI_PROVENANCE.md`, and recorded this provenance record.

## Guiding prompts

> Take a look at everything that has been done in the tam repo and the plans for the tam command, then create a ROADMAP.md file that starts with what needs to be done next and then continues with future work. The ROADMAP should set out future version numbers. Each future version number should correspond to a set of features. Ask if you have any questions.

> 1 yes, I prefer in-pager search implemented first
> 2 tam why feels right

## Decisions and scope

- **Current Baseline Documented:** Recorded milestones 1–6 and versions 0.0.1 through 0.13.0 in `ROADMAP.md` §2.
- **Immediate Next Steps (0.14.0 & 0.15.0):**
  - Version 0.14.0 implements in-pager text search (`/` with `n`/`N` navigation), reading progress telemetry, and shell auto-completion scripts (Bash, Zsh, Fish).
  - Version 0.15.0 implements Milestone 7 (Content-Depth Ladder and Beginner/Intermediate/Expert fading templates).
- **Future Version Sequence (0.16.0 – 1.0.0):**
  - 0.16.0: "Maintain" track corpus expansion.
  - 0.17.0: Milestone 8 read-only self-inspection findings and check contract.
  - 0.18.0: "Improve" track corpus expansion.
  - 0.19.0: Architectural rationale and excluded packages registry via top-level `tam why <topic>`.
  - 0.20.0: Physics/CS depth ladder and guided traces down to quantum fields.
  - 0.21.0: Milestone 9 opt-in grounded `tam ask` connecting to local inference server with strict citations and consent boundaries.
  - 0.22.0: Offline skills progression (`tam learn`).
  - 0.23.0: Safe non-executing shell syntax dissection.
  - 0.24.0: Installation framework integration (`tam install plan/apply/verify`).
  - 1.0.0: Production release with full tripartite knowledge graph and multi-distro packages.
- **Multi-Distribution Horizon (1.1.0 – 1.3.0):**
  - 1.1.0: Multi-distribution host inspection.
  - 1.2.0: Native Void Linux / runit / Btrfs integration.
  - 1.3.0: Legacy / resource-constrained terminal-only profile.

## Verification

- `ROADMAP.md` created and verified against all settled decisions, requirements (T32), and repository plans.
- Scope managed under `tam-work`.
