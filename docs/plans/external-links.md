# External links in the reader's browser

**Plan status:** In progress
**Reviewed:** 2026-10-27
**Scope:** Pager handoff of HTTP and HTTPS links; command implementation and documentation. Corpus versions are independent.
**Authority:** Developer-approved design of 2026-10-23, recorded in the plan at `feature/external-links` commit `abf0262`; the developer raised its unfinished status again on 2026-10-27.
**Next action:** Review the preserved branch implementation against current `develop`, complete its documentation and checks, then integrate using the shared-checkout rules. Preserve the branch and its remote ref.

## Outcome

The reader can open a web link in their own browser while tam itself remains
offline. In a graphical session the pager hands off to the configured graphical
browser; on a text console it temporarily gives the terminal to a text browser
and redraws after it exits. The first keyboard activation names the browser;
the second opens the link, and another key cancels. Supported terminals also
receive OSC 8 links for their own URL controls; those terminal controls do not
use the pager's keyboard confirmation. Plain output and the Linux console have
no OSC 8 escapes. Only bounded, valid HTTP and HTTPS URLs are handed off, as
arguments without a shell. Browsers are optional runtime tools.

## State and remaining work

- `feature/external-links` and `origin/feature/external-links` both point to
  `abf0262`. That commit contains the implementation, focused PTY tests and
  the original detailed design and handoff. It is not in `develop`.
- The branch records passing command and focused tests on 2026-10-23, but its
  integrated check was incomplete because documentation was missing. This is
  historical evidence, not a current test result or physical acceptance.
- Reconcile the branch with current `develop` and its independent corpus
  version. Review URL handling, browser selection, terminal restoration and
  OSC 8 behavior, including the terminal-controlled confirmation distinction.
- Update the reader help, manual, roadmap, development contract and plan index;
  run focused and integrated tests on a stable checkout, and record results.
  Physical desktop and console checks remain explicit acceptance work.
- Do not switch branches, merge or cherry-pick in the shared checkout. Claim
  affected paths and use a coordination-safe integration method consistent
  with repository rules. No branch or folder deletion is needed. Development
  installation requires a separate developer instruction; packaged Test, Run
  and Release remain separate developer-directed stages.

## Acceptance

A desktop and text-console reader can open a valid link through the selected
browser, cancel the keyboard confirmation, and return to the same guide page.
Unsupported or malformed links produce a useful refusal. Piped output has no
terminal escapes; supported terminal hyperlink behavior and Linux-console
fallback are verified. Source test results and physical review are recorded
separately.
