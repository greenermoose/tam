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

NUMERIC_SECTION = "numeric"
NUMERIC_TOP_LABEL = "0\u20139"
NUMERIC_SECTION_LABEL = "Terms beginning with a number"

TITLE_RE = re.compile(r"<title>(.*?)</title>", re.I | re.S)
META_RE = re.compile(r"<meta\b([^>]*)>", re.I)
ATTR_RE = re.compile(r'([A-Za-z_:][-A-Za-z0-9_:.]*)\s*=\s*(?:"([^"]*)"|\'([^\']*)\')')
HREF_RE = re.compile(r'(href\s*=\s*)(["\'])([^"\']+)(\2)')
BREADCRUMB_RE = re.compile(r'(<nav\b[^>]*aria-label=["\']Breadcrumb["\'][^>]*>.*?</nav>)', re.I | re.S)
SECTION_CRUMB_RE = re.compile(
    r'<a\s+href=(["\'])glossary/[A-Za-z0-9-]+/index\.html\1>[^<]*</a>', re.I
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


def expected_section(title: str) -> str | None:
    """Return the glossary section for a title: numeric first, then A-Z."""
    if re.match(r"\s*\d", title):
        return NUMERIC_SECTION
    return expected_letter(title)


def _section_top_label(section: str) -> str:
    return NUMERIC_TOP_LABEL if section == NUMERIC_SECTION else section.upper()


def _section_label(section: str) -> str:
    if section == NUMERIC_SECTION:
        return NUMERIC_SECTION_LABEL
    return f"Terms beginning with {section.upper()}"


def _section_summary(section: str) -> str:
    if section == NUMERIC_SECTION:
        return "Glossary definitions for technical terms and acronyms beginning with a number."
    return f"Glossary definitions for technical terms and acronyms beginning with {section.upper()}."


def _section_keywords(section: str) -> str:
    if section == NUMERIC_SECTION:
        return "terms beginning with a number"
    return f"terms beginning with {section}"


def _item_sort_key(item: tuple[str, str, str]) -> tuple[int, str, str]:
    title, filename = item[0], item[1]
    match = re.match(r"\s*(\d+)", title)
    number = int(match.group(1)) if match else 0
    return number, title.casefold(), filename


def _is_section_index(rel: str) -> bool:
    parts = rel.split("/")
    if len(parts) != 3 or parts[0] != "glossary" or parts[2] != "index.html":
        return False
    return parts[1] == NUMERIC_SECTION or (
        len(parts[1]) == 1 and parts[1] in string.ascii_lowercase
    )


def find_misplaced_glossary_pages(knowledge_dir: Path) -> list[tuple[str, str, str, str]]:
    """Return [(old_rel, expected_rel, section, title)] for misplaced glossary pages."""
    glossary_dir = knowledge_dir / "glossary"
    if not glossary_dir.is_dir():
        return []
    misplaced: list[tuple[str, str, str, str]] = []
    for path in sorted(knowledge_dir.rglob("*.html")):
        rel = path.relative_to(knowledge_dir).as_posix()
        if rel == "glossary/index.html" or _is_section_index(rel):
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        meta = _meta_map(text)
        is_glossary = rel.startswith("glossary/") or meta.get("kind", "").lower() == "glossary"
        if not is_glossary:
            continue
        title = extract_title(text)
        section = expected_section(title)
        if not section:
            raise GlossaryPlacementError(f"{rel}: glossary page title {title!r} has no section")
        target = f"glossary/{section}/{path.name}"
        if rel != target:
            misplaced.append((rel, target, section, title))
    return misplaced


def _update_moved_page_metadata(path: Path, section: str) -> None:
    text = path.read_text(encoding="utf-8", errors="replace")
    label = _section_label(section)
    parent_target = f"glossary/{section}/index.html"
    updated = re.sub(
        r'(<meta\s+name=(["\'])parent\2\s+content=(["\']))[^"\']*(\3\s*/?>)',
        rf"\g<1>{parent_target}\4",
        text,
        count=1,
        flags=re.I,
    )

    def replace_crumb(match: re.Match[str]) -> str:
        nav = match.group(1)
        new_link = f'<a href="{parent_target}">{label}</a>'
        if SECTION_CRUMB_RE.search(nav):
            return SECTION_CRUMB_RE.sub(new_link, nav, count=1)
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
    """Check that glossary pages live in the folder matching their title's section."""
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
    for old_rel, new_rel, section, _ in misplaced:
        src = knowledge_dir / old_rel
        dst = knowledge_dir / new_rel
        _move_file(root, src, dst)
        _update_moved_page_metadata(dst, section)
        moves.append((old_rel, new_rel))

    _rewrite_inbound_links(knowledge_dir, moves)
    return moves


def _render_section_index(section: str, items: list[tuple[str, str, str]]) -> str:
    label = _section_label(section)
    summary = _section_summary(section)
    lines = [
        "<!DOCTYPE html>",
        "<html><head>",
        f"<title>{label}</title>",
        f'<meta name="topic" content="hub-glossary-{section}">',
        f'<meta name="summary" content="{summary}">',
        f'<meta name="keywords" content="{_section_keywords(section)}">',
        '<meta name="level" content="novice">',
        '<meta name="kind" content="hub">',
        '<meta name="track" content="glossary">',
        '<meta name="parent" content="glossary/index.html">',
        "</head><body>",
        f'<nav aria-label="Breadcrumb"><p><a href="index.html">Guide</a> / <a href="glossary/index.html">Glossary A\u2013Z</a> / {label}</p></nav>',
        f"<p>{summary}</p>",
        "<h2>Explore</h2><table>\n"
        + "\n".join(
            f'<tr><td><a href="glossary/{section}/{filename}">{html.escape(title, quote=True)}</a></td><td>{html.escape(summary, quote=True)}</td></tr>'
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
        "<p>Look up a word or acronym, then follow its links to learn more.</p>",
        "<h2>Explore</h2><ul>" + "\n".join(list_items),
        "</ul>",
        "</body></html>",
    ]
    return "\n".join(lines) + "\n"


def generate_glossary_indexes(knowledge_dir: Path, write_fn=atomic_write) -> None:
    """Regenerate glossary section hubs and knowledge/glossary/index.html."""
    glossary_dir = knowledge_dir / "glossary"
    if not glossary_dir.is_dir():
        return

    ranges: list[tuple[str, str, tuple[str, str], int]] = []
    sections = [NUMERIC_SECTION, *string.ascii_lowercase]
    for section in sections:
        section_dir = glossary_dir / section
        if not section_dir.is_dir():
            continue
        items: list[tuple[str, str, str]] = []
        for path in sorted(section_dir.glob("*.html")):
            if path.name == "index.html":
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            title = extract_title(text) or path.stem
            summary = _meta_map(text).get("summary", "")
            items.append((title, path.name, summary))
        items.sort(key=_item_sort_key)
        if not items:
            continue
        write_fn(section_dir / "index.html", _render_section_index(section, items))
        ranges.append((_section_top_label(section), f"glossary/{section}/index.html", (items[0][0], items[-1][0]), len(items)))

    write_fn(glossary_dir / "index.html", _render_top_index(ranges))
