#!/usr/bin/env python3
"""build-index.py: Extract metadata from knowledge HTML files and generate
knowledge/index.tsv and knowledge/index.json.
"""

import json
import os
import re
import sys
from pathlib import Path


def extract_attr(tag, attr):
    m = re.search(r'\b' + re.escape(attr) + r'\s*=\s*["\']([^"\']*)["\']', tag, re.I)
    return m.group(1).strip() if m else None


def parse_page(path, rel_path):
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
    level = "beginner"
    track = "use"

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
            # Split comma-separated keywords
            kws = [k.strip() for k in content.split(",") if k.strip()]
            keywords.extend(kws)
        elif name_lower == "level":
            level = content.strip().lower()
        elif name_lower == "track":
            track = content.strip().lower()

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
        "level": level,
        "track": track,
    }


def main():
    root = Path(__file__).resolve().parent.parent
    knowledge_dir = root / "knowledge"

    if not knowledge_dir.is_dir():
        print(f"Error: knowledge directory not found at {knowledge_dir}", file=sys.stderr)
        return 1

    entries = []
    for html_file in sorted(knowledge_dir.rglob("*.html")):
        rel = html_file.relative_to(knowledge_dir).as_posix()
        entry = parse_page(html_file, rel)
        entries.append(entry)

    # Sort entries by topic
    entries.sort(key=lambda e: e["topic"])

    # Write index.tsv
    tsv_path = knowledge_dir / "index.tsv"
    with open(tsv_path, "w", encoding="utf-8") as f:
        f.write("topic\tpath\ttitle\tsummary\tkeywords\tlevel\ttrack\n")
        for e in entries:
            kws = ", ".join(e["keywords"])
            f.write(f"{e['topic']}\t{e['path']}\t{e['title']}\t{e['summary']}\t{kws}\t{e['level']}\t{e['track']}\n")

    # Write index.json
    json_path = knowledge_dir / "index.json"
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(entries, f, indent=2)
        f.write("\n")

    print(f"Generated {tsv_path} and {json_path} with {len(entries)} entries.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
