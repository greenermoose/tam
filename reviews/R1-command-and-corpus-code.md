# R1 — Command and corpus code review

- **Date:** 2026-12-03
- **Reviewer:** Codex 0.171.0, `gpt-6.2-sol`
- **Revision:** `4acc95aa83fd90193d7be0fcbce1e50cb682e2f6` (command 0.67.8, corpus 0.106.1)
- **Scope:** C command and pager, document providers, corpus tooling and indexes, source upgrade, and the libtam interface.

## Summary

Fix the source-upgrade pin first. It prevents a clean build of the reviewed
revision through its own updater. The renderer and provider interface
already give command and pager features a shared document model. The
highest-value cleanup is at the edges of that model, where several callers
parse addresses, restore navigation state, or reread the same metadata.
Most foundation functions are already shared through libtam. Key decoding
and UTF-8 handling offer the clearest opportunities for more reuse.
[Data structures and algorithms](../arch/data-structures.md)
· [Dependency assessment](../arch/dependencies.md)

## Confirmed findings

### 1. Source upgrade pins a libtam version the command rejects (high)

- **Where:** [Source pin](../source-build.json#L3) · [Build check](../Makefile#L39) · [Updater build](../bin/tam-upgrade#L669)
- **What:** A source upgrade targeting this revision cannot get through the command build.
- **Evidence:** Extracting the manifest's exact header and passing its scratch prefix to `make libtam-check` exited 2. The same check accepted the 0.8.0 development header, and a clean command build with its archive succeeded. Commit `9ff5a36` raised the minimum to 0.8.0 while the source pin stayed at 0.4.0.
- **Fix:** Pin a tested 0.8.0 revision with current-layout cultivar support, update the build instructions and add a pinned-dependency compatibility check to source-upgrade verification.
- **Status:** [B1](../bugs/B1-source-upgrade-libtam-pin.md)

### 2. File ownership trusts a directory prefix (medium)

- **Where:** [Ownership lookup](../src/tam.c#L117)
- **What:** `tam where` returns success and labels a missing path as owned by Tamlinux.
- **Evidence:** An absent child under `/usr/share/doc/tamlinux/` was reported as cultivar-owned. The function accepts the original path by prefix even when canonicalization failed and before consulting the manifest.
- **Fix:** Use the active manifest's file entries as ownership evidence for deployed paths. Give versioned trees a canonical entry check and report the owning version if known. Test an absent child, an existing unowned child, a manifest member and a native package fallback.
- **Status:** [B2](../bugs/B2-where-claims-missing-files.md)

### 3. Guide search parses the whole address as query data (medium)

- **Where:** [Classification](../src/address.c#L21) · [Search decoding](../src/discovery.c#L83)
- **What:** A literal URL inside a Guide Search address is rejected. A fragment on a search view changes the search term.
- **Evidence:** The literal URL produced a Guide page lookup error while its percent-encoded counterpart reached search. Adding `#details` to the fixture's `pipelines` query changed the displayed term and removed the result. The classifier scans the entire address for `://`, and the search decoder looks only for `&` to terminate the value.
- **Fix:** Parse the scheme, query and fragment once. Decode the parameter only after splitting its boundaries. Cover literal and encoded URLs, anchors, encoded delimiters, plus signs and malformed escape sequences.
- **Status:** [B3](../bugs/B3-guide-search-address-parsing.md)

### 4. Resizing the pager can focus a different link (medium)

- **Where:** [Logical reflow and selection](../src/pager_nav.c#L109)
- **What:** A selected link can lose focus to an earlier link with the same destination.
- **Evidence:** In a PTY, Tab selected the second of two labels linking to one anchor. Resizing from 64 to 56 columns moved the reverse-video selection to the first label. Reflow accepts the first destination match even if the original link ID still exists later in the vector.
- **Fix:** Search for the original ID first and use destination and logical position as a fallback. Cover duplicated targets on one line, wrapped links and breadcrumb focus.
- **Status:** [B4](../bugs/B4-resize-selects-earlier-link.md)

### 5. The catalog ignores the command-line reading level (medium)

- **Where:** [Lens eligibility](../src/address.c#L41) · [CLI forwarding](../src/tam.c#L614) · [Catalog rendering](../src/views.c#L171)
- **What:** `tam show --level expert tam:catalog` omits the level from the title and result links.
- **Evidence:** Its output was byte-for-byte the same as the bare catalog. The explicit `tam:catalog?level=expert` address displays the level and puts it on the links. The CLI consults an eligibility list that omits the catalog route.
- **Fix:** Declare lens eligibility alongside each route's loader. Test that the option and explicit address give equivalent output and that an explicit address level keeps precedence.
- **Status:** [B5](../bugs/B5-catalog-ignores-level-option.md)

## Additional source findings

- **Failed history reloads leave freed strings in the history stack.**
  Both directions pop an entry for a page load. On failure they increment
  the count again, then free the strings still held by that slot. Retrying
  or cleaning up the pager can access them again. Keep the entry owned by
  the stack until the page load and opposite-stack push succeed. Add tests
  for temporarily unavailable pages and a clean exit after a failed load.
  Status is open from source inspection. [Back and forward](../src/pager_nav.c#L938)
- **Pager vector growth can fail before the caller has usable state.**
  Link building and both history push routines assign `realloc` directly
  to the sole array pointer and immediately write through it. History
  allocation also spans several string copies. Use checked capacity
  growth, build complete snapshots before incrementing counts and handle a
  failed append at the call site. Test allocation failure at each owned
  field. Status is open from source inspection. [Link growth](../src/pager_links.c#L84)
  · [History growth](../src/pager_nav.c#L180)
- **The inference HTTP timeout applies to each wait.** The local HTTP
  client resets the wait budget for every response read and appends bytes
  until the stream stops. Use a monotonic request deadline, a response
  size limit and distinct outcomes for incomplete and complete responses.
  Keep connection, framing and inference response handling testable
  independently. Status is open from source inspection.
  [HTTP read loop](../src/ask.c#L308)

## Refactoring order

1. **Repair the build contract and centralize address parsing.** Keep
   `tam_page_loader_t` as the provider seam. Drive route matching and lens
   eligibility from the same descriptors. Use one parser for CLI entry,
   pager navigation and generated views. Characterize current aliases,
   fold and path parameters, relative Guide links and external opener
   confirmation before consolidating code. This also creates a way to
   converge the two initial-page loading paths.
2. **Make navigation transitions atomic in memory.** Combine the two
   snapshot constructors and restore paths after the failure checks are
   in place. Keep original link identity and logical position together.
   Commit the new document, navigation stacks and derived link vector
   only after their allocations succeed. Leave rendering and key mapping
   behind their existing interfaces.
3. **Address the measured index-loading cost.** A local `-O2` harness
   called `tam_index_load` and `tam_index_free` 80 times against the
   warm 1,281-page index with 27 learning paths. It averaged 9.45 ms
   per load with all files and 0.83 ms with the parent and kind files
   omitted. Ranking one three-term query on either loaded index took
   about 5.6 ms over 800 calls. The controlled omission measures the
   projection cost, including I/O and allocation. It does not measure
   end-to-end navigation. Replace repeated path scans with a sorted lookup
   table over the existing array first. Preserve case-insensitive matching
   and first-entry behavior for old indexes. Then measure whether keeping
   one index per pager session is worthwhile and establish how it refreshes
   if the corpus changes. Preserve the ranking algorithm until measurements
   show a reason to change it. [Projection loaders](../src/index.c#L144)
   · [Page-load index use](../src/corpus.c#L588)
4. **Move shared primitives behind narrow adapters.** Pilot the published
   `tam_key` code in the duplicate escape-sequence paths and reuse
   UTF-8 decoding before changing display-width policy. Keep the renderer's
   capability detection, Linux-console behavior and terminal handoff.
   Consider splitting HTML scanning from layout in `render.c` after the
   existing document ownership and reflow contracts are covered by tests.
   Leave the main table and sandbox algorithms intact during that split.
   [libtam assessment](../arch/dependencies.md#libtam-use-and-fit)

## Verification and limits

Built the normal and test binaries from the reviewed checkout against the
published libtam development revision `725dd1f` in temporary storage.
The integration script completed 57 passing groups and the renderer
matched all seven goldens. A further 160 Python tests passed across
upgrade fixtures, command build, corpus tools and repository tooling.
The Guide discovery unit tests inside the shell suite use the existing
checkout binary at the same command revision and report libtam 0.8.0.
The first isolated integration run needed the adjacent upgrade helper
linked into the scratch build. The complete rerun passed.

Additional CLI and PTY checks exercised the reported cases. The upgrade
failure was verified at the dependency gate. Live inference and live
source activation were outside the review. The terminal findings use a
PTY screen model. Corpus validation in this review used fixture tests,
source inspection and the checked-in index for timing.
