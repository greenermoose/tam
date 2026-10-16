#!/usr/bin/env python3
"""Check the knowledge corpus for defects a script can catch.

Usage: lint-corpus.py [--root DIR]

Reads DIR/knowledge/**/*.html and the command sources that define what the
renderer and the command accept (src/render.c, src/tam.c, src/install.c).
Prints one finding per line, "knowledge/<page>:<line>: <RULE>: <message>",
sorted by page, line and rule. Exit 0 when clean, 1 with findings, 2 when a
source file is missing or yields nothing. The rules are described in
docs/corpus-authoring.md.
"""

import argparse
import re
import sys
from pathlib import Path

EXTRA_SUBCOMMANDS = {"help", "--help", "-h", "--version", "-V"}
LATEX_WORDS = r"\\(?:rightarrow|leftarrow|times|approx|text|frac|cdot)\b"
WORKTREE_PAGE = "knowledge/improve/toolchains.html"
BLOCKS = {"p", "li", "dd", "dt", "td", "pre"}

TAG = re.compile(r"<(/?)([A-Za-z][A-Za-z0-9]*)(?:[^>\"']|\"[^\"]*\"|'[^']*')*>"
                 r"|<![^>]*>")
COMMENT = re.compile(r"<!--.*?-->", re.S)
ENTITY = re.compile(r"&([A-Za-z][A-Za-z0-9]*);")
HOME = re.compile(r"/home/([A-Za-z0-9_.-]+)")
PRIVATE_NAMES = re.compile(r"improvements-fred-tamlinux|config-fred-tamlinux")
UUID = re.compile(r"\b[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}\b")
DOLLAR = re.compile(r"\$[^$\n]*\\[A-Za-z]+[^$\n]*\$")
BARE_LATEX = re.compile(LATEX_WORDS)
INTERNAL_ID = re.compile(r"\b(?:D[0-9]{2,3}|V[0-9]{2})\b")
# "tam" as a command word: not part of a path or name, and followed by a
# word that starts with a letter or a dash (not a version number).
TAM_WORD = re.compile(r"(?<![\w./~-])tam[ \t]+([A-Za-z][\w-]*|--?[\w-]+)")
INSTALL_WORD = re.compile(r"(?<![\w./~-])tam[ \t]+install[ \t]+([^\s<]+)")
STUB_HINT = re.compile(r"planned|not implemented", re.I)


class SourceError(Exception):
    pass


def read_source(root, rel):
    path = root / rel
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        raise SourceError(f"{rel}: cannot read source file")


def load_rules(root):
    render = read_source(root, "src/render.c")
    m = re.search(r"entities\[\]\s*=\s*\{(.*?)\};", render, re.S)
    entities = set(re.findall(r'\{"([A-Za-z0-9]+)"', m.group(1))) if m else set()
    if not entities:
        raise SourceError("src/render.c: no entities[] entries found")

    tam = read_source(root, "src/tam.c")
    subcommands = set(re.findall(r'strcmp\(arg, "([^"]+)"\)', tam))
    if not subcommands:
        raise SourceError("src/tam.c: no subcommands found")
    subcommands |= EXTRA_SUBCOMMANDS

    install = read_source(root, "src/install.c").splitlines()
    verbs, stubs = set(), set()
    for i, line in enumerate(install):
        for verb in re.findall(r'strcmp\(sub, "([^"]+)"\)', line):
            verbs.add(verb)
            if any("not implemented yet" in later for later in install[i + 1:i + 6]):
                stubs.add(verb)
    if not verbs:
        raise SourceError("src/install.c: no install verbs found")
    return entities, subcommands, verbs, stubs


class Page:
    def __init__(self, rel, text, rules):
        self.rel = rel
        self.entities, self.subcommands, self.verbs, self.stubs = rules
        # Blank comments but keep their newlines, so offsets keep their lines.
        self.text = COMMENT.sub(lambda m: re.sub(r"[^\n]", " ", m.group(0)), text)
        self.starts = [0] + [m.end() for m in re.finditer("\n", self.text)]
        self.findings = []

    def line(self, offset):
        lo, hi = 0, len(self.starts) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if self.starts[mid] <= offset:
                lo = mid
            else:
                hi = mid - 1
        return lo + 1

    def add(self, offset, rule, message):
        self.findings.append((self.rel, self.line(offset), rule, message))

    def check(self):
        self.check_raw()
        self.check_text()
        return self.findings

    def check_raw(self):
        for m in ENTITY.finditer(self.text):
            if m.group(1) not in self.entities:
                self.add(m.start(), "ENTITY",
                         f"entity &{m.group(1)}; is not decoded by the renderer")
        for m in HOME.finditer(self.text):
            if m.group(1) not in ("alex", "user"):
                self.add(m.start(), "PRIVATE", f"private reference: /home/{m.group(1)}")
        for pattern in (PRIVATE_NAMES, UUID):
            for m in pattern.finditer(self.text):
                self.add(m.start(), "PRIVATE", f"private reference: {m.group(0)}")
        if self.rel != WORKTREE_PAGE:
            for m in re.finditer(r"worktree", self.text, re.I):
                self.add(m.start(), "WORKTREE",
                         "worktrees are mentioned only in improve/toolchains.html")

    def check_text(self):
        pre = code = 0
        blocks = []  # [tag, [(offset, verb), ...], [text pieces]]
        pos = 0
        for m in TAG.finditer(self.text):
            self.text_segment(pos, self.text[pos:m.start()], pre, code, blocks)
            pos = m.end()
            if m.group(2) is None:
                continue
            closing, name = m.group(1) == "/", m.group(2).lower()
            if name == "pre":
                pre += -1 if closing else 1
            elif name == "code":
                code += -1 if closing else 1
            if name in BLOCKS:
                if not closing:
                    blocks.append([name, [], []])
                else:
                    for i in range(len(blocks) - 1, -1, -1):
                        if blocks[i][0] == name:
                            self.close_block(blocks.pop(i))
                            break
            pre, code = max(pre, 0), max(code, 0)
        self.text_segment(pos, self.text[pos:], pre, code, blocks)
        while blocks:
            self.close_block(blocks.pop())

    def text_segment(self, base, text, pre, code, blocks):
        if not text:
            return
        if blocks:
            blocks[-1][2].append(text)
            for m in INSTALL_WORD.finditer(text):
                if m.group(1) in self.stubs:
                    blocks[-1][1].append((base + m.start(), m.group(1)))
        if not pre and not code:
            spans = [(m.start(), m.end()) for m in DOLLAR.finditer(text)]
            for start, _ in spans:
                self.add(base + start, "LATEX", "LaTeX is not rendered")
            for m in BARE_LATEX.finditer(text):
                if not any(s <= m.start() < e for s, e in spans):
                    self.add(base + m.start(), "LATEX", "LaTeX is not rendered")
        if not pre:
            for m in INTERNAL_ID.finditer(text):
                self.add(base + m.start(), "INTERNAL-ID",
                         f"internal identifier {m.group(0)}; state the reasoning instead")
        if pre or code:
            for m in TAM_WORD.finditer(text):
                word = m.group(1)
                if word != "command" and word not in self.subcommands:
                    self.add(base + m.start(), "SUBCOMMAND",
                             f"tam {word} is not a tam subcommand")
            for m in INSTALL_WORD.finditer(text):
                word = m.group(1)
                if not word.startswith("-") and word not in self.verbs:
                    self.add(base + m.start(), "INSTALL-VERB",
                             f"tam install {word} is not an install verb")

    def close_block(self, block):
        _, stubs, pieces = block
        if stubs and not STUB_HINT.search("".join(pieces)):
            for offset, verb in stubs:
                self.add(offset, "STUB",
                         f"tam install {verb} is not implemented; mark it planned")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", type=Path,
                        default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args(argv)
    root = args.root
    try:
        rules = load_rules(root)
    except SourceError as err:
        print(f"lint-corpus: {err}", file=sys.stderr)
        return 2
    knowledge = root / "knowledge"
    if not knowledge.is_dir():
        print("lint-corpus: knowledge/: no corpus directory", file=sys.stderr)
        return 2
    findings = []
    for path in sorted(knowledge.rglob("*.html")):
        rel = "knowledge/" + path.relative_to(knowledge).as_posix()
        text = path.read_text(encoding="utf-8", errors="replace")
        findings.extend(Page(rel, text, rules).check())
    for rel, line, rule, message in sorted(set(findings)):
        print(f"{rel}:{line}: {rule}: {message}")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
