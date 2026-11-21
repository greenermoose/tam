# Network setup and discovery curriculum

**Plan status:** Completed
**Reviewed:** 2026-11-21
**Scope:** Corpus-only content for corpus 0.105.0: an Avahi and Zeroconf page,
a connected network setup series, a goal path, seven glossary pages and two
RFC references. No command or stage change.
**Authority:** The developer approved the "Foundations + Avahi" scope,
general-first teaching with sourced "On Tamlinux" asides, a path with glossary
and source support, and a short repository plan on 2026-11-21.
**Next action:** None. Closed 2026-11-21 at corpus 0.105.0; learner review
and packaged acceptance remain developer-directed.

## Problem and agreed outcome

The networking subject has six pages: a hub, a diagnostic how-to, a command
reference, a read-only recipe, a physical-layer framing page and an IETF/RFC
page. It has no page on local service discovery and no teaching sequence for
addressing, routing, connection setup or name resolution. `curriculum-gaps.md`
records the missing connected curriculum. Two curated tldr pages mention
`avahi-browse` and `avahi-resolve`; a firewall page names mDNS port 5353 and
the CUPS glossary page mentions printer discovery, but nothing explains how
those pieces fit together.

A reader who starts at zero should be able to follow one path from "what is a
network" through addresses, connections, routes and names to local service
discovery, and then read precise protocol detail. The Avahi page is the
discovery destination. Each page teaches general, standards-based behaviour
first and marks Tamlinux choices in sourced asides; no page presents a general
Linux mechanism as a Tamlinux invention.

## Baseline and dependencies

Baseline on `dev`: command 0.67.8, corpus 0.104.1, subject-library hierarchy,
three-level pages, `paths/<id>.html` goal paths with `meta paths` on step
pages, skills registry `networking.*` cells, and the authoring rules in
`docs/corpus-authoring.md`, `docs/corpus-pedagogy.md` and
`docs/corpus-style.md`. No command change is needed. Avahi 0.9-rc5,
`nss-mdns`, NetworkManager and systemd-resolved are present on the authoring
host; examples are captured from real commands. RFC 6762 and RFC 6763 are the
primary sources for the discovery page.

## Implementation and handoff

1. **TCK28 (Avahi and Zeroconf page):** Write
   `knowledge/networking/services/avahi-and-zeroconf.html` and its folder hub,
   with a three-level explanation, a captured `avahi-browse` example, and
   RFC-level detail on multicast DNS and DNS-Based Service Discovery.
2. **TCK29 (Network setup pages and path):** Write
   `knowledge/networking/setup/` with its hub and five pages: how networks fit
   together, addresses and subnets, adding a connection, gateways and routing,
   and name resolution. Add `knowledge/paths/network-setup.html` with five
   staged checkpoints and a reversible capstone; link every step from the
   networking hub and declare path membership on each step.
3. **TCK30 (References and verification):** Add three-level glossary pages for
   Avahi, Zeroconf, Bonjour, mDNS, DNS-SD, CIDR and ARP; add RFC 6762 and RFC
   6763 bibliography entries and link them from the bibliography hub. Regenerate
   indexes, run `make corpus-and-test` and `make check-repo`, read each page
   with `tam show --pipe`, and check `tam explain` on the topic's query words.
   Bump `knowledge/version` to 0.105.0 and update `ROADMAP.md` and the
   development contract in the same change. Record the session under `work/`.

## Acceptance and limits

- `make corpus-and-test` passes with no new lint warnings; every new glossary
  link, citation and path step resolves; `validate_path_pages` accepts the
  path with five stages and a capstone.
- Each new teaching page declares novice, learner and expert sections, and the
  novice on-ramp has no quiz or worked example.
- `tam explain` finds the intended page for avahi, zeroconf, mdns, dns-sd,
  subnet mask, CIDR, default route, DHCP and DNS resolver.
- Every Tamlinux claim traces to a public product file, the command's real
  help/output or a public plan; the session record lists what was checked.
- No claim of learner validation or packaged acceptance follows from these
  checks. Test, Run and Release remain developer-directed.

## Verification evidence

- `make corpus-and-test`: 1267 pages parsed and rendered, checks passed,
  93 corpus tooling unit tests OK, repository consistency passed.
- `make corpus`: 1267 pages, 1267 parsed; indexes current. The one learning
  warning is the pre-existing `tamlinux/components.html` level section.
- All new internal links, `cite data-ref` IDs and the six `?level=` step links
  resolve; zero missing links.
- `tam show --pipe` rendered each new teaching, glossary, bibliography and
  path page; `tam explain` found the intended page for avahi, zeroconf, mdns,
  dns-sd, subnet mask, CIDR, default route, DHCP and DNS resolver.
- Corpus checks were blocked at baseline because commit `3496cea` moved
  `entities[]` from `src/render.c` to `src/html_entities.inc`. With developer
  approval on 2026-11-21 the linter, its dependency input and the corpus test
  fixtures were updated to read the new file; no lint rule or corpus page was
  weakened.

## Disposition

Completed 2026-11-21 at corpus 0.105.0. TCK28, TCK29 and TCK30 are done with
the evidence above and recorded in `work/W38-network-setup-curriculum`. The
performance, protocols and development tranches are separate plans and were
not part of this tranche. No learner review or packaged acceptance is claimed;
Test, Run and Release remain developer-directed.
