# Networking chapter (corpus 0.52.0)

- **Date:** 2026-10-31
- **Time:** 22:11 to 22:30 -04:00
- **Stage:** Develop, corpus 0.52.0
- **CLI Tool:** OpenCode 1.19.4 (from `opencode --version`)
- **Model:** big-pickle (`big-pickle`)
- **Transcript:** Retained privately by the author.
- **Commits:** `055fd93` adds the page, the Using hub link, the corpus version bump and the roadmap update; this commit adds the record, the provenance row and the refreshed indexes.
- **Authorship:** Fred asked for a networking chapter. An earlier session gathered the product sources and captured read-only command output but wrote no files; this session wrote the page, hub link, version and roadmap records, this record and the index refresh.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/templates/ladder.html`, `knowledge/use/index.html`, `knowledge/hardware/resources.html`; the public `tamlinux` repository's `commands/tam-network-status`, `commands/tam-dns`, `commands/tam-network-band`, `commands/tam-network-qr`, `commands/tam-network-password`, `commands/tam-network-speedtest`, `commands/tam-restart-wifi`, `desktop/shell/panels/network/Panel.qml`, `desktop/session/hypr/tamlinux/bindings/utilities.lua` and `desktop/menu/default.jsonc`; live read-only output from `tam-network-status --verbose`, `nmcli device status`, `ip -4 -br addr`, `ip route`, `getent ahosts example.com` and `ping -c 1 -W 2 1.1.1.1` on 2026-10-31; `nmcli(1)`, `ip-route(8)`, `resolvectl(1)` and `iw(8)`.

## Guiding prompts

> Add a networking chapter to the tam knowledge corpus.

> A prior opencode session on this system started working on adding networking to the tam knowledge corpus. Can you find the work that prior session was doing and continue?

## Key decisions

- The page lives at `hardware/networking.html` with track `use` and parent `use/index.html`. The corpus already keeps hardware topics in that directory, and the Using track owns everyday connection tasks. It declares all three levels and maps the three networking skills.
- The page teaches a bottom-up diagnosis: link, address and route, name resolution, then reachability. That order matches the three networking can-do statements and keeps a numeric ping separate from a name lookup.
- Commands and examples use installed tools. The output was captured on the wired workstation and the page says it will differ. No Wi-Fi association output was invented.
- No goal path was added. The networking skill cells now have a page, but a goal path is a separate teaching artifact and no note requested one.
- No glossary pages were added. The beginner "New words" section defines terms in place and links the existing terminal and kernel glossary entries.
- Tab characters in a `<pre>` render as question marks, so the verbose output is aligned with spaces and the page says so. The short form still describes its tab-separated fields in prose.

## Verification

- `make corpus-and-test`: the corpus linter, 32 corpus-linter tests, 21 index tests and all guide checks pass.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe hardware/networking` renders the breadcrumb, levels, examples, tables, quizzes and links without tabs or control sequences.
- `tam explain networking` and `tam search networking` find the page; the other page keywords resolve to the topic guide.
- `python3 tools/report-coverage.py --warnings`: each networking skill cell now reports one page; the path gap remains.
