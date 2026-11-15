#!/usr/bin/env python3
"""Generate glossary index pages and check/fix misplaced glossary term pages."""
from __future__ import annotations

import html
import os
from pathlib import Path
import re
import shutil
import string
import subprocess
import tempfile

TITLE_RE = re.compile(r"<title>(.*?)</title>", re.I | re.S)
META_RE = re.compile(r"<meta\b([^>]*)>", re.I)
ATTR_RE = re.compile(r'([A-Za-z_:][-A-Za-z0-9_:.]*)\s*=\s*(?:"([^"]*)"|\'([^\']*)\')')
HREF_RE = re.compile(r'(href\s*=\s*)(["\'])([^"\']+)(\2)')
BREADCRUMB_RE = re.compile(r'(<nav\b[^>]*aria-label=["\']Breadcrumb["\'][^>]*>.*?</nav>)', re.I | re.S)
LETTER_CRUMB_RE = re.compile(
    r'<a\s+href=(["\'])glossary/[a-z]/index\.html\1>[^<]*</a>', re.I
)


class GlossaryPlacementError(Exception):
    pass


def atomic_write(path: Path, content: str | bytes) -> None:
    data = content.encode("utf-8") if isinstance(content, str) else content
    if path.is_file() and path.read_bytes() == data:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix="." + path.name, dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(data)
        os.chmod(temporary, 0o644)
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def _meta_map(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for match in META_RE.finditer(text):
        attrs = {k.lower(): (v1 if v1 != "" else v2) for k, v1, v2 in ATTR_RE.findall(match.group(1))}
        name = attrs.get("name", "").strip().lower()
        if name and "content" in attrs:
            out[name] = html.unescape(attrs["content"].strip())
    return out


def extract_title(text: str) -> str:
    match = TITLE_RE.search(text)
    if not match:
        return ""
    return html.unescape(re.sub(r"\s+", " ", match.group(1)).strip())


def expected_letter(title: str) -> str | None:
    match = re.search(r"[A-Za-z]", title)
    return match.group(0).lower() if match else None


def _is_letter_index(rel: str) -> bool:
    parts = rel.split("/")
    return len(parts) == 3 and parts[0] == "glossary" and len(parts[1]) == 1 and parts[1] in string.ascii_lowercase and parts[2] == "index.html"


def find_misplaced_glossary_pages(knowledge_dir: Path) -> list[tuple[str, str, str, str]]:
    """Return [(old_rel, expected_rel, letter, title)] for misplaced glossary term pages."""
    glossary_dir = knowledge_dir / "glossary"
    if not glossary_dir.is_dir():
        return []
    misplaced: list[tuple[str, str, str, str]] = []
    for path in sorted(knowledge_dir.rglob("*.html")):
        rel = path.relative_to(knowledge_dir).as_posix()
        if rel == "glossary/index.html" or _is_letter_index(rel):
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        meta = _meta_map(text)
        is_glossary = rel.startswith("glossary/") or meta.get("kind", "").lower() == "glossary"
        if not is_glossary:
            continue
        title = extract_title(text)
        letter = expected_letter(title)
        if not letter:
            raise GlossaryPlacementError(f"{rel}: glossary page title {title!r} has no ASCII letter")
        target = f"glossary/{letter}/{path.name}"
        if rel != target:
            misplaced.append((rel, target, letter, title))
    return misplaced


def _update_moved_page_metadata(path: Path, letter: str) -> None:
    text = path.read_text(encoding="utf-8", errors="replace")
    upper = letter.upper()
    parent_target = f"glossary/{letter}/index.html"
    updated = re.sub(
        r'(<meta\s+name=(["\'])parent\2\s+content=(["\']))[^"\']*(\3\s*/?>)',
        rf"\g<1>{parent_target}\4",
        text,
        count=1,
        flags=re.I,
    )

    def replace_crumb(match: re.Match[str]) -> str:
        nav = match.group(1)
        new_link = f'<a href="{parent_target}">Terms beginning with {upper}</a>'
        if LETTER_CRUMB_RE.search(nav):
            return LETTER_CRUMB_RE.sub(new_link, nav, count=1)
        return re.sub(
            r'(<a\s+href=(["\'])glossary/index\.html\2>[^<]*</a>\s*/\s*)',
            rf"\1{new_link} / ",
            nav,
            count=1,
            flags=re.I,
        )

    updated = BREADCRUMB_RE.sub(replace_crumb, updated, count=1)
    if updated != text:
        atomic_write(path, updated)


def _rewrite_inbound_links(knowledge_dir: Path, moves: list[tuple[str, str]]) -> None:
    mapping = dict(moves)
    for path in sorted(knowledge_dir.rglob("*.html")):
        text = path.read_text(encoding="utf-8", errors="replace")

        def replace_href(match: re.Match[str]) -> str:
            prefix, quote, href, _ = match.groups()
            base, sep, suffix = href.partition("#")
            base_path, qsep, qpart = base.partition("?")
            if base_path in mapping:
                return f"{prefix}{quote}{mapping[base_path]}{qsep}{qpart}{sep}{suffix}{quote}"
            return match.group(0)

        updated = HREF_RE.sub(replace_href, text)
        if updated != text:
            atomic_write(path, updated)


def _move_file(root: Path, src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    if (root / ".git").exists():
        tracked = subprocess.run(
            ["git", "-C", str(root), "ls-files", "--error-unmatch", str(src.relative_to(root))],
            capture_output=True,
        )
        if tracked.returncode == 0:
            res = subprocess.run(
                ["git", "-C", str(root), "mv", str(src.relative_to(root)), str(dst.relative_to(root))],
                capture_output=True,
                text=True,
            )
            if res.returncode == 0:
                return
    shutil.move(str(src), str(dst))


def check_glossary_placement(
    knowledge_dir: Path,
    fix: bool = False,
    interactive: bool = False,
    input_fn=input,
) -> list[tuple[str, str]]:
    """Check that glossary pages live in the folder matching their title's first letter."""
    misplaced = find_misplaced_glossary_pages(knowledge_dir)
    if not misplaced:
        return []

    for old_rel, new_rel, _, title in misplaced:
        print(f"glossary: {old_rel} (title {title!r}) belongs in {new_rel}")

    should_fix = fix
    if not should_fix and interactive:
        try:
            answer = input_fn(f"Move {len(misplaced)} misplaced glossary page(s) to the right folder? [y/N] ")
        except (EOFError, KeyboardInterrupt):
            answer = ""
        should_fix = answer.strip().lower() in {"y", "yes"}

    if not should_fix:
        details = "; ".join(f"{old} -> {new}" for old, new, _, _ in misplaced)
        raise GlossaryPlacementError(
            f"misplaced glossary page(s): {details}. "
            "Run 'python3 tools/corpus_pipeline.py check --fix-glossary' to file them in the right folder."
        )

    seen_targets: set[str] = set()
    for _, new_rel, _, _ in misplaced:
        if new_rel in seen_targets or (knowledge_dir / new_rel).exists():
            raise GlossaryPlacementError(f"cannot move to {new_rel}: destination already exists")
        seen_targets.add(new_rel)

    root = knowledge_dir.parent
    moves: list[tuple[str, str]] = []
    for old_rel, new_rel, letter, _ in misplaced:
        src = knowledge_dir / old_rel
        dst = knowledge_dir / new_rel
        _move_file(root, src, dst)
        _update_moved_page_metadata(dst, letter)
        moves.append((old_rel, new_rel))

    _rewrite_inbound_links(knowledge_dir, moves)
    return moves


def _render_letter_index(letter: str, items: list[tuple[str, str, str]]) -> str:
    upper = letter.upper()
    lines = [
        "<!DOCTYPE html>",
        "<html><head>",
        f"<title>Terms beginning with {upper}</title>",
        f'<meta name="topic" content="hub-glossary-{letter}">',
        f'<meta name="summary" content="Glossary definitions for technical terms and acronyms beginning with {upper}.">',
        f'<meta name="keywords" content="terms beginning with {letter}">',
        '<meta name="level" content="novice">',
        '<meta name="kind" content="hub">',
        '<meta name="track" content="glossary">',
        '<meta name="parent" content="glossary/index.html">',
        "</head><body>",
        f'<nav aria-label="Breadcrumb"><p><a href="index.html">Guide</a> / <a href="glossary/index.html">Glossary A\u2013Z</a> / Terms beginning with {upper}</p></nav>',
        f"<h1>Terms beginning with {upper}</h1>",
        f"<p>Glossary definitions for technical terms and acronyms beginning with {upper}.</p>",
        "<h2>Explore</h2><table>\n"
        + "\n".join(
            f'<tr><td><a href="glossary/{letter}/{filename}">{html.escape(title, quote=True)}</a></td><td>{html.escape(summary, quote=True)}</td></tr>'
            for title, filename, summary in items
        ),
        "</table>",
        "</body></html>",
    ]
    return "\n".join(lines) + "\n"


def _render_top_index(ranges: list[tuple[str, str, tuple[str, str], int]]) -> str:
    list_items = []
    for upper, href, first_last, count in ranges:
        first, last = first_last
        label = f"{upper} : {html.escape(first, quote=True)}" if count == 1 else f"{upper} : {html.escape(first, quote=True)} - {html.escape(last, quote=True)}"
        list_items.append(f'<li><a href="{href}">{label}</a></li>')
    lines = [
        "<!DOCTYPE html>",
        "<html><head>",
        "<title>Glossary A\u2013Z</title>",
        '<meta name="topic" content="glossary">',
        '<meta name="summary" content="Look up a word or acronym, then follow its links to learn more.">',
        '<meta name="keywords" content="glossary a\u2013z, glossary, learning">',
        '<meta name="level" content="novice">',
        '<meta name="kind" content="hub">',
        '<meta name="track" content="glossary">',
        '<meta name="parent" content="index.html">',
        "</head><body>",
        '<nav aria-label="Breadcrumb"><p><a href="index.html">Guide</a> / Glossary A\u2013Z</p></nav>',
        "<h1>Glossary A\u2013Z</h1>",
        "<p>Look up a word or acronym, then follow its links to learn more.</p>",
        "<h2>Explore</h2><ul>" + "\n".join(list_items),
        "</ul>",
        "</body></html>",
    ]
    return "\n".join(lines) + "\n"


def generate_glossary_indexes(knowledge_dir: Path, write_fn=atomic_write) -> None:
    """Regenerate knowledge/glossary/<letter>/index.html and knowledge/glossary/index.html."""
    glossary_dir = knowledge_dir / "glossary"
    if not glossary_dir.is_dir():
        return

    ranges: list[tuple[str, str, tuple[str, str], int]] = []
    for letter in string.ascii_lowercase:
        letter_dir = glossary_dir / letter
        if not letter_dir.is_dir():
            continue
        items: list[tuple[str, str, str]] = []
        for path in sorted(letter_dir.glob("*.html")):
            if path.name == "index.html":
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            title = extract_title(text) or path.stem
            summary = _meta_map(text).get("summary", "")
            items.append((title, path.name, summary))
        items.sort(key=lambda item: (item[0].casefold(), item[1]))
        if not items:
            continue
        write_fn(letter_dir / "index.html", _render_letter_index(letter, items))
        ranges.append((letter.upper(), f"glossary/{letter}/index.html", (items[0][0], items[-1][0]), len(items)))

    write_fn(glossary_dir / "index.html", _render_top_index(ranges))
