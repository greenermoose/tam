#!/usr/bin/env python3
"""Small PTY screen model for the pager's emitted terminal subset."""

import fcntl
import os
import pty
import re
import select
import signal
import struct
import sys
import termios
import time
import unicodedata


class Screen:
    def __init__(self, rows, cols, bce=True):
        self.rows, self.cols, self.bce = rows, cols, bce
        self.cells = [[(" ", frozenset()) for _ in range(cols)] for _ in range(rows)]
        self.row = self.col = 0
        self.attrs = set()

    def resize(self, rows, cols):
        old = self.cells
        self.rows, self.cols = rows, cols
        self.cells = [[(" ", frozenset()) for _ in range(cols)] for _ in range(rows)]
        for r in range(min(rows, len(old))):
            for c in range(min(cols, len(old[r]))):
                self.cells[r][c] = old[r][c]
        self.row = min(self.row, rows - 1)
        self.col = min(self.col, cols - 1)

    def feed(self, data):
        i = 0
        while i < len(data):
            if data[i] == 0x1b:
                if i + 1 >= len(data):
                    break
                if data[i + 1] == ord("["):
                    j = i + 2
                    while j < len(data) and not (0x40 <= data[j] <= 0x7e):
                        j += 1
                    if j >= len(data):
                        break
                    self._csi(data[i + 2:j].decode("ascii", "ignore"), chr(data[j]))
                    i = j + 1
                    continue
                if data[i + 1] == ord("("):
                    i += 3
                    continue
                i += 2
                continue
            j = i + 1
            while j < len(data) and data[j] != 0x1b:
                j += 1
            self._text(data[i:j].decode("utf-8", "replace"))
            i = j

    def _csi(self, params, final):
        raw = params
        private = raw.startswith("?")
        raw = raw.lstrip("?")
        nums = [int(x) if x.isdigit() else 0 for x in raw.split(";") if x != ""]
        if final in ("H", "f"):
            self.row = max(0, min(self.rows - 1, (nums[0] if nums else 1) - 1))
            self.col = max(0, min(self.cols - 1, (nums[1] if len(nums) > 1 else 1) - 1))
        elif final == "J" and (not nums or nums[0] in (2, 3)):
            self.cells = [[(" ", frozenset()) for _ in range(self.cols)] for _ in range(self.rows)]
        elif final == "K":
            bg = next((x for x in self.attrs if x.startswith("bg")), "") if self.bce else ""
            for c in range(self.col, self.cols):
                self.cells[self.row][c] = (" ", frozenset({bg}) if bg else frozenset())
        elif final == "m":
            if not nums or nums == [0]:
                self.attrs.clear()
            else:
                k = 0
                while k < len(nums):
                    n = nums[k]
                    if n == 0:
                        self.attrs.clear()
                    elif n == 1:
                        self.attrs.add("bold")
                    elif n == 4:
                        self.attrs.add("underline")
                    elif n == 7:
                        self.attrs.add("reverse")
                    elif n == 27:
                        self.attrs.discard("reverse")
                    elif n == 39:
                        self.attrs = {x for x in self.attrs if not x.startswith("fg")}
                    elif n == 49:
                        self.attrs = {x for x in self.attrs if not x.startswith("bg")}
                    elif n == 38 and k + 2 < len(nums) and nums[k + 1] == 5:
                        self.attrs = {x for x in self.attrs if not x.startswith("fg")}
                        self.attrs.add(f"fg{nums[k + 2]}")
                        k += 2
                    elif n == 48 and k + 2 < len(nums) and nums[k + 1] == 5:
                        self.attrs = {x for x in self.attrs if not x.startswith("bg")}
                        self.attrs.add(f"bg{nums[k + 2]}")
                        k += 2
                    elif 30 <= n <= 37:
                        self.attrs.add(f"fg{n - 30}")
                    elif 90 <= n <= 97:
                        self.attrs.add(f"fg{n - 90 + 8}")
                    k += 1

    def _text(self, text):
        for ch in text:
            if ch in "\r\n":
                if ch == "\r":
                    self.col = 0
                else:
                    self.row = min(self.rows - 1, self.row + 1)
                    self.col = 0
                continue
            if ch == "\b":
                self.col = max(0, self.col - 1)
                continue
            width = 0 if unicodedata.combining(ch) else (2 if unicodedata.east_asian_width(ch) in "WF" else 1)
            if width == 0:
                continue
            if self.row < self.rows and self.col < self.cols:
                self.cells[self.row][self.col] = (ch, frozenset(self.attrs))
                if width == 2 and self.col + 1 < self.cols:
                    self.cells[self.row][self.col + 1] = ("", frozenset(self.attrs))
            self.col += width
            if self.col >= self.cols:
                self.col = self.cols - 1

    def line(self, row):
        return "".join(ch for ch, _ in self.cells[row])


def read_available(fd, timeout=0.08):
    out = bytearray()
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        left = deadline - time.monotonic()
        ready, _, _ = select.select([fd], [], [], max(0, left))
        if not ready:
            break
        try:
            part = os.read(fd, 65536)
        except OSError:
            break
        if not part:
            break
        out.extend(part)
    return bytes(out)


def start(binary, fixture_dir, term, rows, cols, extra_env=None, page="pager-plain"):
    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", rows, cols, 0, 0))
    pid = os.fork()
    if pid == 0:
        os.close(master)
        os.dup2(slave, 0)
        os.dup2(slave, 1)
        os.dup2(slave, 2)
        os.close(slave)
        os.environ["TAM_KNOWLEDGE_DIR"] = fixture_dir
        os.environ["TERM"] = term
        os.environ.pop("NO_COLOR", None)
        if extra_env:
            os.environ.update(extra_env)
        os.execl(binary, binary, "show", page)
    os.close(slave)
    return pid, master


def collect_screen(binary, fixture_dir, term, rows=8, cols=40, extra_env=None,
                   page="pager-plain"):
    pid, master = start(binary, fixture_dir, term, rows, cols, extra_env, page)
    screen = Screen(rows, cols)
    initial = read_available(master, 0.2)
    screen.feed(initial)
    return pid, master, screen, initial


def stop(pid, master):
    try:
        os.write(master, b"q")
    except OSError:
        pass
    try:
        os.waitpid(pid, 0)
    except ChildProcessError:
        pass
    os.close(master)


def main():
    binary = os.path.abspath(sys.argv[1])
    fixtures = os.path.abspath(sys.argv[2])

    pid, master, screen, initial = collect_screen(binary, fixtures, "xterm-256color")
    assert "Guide > Pager fixture" in screen.line(0), screen.line(0)
    assert "First heading" in screen.line(1), screen.line(1)
    focus_attrs = screen.cells[1][0][1]
    assert "bg236" in focus_attrs and "fg15" in focus_attrs, focus_attrs
    assert b"\x1b[2J" not in initial

    os.write(master, b"h")
    help_frame = read_available(master)
    screen.feed(help_frame)
    assert "Navigation help" in screen.line(1), screen.line(1)
    os.write(master, b"k")
    ignored_help_key = read_available(master)
    assert ignored_help_key == b"", f"help overlay moved the underlying page: {ignored_help_key!r}"
    os.write(master, b"?")
    close_help = read_available(master)
    screen.feed(close_help)
    assert "First heading" in screen.line(1), screen.line(1)
    assert "bg236" in screen.cells[1][0][1], screen.cells[1][0][1]

    os.write(master, b"z")
    ignored = read_available(master)
    assert ignored == b"", f"ignored key emitted {ignored!r}"
    os.write(master, b"k")
    first_move = read_available(master)
    screen.feed(first_move)
    os.write(master, b"k")
    moved = read_available(master)
    assert b"\x1b[2J" not in moved
    cups = re.findall(rb"\x1b\[(\d+);(\d+)H", moved)
    assert len(cups) == 3, f"one line move should repaint two content rows and status: {cups!r}"
    screen.feed(moved)
    assert "first attention row" in screen.line(3), screen.line(3)
    stop(pid, master)

    # Width resize relayouts retained parsed blocks and keeps later text reachable.
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color", 8, 40)
    fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack("HHHH", 4, 12, 0, 0))
    os.kill(pid, signal.SIGWINCH)
    resized = read_available(master, 0.2)
    assert b"\x1b[2J" not in resized
    screen.resize(4, 12)
    screen.feed(resized)
    seen = ""
    for _ in range(32):
        seen += "".join(screen.line(r) for r in range(screen.rows))
        os.write(master, b"k")
        screen.feed(read_available(master))
    normalized = "".join(seen.split())
    assert all(word in normalized for word in ("first", "attention", "row")), normalized
    assert "unbroken-to" in normalized and "ken-abcdefghijklmno" in normalized, normalized
    stop(pid, master)

    # Common narrow widths and two-/three-row terminals stay interactive.
    for rows, cols in ((2, 8), (3, 15), (3, 16), (3, 40)):
        pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color", rows, cols)
        os.write(master, b"k")
        moved = read_available(master)
        screen.feed(moved)
        assert b"\x1b[2J" not in moved
        stop(pid, master)

    # A split DECRQM reply enables sync output; an interleaved key remains queued.
    pid, master, screen, initial = collect_screen(binary, fixtures, "xterm-256color")
    assert b"\x1b[?2026$p" in initial
    os.write(master, b"\x1b[?20")
    time.sleep(0.02)
    os.write(master, b"2k6;2$y")
    synced = read_available(master, 0.3)
    assert b"\x1b[?2026h" in synced and b"\x1b[?2026l" in synced, synced
    stop(pid, master)

    # A supported terminal reporting status 0 leaves the enhancement disabled.
    pid, master, _, _ = collect_screen(binary, fixtures, "xterm-256color")
    os.write(master, b"\x1b[?2026;0$y")
    read_available(master, 0.2)
    os.write(master, b"k")
    rejected = read_available(master)
    assert b"\x1b[?2026h" not in rejected and b"\x1b[?2026l" not in rejected
    stop(pid, master)

    # The attention column spans every content row at the caught link's
    # columns, and moving the attention line clears it (D175).
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color", page="nav-grid")
    for key in (b"k", b"k", b"f"):
        os.write(master, key)
        screen.feed(read_available(master))
    link_row = next(r for r in range(screen.rows) if "TopRight" in screen.line(r))
    band_start = screen.line(link_row).index("TopRight")
    band_end = band_start + len("TopRight")
    heading_row = next(r for r in range(1, screen.rows) if "Nav grid" in screen.line(r))
    for row in (heading_row, link_row + 1, screen.rows - 2):
        for col in range(band_start, band_end):
            assert "bg236" in screen.cells[row][col][1], (row, col, screen.cells[row][col])
        assert "bg236" not in screen.cells[row][band_start - 1][1], (row, band_start - 1)
        assert "bg236" not in screen.cells[row][band_end][1], (row, band_end)
    assert "reverse" in screen.cells[link_row][band_start][1], screen.cells[link_row][band_start]
    os.write(master, b"d")
    screen.feed(read_available(master))
    assert "bg236" not in screen.cells[heading_row][band_start][1], screen.cells[heading_row][band_start]
    stop(pid, master)

    # Linux's eight-color profile keeps an interactive pager and uses reverse.
    pid, master, screen, _ = collect_screen(binary, fixtures, "linux")
    assert "reverse" in screen.cells[1][0][1], screen.cells[1][0][1]
    stop(pid, master)

    # NO_COLOR retains a visible focus indication; an empty value keeps color.
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color", extra_env={"NO_COLOR": "1"})
    assert "reverse" in screen.cells[1][0][1], screen.cells[1][0][1]
    stop(pid, master)
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color", extra_env={"NO_COLOR": ""})
    assert "bg236" in screen.cells[1][0][1], screen.cells[1][0][1]
    stop(pid, master)

    # TERM=dumb emits readable plain text and exits without terminal control.
    pid, master = start(binary, fixtures, "dumb", 8, 40)
    plain = read_available(master, 0.4)
    _, status = os.waitpid(pid, 0)
    assert os.waitstatus_to_exitcode(status) == 0
    assert b"\x1b" not in plain and b"First heading" in plain, plain
    os.close(master)

    # A one-row terminal remains navigable and does not report a status row.
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color", 1, 12)
    assert "First" in screen.line(0), screen.line(0)
    assert b"L1/" not in screen.line(0).encode()
    stop(pid, master)

    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color",
                                             2, 20, page="pager-empty")
    assert b"L1/0" not in b"".join(screen.line(r).encode() for r in range(2))
    stop(pid, master)

    # In-page links jump to their name=/id= target, not to the link's own text,
    # and put the target on the first content row; Left Arrow returns.
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color",
                                             8, 40, page="pager-anchors")
    os.write(master, b"\t")
    screen.feed(read_available(master))
    os.write(master, b"\r")
    screen.feed(read_available(master))
    assert "Second part" in screen.line(1), screen.line(1)
    assert "second body" in "".join(screen.line(r) for r in range(2, 4))
    os.write(master, b"\x1b[D")
    screen.feed(read_available(master))
    assert any("Third part" in screen.line(r) for r in range(1, 7))
    os.write(master, b"\t")
    screen.feed(read_available(master))
    os.write(master, b"\x1b[C")
    screen.feed(read_available(master))
    assert "Third part" in screen.line(1), screen.line(1)
    fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack("HHHH", 6, 20, 0, 0))
    os.kill(pid, signal.SIGWINCH)
    screen.resize(6, 20)
    screen.feed(read_available(master, 0.2))
    assert any("Third part" in screen.line(r) for r in range(1, 5))
    os.write(master, b"\x1b[D")
    screen.feed(read_available(master))
    assert any("Second part" in screen.line(r) for r in range(1, 5))
    stop(pid, master)

    print("ok - pager PTY screen model: focus, row diffs, no-op keys, capability fallback, tiny geometry, and in-page links")


if __name__ == "__main__":
    main()
