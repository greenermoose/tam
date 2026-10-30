#!/usr/bin/env python3
"""build-index.py: Extract metadata from knowledge HTML files and generate
knowledge/index.tsv, knowledge/index.json and knowledge/index.parents.tsv.

Each page declares its metadata in <meta> tags: topic, summary, keywords,
level, track, optional parent and prereqs. Level lists the depths a page offers (beginner,
intermediate, expert); a page offering more than one wraps each depth in
<section data-level="..."> (the content-depth ladder). The build fails on
unknown levels or tracks, on topics that are not slugs, on prerequisites that
name no topic, and on laddered pages whose sections do not match their declared levels.

Every page except the guide index carries a breadcrumb nav before its <h1>.
Parent metadata defines the canonical hierarchy; older corpora without it use
the historical directory-based fallback. Breadcrumb ancestors must match the
hierarchy, which must reach the guide root without cycles. Corpus 0.31.0
requires explicit parents and track ownership.
"""

from datetime import datetime, timezone
import json
from html.parser import HTMLParser
import re
import subprocess
import sys
import argparse
from pathlib import Path

LEVELS = ("beginner", "intermediate", "expert")
TRACKS = ("install", "learn", "use", "maintain", "improve")
# A topic is one command-line word and one line of `tam explain topics --terse`.
TOPIC_RE = re.compile(r"^[a-z0-9][a-z0-9-]*$")


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
    page_kind = ""
    skills = []

    for tag in re.findall(r'<meta\b(?:[^>"\'/]|"[^"]*"|\'[^\']*\')*>', text, re.I):
        name = extract_attr(tag, "name")
        content = extract_attr(tag, "content")
        if not name or content is None:
            continue
        name_lower = name.lower()
        if name_lower == "topic":
            topic = content.strip()
            if not TOPIC_RE.match(topic):
                errors.append(f"{rel_path}: topic {topic!r} is not a slug"
                              f" (lowercase letters, digits and hyphens)")
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
        elif name_lower == "kind":
            page_kind = content.strip()
        elif name_lower == "skills":
            skills.extend(split_list(content))

    # Laddered sections define the page's levels; smaller tagged blocks may
    # refine any of those levels without adding another ladder section.
    sections = []
    layered = False
    for match in re.finditer(r'<(section|aside|p|li|pre|details|div|table)\b(?:[^>"\']|"[^"]*"|\'[^\']*\')*>', text, re.I):
        kind = match.group(1).lower()
        value = extract_attr(match.group(0), "data-level")
        if value is None:
            continue
        value = value.lower()
        classes = (extract_attr(match.group(0), "class") or "").split()
        if kind == "section" and "quiz" not in classes:
            sections.append(value)
        else:
            layered = True
        if value not in LEVELS:
            errors.append(f"{rel_path}: {kind} has unknown data-level {value!r}")
        elif value not in levels:
            errors.append(f"{rel_path}: {kind} level {value!r} is not declared in <meta name=\"level\">")
    # Shared-body pages may express their levels with smaller blocks or
    # quiz sections. A ladder still requires exactly one section per level.
    if len(levels) > 1 and (sections or not layered):
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
        "kind": page_kind,
        "skills": skills,
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
        want_hrefs = ["index.html"] + [a for a in chain if a != "index.html"]
        got_hrefs = [href for href, _ in parts[0]]
        if got_hrefs != want_hrefs:
            errors.append(
                f"{rel}: breadcrumb ancestor links {got_hrefs} do not match "
                f"parent hierarchy {want_hrefs}")
        if not parts[1]:
            errors.append(f"{rel}: breadcrumb current page label is empty")


KINDS = {"tutorial", "how-to", "reference", "explanation", "path", "hub",
         "glossary", "bibliography"}


class LearningHTML(HTMLParser):
    """Read cross-page teaching references and level sections structurally."""
    def __init__(self, text):
        super().__init__(convert_charrefs=True)
        self.stack = []
        self.refs = []
        self.reference_ids = []
        self.sections = []
        self.ids = []
        self.feed(text)

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if attrs.get("id"):
            self.ids.append(attrs["id"])
        if "reference" in attrs.get("class", "").split():
            self.reference_ids.append(attrs.get("id", ""))
        if tag in {"meta", "br", "hr", "img", "link", "input", "wbr"}:
            return
        node = {"tag": tag, "attrs": attrs, "text": [], "links": [],
                "quiz": False, "first-time": False, "tamlinux": False}
        for parent in self.stack:
            classes = attrs.get("class", "").split()
            parent["quiz"] |= tag == "details" and "q" in classes
            parent["first-time"] |= tag == "aside" and "first-time" in classes
            parent["tamlinux"] |= tag == "aside" and "tamlinux" in classes
            if tag == "a" and attrs.get("href"):
                parent["links"].append(attrs["href"])
        self.stack.append(node)

    def handle_startendtag(self, tag, attrs):
        self.handle_starttag(tag, attrs)
        self.handle_endtag(tag)

    def handle_data(self, text):
        for node in self.stack:
            node["text"].append(text)

    def handle_endtag(self, tag):
        for i in range(len(self.stack) - 1, -1, -1):
            if self.stack[i]["tag"] != tag:
                continue
            nodes, self.stack = self.stack[i:], self.stack[:i]
            for node in nodes:
                if node["tag"] in {"dfn", "cite"}:
                    self.refs.append(node)
                if node["tag"] == "section" and node["attrs"].get("data-level"):
                    self.sections.append(node)
            break


def learning_metadata(knowledge, entries, errors):
    """Optional additive metadata: legacy corpora need no registry or kinds."""
    registry_path = knowledge / "learn/skills/skills.json"
    cells = []
    enforce_tracks = []
    if registry_path.exists():
        try:
            registry = json.loads(registry_path.read_text())
            if not isinstance(registry, dict) or registry.get("version") != 1 or not isinstance(registry.get("cells"), list):
                raise ValueError("expected version 1 and a cells list")
            cells = registry["cells"]
            enforce_tracks = registry.get("enforce_tracks", [])
            if (not isinstance(enforce_tracks, list) or
                any(not isinstance(track, str) or track not in TRACKS for track in enforce_tracks) or
                len(set(enforce_tracks)) != len(enforce_tracks)):
                raise ValueError("enforce_tracks must list unique known tracks")
            seen = set()
            for cell in cells:
                if (not isinstance(cell, dict) or
                    not all(isinstance(cell.get(k), str) and cell[k].strip()
                            for k in ("id", "domain", "level", "can_do")) or
                    cell["level"] not in LEVELS or
                    cell["id"] != cell["domain"] + "." + cell["level"] or
                    not re.fullmatch(r"[a-z][a-z0-9-]*\.(beginner|intermediate|expert)", cell["id"]) or
                    cell["id"] in seen):
                    raise ValueError("invalid or duplicate skill cell")
                seen.add(cell["id"])
        except (OSError, ValueError, TypeError) as error:
            errors.append(f"learn/skills/skills.json: {error}")
            cells = []
            enforce_tracks = []
    known = {cell["id"]: cell for cell in cells}
    parsed = {e["path"]: LearningHTML((knowledge / e["path"]).read_text())
              for e in entries}
    bibliography = {}
    for path, document in parsed.items():
        if not path.startswith("learn/bibliography/"):
            continue
        for ref in document.reference_ids:
            if not re.fullmatch(r"[a-z][a-z0-9-]*", ref) or ref in bibliography:
                errors.append(f"{path}: invalid or duplicate bibliography ID {ref!r}")
            bibliography[ref] = path
    warnings = []
    for entry in entries:
        path, kind = entry["path"], entry["kind"]
        if kind and kind not in KINDS:
            errors.append(f"{path}: unknown kind {kind!r}")
        for skill in entry["skills"]:
            if skill not in known:
                errors.append(f"{path}: unknown skill ID {skill!r}")
            elif kind != "path" and known[skill]["level"] not in entry["levels"]:
                errors.append(f"{path}: skill {skill!r} is outside the declared page levels")
        if len(entry["skills"]) != len(set(entry["skills"])):
            errors.append(f"{path}: duplicate skill IDs")
        for node in parsed[path].refs:
            links = node["links"]
            if node["tag"] == "dfn":
                glossary = [link for link in links if link.split("?", 1)[0].split("#", 1)[0].startswith("learn/glossary/")]
                if not glossary:
                    errors.append(f"{path}: dfn requires a glossary link")
                for link in glossary:
                    target, _, fragment = link.partition("#")
                    target = target.split("?", 1)[0]
                    if target not in parsed:
                        errors.append(f"{path}: missing glossary target {link!r}")
                    elif fragment:
                        errors.append(f"{path}: cross-page glossary fragments are unsupported; link to a term page")
            else:
                ref = node["attrs"].get("data-ref", "")
                if ref not in bibliography:
                    errors.append(f"{path}: unknown bibliography ID {ref!r}")
                elif bibliography[ref] not in links:
                    errors.append(f"{path}: cite {ref!r} requires its bibliography link")
        if kind in {"glossary", "reference", "hub", "path", "bibliography"}:
            continue
        for section in parsed[path].sections:
            level = section["attrs"]["data-level"]
            if level not in {"beginner", "intermediate"}:
                continue
            text = " ".join(section["text"]).lower()
            required = (["goal:", "why it matters:", "before you start:", "worked example",
                         "try it", "if it goes wrong", "new words", "next step"]
                        if level == "beginner" else
                        ["goal:", "mental model", "worked example", "completion task",
                         "independent task", "choosing", "troubleshooting", "further reading"])
            missing = [name for name in required if name not in text]
            if not section["quiz"]:
                missing.append("quiz")
            if level == "beginner" and not section["first-time"]:
                missing.append("New to this? aside")
            if level == "intermediate" and not section["tamlinux"]:
                missing.append("On Tamlinux aside")
            if missing:
                if entry["track"] in enforce_tracks:
                    errors.append(f"{path}: {level} anatomy: missing " + ", ".join(missing))
                else:
                    warnings.append({"path": path, "level": level, "missing": missing})
    coverage = []
    for cell in cells:
        matched = [entry for entry in entries if cell["id"] in entry["skills"]]
        pages = sorted(e["path"] for e in matched if e["kind"] != "path")
        paths = sorted(e["path"] for e in matched if e["kind"] == "path")
        coverage.append({**cell, "pages": pages, "paths": paths,
                         "page_gap": not pages, "path_gap": not paths})
    return {"version": 1, "enforced_tracks": enforce_tracks,
            "cells": coverage, "warnings": warnings,
            "unclassified_pages": sorted(e["path"] for e in entries if not e["kind"]),
            "unmapped_pages": sorted(e["path"] for e in entries if not e["skills"])}


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
    coverage = learning_metadata(knowledge_dir, entries, errors)

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
    kinds = ["path\tkind\n"]
    skills = ["path\tskills\n"]
    for e in entries:
        kws = ", ".join(e["keywords"])
        levels = ",".join(e["levels"])
        prereqs = ", ".join(e["prereqs"])
        rows.append(f"{e['topic']}\t{e['path']}\t{e['title']}\t{e['summary']}\t{kws}\t"
                    f"{levels}\t{e['track']}\t{prereqs}\t{e['rev']}\t{e['updated']}\n")
        parents.append(f"{e['path']}\t{e['parent'] or ''}\n")
        if e['kind']:
            kinds.append(f"{e['path']}\t{e['kind']}\n")
        if e['skills']:
            skills.append(f"{e['path']}\t{','.join(e['skills'])}\n")

    outputs = {
        "index.tsv": "".join(rows),
        "index.parents.tsv": "".join(parents),
        "index.kinds.tsv": "".join(kinds),
        "index.skills.tsv": "".join(skills),
        "coverage.json": json.dumps(coverage, indent=2) + "\n",
        "index.json": json.dumps(entries, indent=2) + "\n",
    }
    for name, content in outputs.items():
        write_if_changed(knowledge_dir / name, content)

    tldr_dir = knowledge_dir / "tldr"
    if (tldr_dir / "pages").is_dir():
        tldr_builder = Path(__file__).resolve().parent / "build-tldr-index.py"
        if tldr_builder.is_file():
            import importlib.util
            spec = importlib.util.spec_from_file_location("build_tldr_index", tldr_builder)
            if spec and spec.loader:
                mod = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(mod)
                mod.build_tldr_index(tldr_dir)

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

    print(f"Generated index.tsv and five companion outputs with {len(entries)} entries.")
    if coverage["warnings"]:
        print(f"Learning warnings: {len(coverage['warnings'])} level sections need migration; see coverage.json.")
    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", help="Tam command checkout (used for isolated fixtures)")
    args = parser.parse_args()
    sys.exit(main(args.root))
