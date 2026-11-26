#!/usr/bin/env python3
"""build-index.py: Extract metadata from knowledge HTML files and generate
knowledge/index.tsv, its companion projections and knowledge/index.json.

Each page declares its metadata in <meta> tags: topic, summary, keywords,
level, track, optional parent and prereqs. `topic` is the page's permanent
stable ID and `summary` its canonical one-line description; both are required,
topics are unique, and summaries are at most 160 characters. Optional
`search-boost` and `search-exclude` lists tune the generated search terms.
Path pages may also declare `path-category` and `path-order`; their opening
Outcome, Prerequisites and Time lines are projected into `index.paths.tsv`
after the established skills column, so older readers can ignore them.
Level lists the
lenses a page offers (novice, learner, expert); a page offering more than one
wraps each level in <section data-level="..."> (the content-level ladder). The
build fails on missing or duplicate topics, missing, multi-line or overlong
summaries, unknown levels or tracks, on topics that are not slugs, on
prerequisites that name no topic, on invalid search boost/exclude terms, and
on laddered pages whose sections do not match their declared levels.

Every page except the guide index carries a breadcrumb nav before its opening
paragraph, which introduces the page without repeating the breadcrumb title.
Parent metadata defines the canonical hierarchy; older corpora without it use
the historical directory-based fallback. Breadcrumb ancestors must match the
hierarchy, which must reach the guide root without cycles. Corpus 0.31.0
requires explicit parents and track ownership.
"""

from datetime import datetime, timezone
import json
from html.parser import HTMLParser
from html import escape, unescape
import re
import subprocess
import sys
import argparse
from pathlib import Path
from urllib.parse import unquote

# Imported by path as well as run as a script (corpus_pipeline and tests).
_TOOL_DIR = str(Path(__file__).resolve().parent)
if _TOOL_DIR not in sys.path:
    sys.path.insert(0, _TOOL_DIR)
import search_tokens

LEVELS = ("novice", "learner", "expert")
CANONICAL_LEVELS = ("novice", "learner", "expert")
SUBJECTS = ("hardware", "systems", "terminal", "networking", "security", "programming", "desktop", "tamlinux")
TRACKS = SUBJECTS + ("paths", "glossary", "bibliography", "skills", "help", "install", "learn", "use", "maintain", "improve")
# D221 migrated pages: new novice/learner anatomy is hard-enforced only here
# until the rest of the corpus migrates (warnings elsewhere).
D221_PILOT_PAGES = frozenset({
    "terminal/shell/pipelines.html",
    "terminal/shell/index.html",
    "terminal/shell/jobs.html",
    "security/permissions.html",
    "systems/storage/hierarchy.html",
    "systems/maintenance/updates.html",
    "systems/maintenance/index.html",
    "systems/recovery/rollback.html",
    "desktop/navigation/index.html",
    "desktop/navigation/meta-key.html",
    "desktop/navigation/workspaces.html",
    "networking/connections.html",
    "systems/maintenance/logs.html",
    "help/documentation.html",
    "desktop/navigation/voice.html",
    "desktop/navigation/voice-commands.html",
    "desktop/tasks/screenshots.html",
    "desktop/tasks/clipboard.html",
    "desktop/tasks/notifications.html",
    "desktop/tasks/audio.html",
    "desktop/tasks/bluetooth.html",
    "desktop/tasks/displays.html",
    "desktop/tasks/keyboard-touchpad.html",
    "desktop/tasks/lock-and-idle.html",
    "desktop/tasks/night-light.html",
    "systems/recovery/recovery.html",
    "systems/power.html",
    "systems/boot/boot.html",
    "systems/boot/boot-recovery.html",
    "systems/recovery/snapper.html",
    "systems/recovery/checkpoint.html",
    "systems/maintenance/resources.html",
    "programming/efficiency.html",
    "systems/memory-compression.html",
    "terminal/configuration.html",
    "programming/scripting.html",
    "programming/c/toolchains.html",
    "programming/ai/local-inference.html",
    "hardware/foundations/matter-and-energy.html",
    "hardware/foundations/circuits-and-power.html",
    "hardware/foundations/digital-logic.html",
    "hardware/architecture/computer-architecture.html",
    "hardware/architecture/instruction-cycle.html",
    "programming/assembly.html",
    "hardware/signals/serial-and-parallel.html",
    "hardware/signals/cables-and-signaling.html",
    "networking/communication-protocols.html",
    "hardware/signals/error-detection-and-correction.html",
    "hardware/diagnostics/hardware-diagnostics.html",
    "hardware/displays/computer-graphics.html",
    "hardware/architecture/gpu-architecture.html",
    "hardware/displays/display-cables.html",
    "hardware/displays/display-history.html",
    "desktop/session/compositors.html",
    "systems/kernel-and-processes.html",
    "systems/storage/filesystems.html",
    "systems/init.html",
    "tamlinux/development/package-versions.html",
    "programming/c/c11.html",
    "terminal/ttys-and-virtual-consoles.html",
    "desktop/session/sddm.html",
    "desktop/session/evdev.html",
    "systems/packages/packaging.html",
    "systems/packages/nix.html",
    "tamlinux/design/distros.html",
    "tamlinux/development/patches.html",
    "systems/virtualization.html",
    "programming/interfaces/toolkits.html",
    "terminal/terminfo-and-termcap.html",
    "programming/interfaces/ncurses.html",
    "desktop/appearance/fonts.html",
    "desktop/appearance/themes.html",
    "programming/lua.html",
    "tamlinux/design/philosophy.html",
    "hardware/energy/sustainability.html",
    "tamlinux/development/ai.html",
    "help/tam.html",
    "help/practice.html",
    "help/manuals.html",
    "tamlinux/development/tam.html",
})

LEGACY_PILOT_PAGES = frozenset({
    "shell/pipelines.html",
    "shell/index.html",
    "shell/jobs.html",
    "filesystem/permissions.html",
    "filesystem/index.html",
    "maintain/updates.html",
    "maintain/index.html",
    "maintain/rollback.html",
    "desktop/index.html",
    "desktop/meta-key.html",
    "desktop/workspaces.html",
    "hardware/networking.html",
    "maintain/logs.html",
    "learn/documentation.html",
    "desktop/voice.html",
    "desktop/voice-commands.html",
    "use/desktop/screenshots.html",
    "use/desktop/clipboard.html",
    "use/desktop/notifications.html",
    "use/desktop/audio.html",
    "use/desktop/bluetooth.html",
    "use/desktop/displays.html",
    "use/desktop/keyboard-touchpad.html",
    "use/desktop/lock-and-idle.html",
    "use/desktop/night-light.html",
    "maintain/recovery.html",
    "maintain/power.html",
    "maintain/boot.html",
    "maintain/boot-recovery.html",
    "maintain/snapper.html",
    "maintain/checkpoint.html",
    "hardware/resources.html",
    "improve/efficiency.html",
    "improve/zram.html",
    "improve/terminal.html",
    "improve/scripting.html",
    "improve/toolchains.html",
    "improve/ai-performance.html",
    "ecosystem/matter-and-energy.html",
    "ecosystem/circuits-and-power.html",
    "ecosystem/digital-logic.html",
    "ecosystem/computer-architecture.html",
    "ecosystem/instruction-cycle.html",
    "ecosystem/assembly.html",
    "ecosystem/serial-and-parallel.html",
    "ecosystem/cables-and-signaling.html",
    "ecosystem/communication-protocols.html",
    "ecosystem/error-detection-and-correction.html",
    "ecosystem/hardware-diagnostics.html",
    "ecosystem/computer-graphics.html",
    "ecosystem/gpu-architecture.html",
    "ecosystem/display-cables.html",
    "ecosystem/display-history.html",
    "ecosystem/compositors.html",
    "ecosystem/kernel-and-processes.html",
    "ecosystem/filesystems.html",
    "ecosystem/init.html",
    "ecosystem/package-versions.html",
    "ecosystem/c11.html",
    "ecosystem/ttys-and-virtual-consoles.html",
    "ecosystem/sddm.html",
    "ecosystem/evdev.html",
    "ecosystem/packaging.html",
    "ecosystem/nix.html",
    "ecosystem/distros.html",
    "ecosystem/patches.html",
    "ecosystem/virtualization.html",
    "ecosystem/toolkits.html",
    "ecosystem/terminfo-and-termcap.html",
    "ecosystem/ncurses.html",
    "ecosystem/fonts.html",
    "ecosystem/themes.html",
    "ecosystem/lua.html",
    "ecosystem/philosophy.html",
    "ecosystem/sustainability.html",
    "ecosystem/ai.html",
    "learn/tam.html",
    "learn/practice.html",
    "man/index.html",
    "tam.html",
})
D221_PILOT_PAGES = D221_PILOT_PAGES | LEGACY_PILOT_PAGES

# A topic is one command-line word and one line of `tam explain topics --terse`.
TOPIC_RE = re.compile(r"^[a-z0-9][a-z0-9-]*$")
# The canonical summary is authored text for one pager line, not a page excerpt.
SUMMARY_MAX = 160
# Generated search terms supplement, never replace, the TSV's field metadata.
SEARCH_TERMS_MAX = 32
SEARCH_WEIGHTS = {"topic": 7, "title": 6, "summary": 4, "keyword": 5,
                  "text": 1, "boost": 20}
SEARCH_TEXT_WEIGHT_CAP = 3


def level_matches(skill_level, page_levels):
    return skill_level in page_levels


def order_levels(declared):
    return [v for v in CANONICAL_LEVELS if v in declared]


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


def load_all_git_page_revisions(root):
    """Derive deterministic (rev, updated) for all knowledge pages in one git log pass."""
    revisions = {}
    if not (root / ".git").exists():
        return revisions
    try:
        res = subprocess.run(
            ["git", "-C", str(root), "log", "--name-only", "--format=COMMIT:%cd",
             "--date=format:%Y-%m-%d %H:%M", "--", "knowledge"],
            capture_output=True, text=True, check=False
        )
        if res.returncode == 0 and res.stdout:
            file_counts = {}
            file_dates = {}
            current_date = None
            for line in res.stdout.splitlines():
                line = line.strip()
                if not line:
                    continue
                if line.startswith("COMMIT:"):
                    current_date = line[7:]
                elif line.startswith("knowledge/"):
                    rel = line[len("knowledge/"):]
                    file_counts[rel] = file_counts.get(rel, 0) + 1
                    if rel not in file_dates and current_date:
                        file_dates[rel] = current_date
            for rel, count in file_counts.items():
                revisions[rel] = (count, file_dates.get(rel, ""))
    except Exception:
        pass
    return revisions


def git_page_revision(root, path, rel_path, git_cache=None):
    """Derive deterministic (rev, updated) for a knowledge page from Git history.

    rev: integer commit count touching knowledge/<rel_path> (git rev-list --count HEAD -- ...),
         at least 1.
    updated: committer timestamp of the latest commit touching knowledge/<rel_path>
             in its recorded timezone formatted as 'YYYY-MM-DD HH:MM'
             (git log -1 --format=%cd --date=format:%Y-%m-%d %H:%M -- ...).
             Falls back to the file's UTC mtime if the file is not yet committed
             or if Git is unavailable.
    """
    if git_cache is not None:
        if rel_path in git_cache:
            rev, updated = git_cache[rel_path]
            if rev <= 0:
                rev = 1
            if not updated:
                try:
                    mtime = path.stat().st_mtime
                    updated = datetime.fromtimestamp(mtime, tz=timezone.utc).strftime("%Y-%m-%d %H:%M")
                except Exception:
                    updated = "unknown"
            return rev, updated
        else:
            # File is in working tree but not in git history
            rev = 1
            try:
                mtime = path.stat().st_mtime
                updated = datetime.fromtimestamp(mtime, tz=timezone.utc).strftime("%Y-%m-%d %H:%M")
            except Exception:
                updated = "unknown"
            return rev, updated

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


def parse_page(root, path, rel_path, errors, git_cache=None, text=None):
    if text is None:
        text = path.read_text(encoding="utf-8", errors="replace")

    # Title
    m_title = re.search(r'<title>(.*?)</title>', text, re.I | re.S)
    title = unescape(m_title.group(1).strip()) if m_title else ""

    # H1
    m_h1 = re.search(r'<h1>(.*?)</h1>', text, re.I | re.S)
    h1 = m_h1.group(1).strip() if m_h1 else ""
    if not title:
        title = h1

    # Meta tags
    topic = None
    summary = None
    keywords = []
    levels = ["novice"]
    track = "use"
    parent = None
    prereqs = []
    page_kind = ""
    skills = []
    paths = []
    search_boost = []
    search_exclude = []
    path_category = ""
    path_order = ""
    path_outcome = ""
    path_prerequisites = ""
    path_time = ""

    for tag in re.findall(r'<meta\b(?:[^>"\'/]|"[^"]*"|\'[^\']*\')*>', text, re.I):
        name = extract_attr(tag, "name")
        content = extract_attr(tag, "content")
        if not name or content is None:
            continue
        content = unescape(content)
        name_lower = name.lower()
        if name_lower == "topic":
            topic = content.strip()
            if not topic:
                errors.append(f"{rel_path}: topic meta is empty; every page needs a stable ID")
            elif not TOPIC_RE.match(topic):
                errors.append(f"{rel_path}: topic {topic!r} is not a slug"
                              f" (lowercase letters, digits and hyphens)")
        elif name_lower == "summary":
            raw_summary = content
            summary = raw_summary.strip()
            if not summary:
                errors.append(f"{rel_path}: summary meta is empty; every page needs a one-line description")
            elif "\n" in raw_summary or "\r" in raw_summary:
                errors.append(f"{rel_path}: summary must be one line")
            elif len(summary) > SUMMARY_MAX:
                errors.append(f"{rel_path}: summary is {len(summary)} characters; maximum is {SUMMARY_MAX}")
        elif name_lower == "keywords":
            keywords.extend(split_list(content))
        elif name_lower == "level":
            # meta name="level" is ignored; reader lens applies to pages, not topics.
            pass
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
        elif name_lower == "path-category":
            path_category = content.strip().lower()
        elif name_lower == "path-order":
            path_order = content.strip()
        elif name_lower == "search-boost":
            search_boost.extend(split_list(content))
        elif name_lower == "search-exclude":
            search_exclude.extend(split_list(content))
        elif name_lower == "paths":
            seen_page_paths = {p["id"] for p in paths}
            for item in split_list(content):
                pid, sep, step_raw = item.partition(":")
                pid = pid.strip()
                step_raw = step_raw.strip()
                if not sep or not TOPIC_RE.match(pid):
                    errors.append(f"{rel_path}: path entry {item!r} must be '<path-id>:<step>' with slug path ID")
                    continue
                if not step_raw.isdigit() or int(step_raw) < 1:
                    errors.append(f"{rel_path}: path step in {item!r} must be a positive integer")
                    continue
                if pid in seen_page_paths:
                    errors.append(f"{rel_path}: duplicate path ID {pid!r} in paths metadata")
                    continue
                seen_page_paths.add(pid)
                paths.append({"id": pid, "step": int(step_raw)})

    if topic is None:
        errors.append(f"{rel_path}: topic meta is missing; every page needs a stable ID")
    if summary is None:
        errors.append(f"{rel_path}: summary meta is missing; every page needs a one-line description")

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

    if not topic:
        # Placeholder only: the missing-meta error above already fails the build.
        base = rel_path.replace(".html", "")
        if base.endswith("/index"):
            topic = base[:-6]
        elif "/" in base:
            topic = base.split("/")[-1]
        else:
            topic = base

    if not summary:
        # Placeholder only: the missing-meta error above already fails the build.
        summary = title

    if page_kind == "path":
        presentation = extract_path_presentation(text, root / "knowledge")
        path_outcome = presentation["outcome"]
        path_prerequisites = presentation["prerequisites_html"]
        path_time = presentation["time"]

    rev, updated = git_page_revision(root, path, rel_path, git_cache=git_cache)

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
        "paths": paths,
        "path_category": path_category,
        "path_order": path_order,
        "path_outcome": path_outcome,
        "path_prerequisites": path_prerequisites,
        "path_time": path_time,
        "search_boost": search_boost,
        "search_exclude": search_exclude,
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


def check_hierarchy(knowledge_dir, entries, version, errors, pages=None):
    if pages is None:
        pages = {p.relative_to(knowledge_dir).as_posix():
                 {"crumbs": breadcrumb_parts(p.read_text(encoding="utf-8"))}
                 for p in sorted(knowledge_dir.rglob("*.html"))}
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

    for rel, summary in pages.items():
        parts = summary["crumbs"]
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
    """Read links, teaching references, anchors and search text structurally."""
    def __init__(self, text):
        super().__init__(convert_charrefs=True)
        self.stack = []
        self.refs = []
        self.reference_ids = []
        self.sections = []
        self.ids = []
        self.hrefs = []
        self.text = []
        self.headings = []
        self.anchors = []
        self.feed(text)

    @property
    def heading_texts(self):
        return [heading["text"] for heading in self.headings]

    @property
    def search_text(self):
        return " ".join(self.text)

    def skipped_text(self):
        return any(node["tag"] in {"head", "script", "style", "nav"}
                   for node in self.stack)

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if attrs.get("href"):
            self.hrefs.append(attrs["href"])
        if attrs.get("id"):
            self.ids.append(attrs["id"])
            self.anchors.append({"name": unquote(attrs["id"]), "source": "id"})
        if attrs.get("name") and tag == "a":
            self.anchors.append({"name": unquote(attrs["name"]), "source": "name"})
        if "reference" in attrs.get("class", "").split():
            self.reference_ids.append(attrs.get("id", ""))
        if tag in {"meta", "br", "hr", "img", "link", "input", "wbr"}:
            return
        node = {"tag": tag, "attrs": attrs, "text": [], "links": [],
                "quiz": False, "sandbox-example": False,
                "first-time": False, "tamlinux": False}
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
        if not self.skipped_text() and text.strip():
            self.text.append(text)
        for node in self.stack:
            node["text"].append(text)

    def handle_endtag(self, tag):
        for i in range(len(self.stack) - 1, -1, -1):
            if self.stack[i]["tag"] != tag:
                continue
            nodes, self.stack = self.stack[i:], self.stack[:i]
            if re.fullmatch(r"h[1-6]", tag) and not any(
                    node["tag"] in {"head", "script", "style", "nav"}
                    for node in self.stack):
                label = unescape(" ".join(nodes[0]["text"])).strip()
                if label:
                    self.headings.append({"text": label, "tag": tag})
            if (tag == "out" and len(self.stack) >= 2
                    and self.stack[-2]["tag"] == "tam-sandbox"
                    and self.stack[-1]["tag"] == "tam-cmd"
                    and (self.stack[-1]["attrs"].get("match") or "").strip()
                    and "".join(nodes[0]["text"]).strip()):
                for parent in self.stack[:-2]:
                    parent["sandbox-example"] = True
            for node in nodes:
                if node["tag"] in {"dfn", "cite"}:
                    self.refs.append(node)
                if node["tag"] == "section" and node["attrs"].get("data-level"):
                    self.sections.append(node)
            break


def learning_metadata(knowledge, entries, errors, parsed=None):
    """Optional additive metadata: legacy corpora need no registry or kinds."""
    registry_path = knowledge / "skills/skills.json"
    if not registry_path.exists():
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
                    not re.fullmatch(r"[a-z][a-z0-9-]*\.(novice|learner|expert)", cell["id"]) or
                    cell["id"] in seen):
                    raise ValueError("invalid or duplicate skill cell")
                seen.add(cell["id"])
        except (OSError, ValueError, TypeError) as error:
            errors.append(f"learn/skills/skills.json: {error}")
            cells = []
            enforce_tracks = []
    known = {cell["id"]: cell for cell in cells}
    if parsed is None:
        parsed = {e["path"]: LearningHTML((knowledge / e["path"]).read_text())
                  for e in entries}
    bibliography = {}
    for path, document in parsed.items():
        if not path.startswith(("bibliography/", "learn/bibliography/")):
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
        if len(entry["skills"]) != len(set(entry["skills"])):
            errors.append(f"{path}: duplicate skill IDs")
        for node in parsed[path].refs:
            links = node["links"]
            if node["tag"] == "dfn":
                glossary = [link for link in links if link.split("?", 1)[0].split("#", 1)[0].startswith(("glossary/", "learn/glossary/"))]
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
            if level not in {"novice", "learner"}:
                continue
            text = " ".join(section["text"]).lower()
            if level == "novice":
                forbidden = []
                if "goal:" in text:
                    forbidden.append("goal:")
                if "try it" in text:
                    forbidden.append("try it")
                if section["quiz"]:
                    forbidden.append("quiz")
                if forbidden:
                    msg = f"{path}: novice anatomy: forbidden " + ", ".join(forbidden)
                    if path in D221_PILOT_PAGES:
                        errors.append(msg)
                    else:
                        warnings.append({"path": path, "level": level, "forbidden": forbidden})
                continue
            if level == "learner":
                required = ["goal:", "try it", "if it goes wrong"]
                if not section["sandbox-example"]:
                    required.insert(1, "worked example")
                missing = [name for name in required if name not in text]
                if not section["quiz"]:
                    missing.append("quiz")
                if missing:
                    msg = f"{path}: learner anatomy: missing " + ", ".join(missing)
                    if path in D221_PILOT_PAGES:
                        errors.append(msg)
                    else:
                        warnings.append({"path": path, "level": level, "missing": missing})
                continue
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


def document_fields(document):
    """The fields consumed by learning_metadata, plus anchor/search inputs."""
    return {"refs": document.refs, "reference_ids": document.reference_ids,
            "sections": document.sections, "anchors": document.anchors,
            "heading_texts": document.heading_texts,
            "search_text": " ".join(document.text)}


def load_documents(knowledge, entries):
    return {entry["path"]: LearningHTML(
        (knowledge / entry["path"]).read_text(encoding="utf-8", errors="replace"))
        for entry in entries}


def normalize_search_meta(entries, errors):
    """Validate authored boost/exclude tokens and normalize their spelling."""
    for entry in entries:
        for field in ("search_boost", "search_exclude"):
            normalized = []
            seen = set()
            for authored in entry.get(field, []):
                token = search_tokens.single_token(authored)
                if token is None or token["raw"].lower() in search_tokens.STOP_WORDS:
                    errors.append(f"{entry['path']}: {field} entry {authored!r} "
                                  "is not a single search term")
                    continue
                key = token["stem"]
                if key in seen:
                    errors.append(f"{entry['path']}: duplicate {field} term {authored!r}")
                    continue
                seen.add(key)
                normalized.append(token)
            entry[field] = normalized
        overlap = {token["stem"] for token in entry["search_boost"]} & {
            token["stem"] for token in entry["search_exclude"]}
        for stem in sorted(overlap):
            errors.append(f"{entry['path']}: search term {stem!r} is both boosted and excluded")


def anchor_rows(document):
    """Explicit anchors plus the heading text the reader can fall back to."""
    rows = []
    seen = set()
    candidates = list(document.anchors)
    candidates.extend({"name": text, "source": "heading"}
                      for text in document.heading_texts)
    for candidate in candidates:
        name = candidate["name"].strip()
        if not name:
            continue
        key = (name.lower(), candidate["source"])
        if key in seen:
            continue
        seen.add(key)
        rows.append({"name": name, "source": candidate["source"]})
    return sorted(rows, key=lambda row: (row["name"].lower(), row["source"]))


def derive_search_terms(entry, document, include_boost=True):
    """Build a compact, weighted term projection from authored page material."""
    terms = {}

    def add_text(text, source):
        for token in search_tokens.tokenize(text):
            stem = token["stem"]
            current = terms.get(stem)
            if source == "text":
                if current is None:
                    terms[stem] = {"term": token["raw"], "stem": stem,
                                   "weight": SEARCH_WEIGHTS["text"], "source": "text"}
                elif current["source"] == "text":
                    current["weight"] = min(current["weight"] + SEARCH_WEIGHTS["text"],
                                            SEARCH_TEXT_WEIGHT_CAP)
                continue
            if current is None or SEARCH_WEIGHTS[source] > current["weight"]:
                terms[stem] = {"term": token["raw"], "stem": stem,
                               "weight": SEARCH_WEIGHTS[source], "source": source}

    add_text(entry["topic"], "topic")
    add_text(entry["title"], "title")
    add_text(entry["summary"], "summary")
    for keyword in entry["keywords"]:
        add_text(keyword, "keyword")
    add_text(document.search_text, "text")
    if include_boost:
        for token in entry["search_boost"]:
            current = terms.get(token["stem"])
            weight = SEARCH_WEIGHTS["boost"]
            if current is not None:
                weight += current["weight"]
            terms[token["stem"]] = {"term": token["raw"], "stem": token["stem"],
                                    "weight": weight, "source": "boost"}
    return terms


def prepare_search(entries, documents, errors):
    """Attach the generated search/anchor projection to each entry."""
    normalize_search_meta(entries, errors)
    for entry in entries:
        document = documents[entry["path"]]
        available = set(derive_search_terms(entry, document, include_boost=False))
        for field in ("search_boost", "search_exclude"):
            for token in entry[field]:
                if token["stem"] not in available:
                    errors.append(f"{entry['path']}: {field} term {token['raw']!r} "
                                  "does not appear on the page")
        terms = derive_search_terms(entry, document)
        for token in entry["search_exclude"]:
            terms.pop(token["stem"], None)
        if entry["search_exclude"] and not terms:
            errors.append(f"{entry['path']}: search-exclude removes every search term")
        rows = [terms[stem] for stem in sorted(
            terms, key=lambda stem: (-terms[stem]["weight"], terms[stem]["term"], stem))]
        entry["search"] = {
            "boost": [token["raw"] for token in entry["search_boost"]],
            "exclude": [token["raw"] for token in entry["search_exclude"]],
            "terms": rows[:SEARCH_TERMS_MAX],
            "anchors": anchor_rows(document),
        }


def _clean_tsv(value):
    return re.sub(r"[\t\r\n]+", " ", value or "").strip()


PATH_CATEGORIES = ("start-here", "understand-computer", "keep-running",
                   "connect", "stay-safe", "write-software", "improve-work")
PATH_PRESENTATION_MIN = (0, 139)


def _path_plain(fragment):
    """Plain text for a path opening field, with entities resolved once."""
    return unescape(re.sub(r"<[^>]+>", "", fragment or "")).strip()


def _path_prerequisite_html(fragment, knowledge):
    """Render an authored prerequisite line with path links retargeted.

    Only a link to an existing paths/ or learn/paths/ page becomes a
    tam:path link; everything else is emitted as escaped text. The reader
    can append this fragment without further escaping.
    """
    fragment = fragment or ""
    pieces = []
    cursor = 0
    for match in PATH_ANCHOR_RE.finditer(fragment):
        pieces.append(escape(unescape(fragment[cursor:match.start()])))
        href = re.search(r'href\s*=\s*["\']([^"\']+)["\']',
                         match.group(1), re.I)
        target = href.group(1).split("#", 1)[0].split("?", 1)[0] if href else ""
        label = _path_plain(match.group(2))
        pid = ""
        if target.startswith(("paths/", "learn/paths/")) and (
                knowledge / target).is_file():
            candidate = Path(target).stem
            if TOPIC_RE.match(candidate):
                pid = candidate
        if pid:
            pieces.append(f'<a href="tam:path?id={pid}">{escape(label)}</a>')
        else:
            pieces.append(escape(label))
        cursor = match.end()
    pieces.append(escape(unescape(fragment[cursor:])))
    return "".join(pieces).strip()


def extract_path_presentation(text, knowledge):
    """Project a path page's authored opening into generated-view fields."""
    first = PATH_H2_RE.search(text)
    opening = text[:first.start()] if first else text
    raw = {}
    fields = {}
    for label, key in (("Outcome", "outcome"),
                       ("Prerequisites", "prerequisites"),
                       ("Time", "time")):
        match = re.search(PATH_FIELD_RE.format(label), opening, re.I | re.S)
        raw[key] = match.group(1) if match else ""
        fields[key] = _path_plain(raw[key])
    fields["prerequisites_html"] = _path_prerequisite_html(
        raw["prerequisites"], knowledge)
    return fields


def validate_path_presentation(entries, errors, version="0.0.0"):
    """Check the authored presentation metadata added in corpus 0.139.0."""
    try:
        current = tuple(int(part) for part in version.split(".")[:2])
    except ValueError:
        current = (0, 0)
    if current < PATH_PRESENTATION_MIN:
        return
    seen = {}
    for entry in entries:
        if entry.get("kind") != "path":
            continue
        rel = entry["path"]
        category = entry.get("path_category", "")
        order_raw = entry.get("path_order", "")
        if category not in PATH_CATEGORIES:
            errors.append(f"{rel}: unknown path category {category!r}"
                          f" (expected {', '.join(PATH_CATEGORIES)})")
        if not str(order_raw).isdigit() or int(order_raw) < 1:
            errors.append(f"{rel}: path order must be a positive integer")
            continue
        key = (category, int(order_raw))
        if key in seen:
            errors.append(f"{rel}: duplicate path presentation order "
                          f"{category}/{order_raw} (already used by {seen[key]})")
        else:
            seen[key] = rel


def validate_global_entries(entries, errors):
    seen_topics = {}
    topics = set()
    for e in entries:
        topic = e["topic"]
        if topic in seen_topics:
            errors.append(f"{e['path']}: duplicate topic {topic!r} (already used by {seen_topics[topic]})")
        else:
            seen_topics[topic] = e["path"]
            topics.add(topic)

    for e in entries:
        for p in e["prereqs"]:
            if p not in topics:
                errors.append(f"{e['path']}: prerequisite {p!r} is not a topic")

    by_topic = {e["topic"]: e for e in entries}
    done = set()
    visiting = set()
    def visit(topic):
        if topic in visiting:
            errors.append(f"{by_topic[topic]['path']}: prerequisite cycle at {topic!r}")
            return
        if topic in done or topic not in by_topic:
            return
        visiting.add(topic)
        for prerequisite in by_topic[topic]["prereqs"]:
            visit(prerequisite)
        visiting.remove(topic)
        done.add(topic)
    for topic in by_topic:
        visit(topic)

    path_defs = {e["topic"]: e for e in entries if e.get("kind") == "path"}
    seen_steps = {}
    for e in entries:
        for ref in e.get("paths", []):
            pid, step = ref["id"], ref["step"]
            if pid not in path_defs:
                errors.append(f"{e['path']}: unknown learning path {pid!r}")
                continue
            key = (pid, step)
            if key in seen_steps:
                errors.append(f"{e['path']}: duplicate step {step} on path {pid!r} (already used by {seen_steps[key]})")
            else:
                seen_steps[key] = e["path"]

    for pid, path_def in path_defs.items():
        steps = sorted(step for (path_id, step) in seen_steps if path_id == pid)
        if steps != list(range(1, len(steps) + 1)):
            errors.append(f"{path_def['path']}: path {pid!r} steps {steps} "
                          f"are not contiguous from 1")


PATH_H2_RE = re.compile(r'<h2\b[^>]*>(.*?)</h2>', re.I | re.S)
PATH_STAGE_RE = re.compile(r'^(?:Stage\s+(\d+)\s*:|\s*(\d+)\.)\s*(.*)$', re.I | re.S)
PATH_LEVEL_RE = re.compile(r'level=[0-3]')
PATH_DETAILS_RE = re.compile(r'<details\b([^>]*)>(.*?)</details>', re.I | re.S)
PATH_HREF_RE = re.compile(r'<a\b[^>]*href\s*=\s*["\']([^"\']+)["\']', re.I)
PATH_ANCHOR_RE = re.compile(r'<a\b([^>]*)>(.*?)</a>', re.I | re.S)
PATH_NON_PAGES = ("glossary/", "bibliography/", "paths/", "learn/glossary/", "learn/bibliography/", "learn/paths/")
PATH_TIME_BY_STAGES = {3: 30, 4: 40, 5: 50, 6: 60}
PATH_FIELD_RE = r'<strong>\s*{0}\s*:</strong>\s*(.*?)</p>'
PATH_STATE_RE = re.compile(
    r'<strong>\s*Expected sandbox state:\s*</strong>\s*(.*?)(?:</p>|$)',
    re.I | re.S)


def _plain(fragment):
    return re.sub(r'<[^>]+>', '', fragment).strip()


def _class_tokens(attrs):
    match = re.search(r'class\s*=\s*["\']([^"\']*)["\']', attrs, re.I)
    return match.group(1).split() if match else []


def validate_path_pages(knowledge, entries, errors, version="0.0.0", texts=None):
    """Enforce the Paths contract in docs/corpus-pedagogy.md.

    A path page has three to six numbered stages, each with one to four
    existing page steps at a stated lens and a checkpoint quiz, then one
    capstone with a folded solution and a Next paths list. Corpora before
    0.82.0 keep their historical pages and are not re-checked."""
    try:
        current = tuple(int(part) for part in version.split(".")[:2])
    except ValueError:
        current = (0, 0)
    if current < (0, 82):
        return
    for entry in entries:
        if entry.get("kind") != "path":
            continue
        rel = entry["path"]
        text = texts.get(rel) if texts is not None else None
        if text is None:
            page = knowledge / rel
            if not page.is_file():
                continue
            text = page.read_text(encoding="utf-8", errors="replace")
        headings = [(match.start(), match.end(), _plain(match.group(1)))
                    for match in PATH_H2_RE.finditer(text)]
        stages = []
        for index, (_, end, label) in enumerate(headings):
            match = PATH_STAGE_RE.match(label)
            if not match:
                continue
            stop = headings[index + 1][0] if index + 1 < len(headings) else len(text)
            stages.append((int(match.group(1) or match.group(2)), label,
                           text[end:stop]))
        if not 3 <= len(stages) <= 6:
            errors.append(f"{rel}: path needs three to six stages, found {len(stages)}")
        numbers = [number for number, _, _ in stages]
        if numbers != list(range(1, len(numbers) + 1)):
            errors.append(f"{rel}: path stages {numbers} are not numbered from 1")
        for number, _, body in stages:
            steps = []
            for link in PATH_HREF_RE.findall(body):
                target, _, query = link.partition("?")
                if target.startswith(("http://", "https://", "mailto:", "man:", "tam:", "#")):
                    continue
                if target.startswith(PATH_NON_PAGES) or target == "index.html":
                    continue
                if not PATH_LEVEL_RE.fullmatch(query):
                    errors.append(f"{rel}: stage {number} link {link!r} "
                                  f"needs a ?level=0-3 lens")
                    continue
                target = target.split("#", 1)[0]
                if not (knowledge / target).is_file():
                    errors.append(f"{rel}: stage {number} step {target!r} "
                                  f"does not name an existing page")
                    continue
                steps.append(target)
            if not 1 <= len(steps) <= 4:
                errors.append(f"{rel}: stage {number} needs one to four page steps, "
                              f"found {len(steps)}")
            quizzes = [(attrs, block) for attrs, block in PATH_DETAILS_RE.findall(body)
                       if "q" in _class_tokens(attrs)]
            if not quizzes:
                errors.append(f"{rel}: stage {number} needs a checkpoint quiz")
            for _, block in quizzes:
                summary = re.search(r'<summary\b[^>]*>(.*?)</summary>', block, re.I | re.S)
                if not summary or not _plain(summary.group(1)):
                    errors.append(f"{rel}: stage {number} quiz needs a question summary")
                answer = re.sub(r'<summary\b[^>]*>.*?</summary>', '', block, count=1,
                                flags=re.I | re.S)
                if not _plain(answer):
                    errors.append(f"{rel}: stage {number} quiz needs an answer")
        capstones = [(index, label) for index, (_, _, label) in enumerate(headings)
                     if label.lower().startswith("capstone")]
        if len(capstones) != 1:
            errors.append(f"{rel}: path needs exactly one capstone heading, "
                          f"found {len(capstones)}")
        else:
            index = capstones[0][0]
            start = headings[index][1]
            stop = headings[index + 1][0] if index + 1 < len(headings) else len(text)
            solutions = [(attrs, block) for attrs, block in
                         PATH_DETAILS_RE.findall(text[start:stop])
                         if "solution" in _class_tokens(attrs)]
            if len(solutions) != 1:
                errors.append(f"{rel}: capstone needs exactly one folded solution, "
                              f"found {len(solutions)}")
            elif not _plain(solutions[0][1]):
                errors.append(f"{rel}: capstone solution is empty")
        nexts = [(index, label) for index, (_, _, label) in enumerate(headings)
                 if label.lower().startswith("next path")]
        if len(nexts) != 1:
            errors.append(f"{rel}: path needs one Next paths heading, found {len(nexts)}")
        else:
            index = nexts[0][0]
            start = headings[index][1]
            stop = headings[index + 1][0] if index + 1 < len(headings) else len(text)
            body = text[start:stop]
            if not any(not link.startswith(("http://", "https://", "mailto:", "man:", "tam:"))
                       for link in PATH_HREF_RE.findall(body)):
                errors.append(f"{rel}: Next paths needs at least one internal link")

        first_heading = headings[0][0] if headings else len(text)
        opening = text[:first_heading]
        for label in ("Outcome", "Prerequisites", "Time"):
            field = re.search(PATH_FIELD_RE.format(label), opening, re.I | re.S)
            if field is None or not _plain(field.group(1)):
                errors.append(f"{rel}: path opening needs a {label.lower()} line")
        prereq = re.search(PATH_FIELD_RE.format("Prerequisites"), opening, re.I | re.S)
        if prereq is not None:
            for link in PATH_HREF_RE.findall(prereq.group(1)):
                target = link.split("#", 1)[0].split("?", 1)[0]
                if target and not (knowledge / target).is_file():
                    errors.append(f"{rel}: prerequisite link {link!r} "
                                  "does not name an existing page")
        timed = re.search(r'<strong>\s*Time\s*:</strong>\s*About\s+(\d+)\s+minutes?',
                          opening, re.I)
        if timed is not None and len(stages) in PATH_TIME_BY_STAGES:
            expected = PATH_TIME_BY_STAGES[len(stages)]
            if int(timed.group(1)) != expected:
                errors.append(f"{rel}: time should be about {expected} minutes "
                              f"for {len(stages)} stages")
        aside = re.search(r'<aside\b([^>]*)>(.*?)</aside>', opening, re.I | re.S)
        if (aside is None or "note" not in _class_tokens(aside.group(1)) or
                not re.search(r"<strong>\s*If you're stuck:\s*</strong>",
                              aside.group(2), re.I)):
            errors.append(f"{rel}: path opening needs an If you're stuck note")
        else:
            for needle, label in (("tam man", "manuals"),
                                  ("tam help", "built-in help"),
                                  ("tam search", "search"),
                                  ("help/manuals.html", "manuals page")):
                if needle not in aside.group(2):
                    errors.append(f"{rel}: failure branch needs {label}")
        checkpoints = [(attrs, block) for attrs, block in PATH_DETAILS_RE.findall(text)
                       if "q" in _class_tokens(attrs)]
        for number, (_, block) in enumerate(checkpoints, 1):
            answer = re.sub(r'<summary\b[^>]*>.*?</summary>', '', block, count=1,
                            flags=re.I | re.S)
            state = PATH_STATE_RE.search(answer)
            if state is None or not _plain(state.group(1)):
                errors.append(f"{rel}: checkpoint {number} needs an expected "
                              "sandbox state")


def index_outputs(entries, coverage):
    # Sort entries by topic
    entries.sort(key=lambda e: e["topic"])

    # Keep the established TSV projection byte-compatible for older readers.
    # Parent is additive in JSON and in a separate compact TSV projection.
    rows = ["topic\tpath\ttitle\tsummary\tkeywords\ttrack\tprereqs\trev\tupdated\n"]
    parents = ["path\tparent\n"]
    kinds = ["path\tkind\n"]
    skills = ["path\tskills\n"]
    path_defs = {}
    path_steps = {}
    for e in entries:
        kws = ", ".join(e["keywords"])
        prereqs = ", ".join(e["prereqs"])
        rows.append(f"{e['topic']}\t{e['path']}\t{e['title']}\t{e['summary']}\t{kws}\t"
                    f"{e['track']}\t{prereqs}\t{e['rev']}\t{e['updated']}\n")
        parents.append(f"{e['path']}\t{e['parent'] or ''}\n")
        if e['kind']:
            kinds.append(f"{e['path']}\t{e['kind']}\n")
        if e['skills']:
            skills.append(f"{e['path']}\t{','.join(e['skills'])}\n")
        if e['kind'] == "path":
            path_defs[e["topic"]] = e
        for ref in e.get("paths", []):
            path_steps.setdefault(ref["id"], []).append((ref["step"], e["path"], e["title"]))

    # Generated search terms and the validated same-page anchor graph live in
    # their own projections so older readers can ignore them. Exclusions are
    # emitted as weight-0 rows: the reader applies them to every field, so a
    # excluded word cannot re-enter through a title or keyword.
    search = ["path\tterm\tstem\tweight\tsource\n"]
    anchors = ["path\tname\tsource\n"]
    for e in entries:
        for term in e["search"]["terms"]:
            search.append(f"{e['path']}\t{_clean_tsv(term['term'])}\t"
                          f"{_clean_tsv(term['stem'])}\t{term['weight']}\t{term['source']}\n")
        for token in e["search_exclude"]:
            search.append(f"{e['path']}\t{_clean_tsv(token['raw'])}\t"
                          f"{_clean_tsv(token['stem'])}\t0\texclude\n")
        for anchor in e["search"]["anchors"]:
            anchors.append(f"{e['path']}\t{_clean_tsv(anchor['name'])}\t{anchor['source']}\n")

    def path_sort_key(pid):
        pdef = path_defs[pid]
        category = pdef.get("path_category", "")
        rank = (PATH_CATEGORIES.index(category) if category in PATH_CATEGORIES
                else len(PATH_CATEGORIES))
        order_raw = str(pdef.get("path_order", ""))
        order = int(order_raw) if order_raw.isdigit() else 0
        return (rank, order, pid)

    paths_tsv = ["record\tpath_id\targ1\targ2\targ3\targ4\tcategory\torder\t"
                 "outcome\tprerequisites\ttime\n"]
    for pid in sorted(path_defs, key=path_sort_key):
        pdef = path_defs[pid]
        paths_tsv.append(
            f"P\t{_clean_tsv(pid)}\t{_clean_tsv(pdef['title'])}\t"
            f"{_clean_tsv(pdef['path'])}\t{_clean_tsv(pdef['summary'])}\t"
            f"{','.join(pdef['skills'])}\t"
            f"{_clean_tsv(pdef.get('path_category', ''))}\t"
            f"{_clean_tsv(pdef.get('path_order', ''))}\t"
            f"{_clean_tsv(pdef.get('path_outcome', ''))}\t"
            f"{_clean_tsv(pdef.get('path_prerequisites', ''))}\t"
            f"{_clean_tsv(pdef.get('path_time', ''))}\n"
        )
        for step, page_path, page_title in sorted(path_steps.get(pid, []), key=lambda x: (x[0], x[1])):
            paths_tsv.append(
                f"S\t{_clean_tsv(pid)}\t{step}\t{_clean_tsv(page_path)}\t"
                f"{_clean_tsv(page_title)}\t\n"
            )

    outputs = {
        "index.tsv": "".join(rows),
        "index.parents.tsv": "".join(parents),
        "index.kinds.tsv": "".join(kinds),
        "index.skills.tsv": "".join(skills),
        "index.paths.tsv": "".join(paths_tsv),
        "index.search.tsv": "".join(search),
        "index.anchors.tsv": "".join(anchors),
        "coverage.json": json.dumps(coverage, indent=2) + "\n",
        "index.json": json.dumps(entries, indent=2) + "\n",
    }
    return outputs


def reference_main(root=None):
    root = Path(root).resolve() if root else Path(__file__).resolve().parent.parent
    knowledge_dir = root / "knowledge"

    if not knowledge_dir.is_dir():
        print(f"Error: knowledge directory not found at {knowledge_dir}", file=sys.stderr)
        return 1

    errors = []
    entries = []
    git_cache = load_all_git_page_revisions(root)
    for html_file in sorted(knowledge_dir.rglob("*.html")):
        rel = html_file.relative_to(knowledge_dir).as_posix()
        entry = parse_page(root, html_file, rel, errors, git_cache=git_cache)
        entries.append(entry)

    validate_global_entries(entries, errors)

    version_file = knowledge_dir / "version"
    if not version_file.is_file():
        errors.append("knowledge/version: missing version file")
    else:
        v_str = version_file.read_text(encoding="utf-8").strip()
        if not re.match(r'^\d+\.\d+\.\d+', v_str):
            errors.append(f"knowledge/version: invalid version {v_str!r} (expected X.Y.Z)")

    version = (version_file.read_text(encoding="utf-8").strip()
               if version_file.is_file() else "0.0.0")
    check_hierarchy(knowledge_dir, entries, version, errors)
    documents = load_documents(knowledge_dir, entries)
    coverage = learning_metadata(knowledge_dir, entries, errors, parsed=documents)
    prepare_search(entries, documents, errors)
    validate_path_pages(knowledge_dir, entries, errors, version)
    validate_path_presentation(entries, errors, version)

    if errors:
        for message in errors:
            print(f"Error: {message}", file=sys.stderr)
        return 1

    outputs = index_outputs(entries, coverage)
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

    print(f"Generated index.tsv and its companion outputs with {len(entries)} entries.")
    if coverage["warnings"]:
        print(f"Learning warnings: {len(coverage['warnings'])} level sections need migration; see coverage.json.")
    return 0


def main(root=None):
    # Isolated legacy fixtures copy only this file; keep their uncached oracle.
    if not Path(__file__).with_name("corpus_pipeline.py").is_file():
        return reference_main(root)
    import importlib.util
    spec = importlib.util.spec_from_file_location("corpus_pipeline", Path(__file__).with_name("corpus_pipeline.py"))
    pipeline = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(pipeline)
    run = pipeline.run
    return run(root, mode="index")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", help="Tam command checkout (used for isolated fixtures)")
    args = parser.parse_args()
    sys.exit(main(args.root))
