# Email systems curriculum

**Plan status:** Completed
**Reviewed:** 2026-11-25
**Scope:** Corpus-only content ending at corpus 0.132.0: a Networking > Email
hub, ten three-level teaching pages, a five-stage `email` goal path, glossary
pages for the requested mail protocols and the practical full set of related
standards, bibliography entries, `mail.*` skill cells and cross-links from
existing pages. Teaching examples use labelled sandboxes; no mail software is
installed or configured. No command, install or stage change.
**Authority:** The developer asked: "Write a series about email. Explain what a
mail server is and how it works. Add glossary pages for POP, SMTP, IMAP, JMAP,
and all other relevant email protocols and standards. Be sure to use the
correct title format for glossary pages, e.g. Acronym (Written Out). Discuss
the difference between a mail client and a mail server. Thoroughly cover all
the ways DNS and other systems interact with email, and what steps are being
taken to combat spam. Add a glossary page for spam, white list, etc." When
asked about the two design choices, he chose Networking > Email as the subject
home and the practical full set of standards as the scope. This authorizes
Develop work only; Test, Run, Release, install and push remain his word.
**Next action:** None. All four slices are complete and verified at corpus
0.132.0; no stage promotion, install or push is authorized.

## Problem

Email is an application-layer service, but the current corpus has no email
hub, path or protocol glossary. SMTP appears once as an application-layer
example; TLS, CR/LF and DNSSEC pages mention email in passing. A reader cannot
start from "what is a mail server" and follow a sequence through message
format, submission, retrieval, DNS, identity, transport security, spam
filtering and deliverability. The existing `glossary/m/mx.html` entry means MX
Linux, so the DNS term needs a distinct page and unambiguous lookup keywords.

## Baseline

Command 0.69.7 and corpus 0.131.1 are on `dev`. The closest pattern is the
[databases and data storage curriculum](database-systems-curriculum.md): a
subject hub, ten ladder pages with `?level=` path steps, a five-stage goal
path with checkpoint quizzes and a capstone, glossary entries filed by first
letter, bibliography pages maintained in all five bibliography views, and
three registered skill cells. The corpus builder enforces path structure,
glossary links, citation IDs, topic slugs and level metadata.

## Design decisions

- **Subject home:** `knowledge/networking/email/`, parent
  `knowledge/networking/index.html`, topic `hub-networking-email`, track
  `networking`. Link from the Networking hub and the application-layer page.
  Do not create a new root track.
- **Practical full set:** teach the requested core protocols (SMTP, POP3,
  IMAP4, JMAP), message format and addressing (RFC 5322, MIME, SMTPUTF8,
  internationalized addresses), DNS and trust (MX, PTR, DNSSEC, SPF, DKIM,
  DMARC, ARC, SRS, BIMI, DANE/TLSA, MTA-STS, TLS-RPT, STARTTLS), and anti-spam
  systems (spam, allowlist, denylist, DNSBL/RBL, greylisting, reputation,
  content and Bayesian filtering, rate limits, quarantine and reporting).
  Cover adjacent ideas in the teaching pages without turning each into a
  glossary page. This is the developer-approved meaning of "all other
  relevant email protocols and standards."
- **Acronym titles:** spell the expansion in parentheses in `<title>` and the
  breadcrumb, and explain the letters in the body, for example `SMTP (Simple
  Mail Transfer Protocol)` and `DMARC (Domain-based Message Authentication,
  Reporting and Conformance)`.
- **MX collision:** add `glossary/m/mail-exchange.html` titled `MX (Mail
  Exchange)` with topic `glossary-mx-mail-exchange`. Keep the existing MX Linux
  page unchanged. Give the new page `mx record` and `mail exchange` keywords
  so `whatis` distinguishes the DNS term without burying either result.
- **Client and server:** the first teaching page compares the mail client
  (MUA) that a person reads and writes with the mail server roles that accept,
  relay and store messages. Later pages return to that boundary whenever a
  protocol changes which side holds state.
- **Examples:** use `<tam-sandbox>` in Learner sections with sample files and
  scripted output. State plainly that the output is a teaching sample and that
  the series installs no mail server, client or DNS record. No privileged
  commands and no live network requests.
- **Glossary shape:** one concise definition per level, links to the relevant
  teaching page and source, and no distro comparisons. Update the existing
  allowlist, TLS, DNSSEC, CR, LF and application-layer pages with contextual
  links rather than duplicating their lessons.

## Slices

### TCK34 (Email systems curriculum)

Rollup for the complete series at corpus 0.132.0. Closes when every child
below has evidence and the corpus is clean.

### TCK34.1 (Email hub and teaching pages)

Create `knowledge/networking/email/index.html` and ten ladder pages:

1. `what-is-email.html` — messages, addresses, client versus server, store
   and forward.
2. `how-a-mail-server-works.html` — MUA, MSA, MTA, MDA, queues, relay and
   mailbox delivery.
3. `addresses-and-message-format.html` — envelope versus RFC 5322 headers,
   body, MIME and internationalized addresses.
4. `smtp-and-submission.html` — SMTP/ESMTP, submission, ports, STARTTLS and
   implicit TLS, relay and DSN.
5. `retrieval-pop-imap-and-jmap.html` — POP3, IMAP4, JMAP, webmail and where
   messages are stored.
6. `dns-and-email.html` — MX lookup, A/AAAA, CNAME rules, PTR, DNSSEC and the
   order a sender uses DNS.
7. `identity-and-transport-security.html` — SPF, DKIM, DMARC, alignment,
   reports, ARC, SRS, BIMI, DANE/TLSA, MTA-STS and TLS-RPT.
8. `spam-and-abuse.html` — spam, phishing, reputation, DNSBL/RBL, greylisting,
   content filters, rate limits, challenge-response, quarantine, reporting
   and the allowlist/denylist trade-off.
9. `mail-clients-and-mailboxes.html` — configuration, folders and flags,
   synchronization, search, aliases, offline work and end-to-end encryption.
10. `deliverability-and-troubleshooting.html` — bounces, backscatter, queues,
    deferrals, blocks, logs and privacy limits.

Each page declares `mail.novice, mail.learner, mail.expert`, the email path
step, a backward prerequisite and at least one sandbox in Learner. Steps
1–10 are declared in `meta paths content="email:N"`.

### TCK34.2 (Email path and supporting corpus)

Create `knowledge/paths/email.html` with five stages, checkpoint quizzes, a
capstone and next paths. Add the glossary pages decided in the practical full
set, bibliography entries for the primary RFCs and official documentation,
`mail.*` skill cells in `skills.json` and the hand-written skills table, and
contextual links from the Networking hub, application-layer page and relevant
glossary pages. Regenerate glossary and corpus indexes.

### TCK34.3 (Email bookkeeping and verification)

Bump corpus to 0.132.0, update the roadmap, development contract, plan index,
checklist and work record. Run `make corpus`, `make corpus-and-test`,
`make check-repo` and the coverage report; read representative pages with
`TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe`. Commit pages first, then
the regenerated index follow-up. Do not push, install or promote.

### TCK34.4 (Email glossary whatis discoverability)

Check with the live corpus that `SMTP`, `POP`, `IMAP`, `JMAP`, `MX record`,
`spam`, `allowlist` and the other requested names resolve to their own
glossary entries, and that MX Linux remains reachable. Record the result with
the corpus version and commit.

## Acceptance criteria

- [x] The Networking hub links the email hub; the email hub links all ten
      pages and the goal path; every page has all three levels, one path step
      and a labelled sandbox.
- [x] The five-stage path has checkpoint quizzes, a folded capstone solution
      and next paths, and passes `validate_path_pages`.
- [x] Every requested acronym glossary title uses `Acronym (Written Out)`;
      the MX record entry is distinct from MX Linux.
- [x] DNS coverage includes MX, A/AAAA, CNAME restrictions, PTR, DNSSEC and
      the records used by SPF, DKIM, DMARC, DANE/TLSA, MTA-STS and TLS-RPT.
- [x] Anti-spam coverage includes allowlist, denylist, spam, DNSBL/RBL,
      greylisting, reputation, filters, rate limits and reporting.
- [x] `mail.*` skills are registered at all three levels and mapped from the
      skills table and teaching pages.
- [x] Bibliography entries are visible in all bibliography views and are
      cited from the pages that use them.
- [x] Corpus 0.132.0 is recorded in `knowledge/version`, the roadmap dev row
      and delivered history, and the development decision record.
- [x] `make corpus`, `make corpus-and-test`, `make check-repo`, the coverage
      report and reader spot checks pass; no stage is promoted.

## Verification

Corpus 0.132.0 on `dev`, command 0.69.7 unchanged. `make corpus` rebuilt 1,624
pages; `make corpus-and-test` passed the corpus checker, component-coverage
check, 96 corpus tooling tests and the repository consistency check; the
coverage report shows `mail.*` with ten pages and one path per level and no
anatomy warnings. Reader spot checks with `TAM_KNOWLEDGE_DIR=knowledge
build/tam show --pipe` rendered the hub, a teaching page, the goal path and the
MX glossary entry. Live `tam whatis --pipe` resolved SMTP, POP, IMAP, JMAP, MX
record, spam, allowlist, whitelist, SPF, DKIM, DMARC and greylisting to their
own glossary entries; MX Linux remained distinct. No stage, tag, release,
install or push was performed.
