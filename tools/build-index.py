#!/usr/bin/env python3
"""build-index.py: Extract metadata from knowledge HTML files and generate
knowledge/index.tsv, knowledge/index.json and knowledge/index.parents.tsv.

Each page declares its metadata in <meta> tags: topic, summary, keywords,
level, track, optional parent and prereqs. Level lists the depths a page offers (beginner,
intermediate, expert); a page offering more than one wraps each depth in
<section data-level="..."> (the content-depth ladder). The build fails on
unknown levels or tracks, on prerequisites that name no topic, and on
laddered pages whose sections do not match their declared levels.

Every page except the guide index carries a breadcrumb nav before its <h1>.
Parent metadata defines the canonical hierarchy; older corpora without it use
the historical directory-based fallback. Breadcrumb ancestors must match the
hierarchy, which must reach the guide root without cycles. Corpus 0.31.0
requires explicit parents and track ownership.
"""

from datetime import datetime, timezone
import json
import re
import subprocess
import sys
import argparse
from pathlib import Path

LEVELS = ("beginner", "intermediate", "expert")
TRACKS = ("install", "learn", "use", "maintain", "improve")


def extract_attr(tag, attr):
    m = re.search(r'(?<![\w-])' + re.escape(attr) + r'\s*=\s*["\']([^"\']*)["\']', tag, re.I)
    return m.group(1).strip() if m else None


def split_list(content):
    return [k.strip() for k in content.split(",") if k.strip()]


def write_if_changed(path, content):
    """Rechecking metadata must not touch an otherwise unchanged output."""
    data = content.encode("utf-8")
    if not path.exists() or path.read_bytes() != data:
        path.write_bytes(data)


def git_page_revision(root, path, rel_path):
    """Derive deterministic (rev, updated) for a knowledge page from Git history.

    rev: integer commit count touching knowledge/<rel_path> (git rev-list --count HEAD -- ...),
         at least 1.
    updated: committer timestamp of the latest commit touching knowledge/<rel_path>
             in its recorded timezone formatted as 'YYYY-MM-DD HH:MM'
             (git log -1 --format=%cd --date=format:%Y-%m-%d %H:%M -- ...).
             Falls back to the file's UTC mtime if the file is not yet committed
             or if Git is unavailable.
    """
    git_rel = f"knowledge/{rel_path}"
    rev = 0
    updated = ""
    if (root / ".git").exists():
        try:
            cnt_res = subprocess.run(
                ["git", "-C", str(root), "rev-list", "--count", "HEAD", "--", git_rel],
                capture_output=True, text=True, check=False
            )
            if cnt_res.returncode == 0 and cnt_res.stdout.strip().isdigit():
                rev = int(cnt_res.stdout.strip())
            if rev > 0:
                date_res = subprocess.run(
                    ["git", "-C", str(root), "log", "-1", "--format=%cd",
                     "--date=format:%Y-%m-%d %H:%M", "--", git_rel],
                    capture_output=True, text=True, check=False
                )
                if date_res.returncode == 0:
                    updated = date_res.stdout.strip()
        except Exception:
            pass
    if rev <= 0:
        rev = 1
    if not updated:
        try:
            mtime = path.stat().st_mtime
            updated = datetime.fromtimestamp(mtime, tz=timezone.utc).strftime("%Y-%m-%d %H:%M")
        except Exception:
            updated = "unknown"
    return rev, updated


def parse_page(root, path, rel_path, errors):
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
    parent = None
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
            if track not in TRACKS and not (rel_path == "index.html" and track == ""):
                errors.append(f"{rel_path}: unknown track {track!r}"
                              f" (expected {', '.join(TRACKS)})")
        elif name_lower == "parent":
            parent = content.strip()
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

    rev, updated = git_page_revision(root, path, rel_path)

    return {
        "topic": topic,
        "path": rel_path,
        "title": title,
        "summary": summary,
        "keywords": keywords,
        "levels": levels,
        "track": track,
        "parent": parent,
        "prereqs": prereqs,
        "rev": rev,
        "updated": updated,
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


def physical_parent(rel, pages):
    """Legacy fallback matching the pre-parent directory hierarchy."""
    if rel == "index.html":
        return None
    if rel.endswith("/index.html"):
        return "index.html"
    section = rel.split("/", 1)[0]
    section_index = f"{section}/index.html"
    return section_index if section_index in pages else "index.html"


def check_hierarchy(knowledge_dir, entries, version, errors):
    pages = {}
    for html_file in sorted(knowledge_dir.rglob("*.html")):
        pages[html_file.relative_to(knowledge_dir).as_posix()] = \
            html_file.read_text(encoding="utf-8")

    h1 = {rel: page_h1(text) for rel, text in pages.items()}
    entries_by_path = {entry["path"]: entry for entry in entries}
    require_explicit_parent = tuple(int(part) for part in version.split(".")[:2]) >= (0, 31)
    parents = {}
    for rel, entry in entries_by_path.items():
        if rel == "index.html":
            if entry["parent"]:
                errors.append(f"{rel}: the guide root must not declare a parent")
            parents[rel] = None
            continue
        if require_explicit_parent and not entry["parent"]:
            errors.append(f"{rel}: corpus {version} requires explicit parent metadata")
        parent = entry["parent"] or physical_parent(rel, pages)
        if parent not in pages:
            errors.append(f"{rel}: parent {parent!r} does not name an existing page")
        parents[rel] = parent
        entry["parent"] = parent

    # Every parent chain must terminate at the guide root, without cycles.
    for rel in parents:
        seen = set()
        current = rel
        while current is not None and current != "index.html":
            if current in seen:
                errors.append(f"{rel}: parent hierarchy contains a cycle at {current!r}")
                break
            seen.add(current)
            current = parents.get(current)
            if current is None and rel != "index.html":
                errors.append(f"{rel}: parent hierarchy does not reach index.html")
                break

    def part_ancestor(rel):
        current = rel
        seen = set()
        while current is not None and current != "index.html":
            if current in seen:
                return ""
            seen.add(current)
            parent = parents.get(current)
            if parent == "index.html":
                return entries_by_path[current]["track"]
            current = parent
        return ""

    if require_explicit_parent and entries_by_path.get("index.html", {}).get("track"):
        errors.append("index.html: the shared guide root must have an empty track")
    for rel, entry in entries_by_path.items():
        if rel == "index.html":
            continue
        home_track = part_ancestor(rel)
        if require_explicit_parent and home_track and entry["track"] != home_track:
            errors.append(f"{rel}: track {entry['track']!r} does not match parent part {home_track!r}")

    for rel, text in pages.items():
        parts = breadcrumb_parts(text)
        if rel == "index.html":
            if parts is not None:
                errors.append(f"{rel}: the guide index is the root and must not carry a breadcrumb")
            continue
        if parts is None:
            errors.append(f"{rel}: missing breadcrumb")
            continue
        chain = []
        current = parents.get(rel)
        seen = set()
        while current is not None and current not in seen:
            seen.add(current)
            chain.append(current)
            current = parents.get(current)
        chain.reverse()
        want = [("index.html", "Guide")]
        for ancestor in chain:
            if ancestor == "index.html":
                continue
            if ancestor in h1:
                want.append((ancestor, h1[ancestor]))
        if parts[0] != want:
            errors.append(
                f"{rel}: breadcrumb ancestors {parts[0]} do not match "
                f"{want}")
        if parts[1] != h1[rel]:
            errors.append(
                f"{rel}: breadcrumb names {parts[1]!r} but the page's <h1> "
                f"is {h1[rel]!r}")


def main(root=None):
    root = Path(root).resolve() if root else Path(__file__).resolve().parent.parent
    knowledge_dir = root / "knowledge"

    if not knowledge_dir.is_dir():
        print(f"Error: knowledge directory not found at {knowledge_dir}", file=sys.stderr)
        return 1

    errors = []
    entries = []
    for html_file in sorted(knowledge_dir.rglob("*.html")):
        rel = html_file.relative_to(knowledge_dir).as_posix()
        entry = parse_page(root, html_file, rel, errors)
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

    check_hierarchy(knowledge_dir, entries, version_file.read_text(encoding="utf-8").strip()
                    if version_file.is_file() else "0.0.0", errors)

    if errors:
        for message in errors:
            print(f"Error: {message}", file=sys.stderr)
        return 1

    # Sort entries by topic
    entries.sort(key=lambda e: e["topic"])

    # Keep the established TSV projection byte-compatible for older readers.
    # Parent is additive in JSON and in a separate compact TSV projection.
    rows = ["topic\tpath\ttitle\tsummary\tkeywords\tlevel\ttrack\tprereqs\trev\tupdated\n"]
    parents = ["path\tparent\n"]
    for e in entries:
        kws = ", ".join(e["keywords"])
        levels = ",".join(e["levels"])
        prereqs = ", ".join(e["prereqs"])
        rows.append(f"{e['topic']}\t{e['path']}\t{e['title']}\t{e['summary']}\t{kws}\t"
                    f"{levels}\t{e['track']}\t{prereqs}\t{e['rev']}\t{e['updated']}\n")
        parents.append(f"{e['path']}\t{e['parent'] or ''}\n")

    outputs = {
        "index.tsv": "".join(rows),
        "index.parents.tsv": "".join(parents),
        "index.json": json.dumps(entries, indent=2) + "\n",
    }
    for name, content in outputs.items():
        write_if_changed(knowledge_dir / name, content)

    # Generate knowledge/.build_id if in a git repository
    git_dir = root / ".git"
    if git_dir.exists():
        try:
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
                write_if_changed(knowledge_dir / ".build_id", cid + "\n")
        except Exception:
            pass

    print(f"Generated index.tsv, index.json and index.parents.tsv with {len(entries)} entries.")
    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", help="Tam command checkout (used for isolated fixtures)")
    args = parser.parse_args()
    sys.exit(main(args.root))
