#!/usr/bin/env python3
"""build-index.py: Extract metadata from knowledge HTML files and generate
knowledge/index.tsv and knowledge/index.json.

Each page declares its metadata in <meta> tags: topic, summary, keywords,
level, track and prereqs. Level lists the depths a page offers (beginner,
intermediate, expert); a page offering more than one wraps each depth in
<section data-level="..."> (the content-depth ladder). The build fails on
unknown levels or tracks, on prerequisites that name no topic, and on
laddered pages whose sections do not match their declared levels.

Every page except the guide index carries a breadcrumb nav before its <h1>,
so a reader always knows where a page sits in the guide. Its first crumb is
the guide index, a section page adds its section index, and the final crumb is
the page itself, as plain text. The build fails on a missing, misplaced or
inaccurate breadcrumb, or on the guide index itself carrying one (it is the
root).
"""

import json
import re
import sys
from pathlib import Path

LEVELS = ("beginner", "intermediate", "expert")
TRACKS = ("use", "maintain", "improve")


def extract_attr(tag, attr):
    m = re.search(r'(?<![\w-])' + re.escape(attr) + r'\s*=\s*["\']([^"\']*)["\']', tag, re.I)
    return m.group(1).strip() if m else None


def split_list(content):
    return [k.strip() for k in content.split(",") if k.strip()]


def parse_page(path, rel_path, errors):
    text = path.read_text(encoding="utf-8", errors="replace")

    # Title
    m_title = re.search(r'<title>(.*?)</title>', text, re.I | re.S)
    title = m_title.group(1).strip() if m_title else ""

    # H1
    m_h1 = re.search(r'<h1>(.*?)</h1>', text, re.I | re.S)
    h1 = m_h1.group(1).strip() if m_h1 else ""
    if not title:
        title = h1

    # Meta tags
    topic = None
    summary = None
    keywords = []
    levels = ["beginner"]
    track = "use"
    prereqs = []

    for tag in re.findall(r'<meta\b(?:[^>"\'/]|"[^"]*"|\'[^\']*\')*>', text, re.I):
        name = extract_attr(tag, "name")
        content = extract_attr(tag, "content")
        if not name or content is None:
            continue
        name_lower = name.lower()
        if name_lower == "topic":
            topic = content.strip()
        elif name_lower == "summary":
            summary = content.strip()
        elif name_lower == "keywords":
            keywords.extend(split_list(content))
        elif name_lower == "level":
            declared = [v.lower() for v in split_list(content)]
            unknown = [v for v in declared if v not in LEVELS]
            if unknown or not declared:
                errors.append(f"{rel_path}: unknown level {', '.join(unknown) or '(empty)'}"
                              f" (expected {', '.join(LEVELS)})")
            levels = [v for v in LEVELS if v in declared] or ["beginner"]
        elif name_lower == "track":
            track = content.strip().lower()
            if track not in TRACKS:
                errors.append(f"{rel_path}: unknown track {track!r}"
                              f" (expected {', '.join(TRACKS)})")
        elif name_lower == "prereqs":
            prereqs.extend(split_list(content))

    # Laddered pages carry one section per declared level, and only those.
    sections = []
    for tag in re.findall(r'<section\b(?:[^>"\']|"[^"]*"|\'[^\']*\')*>', text, re.I):
        value = extract_attr(tag, "data-level")
        if value is not None:
            sections.append(value.lower())
    for value in sections:
        if value not in LEVELS:
            errors.append(f"{rel_path}: section has unknown data-level {value!r}")
        elif value not in levels:
            errors.append(f"{rel_path}: section level {value!r} is not declared in <meta name=\"level\">")
    if len(levels) > 1:
        for value in levels:
            if sections.count(value) != 1:
                errors.append(f"{rel_path}: declares level {value!r} but has "
                              f"{sections.count(value)} matching sections (expected 1)")

    if not topic:
        # Fallback topic
        base = rel_path.replace(".html", "")
        if base.endswith("/index"):
            topic = base[:-6]
        elif "/" in base:
            topic = base.split("/")[-1]
        else:
            topic = base

    if not summary:
        summary = title

    return {
        "topic": topic,
        "path": rel_path,
        "title": title,
        "summary": summary,
        "keywords": keywords,
        "levels": levels,
        "track": track,
        "prereqs": prereqs,
    }


BREADCRUMB_RE = re.compile(
    r'<nav[^>]*\baria-label\s*=\s*["\']Breadcrumb["\'][^>]*>(.*?)</nav>',
    re.I | re.S)
LINK_RE = re.compile(r'<a\b[^>]*href\s*=\s*["\']([^"\']+)["\'][^>]*>(.*?)</a>',
                     re.I | re.S)


def page_h1(text):
    m = re.search(r'<h1>(.*?)</h1>', text, re.I | re.S)
    return re.sub(r'<[^>]+>', '', m.group(1)).strip() if m else ""


def breadcrumb_parts(text):
    """Return (ancestors, current) from the breadcrumb nav, or None.

    ancestors is the list of (href, label) pairs; current is the trailing
    plain-text crumb naming the page itself."""
    m = BREADCRUMB_RE.search(text)
    if not m:
        return None
    body = m.group(1)
    ancestors = [(href, re.sub(r'<[^>]+>', '', label).strip())
                 for href, label in LINK_RE.findall(body)]
    pos = body.rfind('</a>')
    tail = body[pos + 4:] if pos != -1 else body
    current = re.sub(r'<[^>]+>', '', tail).strip().lstrip('/').strip()
    return ancestors, current


def check_breadcrumbs(knowledge_dir, errors):
    pages = {}
    for html_file in sorted(knowledge_dir.rglob("*.html")):
        pages[html_file.relative_to(knowledge_dir).as_posix()] = \
            html_file.read_text(encoding="utf-8")

    h1 = {rel: page_h1(text) for rel, text in pages.items()}

    for rel, text in pages.items():
        parts = breadcrumb_parts(text)
        if rel == "index.html":
            if parts is not None:
                errors.append(f"{rel}: the guide index is the root and must not carry a breadcrumb")
            continue
        if parts is None:
            errors.append(f"{rel}: missing breadcrumb")
            continue
        want = [("index.html", "Guide")]
        if "/" in rel:
            section = rel.split("/")[0]
            if rel != f"{section}/index.html" and f"{section}/index.html" in pages:
                want.append((f"{section}/index.html", h1[f"{section}/index.html"]))
        if parts[0] != want:
            errors.append(
                f"{rel}: breadcrumb ancestors {parts[0]} do not match "
                f"{want}")
        if parts[1] != h1[rel]:
            errors.append(
                f"{rel}: breadcrumb names {parts[1]!r} but the page's <h1> "
                f"is {h1[rel]!r}")


def main():
    root = Path(__file__).resolve().parent.parent
    knowledge_dir = root / "knowledge"

    if not knowledge_dir.is_dir():
        print(f"Error: knowledge directory not found at {knowledge_dir}", file=sys.stderr)
        return 1

    errors = []
    entries = []
    for html_file in sorted(knowledge_dir.rglob("*.html")):
        rel = html_file.relative_to(knowledge_dir).as_posix()
        entry = parse_page(html_file, rel, errors)
        entries.append(entry)

    topics = {e["topic"] for e in entries}
    for e in entries:
        for p in e["prereqs"]:
            if p not in topics:
                errors.append(f"{e['path']}: prerequisite {p!r} is not a topic")

    version_file = knowledge_dir / "version"
    if not version_file.is_file():
        errors.append("knowledge/version: missing version file")
    else:
        v_str = version_file.read_text(encoding="utf-8").strip()
        if not re.match(r'^\d+\.\d+\.\d+', v_str):
            errors.append(f"knowledge/version: invalid version {v_str!r} (expected X.Y.Z)")

    check_breadcrumbs(knowledge_dir, errors)

    if errors:
        for message in errors:
            print(f"Error: {message}", file=sys.stderr)
        return 1

    # Sort entries by topic
    entries.sort(key=lambda e: e["topic"])

    # Write index.tsv
    tsv_path = knowledge_dir / "index.tsv"
    with open(tsv_path, "w", encoding="utf-8") as f:
        f.write("topic\tpath\ttitle\tsummary\tkeywords\tlevel\ttrack\tprereqs\n")
        for e in entries:
            kws = ", ".join(e["keywords"])
            levels = ",".join(e["levels"])
            prereqs = ", ".join(e["prereqs"])
            f.write(f"{e['topic']}\t{e['path']}\t{e['title']}\t{e['summary']}\t{kws}\t"
                    f"{levels}\t{e['track']}\t{prereqs}\n")

    # Write index.json
    json_path = knowledge_dir / "index.json"
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(entries, f, indent=2)
        f.write("\n")

    # Generate knowledge/.build_id if in a git repository
    git_dir = root / ".git"
    if git_dir.exists():
        try:
            import subprocess
            res = subprocess.run(
                ["git", "-C", str(root), "log", "-n", "1", "--format=%h", "--", "knowledge", "tools"],
                capture_output=True, text=True, check=True
            )
            cid = res.stdout.strip()
            if cid:
                status_res = subprocess.run(
                    ["git", "-C", str(root), "status", "--porcelain", "--", "knowledge", "tools"],
                    capture_output=True, text=True
                )
                if status_res.stdout.strip():
                    cid += "-dirty"
                (knowledge_dir / ".build_id").write_text(cid + "\n", encoding="utf-8")
        except Exception:
            pass

    print(f"Generated {tsv_path} and {json_path} with {len(entries)} entries.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
