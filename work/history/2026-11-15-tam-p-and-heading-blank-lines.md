# Blank lines around `<p>` and `<h1>`–`<h6>` (0.59.14)

- **Date:** 2026-11-15
- **Time:** 12:04 to 12:11 -05:00
- **Stage:** Develop; command `0.59.14`
- **CLI Tool:** Antigravity CLI 1.5.9
- **Model:** Gemini 4 Hydrogen (Medium) (`gemini-4-hydrogen-medium`)
- **Transcript:** Retained privately by the author.
- **Commits:** pending
- **Authorship:** Fred directed the `<p>` and heading blank-line behavior; Antigravity CLI implemented the `src/render.c` fix, golden/test updates, and version bump.
- **Checked against:** `src/render.c`, `src/fold.c`, `src/version.h`, `tests/render.sh`, `tests/run.sh`, `knowledge/welcome.html`

## Guiding prompts

> The tam pager should render a <p> or an <h2> element with one blank line before it, unless it is the first element on the page, and one blank line after it, unless it is the last element on the page.
>
> Thus,
>
> <table>
>   <tr><th>Key</th><th>Function in this Guide</th></tr>
>   <tr><td><code>h</code></td><td>help (full list of keybindings)</td></tr>
>   <tr><td><code>Down</code></td><td>move the attention line down</td></tr>
>   <tr><td><code>Up</code></td><td>move the attention line up</td></tr>
>   <tr><td><code>Space</code></td><td>scroll down a full screen</td></tr>
>   <tr><td><code>Tab</code></td><td>jump to the next answer or link</td></tr>
>   <tr><td><code>Right</code></td><td>activate (e.g. follow a hyperlink)</td></tr>
>   <tr><td><code>Left</code></td><td>bring you back</td></tr>
>   <tr><td><code>g</code></td><td>go to the guide index</td></tr>
>   <tr><td><code>q</code></td><td>quit</td></tr>
> </table>
>
> <p><section class="quiz">
> <details class="q">
> <summary><i>When you read this guide, how much detail do you want?</i></summary>
> <section data-level="novice"><code>1</code> Novice text only</section>
> <section data-level="learner"><code>2</code> Learner text only (with teaching examples for practice)</section>
> <section data-level="expert"><code>3</code> Expert text only (with usage examples as reminders)</section>
> <code>0</code> All text - you get the whole enchilada!
> </details>
> </section></p>
>
> should render
>
> q      quit
>
> When you read this guide, how much detail do you want?
> [+] Answer (-> to show)
>
> nextline
>
> Not,
>
> q      quit
> When you read this guide, how much detail do you want?
> [+] Answer (-> to show)
> nextline
>
> The <p> should give the blank lines, not the <section>.

> The h2 rule goes for every level of h heading element.

## Key decisions

- Removed `p->tight_next = 1` from `<section>` in `src/render.c` so `<section>` acts as a transparent container like `<article>` and `<div>` and does not suppress blank lines around `<p>` or headings.
- Extended `B_HEAD` tag recognition from `h1`–`h3` to `h1`–`h6`.
- Updated `layout_box` in `src/render.c` to ignore blank separator lines (`nspans == 0`) when checking whether a `B_META` line is a single line to right-align.

## Verification

- `make command LIBTAM_CFLAGS="-I/home/fred/Code/tamlinux/libtam/include" LIBTAM_LIBS="-L/home/fred/Code/tamlinux/libtam/build -ltam"`: built `build/tam`.
- `tests/render.sh build/tam`: `ok - render goldens match (7 fixtures)`.
- `tests/run.sh build/tam`: all interactive pager, level-lens, fold, and render tests passed (note: `--check storage_btrfs missing status pass` failed in this environment before and after the change).
- `make check-repo`: passed.
