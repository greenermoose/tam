# Peer projects and communities ecosystem pages (corpus 0.51.0)

- **Date:** 2026-10-31
- **Time:** 09:00 to 09:20 -04:00
- **Stage:** Develop, corpus 0.51.0
- **CLI Tool:** OpenCode 1.19.4 (from `opencode --version`)
- **Model:** big-pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `27b6485` adds both pages, hub links, the corpus version bump and the roadmap update; this commit adds the record, the provenance row and the refreshed indexes.
- **Authorship:** Fred asked for pages about the peer projects and developers already gathered for the upstream survey. OpenCode wrote the pages, hub links, version and roadmap records, this record and the index refresh.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/templates/single.html`, `knowledge/ecosystem/index.html`, `knowledge/ecosystem/compositors.html`, `knowledge/ecosystem/toolkits.html`, `knowledge/ecosystem/distros.html`, `knowledge/ecosystem/os/index.html`; public project repositories and sites for Omarchy, Dux Shell, Noctalia, Caelestia, end-4 dots-hyprland, Quickshell, Hyprland, Sway, wlroots, niri, Regolith, nwg-shell, COSMIC, Void Linux and Chimera Linux; public profile and community links on 2026-10-31.

## Guiding prompts

> Add pages to the tam knowledge corpus about these other projects and developers.

> Continue if you have next steps, or stop and ask for clarification if you are unsure how to proceed.

## Key decisions

- Fred chose two ecosystem pages: `peer-projects.html` for what each project is and why Tamlinux watches it, and `peer-communities.html` for public maintainers, channels and approach etiquette.
- Fred chose names, handles, public profile links and project channels, with no personal email addresses.
- Both pages are intermediate-level references in the learn track, parented to `ecosystem/index.html`. They link to the existing compositor, toolkit and rootstock pages instead of repeating that material.
- The roster is dated 31 October 2026 and says so. Versions are left to project sites because an offline guide should not freeze release numbers.
- The pages list public project spaces first and state that a public maintainer is not a support desk.

## Verification

- `make corpus-and-test`: both linters, the 32 corpus-linter tests, the 21 index tests and all guide checks pass.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe ecosystem/peer-projects` and `ecosystem/peer-communities` render with breadcrumbs, terms, tables, quizzes and links.
- `tam explain peer-projects` and `tam explain peer-communities` resolve to the new pages; `tam search 'peer projects'` finds both.
- Every external link in both pages returned HTTP 200 in a scripted check on 2026-10-31.
- The HTML linter reports no findings.
