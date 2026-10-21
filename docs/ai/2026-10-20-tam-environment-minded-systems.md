# Environment-minded systems and desktops (corpus 0.37.0)

- **Date:** 2026-10-20
- **Time:** before 22:13 to 22:17 -04:00 (start time not recorded)
- **Stage:** Develop: new corpus page; nothing installed or promoted
- **CLI Tool:** Claude Code 2.1.305
- **Model:** Claude Opus 5.5 (`claude-opus-5-5`)
- **Transcript:** Retained privately by the author.
- **Commits:** `75f60a8`, `515307b` and the commit that adds this record
- **Authorship:** Fred asked for the page and pointed to an OpenCode survey
  and draft environmental statement as ideas; Claude Code checked the facts,
  wrote the page, its hub links and the roadmap row.
- **Checked against:** a private 2026-10-20 survey of sustainable operating
  systems and a draft environmental statement (ideas only, not published);
  web searches of project sites and coverage for Nura's rename, Emmabuntüs
  Debian Edition 6, wattOS, GALPon MiniNo, KDE Eco (Okular's Blue Angel
  label, KEcoLab, the end of Opt Green), ISO/IEC 21031:2024, Collapse OS,
  End of 10 and the permacomputing principles; `knowledge/ecosystem/os/`,
  `knowledge/ecosystem/sustainability.html`, `knowledge/improve/efficiency.html`,
  `knowledge/maintain/power.html`; `docs/corpus-authoring.md`

## Guiding prompts

> Add a page or section to the tam knowledge corpus about other operating
> systems and desktop environments with similar environmental goals. See the
> report and the environmental statement that opencode did for ideas. Let me
> know if you need help.

## Key decisions

- The page sits in the Operating Systems Reference and gives no project a
  Tamlinux role. Roles come from the `tamlinux` operating-system horizons
  plan, which lists none of these projects.
- Claims from the draft statement that no public source records, such as a
  per-candidate energy reading, are left out. The page says only that
  Tamlinux holds no eco-label and that the guide publishes no energy figures
  yet.
- The survey's personal contact details and named individuals are left out,
  apart from Virgil Dupras as the author of Collapse OS and Dusk OS.
- The survey disagreed with current sources in two places. KDE's Opt Green
  project ended in July 2026, and GALPon MiniNo has had no stable release
  since 2018. The page follows the current sources.
- Projects with "eco" in their name but no environmental programme are
  covered by a general warning, not named.

## Verification

`make corpus-and-test`: lint, index tests and `tests/guide.sh` pass.
`tam show --pipe ecosystem/os/environment-minded` renders cleanly;
`tam search` for "sustainable operating system", "blue angel", "emmabuntus"
and "e-waste" finds the page.
