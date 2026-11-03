# Absolute-beginner desktop tasks, Slice 1 (corpus 0.61.0)

- **Date:** 2026-11-02
- **Time:** ended 22:10 -05:00; start not recorded in this session
- **Stage:** Develop. Corpus 0.60.0 to 0.61.0; the command version did not change.
- **CLI Tool:** opencode 1.19.4 (content); Cursor Composer finished the commit after opencode exhausted tokens
- **Model:** big-pickle (`opencode/big-pickle`); Cursor Composer for handoff commit
- **Transcript:** Retained privately by the author; Cursor takeover under Fred's direction 2026-11-02.
- **Commits:** `1f43879` on `develop` (corpus 0.61.0).
- **Authorship:** Fred approved the plan, set the audience and page granularity, and directed the first slice. Opencode wrote the plan edits, skill cells, pages, links, version bump and this record; Cursor took over the claim, re-ran `make corpus-and-test`, and committed. No live desktop actions or command-lane source changes.
- **Checked against:** `tamlinux/commands/tam-capture-screenshot/tam-capture-screenshot`,
  `tamlinux/commands/tam-capture-region/tam-capture-region`,
  `tamlinux/commands/tam-capture-text/tam-capture-text`,
  `tamlinux/commands/tam-capture-screenrecording/tam-capture-screenrecording`,
  `tamlinux/commands/tam-capture-edit/tam-capture-edit`,
  `tamlinux/desktop/session/hypr/tamlinux/bindings/utilities.lua`,
  `tamlinux/desktop/session/hypr/tamlinux/bindings/media.lua`,
  `tamlinux/desktop/session/hypr/tamlinux/bindings/clipboard.lua`,
  `tamlinux/desktop/shell/services/clipboard/Service.qml`,
  `tamlinux/desktop/shell/services/clipboard/open.sh`,
  `tamlinux/desktop/shell/services/idle/Service.qml`, `IdleModel.js`,
  `tamlinux/desktop/shell/services/notifications/Service.qml`,
  `tamlinux/desktop/shell/panels/bluetooth/Panel.qml`,
  `tamlinux/desktop/plugins/fred.monitor/README.md`,
  `tamlinux/desktop/menu/default.jsonc`,
  `tamlinux/commands/tam-toggle-idle/tam-toggle-idle`,
  `tamlinux/commands/tam-toggle-nightlight/tam-toggle-nightlight`,
  `tamlinux/commands/tam-display-text-size/tam-display-text-size`,
  `tamlinux/commands/tam-bluetooth-power/tam-bluetooth-power`,
  and the live outputs of `tam-display-text-size`, `tam-toggle-idle status`,
  `tam-toggle-nightlight --status`, `tam-audio-output-sink`,
  `tam-bluetooth-power is-on` and `tam-hw-touchpad`.

## Guiding prompts

> Mark the plan Ready and start Slice 1.

## Key decisions

The first slice stays beginner-only and covers nine job pages plus one grouped
command reference. The command hub was added in this slice although the plan
assigned a hub to a later slice, because the reference needs an existing parent
page. The pages use only captured or source-read output; live key chords are
described from the packaged bindings and are marked for developer verification.
Commands are hand-checked against their scripts rather than generated from
`tam-* --help`.

## Verification

`make corpus-and-test`: index generation reports 242 entries; the HTML linter,
35 linter unit tests, 21 index unit tests and all guide checks pass with no
warnings. With `TAM_KNOWLEDGE_DIR` pointed at the checkout corpus, `tam explain
screenshot` lists the new task page and command pages. No live desktop action
was run: screenshot, clipboard, Bluetooth, display and lock chords were read
from source, and the pages say so where behaviour needs the running session.
