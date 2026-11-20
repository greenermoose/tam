# Tam Knowledge Corpus Audit (corpus 0.59.1)

- **Date:** 2026-11-02
- **Time:** 17:30 to 17:45 -05:00
- **Stage:** Develop — read-only review. No corpus page edits in this record;
  remediation follows as separate Develop work under the corpus gap plan.
- **CLI Tool:** Cursor Agent
- **Model:** Composer
- **Transcript:** Retained privately by the author.
- **Commits:** audit record only in this change; product authority
  `tamlinux` `06c429ffa0c9bde7da029f89fbb51af429b2c8ef`
- **Authorship:** The developer asked for a careful corpus review and a plan
  to fill gaps; this audit records the factual findings that plan remediates.
- **Checked against:** `knowledge/version` (0.59.1), `src/version.h`
  (command 0.48.1), `tam --version` (cultivar KBN WB 0.6.1),
  `tamlinux` `desktop/session/hypr/tamlinux/bindings/{utilities,voxtype}.lua`,
  `tamlinux-packages/docs/console-startup.md`, `coverage.json`,
  `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, prior audit
  `docs/ai/2026-10-16-tam-corpus-audit.md`

## Guiding prompts

> Do a careful review of the tam knowledge corpus. Look for pages that are
> wrong, incomplete, out-dated, or confusing. Also look for gaps. The aim of
> the knowledge corpus is to enable novices with absolutely no background in
> or knowledge of computing to become experts, able to troubleshoot, diagnose,
> fix, and improve their Tamlinux system. A diligent reader after reading the
> entire knowledge corpus should know how their workstation works from first
> principles, all the way down to the quantum physics level.
>
> Prepare a plan to fill in gaps and fix problems in the knowledge corpus.
>
> Implement the plan as specified.

## Summary

The corpus is pedagogically ambitious and checker-clean, with a strong
physical-layer spine and Maintain/boot coverage. High-severity defects are
**behaviour drift** a novice will act on (wrong Apps menu chord; F9 as
tap-toggle instead of push-to-talk) and **status/currency drift** (SDDM /
Blue Sparkler 0.5.4 centered while the live cultivar is console-first
KBN WB 0.6.1). Coverage reports zero page gaps and twelve path gaps.
Structural gaps block the mastery goal: no quantum foundation page, no
OS/session ladders joining silicon to the desktop, and the pedagogy-named
`use/recipes/` and `maintain/symptoms/` trees are absent.

## Re-check of 2026-10-16 findings

| ID | 2026-10-16 | 2026-11-02 |
| --- | --- | --- |
| F1 | High — Omarchy keybindings on desktop hub | **Recurs** as H1/H2: Meta migration landed, but Apps menu and F9 still wrong vs product bindings |
| F2 | High — install verbs as implemented | **Fixed** — `install/index.html` marks plan/apply/verify planned |
| F3 | High — worktree guidance | Not re-sampled this cycle; treat as open Medium until verified |
| F4 | Medium — session command names in sddm | Open; page still SDDM-centered (H3) |
| F5–F6 | Medium — entities / LaTeX | Not re-sampled this cycle |
| F7 | Medium — Sway as current stack in terminology | Not re-sampled; compositors page still careful about planned Sway |
| F8–F16 | Low | Partially stale examples remain (see M2) |

## Findings

| ID | Severity | Page | One-line |
| --- | --- | --- | --- |
| H1 | High | `desktop/index.html` | Apps menu documented as Meta+Alt+Space; product binds Ctrl+F12 |
| H2 | High | `desktop/voice.html`, `voice-commands.html`, `desktop/index.html` | F9 documented as tap-toggle; product is push-to-talk (press start, release stop) |
| H3 | High | `ecosystem/sddm.html`, `learn/paths/understand-design.html`, `maintain/boot-phases.html` | Login teaching still SDDM / “text-startup plan”; packages deliver console-first + `tam-desktop` |
| M1 | Medium | `ecosystem/toolkits.html` | “Is removing” Omarchy dependencies; Gate A independence complete at 0.4.8 |
| M2 | Medium | `tam.html`, `ecosystem/package-versions.html`, `improve/toolchains.html` | Worked examples still Blue Sparkler 0.5.4 / command 0.43.x |
| M3 | Medium | `learn/paths/first-steps.html` | Follow-link gesture inconsistent; jumps to permissions without pwd/ls/cd |
| M4 | Medium | coverage path gaps | Twelve skill cells have pages but no path membership (networking all levels) |
| L1 | Low | entry hubs | Repeated Arch→Void→microkernel rootstock paragraph on many pages |
| L2 | Low | `docs/plans/corpus/keyboard-modifiers.md` | Status Ready though Meta pages shipped in corpus 0.56.0 |
| G1 | Gap | first-principles | No matter/energy page; cables path asserts quantum literacy |
| G2 | Gap | OS/session | Thin init/compositor/kernel teaching; no bridging goal paths |
| G3 | Gap | pedagogy scaffolds | `use/recipes/`, `maintain/symptoms/`, part FAQs not built |
| G4 | Gap | glossary | Missing electron/photon/bandgap/quantum and Wayland/compositor/systemd/UEFI/Btrfs/Limine/Nix/HM/deploy |

## High severity detail

### H1 — Apps menu chord

- **Claim:** `desktop/index.html` lists `Meta + Alt + Space` for the apps menu.
- **Authority:** `tamlinux` `desktop/session/hypr/tamlinux/bindings/utilities.lua`
  binds `CTRL + F12` → `tam-menu toggle apps`. No `META + ALT + SPACE`.
- **Fix:** Correct the list and key table to `Ctrl + F12`.

### H2 — F9 dictation mode

- **Claim:** Voice pages teach F9 as tap start / tap stop toggle.
- **Authority:** `voxtype.lua` binds F9 start and F9 stop with `{ release = true }`
  (push-to-talk). Toggle remains `META + CTRL + X`.
- **Fix:** Rewrite beginner steps and tables as hold-to-talk / release-to-stop.

### H3 — Login centering

- **Claim:** SDDM page and design path Stage 3 teach graphical greeter-first;
  boot-phases still says “text-first startup plan”.
- **Authority:** `tamlinux-packages/docs/console-startup.md` — native recipe no
  longer depends on SDDM; `tam-desktop` is the explicit desktop entry.
  Live `tam --version` reports cultivar KBN WB 0.6.1.
- **Fix:** Console-first + `tam-desktop` as current; SDDM as legacy/optional;
  retarget understand-design Stage 3; align boot-phases wording.

## Verified correct (sample)

- Install hub marks planned install verbs as unavailable.
- Meta terminology: living pages use Meta; SUPER only on `desktop/meta-key.html`
  and `learn/glossary/meta.html`.
- Several desktop chords (Meta+Return, Meta+Space, Meta+K, Meta+C/V) match
  product bindings.
- `cables-and-protocols` path and circuits/serial/cables/protocols/ECC pages
  form a coherent physical-layer spine.
- Maintain boot/snapper/recovery pages exist and are linked from keep-it-healthy
  and rescue paths.

## Follow-up

- **Corpus fix:** Remediation plan phases 1–6 (behaviour, glossary/skills,
  physics ladder, OS/session, recipes/symptoms, synthesis).
- **Command fix:** None required for H1–H3.
- **Developer ruling:** None; defaults in the approved plan (install stays stub;
  quantum = solid-state semiconductor depth).

## Key decisions

None beyond the prompts and the approved remediation plan.
