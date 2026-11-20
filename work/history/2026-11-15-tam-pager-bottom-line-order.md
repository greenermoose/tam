# Pager bottom line keybindings order (command 0.59.5)

- **Date:** 2026-11-15
- **Time:** 09:55 to 09:59 -05:00
- **Stage:** Develop — reorder bottom status row keybindings and lens in `tam`
- **CLI Tool:** agy 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending commit
- **Authorship:** Fred requested the reordered bottom status row; Antigravity updated `src/pager_draw.c`, `src/version.h`, `tests/run.sh`, `man/tam.1`, `ROADMAP.md`, `docs/plans/development.md`, and provenance records.
- **Checked against:** `src/pager_draw.c`, `src/version.h`, `tests/run.sh`, `man/tam.1`, `ROADMAP.md`, `docs/plans/development.md`

## Guiding prompts

> Change the bottom line display in tam command to put the keybindings in a different order:
>
> asdf jkl; Navigate  [] Select links  h Help  q Quit | 0123 showing All text          vX.Y.Z (P.D.Q)

## Key decisions

- Placed `key_text` (`"asdf jkl; Navigate  [] Select links  h Help  q Quit"`, narrow fallback `"h Help  q Quit"`) before `" | "` and `lens_part`, adjusting `lens_digit` by `strlen(footer1)` so the active lens digit remains underlined at its new column.

## Verification

- `make -C /home/fred/Code/tamlinux/tam check-repo`
- `make -C /home/fred/Code/tamlinux/tam command test-command`
