#!/usr/bin/env python3
"""External links: confirm, then hand off to a browser; OSC 8 hyperlinks.

Stub browsers record their arguments, so no real browser or network is used.
"""

import os
import re
import stat
import subprocess
import sys
import tempfile
import time

from pager_screen import Screen, read_available, start, stop

PAGE = "external-links"
URL = "https://example.org/tam"


def stub(directory, name, body):
    path = os.path.join(directory, name)
    with open(path, "w") as f:
        f.write(body)
    os.chmod(path, os.stat(path).st_mode | stat.S_IXUSR)
    return path


def wait_for(path, timeout=2.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if os.path.exists(path) and os.path.getsize(path) > 0:
            return True
        time.sleep(0.02)
    return False


class Pager:
    def __init__(self, binary, fixtures, env, term="xterm-256color", cols=120, page=PAGE):
        self.pid, self.master = start(binary, fixtures, term, 24, cols, env, page)
        self.screen = Screen(24, cols)
        self.initial = read_available(self.master, 0.3)
        self.screen.feed(self.initial)

    def key(self, data, wait=0.12):
        os.write(self.master, data)
        raw = read_available(self.master, wait)
        self.screen.feed(raw)
        return raw

    def status(self):
        # The third-from-bottom status row carries the link target and messages.
        return self.screen.line(21)

    def stop(self):
        stop(self.pid, self.master)


def manual_links(binary, fixtures):
    url = "https://example.org/long-path/one(two)/another-very-long-component?x=1&y=2#end"
    page = "man:link-fixture(1)"
    with tempfile.TemporaryDirectory() as work:
        out = os.path.join(work, "browser-args")
        stub(work, "man", '#!/bin/sh\nprintf "%s\\n" "$TAM_TEST_MAN_PAGE"\n')
        stub(work, "groff", '#!/bin/sh\ncat "$TAM_TEST_MAN_PAGE"\n')
        stub(work, "browser-stub", '#!/bin/sh\nprintf "%s\\n" "$@" > "$TAM_TEST_OUT"\n')
        env = {
            "PATH": work + os.pathsep + os.environ.get("PATH", "/usr/bin:/bin"),
            "WAYLAND_DISPLAY": "test",
            "BROWSER": "browser-stub %s",
            "TAM_TEST_OUT": out,
            "TAM_TEST_MAN_PAGE": os.path.join(fixtures, "man-urls.html"),
            "XDG_CACHE_HOME": work, "XDG_RUNTIME_DIR": work, "XDG_STATE_HOME": work,
        }
        expected = [
            url, "https://example.com/docs", "HTTP://[::1]/keep-period.",
            "https://example.org/keep-comma,", "https://example.org/a%20b(item)",
            "https://example.net/explicit", "https://example.org/code",
            "https://example.org/cell", "man:sed(1)",
        ]
        for args in (["show", page], ["man", "1", "link-fixture"]):
            output = subprocess.run([binary, *args, "--pipe"],
                                    env=dict(os.environ, TAM_KNOWLEDGE_DIR=fixtures, **env),
                                    capture_output=True, check=True).stdout
            assert b"\x1b" not in output, "manual pipe output carries escapes"
            targets = re.findall(rb"^\s*\[\d+\] ((?:https?://|ftp://|man:).+)$",
                                 output, re.MULTILINE | re.IGNORECASE)
            assert [target.decode() for target in targets] == expected, output.decode()

        p = Pager(binary, fixtures, env, cols=52, page=page)
        try:
            p.key(b"\t")
            osc_ids = re.findall(rb"\x1b\]8;id=(tam\d+);" + re.escape(url.encode()) +
                                 rb"\x1b\\", p.initial)
            assert len(osc_ids) > 1 and len(set(osc_ids)) == 1, osc_ids
            p.key(b"\r")
            assert "Enter again opens it" in p.status(), p.status()
            assert not os.path.exists(out), "manual link opened on the first press"
            p.key(b"\r", 0.3)
            assert wait_for(out), "manual link did not reach the browser"
            with open(out) as f:
                assert f.read().splitlines() == [url], "manual URL changed before handoff"
            p.key(b"\t")
            assert "-> https://example.com/docs" in p.status(), p.status()
        finally:
            p.stop()


def main():
    binary = os.path.abspath(sys.argv[1])
    fixtures = os.path.abspath(sys.argv[2])
    work = tempfile.mkdtemp()
    bin_dir = os.path.join(work, "bin")
    os.mkdir(bin_dir)
    out = os.path.join(work, "args")
    stub(bin_dir, "browser-stub", '#!/bin/sh\nprintf "%s\\n" "$@" > "$TAM_TEST_OUT"\n')
    stub(bin_dir, "broken-stub", "#!/nonexistent/interpreter\n")
    stub(bin_dir, "text-stub",
         '#!/bin/sh\n'
         'printf "%s\\n" "$@" > "$TAM_TEST_OUT.tmp"\n'
         '[ -t 0 ] && [ -t 1 ] && echo tty >> "$TAM_TEST_OUT.tmp"\n'
         'stty -a | grep -q -- " icanon" && echo cooked >> "$TAM_TEST_OUT.tmp"\n'
         'echo TEXT-BROWSER-RAN\n'
         'mv "$TAM_TEST_OUT.tmp" "$TAM_TEST_OUT"\n')
    path = bin_dir + os.pathsep + os.environ.get("PATH", "/usr/bin:/bin")
    graphical = {"WAYLAND_DISPLAY": "test", "DISPLAY": "", "PATH": path,
                 "BROWSER": "browser-stub --new %s", "TAM_TEST_OUT": out}

    # Graphical session: the first press names the browser, the second opens.
    p = Pager(binary, fixtures, graphical)
    assert b"\x1b]8;id=tam" in p.initial and (URL.encode() + b"\x1b\\") in p.initial, \
        "web link is not an OSC 8 hyperlink"
    assert b"ftp://example.org/file\x1b\\" not in p.initial, "ftp link became a hyperlink"
    assert b"has space\x1b\\" not in p.initial, "malformed link became a hyperlink"
    p.key(b"\t")
    assert "-> " + URL in p.status(), p.status()
    p.key(b"\r")
    assert "Enter again opens it in browser-stub" in p.status(), p.status()
    assert not os.path.exists(out), "browser ran on the first press"
    # Any other key forgets the pending link and its prompt.
    p.key(b"N")
    assert "Enter again" not in p.status(), p.status()
    p.key(b"\r")
    assert "Enter again opens it in browser-stub" in p.status(), p.status()
    p.key(b"\r", 0.3)
    assert wait_for(out), "browser did not run on the second press"
    with open(out) as f:
        assert f.read().splitlines() == ["--new", URL], "browser got the wrong arguments"
    assert "opened in browser-stub" in p.status(), p.status()
    os.unlink(out)

    # Other schemes and malformed URLs are refused without running anything.
    p.key(b"\t")
    p.key(b"\r")
    assert "cannot open ftp: links" in p.status(), p.status()
    p.key(b"\t")
    p.key(b"\r")
    assert "cannot open malformed link" in p.status(), p.status()
    p.key(b"\r")
    assert not os.path.exists(out), "a refused link ran the browser"
    p.stop()

    # The confirmation cannot disappear when the usual status fields fill
    # a narrow terminal.
    p = Pager(binary, fixtures, graphical, cols=60)
    p.key(b"\t")
    p.key(b"\r")
    assert "Enter again opens it in browser-stub" in p.status(), p.status()
    p.stop()

    # No installed browser, and a browser that cannot start.
    p = Pager(binary, fixtures, dict(graphical, BROWSER="no-such-browser", PATH=bin_dir))
    p.key(b"\t")
    p.key(b"\r")
    assert "no browser found; set BROWSER" in p.status(), p.status()
    p.stop()
    p = Pager(binary, fixtures, dict(graphical, BROWSER="broken-stub"))
    p.key(b"\t")
    p.key(b"\r")
    p.key(b"\r", 0.3)
    assert "cannot start broken-stub" in p.status(), p.status()
    p.stop()

    # Console: the text browser gets this terminal in its usual modes, then
    # the pager comes back and still takes keys.
    console = dict(graphical, WAYLAND_DISPLAY="", TAM_TEXT_BROWSER="text-stub")
    p = Pager(binary, fixtures, console)
    p.key(b"\t")
    p.key(b"\r")
    assert "Enter again opens it in text-stub" in p.status(), p.status()
    raw = p.key(b"\r", 0.6)
    assert wait_for(out), "text browser did not run"
    with open(out) as f:
        seen = f.read().splitlines()
    assert seen == [URL, "tty", "cooked"], f"text browser saw {seen}"
    assert b"TEXT-BROWSER-RAN" in raw, "text browser output missing"
    raw += p.key(b"", 0.2)
    p.screen.feed(b"")
    assert "back from text-stub" in p.status(), p.status()
    assert any("External links" in p.screen.line(r) for r in range(23)), "page not redrawn"
    p.key(b"\t")
    assert "-> ftp://example.org/file" in p.status(), p.status()
    p.stop()

    # The Linux console gets no hyperlinks, and piped text no escapes.
    p = Pager(binary, fixtures, graphical, term="linux")
    assert b"\x1b]8;" not in p.initial, "OSC 8 sent to the Linux console"
    p.stop()
    env = dict(os.environ, TAM_KNOWLEDGE_DIR=fixtures)
    text = subprocess.run([binary, "show", PAGE, "--pipe"], env=env,
                          capture_output=True, check=True).stdout
    assert b"\x1b" not in text, "piped output carries escapes"

    manual_links(binary, fixtures)
    print("ok - external links: confirm then open, refusals, console hand-off, OSC 8")


if __name__ == "__main__":
    main()
