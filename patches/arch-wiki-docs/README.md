# Arch Wiki Link-Mend Patches

This directory holds curated unified-diff `.patch` files applied to the
upstream `arch-wiki-docs` English HTML (`usr/share/doc/arch-wiki/html/en/`)
before `tools/build-archwiki-tam.py` builds `/usr/share/doc/arch-wiki/tam/`.

## Workflow

1. Run `tools/build-archwiki-tam.py` against `arch-wiki-docs` and
   `arch-wiki-lite`. Any internal links that cannot be resolved are rewritten
   to `tam:archwiki:broken?target=...&from=...` and logged in
   `broken-links.json`.
2. An AI agent (or maintainer) inspects `broken-links.json` and
   `repaired-links.json` to determine the canonical target page for broken
   links.
3. Add a numbered unified diff here (e.g. `0001-mend-foo-links.patch`) relative
   to `en/` (i.e. `--- a/Page.html` / `+++ b/Page.html`).
4. `build-archwiki-tam.py` applies all `*.patch` files in sorted order on
   every build.
