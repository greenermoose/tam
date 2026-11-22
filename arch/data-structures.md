# Data structures and algorithms

This describes command 0.67.8 and corpus tooling at `4acc95a`. See the
[repository map](../docs/repo-map.md) for file ownership and the
[code review](../reviews/R1-command-and-corpus-code.md) for proposed changes.

## From address to document

`tam.c` parses command arguments and chooses pipe or terminal output. Guide
lookup resolves a page name at the selected knowledge root. The pager accepts
a `tam_page_loader_t` callback, normally `tam_address_load`. Its context
borrows the root and any retained stdin HTML for the duration of the pager.
Address classification distinguishes guide pages, installed manuals and Info
nodes, tldr and Arch Wiki pages, generated views, fragments, and actions.
Pager navigation handles actions and external links. Loader routes return a
document laid out for the requested width. The first guide page and stdin
also have a direct rendering path in the command. [Dispatch](../src/tam.c)
· [Address types and loader](../src/address.h)

Guide pages run through a series of HTML transformations. Corpus loading
adjusts links to optional documentation, looks up revision and learning-path
metadata, adds path navigation, numbers folds and applies the reading level.
The level is a bit mask for novice, learner and expert. A zero selection
means all levels. The lens can come from an address or command-line override,
the environment, or saved state. Fold rendering expands or replaces numbered
regions before the HTML parser runs again. Generated views and documentation
providers adapt their sources to the same document interface.
[Corpus loader](../src/corpus.c) · [Levels](../src/depth.c)
· [Folds](../src/fold.c) · [Generated views](../src/views.c)

## Parsed blocks and visual lines

The renderer scans markup into ordered blocks rather than building a DOM.
Blocks describe headings, paragraphs, lists, preformatted text, tables and
other supported elements. Each has styled text runs or a table containing
rows and cells with column spans. Runs carry text, style bits, an optional
destination and a link ID. Adjacent compatible runs can be coalesced.
Anchors map an HTML ID or name to a logical block. The layout backing owns
those arrays and the sandbox specifications.

Layout wraps text using terminal cell widths, keeps adjacent unspaced runs
together where possible, treats preformatted text separately, and chooses a
column layout or a stacked layout for tables. It emits a vector of visual
lines. Each line owns its spans and records its logical block and offset,
which can be mapped back after wrapping changes. The document also owns the
title, location row, navigation strings, revision label and fold metadata.
Reflow builds replacement visual lines from retained blocks. Plain output
serializes the same document to text with numbered destinations.
[Renderer](../src/render.c) · [Document types](../src/render.h)

## Pager state and derived data

The pager owns the current document, address, viewport top, attention line,
selected link, current lens and learning path. Back and forward stacks own
address and focus snapshots with visual positions and logical block offsets.
Going back or forward restores the saved position, reloading the
destination when the address or learning-path context differs.
Fold overrides are held as a byte array indexed by fold ID.

A link vector is rebuilt by scanning the visual spans and their terminal
columns. It merges consecutive spans belonging to the same link and
includes breadcrumb links. Entries borrow span and destination pointers
from the current document. Page replacement, width reflow or a sandbox
line splice can invalidate those pointers. In-page search flattens each
visual line, scans for non-overlapping case-insensitive matches, and stores
line numbers and byte ranges. This search is separate from corpus ranking
and does not match across visual line breaks.

On resize, the pager maps saved viewport and attention positions onto the
new layout by logical block and offset, rebuilds links and repeats any
active text search. A screen update builds terminal bytes for each viewport
row and compares them with an owned cache of the previous frame. It emits
changes at row granularity, then copies the row bytes into the cache.
Output uses terminal capabilities and an optional synchronized-frame
protocol. [Pager state](../src/pager_int.h) · [Navigation](../src/pager_nav.c)
· [Links](../src/pager_links.c) · [Search](../src/pager_search.c)
· [Screen cache](../src/pager_screen.c)

Each page's sandbox specification contains initial files, scripted rules,
hints and a goal. Pager state borrows the specification and owns a clone
of the virtual filesystem, current directory, transcript, command history
and a fixed-size input buffer with a byte cursor. A virtual directory has
an array of children with parent pointers. Path resolution walks components
and searches each child array. Commands update the simulated tree and append
log entries. The renderer replaces visual lines for the affected sandbox.
[Sandbox data](../src/sandbox.h) · [Simulation](../src/sandbox.c)

## Corpus index and query costs

| Structure | Contents and lookup |
| --- | --- |
| `tam_index_t.entries` | Owned array of page records from `index.tsv` with path, topic, title, summary, levels, keywords, prerequisites and revision. Optional parent and kind TSV files enrich entries by path. |
| `tam_index_t.paths` | Learning paths from `index.paths.tsv`, with IDs, titles, intro pages, skill tags and ordered step vectors. ID lookup scans paths. Page membership scans path steps. |
| `tam_search_result_t` | A score and a pointer borrowed from the loaded index. A result is valid for the index lifetime. |

Guide lookup checks for a file or directory landing page and then consults
exact index matches. Exact matching compares fields at different strengths
and can report a tie. Ranked search normalizes the query, splits it into up to
16 terms, and scans the entries for exact matches or scored field matches.
Coverage and field quality affect the score. A single-term prefix scan is
used when scoring finds no candidates. Matches are sorted by descending
score and then by title and path before truncation. With `M` matches the
sort costs `O(M log M)`, plus the scan and field comparisons.

Fuzzy suggestions scan topics and aliases using case-insensitive,
byte-oriented Levenshtein distance. Length differences prune candidates.
The dynamic program uses one row and takes `O(ab)` time for compared
strings of length `a` and `b`, with a fixed row sized for the accepted
candidate length. Loading each parent or kind row scans the entry array,
so a projection with `P` rows over `N` pages takes `O(PN)` path
comparisons in the worst case. Callers generally load and free their own
index. Opening successive guide pages repeats that load.
[Index types](../src/index.h) · [Lookup and ranking](../src/index.c)

## Corpus build

The Python pipeline snapshots inputs by path, file metadata and content
hash. It also fingerprints tool sources, component inventory and Git
history, including changes to shallow history and replacement refs.
The build cache is a checksummed, schema-versioned JSON file. It reuses
page metadata and lint results when their inputs still match. Global
checks reconstruct and validate hierarchy and learning paths across all
entries. Dedicated builders generate the main index, projections, tldr
index and coverage reports.

A lock serializes the indexed build and its cache. Glossary index
generation runs before that lock. The pipeline rechecks input stability,
publishes changed outputs by replacing each file individually, and retries
once if it detects changed inputs. Corpus checks can render changed pages
through the command in a bounded worker pool and cache successful renders.
The authoritative inputs are the authored pages, tools and Git history.
[Pipeline](../tools/corpus_pipeline.py) · [Index generation](../tools/build-index.py)
