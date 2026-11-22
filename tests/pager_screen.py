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
                if data[i + 1] == ord("]"):
                    # OSC string (hyperlinks), ended by BEL or ESC backslash.
                    j = i + 2
                    while j < len(data) and data[j] != 0x07 and \
                            not (data[j] == 0x1b and j + 1 < len(data) and data[j + 1] == ord("\\")):
                        j += 1
                    if j >= len(data):
                        break
                    i = j + 1 if data[j] == 0x07 else j + 2
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
                    elif n == 2:
                        self.attrs.add("dim")
                    elif n == 4:
                        self.attrs.add("underline")
                    elif n == 7:
                        self.attrs.add("reverse")
                    elif n == 22:
                        self.attrs.discard("dim")
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
                elif self.row == self.rows - 1:
                    # A newline on the bottom row scrolls the screen up.
                    self.cells = self.cells[1:] + [
                        [(" ", frozenset()) for _ in range(self.cols)]]
                    self.col = 0
                else:
                    self.row += 1
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


def read_available(fd, timeout=0.08, quiet=None):
    if quiet is None:
        quiet = 0.04 if timeout >= 0.3 else 0.005
    out = bytearray()
    deadline = time.monotonic() + timeout
    while True:
        wait = quiet if out else max(0.0, deadline - time.monotonic())
        ready, _, _ = select.select([fd], [], [], wait)
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


def focused_rows(screen):
    # The status row (when height >= 5) and two footer rows share the attention
    # tint, so only content rows count.
    limit = screen.rows - 3 if screen.rows >= 5 else screen.rows - 2
    return [row for row in range(limit)
            if any("bg236" in attrs for _, attrs in screen.cells[row])]


def check_vanished_history(binary):
    import tempfile
    # A history entry whose page has disappeared stays on the stack intact:
    # repeated back presses keep reporting the failure and the pager survives.
    with tempfile.TemporaryDirectory() as gone:
        page = ('<!DOCTYPE html><html><head><title>%s</title></head><body>'
                '<h1>%s</h1><p>%s</p></body></html>')
        for name, body in (("index", "Index"), ("b", "Plain page B")):
            with open(f"{gone}/{name}.html", "w") as f:
                f.write(page % (body, body, "text"))
        with open(f"{gone}/a.html", "w") as f:
            f.write(page % ("A", "A", 'Go to <a href="b.html">Page B</a>.'))
        pid, master, screen, _ = collect_screen(
            binary, gone, "xterm-256color", 24, 80, page="page:a.html")
        os.write(master, b"\t")
        screen.feed(read_available(master, 0.3))
        os.write(master, b"\r")
        screen.feed(read_available(master, 0.4))
        assert "Plain page B" in screen.line(0), screen.line(0)
        os.remove(f"{gone}/a.html")
        for _ in range(2):
            os.write(master, b"\x1b[D")
            screen.feed(read_available(master, 0.4))
            assert any("cannot open" in screen.line(r) for r in range(screen.rows)), \
                [screen.line(r) for r in range(screen.rows)]
        os.write(master, b"q")
        _, status = os.waitpid(pid, 0)
        os.close(master)
        assert os.WIFEXITED(status) and os.WEXITSTATUS(status) == 0, status


def main():
    binary = os.path.abspath(sys.argv[1])
    fixtures = os.path.abspath(sys.argv[2])

    pid, master, screen, initial = collect_screen(binary, fixtures, "xterm-256color")
    assert "Guide > Pager fixture" in screen.line(0), screen.line(0)
    assert "First heading" in screen.line(1), screen.line(1)
    focus_attrs = screen.cells[1][0][1]
    assert "bg236" in focus_attrs and "fg15" in focus_attrs, focus_attrs
    assert b"\x1b[2J" not in initial

    os.write(master, b"\x1b[A")
    screen.feed(read_available(master))
    assert focused_rows(screen) == [0], focused_rows(screen)
    assert "bg236" in screen.cells[0][-2][1] and "fg15" in screen.cells[0][-2][1], screen.cells[0][-2][1]
    os.write(master, b"\x1b[B")
    screen.feed(read_available(master))
    assert focused_rows(screen) == [1], focused_rows(screen)

    os.write(master, b"g")
    help_frame = read_available(master)
    screen.feed(help_frame)
    assert any("Esc to close" in screen.line(r) or "help" in screen.line(r).lower() for r in range(10)), \
        [screen.line(r) for r in range(10)]
    os.write(master, b"k")
    help_move = read_available(master)
    screen.feed(help_move)
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

    # Reflow keeps the focused text row when a separator shares its logical
    # block and offset. Growing back to the original width must not drift again.
    pid, master, screen, _ = collect_screen(
        binary, fixtures + "/guide", "xterm-256color", 24, 100,
        page="page:welcome.html")
    for _ in range(40):
        rows = focused_rows(screen)
        if len(rows) == 1 and "Welcome line 2" in screen.line(rows[0]):
            break
        os.write(master, b"k")
        screen.feed(read_available(master))
    rows = focused_rows(screen)
    assert len(rows) == 1 and "Welcome line 2" in screen.line(rows[0]), rows
    fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 80, 0, 0))
    os.kill(pid, signal.SIGWINCH)
    screen.resize(24, 80)
    screen.feed(read_available(master, 0.2))
    rows = focused_rows(screen)
    assert len(rows) == 1 and "Welcome line 2" in screen.line(rows[0]), rows
    fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 100, 0, 0))
    os.kill(pid, signal.SIGWINCH)
    screen.resize(24, 100)
    screen.feed(read_available(master, 0.2))
    rows = focused_rows(screen)
    assert len(rows) == 1 and "Welcome line 2" in screen.line(rows[0]), rows
    stop(pid, master)

    # A cursor already on a blank separator remains on a blank after reflow.
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color")
    for _ in range(20):
        rows = focused_rows(screen)
        if len(rows) == 1 and screen.line(rows[0]).strip() == "":
            break
        os.write(master, b"k")
        screen.feed(read_available(master))
    rows = focused_rows(screen)
    assert len(rows) == 1 and screen.line(rows[0]).strip() == "", rows
    fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack("HHHH", 8, 30, 0, 0))
    os.kill(pid, signal.SIGWINCH)
    screen.resize(8, 30)
    screen.feed(read_available(master, 0.2))
    rows = focused_rows(screen)
    assert len(rows) == 1 and screen.line(rows[0]).strip() == "", rows
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
        seen += "".join(screen.line(r) for r in range(1, screen.rows - 2))
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

    # ] works like Tab to select next link; [ works like Shift-Tab to select previous link.
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color", page="nav-grid")
    for key in (b"k", b"k", b"]"):
        os.write(master, key)
        screen.feed(read_available(master))
    link_row = next(r for r in range(screen.rows) if "TopRight" in screen.line(r))
    tr_start = screen.line(link_row).index("TopRight")
    assert "reverse" in screen.cells[link_row][tr_start][1], screen.cells[link_row][tr_start]
    os.write(master, b"[")
    screen.feed(read_available(master))
    tl_start = screen.line(link_row).index("TopLeft")
    assert "reverse" in screen.cells[link_row][tl_start][1], screen.cells[link_row][tl_start]
    stop(pid, master)

    # Linux's eight-color profile keeps an interactive pager and uses reverse.
    pid, master, screen, _ = collect_screen(binary, fixtures, "linux")
    assert "reverse" in screen.cells[1][0][1], screen.cells[1][0][1]
    # Linux has no alternate screen. Quitting keeps the last page visible,
    # scrolled up one line, and leaves the cursor at the start of a fresh,
    # empty bottom line for the shell prompt.
    page = [screen.line(r) for r in range(screen.rows)]
    os.write(master, b"q")
    screen.feed(read_available(master, 0.4))
    os.waitpid(pid, 0)
    os.close(master)
    after = [screen.line(r) for r in range(screen.rows)]
    assert after[:-1] == page[1:], (page, after)
    assert after[-1].strip() == "", after
    assert (screen.row, screen.col) == (screen.rows - 1, 0), (screen.row, screen.col)

    # Linux's eight-color profile renders semantic colors on content (cyan
    # headings, bright blue links where underline is suppressed by ncv).
    pid, master, screen, _ = collect_screen(binary, fixtures, "linux")
    assert {"bold", "fg4"}.issubset(screen.cells[0][0][1]), screen.cells[0][0][1]
    os.write(master, b"k")
    screen.feed(read_available(master, 0.2))
    assert {"bold", "fg6"}.issubset(screen.cells[1][0][1]), screen.cells[1][0][1]
    assert "reverse" not in screen.cells[1][0][1], screen.cells[1][0][1]
    stop(pid, master)

    # NO_COLOR under linux retains reverse focus and omits foreground colors.
    pid_nc, master_nc, screen_nc, _ = collect_screen(binary, fixtures, "linux", extra_env={"NO_COLOR": "1"})
    assert "reverse" in screen_nc.cells[1][0][1], screen_nc.cells[1][0][1]
    os.write(master_nc, b"k")
    screen_nc.feed(read_available(master_nc, 0.2))
    assert "fg6" not in screen_nc.cells[1][0][1], screen_nc.cells[1][0][1]
    stop(pid_nc, master_nc)

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
    assert b"L1 " not in screen.line(0).encode()
    stop(pid, master)

    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color",
                                             2, 20, page="pager-empty")
    assert b"L1 0-0/0" not in screen.line(0).encode()
    assert b"L1 0-0/0" in screen.line(1).encode()
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

    # Scrolling status row on an overflowing page
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color",
                                             8, 40, page="pager-plain")
    c_rows = 8 - 4  # location row plus status row plus two footer rows

    def parse_pos(line):
        m = re.search(r"L(\d+)\s+(\d+)-(\d+)/(\d+)", line)
        assert m, f"position metric missing in status line: {line!r}"
        return int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4))

    # At top: first is 1
    cur, first, last, total = parse_pos(screen.line(6))
    assert first == 1, f"expected first == 1 at top, got {first}"
    assert last == c_rows, f"expected last == {c_rows} at top, got {last}"
    assert last - first + 1 == c_rows, f"expected {c_rows} content rows, got {last - first + 1}"
    assert total > c_rows, f"expected page to overflow: total={total}, content_rows={c_rows}"

    # Space (page down) from top: previous last line becomes first line
    prev_last = last
    os.write(master, b" ")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert first == prev_last and cur == prev_last, f"expected first==cur=={prev_last} after page down, got {first}, {cur}"
    assert last == prev_last + c_rows - 1, f"expected last=={prev_last + c_rows - 1} after page down, got {last}"
    assert last - first + 1 == c_rows, f"expected {c_rows} content rows, got {last - first + 1}"

    # Page Down escape sequence (\x1b[6~): previous last line becomes first line
    prev_last = last
    os.write(master, b"\x1b[6~")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert first == prev_last and cur == prev_last, f"expected first==cur=={prev_last}, got {first}, {cur}"
    assert last == prev_last + c_rows - 1, f"expected last=={prev_last + c_rows - 1}, got {last}"

    # Space when remaining lines not enough for full step: shows last full page
    os.write(master, b" ")
    screen.feed(read_available(master))
    os.write(master, b" ")
    screen.feed(read_available(master))
    os.write(master, b" ")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert last == total, f"expected last==total ({total}), got {last}"
    assert first == total - c_rows + 1, f"expected first full page top ({total - c_rows + 1}), got {first}"
    assert cur == first, f"expected attention at top of last page ({first}), got {cur}"

    # Space when already at last full page: moves attention row to bottom line
    os.write(master, b" ")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert cur == total, f"expected attention at document end ({total}), got {cur}"
    assert last == total and first == total - c_rows + 1

    # Page Up escape sequence (\x1b[5~): previous first line becomes last line
    prev_first = first
    os.write(master, b"\x1b[5~")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert last == prev_first, f"expected last==prev_first ({prev_first}) after page up, got {last}"
    assert last - first + 1 == c_rows, f"expected {c_rows} content rows, got {last - first + 1}"
    assert cur == first, f"expected attention at top of new screen ({first}), got {cur}"

    # Shift-Space via Kitty CSI u (\x1b[32;2u): previous first line becomes last line
    prev_first = first
    os.write(master, b"\x1b[32;2u")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert last == prev_first, f"expected last==prev_first ({prev_first}) after Shift-Space, got {last}"
    assert last - first + 1 == c_rows, f"expected {c_rows} content rows, got {last - first + 1}"
    assert cur == first, f"expected attention at {first}, got {cur}"

    os.write(master, b"b")
    screen.feed(read_available(master))
    os.write(master, b"b")
    screen.feed(read_available(master))

    # 'b' key when remaining lines toward top not enough for full step: shows first full page
    os.write(master, b"b")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert first == 1, f"expected first==1 after b, got {first}"
    assert last == c_rows, f"expected last=={c_rows}, got {last}"

    # 'b' when already at first full page: moves attention line to top (L1)
    os.write(master, b"b")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert cur == 1 and first == 1, f"expected cur==1 and first==1 at top, got {cur}, {first}"

    # Shift-Space via xterm modifyOtherKeys (\x1b[27;2;32~) works as page up
    # Move down with Space first
    os.write(master, b" ")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert first == c_rows, f"expected first=={c_rows} after Space, got {first}"
    os.write(master, b"\x1b[27;2;32~")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert first == 1, f"expected modifyOtherKeys Shift-Space to page up, got {first}"

    # c (bottom): last equals total
    os.write(master, b"c")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(6))
    assert last == total, f"expected last == total ({total}) at bottom, got {last}"
    assert last - first + 1 == c_rows, f"expected {c_rows} content rows, got {last - first + 1}"
    stop(pid, master)

    # Single-screen document: Space moves attention to bottom, PgUp to top
    pid, master, screen, _ = collect_screen(binary, fixtures, "xterm-256color",
                                             24, 80, page="nav-grid")
    cur, first, last, total = parse_pos(screen.line(22))
    assert total <= 24 - 3, f"expected single screen page: total={total}"
    assert cur == 1, f"expected cur==1 initially, got {cur}"
    os.write(master, b" ")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(22))
    assert cur == total, f"expected Space on single screen to move to bottom ({total}), got {cur}"
    os.write(master, b"\x1b[5~")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(22))
    assert cur == 1, f"expected PgUp on single screen to move to top (1), got {cur}"
    stop(pid, master)

    # The revision stamp sits right-aligned and dim on the second-to-bottom
    # footer row (with the footer attention background), while the
    # third-from-bottom row is the reserved status line. Neither is a page
    # line, so the footer total excludes them and the attention line cannot
    # reach them.
    rows, cols = 8, 70
    stamp = "Rev 3 Last updated 2026-11-16 10:00"
    pid, master, screen, _ = collect_screen(binary, fixtures + "/revision",
                                             "xterm-256color", rows, cols,
                                             page="revision")
    c_rows = rows - 4  # breadcrumb row plus status row plus two footers
    cur, first, last, total = parse_pos(screen.line(rows - 2))
    assert first == 1 and last == c_rows, (first, last)
    assert total > c_rows, f"expected the page to overflow: total={total}"
    footer2_row = screen.line(rows - 2)
    assert footer2_row.rstrip().endswith(stamp), repr(footer2_row)
    stamp_col = cols - 1 - len(stamp)
    assert footer2_row[stamp_col:stamp_col + len(stamp)] == stamp, repr(footer2_row)
    assert "dim" in screen.cells[rows - 2][stamp_col][1], \
        screen.cells[rows - 2][stamp_col][1]
    assert "bg236" in screen.cells[rows - 2][stamp_col][1], \
        screen.cells[rows - 2][stamp_col][1]
    assert all(stamp not in screen.line(r) for r in range(rows - 2)), \
        "the stamp must not be a page line or on the status row"

    # Pressing '2' puts the lens notice on the third-from-bottom status row
    # with the footer tint background, leaving footer2's Rev stamp intact.
    os.write(master, b"2")
    screen.feed(read_available(master))
    status_row_text = screen.line(rows - 3)
    assert "[Level 2 Learner" in status_row_text, repr(status_row_text)
    assert "bg236" in screen.cells[rows - 3][0][1], \
        screen.cells[rows - 3][0][1]
    assert screen.line(rows - 2).rstrip().endswith(stamp), repr(screen.line(rows - 2))

    os.write(master, b"c")
    screen.feed(read_available(master))
    cur, first, last, total = parse_pos(screen.line(rows - 2))
    assert cur == total and last == total, (cur, last, total)
    assert first == total - c_rows + 1, (first, total)
    assert focused_rows(screen) == [rows - 4], focused_rows(screen)
    assert screen.line(rows - 2).rstrip().endswith(stamp), \
        "the stamp must stay on footer2 when scrolling"
    stop(pid, master)

    # A terminal too short for breadcrumb, page, status and footers drops the
    # third-from-bottom status row so the page keeps a line.
    pid, master, screen, _ = collect_screen(binary, fixtures + "/revision",
                                             "xterm-256color", 4, 70,
                                             page="revision")
    assert all("Rev 3 Last updated" not in screen.line(r) for r in range(2)), \
        [screen.line(r) for r in range(4)]
    assert stamp in screen.line(2), [screen.line(r) for r in range(4)]
    stop(pid, master)

    check_vanished_history(binary)

    print("ok - pager PTY screen model: focus, vanished history page, row diffs, no-op keys, capability fallback, tiny geometry, in-page links, and the revision stamp row")


if __name__ == "__main__":
    main()
