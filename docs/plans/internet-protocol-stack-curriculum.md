# Internet protocol stack curriculum

**Plan status:** Completed
**Reviewed:** 2026-11-21
**Scope:** Corpus-only content for corpus 0.106.0: IPv4 and IPv6 glossary pages,
a six-page internet protocol stack series with its hub, one goal path, four RFC
bibliography entries, and links from every prose mention of IPv4 or IPv6 in the
guide. No command, install or stage change.
**Authority:** The developer asked for a guide that takes a reader from no
prior knowledge to expert depth, for every prose IPv4 or IPv6 mention to link
to the correct glossary page, and then for the work to be done. The agreed
scope was six teaching pages plus a hub and a path, prose in guide HTML pages
only, and full repository bookkeeping.
**Next action:** None. Closed 2026-11-21 at corpus 0.106.0; learner review,
installation and packaged acceptance remain developer-directed.

## Problem and agreed outcome

The networking subject could configure a connection and find local services,
but it had no connected explanation of the internet protocol stack itself. A
reader met packets, ports, TCP, UDP and web requests in separate places, with
no path that built those ideas in dependency order. The two address families
had no glossary pages of their own, and prose mentions of them in existing
pages did not link to a definition.

A reader who starts at zero should now be able to follow one path from the
reason for layers through addressing, forwarding, sockets, TCP, UDP and a
complete web request, ending with the precise protocol detail that explains
the observed behaviour. Every teaching page has novice, learner and expert
text; the path gives each stage a lens, a checkpoint and one reversible
capstone. IPv4 and IPv6 have three-level glossary pages, and a reader can
follow every prose mention of either family straight to the right page.

## Baseline and dependencies

Baseline on `dev`: command 0.67.8, corpus 0.105.0, three-level pages and goal
paths with `meta paths` on step pages, skills registry `networking.*` cells,
and the authoring rules in `docs/corpus-authoring.md`,
`docs/corpus-pedagogy.md` and `docs/corpus-style.md`. No command change is
needed. Examples were captured from real commands on a Tamlinux workstation on
2026-11-21; no output was invented. RFC 791, RFC 8200, RFC 9293 and RFC 768 are
the primary protocol sources.

## Implementation

1. **Glossary and sources:** Add `glossary/i/ipv4.html` and
   `glossary/i/ipv6.html`, add them to the letter index, and add four RFC
   bibliography entries with stable citation IDs.
2. **Series:** Add `networking/stack/index.html` and six teaching pages: the
   stack, IP packets and forwarding, ports and sockets, TCP, UDP, and one
   request end to end. Declare path steps 1–6 on those pages and start each
   from a page that exists.
3. **Path and navigation:** Add `paths/internet-protocol-stack.html` with six
   stages, checkpoints, a capstone with a folded solution and next paths. Link
   the series from the networking and setup hubs and the path from the
   `network-setup` path.
4. **Linking pass:** Link every prose IPv4 or IPv6 mention in guide HTML pages
   to the matching glossary page. Leave generated indexes, captured output,
   configuration keys such as `ipv4.method`, metadata, and self-links alone.
5. **Bookkeeping:** Bump `knowledge/version` to 0.106.0, close the plan, update
   the plan index, roadmap and development contract, and write the work
   record. Regenerate indexes and run the corpus and repository checks.

## Acceptance and limits

- `make corpus-and-test` passes; the only learning warning is the pre-existing
  `tamlinux/components.html` section. No new lint finding is introduced.
- Every internal link, citation ID, glossary target and path step resolves.
  The path has six numbered stages with checkpoints, one capstone solution
  and next paths; step numbers are contiguous.
- Each teaching page declares novice, learner and expert sections. Novice
  text carries no quiz or worked example; learner text carries the practice
  and folded quiz. Examples use captured output with the host and date named.
- `tam explain` finds the intended page for IPv4, IPv6, TCP, UDP, internet
  protocol stack, ports and sockets, packet forwarding, path MTU, flow
  control, congestion control and web request.
- Every Tamlinux claim traces to a product file, real command output or a
  public plan; the session record lists what was checked. No learner
  validation, installation or release follows from these checks.

## Verification evidence

- `make corpus`: 1281 pages; indexes current; one pre-existing learning
  warning on `tamlinux/components.html`.
- `make corpus-and-test`: 1281 pages parsed and rendered, corpus checks
  passed, 93 corpus tooling unit tests OK, repository consistency passed.
- Post-edit scan of all guide HTML found no unlinked prose IPv4 or IPv6
  mention outside protected regions; 122 mentions were linked. Generated
  glossary indexes, command output, configuration keys and self-links were
  left as they were.
- All 14 new pages rendered with `tam show --pipe`; no page failed.
- `tam explain` found the intended page for every query in the acceptance
  list above.
- No stage was pushed, tagged, installed or released in this change.

## Checked against

`docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `docs/corpus-style.md`,
`docs/templates/ladder.html`, `docs/templates/path.html`,
`tools/build-index.py` (`validate_path_pages`), `knowledge/version`,
`ip -br addr`, `ip route get 1.1.1.1`, `ip -4 route show default`,
`ip -j route get 1.1.1.1`, `ping -c 2 -W 2 1.1.1.1`, `ss -tan`, `ss -uln`,
`ss -ti`, `getent ahosts example.com` and `sysctl net.ipv4.tcp_congestion_control`.
