# External links in the reader's browser

**Plan status:** In progress
**Reviewed:** 2026-10-27
**Scope:** Command 0.44.0 pager handoff of HTTP and HTTPS links; corpus versions are independent.
**Authority:** Developer-approved design of 2026-10-23, recorded in the plan at `feature/external-links` commit `abf0262`; the developer raised its unfinished status again on 2026-10-27.
**Next action:** Physical desktop and console acceptance of an exact packaged candidate remains developer-directed. Source integration and automated checks are recorded in the session record.

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

- **TCK26:** Browser handoff, confirmation and OSC 8 source, from `abf0262`.
- **TCK27:** Documentation, integrated checks and shared-checkout integration.

The implementation from `feature/external-links` at `abf0262` is integrated in
command 0.44.0 on `develop`; the branch and its remote ref are preserved.
Documentation and automated verification are recorded in the
[session record](../ai/2026-10-27-tam-external-links-integration.md).
Physical desktop and console acceptance remain open; source tests do not
establish it. Development installation and packaged stages are separate.

## Acceptance

A desktop and text-console reader can open a valid link through the selected
browser, cancel the keyboard confirmation, and return to the same guide page.
Unsupported or malformed links produce a useful refusal. Piped output has no
terminal escapes; supported terminal hyperlink behavior and Linux-console
fallback are verified. Source test results and physical review are recorded
separately.
