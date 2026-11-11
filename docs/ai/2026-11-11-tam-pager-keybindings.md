# Home-row arrow synonyms, bracket attention column, and guide index g (command 0.54.0 / corpus 0.78.15)

- **Date:** 2026-11-11
- **Time:** 09:53 to 17:40 -05:00
- **Stage:** Develop; command 0.54.0 and corpus 0.78.15
- **CLI Tool:** Antigravity CLI 1.5.5
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** <pending commit>
- **Authorship:** Fred Horch directed keybinding changes and selected option 3 for `g`; Antigravity implemented the key dispatch, tests, and documentation updates.
- **Checked against:** `src/pager_input.c`, `src/pager_draw.c`, `man/tam.1`, `knowledge/help/index.html`, `knowledge/welcome.html`.

## Guiding prompts

> Let's change how keybindings work in the tam command.
>
> Make "left hand keys" a s d f be synonyms for Left Arrow, Up Arrow, Down Arrow, and Right Arrow, respectively.
>
> Make "right hand keys" j k l ; be synonyms for Right Arrow, Down Arrow, Up Arrow, and Left Arrow, respectively.
>
> Make [ move the attention column left and ] move the attention column right.
>
> To recap,
>
> Function        Old keys    New keys
>
> Move Down       d k         d k
> Move Up         s l         s l
> Go back                     a ;
> Follow link     g           f j
> Column left     a j         [
> Column right    f ;         ]
>
> Check my work; does that all make logical sense? Have I put the correct keys with the correct function? What do you suggest g do in the new regime?

> 3

## Key decisions

- Left-hand home row `a s d f` are exact synonyms for Left Arrow, Up Arrow, Down Arrow, and Right Arrow.
- Right-hand home row `j k l ;` are exact synonyms for Right Arrow, Down Arrow, Up Arrow, and Left Arrow.
- Bilateral finger symmetry mirrors motor function between hands: pinkies (`a`, `;`) go back, ring fingers (`s`, `l`) go up, middle fingers (`d`, `k`) go down, and index fingers (`f`, `j`) follow links / go forward.
- Attention column navigation moves from home-row keys to `[` (column left) and `]` (column right). Tab and Shift-Tab remain whole-page link navigation keys.
- Key `g` is reassigned to jump immediately to the User Guide root index (`index.html`), identical to `` ` `` (backtick).

## Verification

- `make command`: built `tam` clean without warnings.
- `tests/run.sh build/tam`: all command integration tests passed, including home-row navigation, link following, back navigation, bracket column movement, and `g` index return.
- `make test-command`: integration tests, upgrade tests, renderer goldens, and `tools/check-repo.py` passed clean.
