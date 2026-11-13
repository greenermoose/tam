#!/usr/bin/env python3
"""Incremental corpus build/check/preview, with a disposable content-addressed cache.

The cache is an optimization, never an authority: global relationships are
validated from page summaries whenever their inputs change, and CORPUS_CACHE=0 recomputes all
page-local work. No compiler, libtam, daemon or third-party Python modules.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from contextlib import contextmanager
import fcntl
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
from types import SimpleNamespace

sys.dont_write_bytecode = True

SCHEMA = 2
OUTPUTS = {"index.tsv", "index.json", "index.parents.tsv", "index.kinds.tsv",
           "index.skills.tsv", "index.paths.tsv", "coverage.json",
           "component-coverage.json", "tldr/index.tsv", ".build_id"}
REVISION = re.compile(r"^Rev [1-9][0-9]* Last updated \d{4}-\d{2}-\d{2} \d{2}:\d{2}$", re.M)
HREF = re.compile(r'''href=["']([^"']+)["']''')


class CorpusError(Exception):
    pass


class UnstableInputs(CorpusError):
    pass


def digest(data):
    return hashlib.sha256(data).hexdigest()


def fingerprint(value):
    return digest(json.dumps(value, sort_keys=True, separators=(",", ":")).encode())


def load_module(name):
    path = Path(__file__).with_name(name + ".py")
    spec = importlib.util.spec_from_file_location(name.replace("-", "_"), path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def atomic_write(path, content):
    data = content.encode() if isinstance(content, str) else content
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


@contextmanager
def cache_lock(root):
    directory = root / "build/corpus"
    directory.mkdir(parents=True, exist_ok=True)
    with (directory / "lock").open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        yield directory / "state.json"


def read_cache(path):
    try:
        envelope = json.loads(path.read_text())
        if not isinstance(envelope, dict) or not isinstance(envelope.get("payload"), str):
            return {}
        payload = envelope["payload"]
        if digest(payload.encode()) != envelope["checksum"]:
            return {}
        state = json.loads(payload)
        if isinstance(state, dict) and state.get("schema") == SCHEMA and isinstance(state.get("pages"), dict):
            return state
    except (OSError, ValueError, TypeError, KeyError):
        pass
    return {}


def save_cache(path, state):
    payload = json.dumps(state, separators=(",", ":"), sort_keys=True)
    atomic_write(path, json.dumps({"checksum": digest(payload.encode()), "payload": payload}))


def git(root, *args):
    if not (root / ".git").exists():
        return ""
    result = subprocess.run(["git", "-C", str(root), *args], capture_output=True, text=True)
    return result.stdout.strip() if result.returncode == 0 else ""


def history_key(root, head):
    """Shallow boundaries/grafts/replacement refs can change history at the same HEAD."""
    directory = root / ".git"
    if directory.is_file():
        marker = directory.read_text().strip()
        if marker.startswith("gitdir: "):
            directory = (root / marker[8:]).resolve()
    common = directory / "commondir"
    if common.is_file():
        directory = (directory / common.read_text().strip()).resolve()
    paths = [directory / rel for rel in ("shallow", "info/grafts", "packed-refs")]
    paths.extend(sorted((directory / "refs/replace").rglob("*")))
    metadata = [(str(path.relative_to(directory)), digest(path.read_bytes()))
                for path in paths if path.is_file()]
    return fingerprint([head, metadata, os.environ.get("GIT_NO_REPLACE_OBJECTS"),
                        os.environ.get("GIT_REPLACE_REF_BASE")])


def signatures(knowledge):
    """Include directory membership and ctime, so atomic saves/deletions are visible."""
    out = {}
    base = str(knowledge)
    prefix_length = len(base) + 1
    for directory, dirs, files in os.walk(base):
        dirs.sort()
        for name in sorted(files):
            filename = os.path.join(directory, name)
            rel = filename[prefix_length:]
            if rel in OUTPUTS:
                continue
            st = os.stat(filename)
            out[rel] = (st.st_ino, st.st_size, st.st_mtime_ns, st.st_ctime_ns)
    return out


def snapshot(root, coverage):
    knowledge = root / "knowledge"
    if not knowledge.is_dir():
        raise CorpusError("knowledge/: no corpus directory")
    before = signatures(knowledge)
    texts, hashes = {}, {}
    for rel in before:
        if rel.endswith((".html", ".md", ".json")) or rel == "version":
            data = (knowledge / rel).read_bytes()
            hashes[rel] = digest(data)
            texts[rel] = data.decode("utf-8", errors="replace")
    # Tool/rule inputs are small and hashed, including content-preserving timestamp edits.
    dependencies = sorted((root / "tools").glob("*.py"))
    dependencies += [root / rel for rel in (
        "src/render.c", "src/tam.c", "src/install.c", "tests/guide.sh",
        "tools/component-coverage-exemptions.tsv")]
    deps = {str(p.relative_to(root)): digest(p.read_bytes()) if p.is_file() else None
            for p in dependencies}
    product = coverage.resolve_tamlinux(root, None)
    components = coverage.inventory(product) if product else []
    head = git(root, "rev-parse", "HEAD")
    return SimpleNamespace(signatures=before, texts=texts, hashes=hashes,
                           dependencies=deps, head=head, history_key=history_key(root, head),
                           product=product, components=components,
                           tldr_present=(knowledge / "tldr/pages").is_dir())


def stable(root, snap, coverage):
    # Metadata comparison is sufficient here: ctime cannot be restored by an editor.
    if (root / "knowledge/tldr/pages").is_dir() != snap.tldr_present:
        return False
    if signatures(root / "knowledge") != snap.signatures:
        return False
    head = git(root, "rev-parse", "HEAD")
    if head != snap.head or history_key(root, head) != snap.history_key:
        return False
    for rel, expected in snap.dependencies.items():
        p = root / rel
        if (digest(p.read_bytes()) if p.is_file() else None) != expected:
            return False
    product = coverage.resolve_tamlinux(root, None)
    return product == snap.product and (coverage.inventory(product) if product else []) == snap.components


def reader_path(root, requested=None):
    explicit = requested or os.environ.get("TAM_READER")
    candidate = explicit or (str(root / "build/tam") if os.access(root / "build/tam", os.X_OK)
                             else shutil.which("tam"))
    found = shutil.which(candidate) if candidate else None
    if not found:
        raise CorpusError("no executable reader; run make command separately, or set TAM_READER=/path/to/tam")
    return Path(found).resolve()


def page_summary(builder, root, rel, text, history):
    errors = []
    entry = builder.parse_page(root, root / "knowledge" / rel, rel, errors,
                               git_cache=history, text=text)
    document = builder.LearningHTML(text)
    # Only these fields are consumed by learning_metadata; omit HTMLParser internals.
    learning = {key: getattr(document, key) for key in ("refs", "reference_ids", "sections")}
    for section in learning["sections"]:
        section["text"] = [" ".join(section["text"])]
    return {"entry": entry, "errors": errors, "crumbs": builder.breadcrumb_parts(text),
            "learning": learning, "links": HREF.findall(text)}


def identity(root):
    cid = git(root, "log", "-n", "1", "--format=%h", "--", "knowledge", "tools")
    if cid:
        if git(root, "status", "--porcelain", "--", "knowledge", "tools"):
            cid += "-dirty"
        atomic_write(root / "knowledge/.build_id", cid + "\n")


def render_page(reader, root, rel):
    env = {**os.environ, "TAM_KNOWLEDGE_DIR": str(root / "knowledge")}
    res = subprocess.run([str(reader), "show", "--pipe", rel[:-5]], env=env,
                         capture_output=True, text=True, timeout=15)
    if res.returncode or not res.stdout:
        raise CorpusError(f"{rel}: reader produced no text: {res.stderr.strip()}")
    if not REVISION.search(res.stdout):
        raise CorpusError(f"{rel}: reader missing Rev <N> Last updated <YYYY-MM-DD HH:MM> stamp; check TAM_READER compatibility")
    if "\x1b" in res.stdout:
        raise CorpusError(f"{rel}: piped output contains escape sequences")


def process(root, mode, state, snap, reader=None, full=False):
    builder = load_module("build-index")
    coverage_tool = load_module("report-component-coverage")
    tool_key = fingerprint([SCHEMA, sys.version_info[:2], snap.dependencies])
    if state.get("tools") != tool_key:
        state = {}
    build_key = fingerprint([snap.hashes, snap.signatures, snap.history_key,
                             str(snap.product), snap.components, snap.tldr_present, tool_key])
    if mode in {"index", "build"} and state.get("build_key") == build_key and "outputs" in state:
        outputs = state["outputs"]
        if mode == "index" or "component-coverage.json" in outputs:
            if not stable(root, snap, coverage_tool):
                raise UnstableInputs("corpus inputs changed while processing")
            for rel, text in outputs.items():
                if mode != "index" or rel != "component-coverage.json":
                    atomic_write(root / "knowledge" / rel, text)
            identity(root)
            if not stable(root, snap, coverage_tool):
                raise UnstableInputs("corpus inputs changed while publishing")
            print(f"Corpus: {len(state['pages'])} pages, 0 parsed; indexes current.")
            return state
    if state.get("history_key") == snap.history_key and "history" in state:
        history = state["history"]
    else:
        history = builder.load_all_git_page_revisions(root)
    pages, entries, errors = {}, [], []
    changed = 0
    for rel, text in sorted(snap.texts.items()):
        if not rel.endswith(".html"):
            continue
        old = state.get("pages", {}).get(rel, {})
        if old.get("hash") == snap.hashes[rel]:
            row = old
        else:
            row = page_summary(builder, root, rel, text, history)
            row["hash"] = snap.hashes[rel]
            changed += 1
        # The cached raw entry must keep its authored parent (legacy fallback is derived).
        entry = {key: row["entry"][key] for key in (
            "topic", "path", "title", "summary", "keywords", "levels", "track",
            "parent", "prereqs", "kind", "skills", "paths", "rev", "updated")}
        entry["rev"], entry["updated"] = builder.git_page_revision(
            root, root / "knowledge" / rel, rel, git_cache=history)
        entries.append(entry)
        errors.extend(row["errors"])
        pages[rel] = row
    version = snap.texts.get("version", "").strip()
    if not re.match(r"^\d+\.\d+\.\d+", version):
        errors.append(f"knowledge/version: invalid or missing version {version!r}")
    else:
        builder.check_hierarchy(root / "knowledge", entries, version, errors, pages=pages)
    builder.validate_global_entries(entries, errors)
    parsed = {rel: SimpleNamespace(**row["learning"]) for rel, row in pages.items()}
    learning = builder.learning_metadata(root / "knowledge", entries, errors, parsed=parsed)
    builder.validate_path_pages(root / "knowledge", entries, errors, version,
                                texts=snap.texts)
    if errors:
        raise CorpusError("\n".join("Error: " + error for error in errors))
    outputs = builder.index_outputs(entries, learning)

    # Cache component matches independently of page metadata and inventory paths.
    inventory_key = fingerprint([str(snap.product), snap.components])
    if mode != "index":
        components = snap.components
        ids = [row["id"] for row in components]
        union = re.compile(r"(?<![A-Za-z0-9_.-])(" + "|".join(
            re.escape(i) for i in sorted(ids, key=lambda i: -len(i))) + r")(?![A-Za-z0-9_-])") if ids else None
        for rel, row in pages.items():
            if row.get("inventory") != inventory_key:
                row["mentions"] = sorted(set(union.findall(snap.texts[rel]))) if union else []
                row["inventory"] = inventory_key
        hits = {item["id"]: [rel for rel, row in pages.items() if item["id"] in row["mentions"]]
                for item in components}
        report = coverage_tool.build_report(root, snap.product, pages=[], components=components, comp_hits=hits)
        outputs["component-coverage.json"] = json.dumps(report, indent=2) + "\n"
        if mode == "check" and os.environ.get("SKIP_COVERAGE_CHECK") != "1":
            if report["counts"]["gap"]:
                raise CorpusError("component coverage gaps; run tools/report-component-coverage.py for details")

    tldr = {rel: h for rel, h in snap.hashes.items() if rel.startswith("tldr/pages/") and rel.endswith(".md")}
    tldr_key = fingerprint([tldr, snap.tldr_present])
    if snap.tldr_present:
        if state.get("tldr_key") == tldr_key and "tldr_output" in state:
            tldr_output = state["tldr_output"]
        else:
            module = load_module("build-tldr-index")
            rows = []
            for rel in tldr:
                parts = Path(rel).parts
                if len(parts) == 4:
                    rows.append((Path(rel).stem, parts[2], module.extract_summary(snap.texts[rel]).replace("\t", " ")))
            rows.sort(key=lambda r: (r[0], module.PLATFORM_ORDER.get(r[1], 99), r[1]))
            tldr_output = "name\tplatform\tsummary\n" + "".join("\t".join(row) + "\n" for row in rows)
        outputs["tldr/index.tsv"] = tldr_output
    else:
        tldr_output = ""

    if mode == "check":
        linter = load_module("lint-corpus")
        try:
            rules = linter.load_rules(root)
        except linter.SourceError as error:
            raise CorpusError(f"lint-corpus: {error}") from error
        findings = []
        available = set(snap.signatures) | set(outputs) | {
            rel for rel in OUTPUTS if (root / "knowledge" / rel).is_file()}
        for rel, row in pages.items():
            if "lint" not in row:
                row["lint"] = linter.Page("knowledge/" + rel, snap.texts[rel], rules).check()
            findings.extend(row["lint"])
            for href in row["links"]:
                if any(href.startswith(prefix) or prefix in href
                       for prefix in ("://", "mailto:", "#", "man:", "tam:", "info:", "tldr:")):
                    continue
                if href.startswith("/") or ".." in href or href.split("?")[0] not in available:
                    findings.append(("knowledge/" + rel, 0, "LINK", "broken internal link: " + href))
        if findings:
            raise CorpusError("\n".join(f"{p}:{line}: {rule}: {msg}" for p, line, rule, msg in sorted(set(map(tuple, findings)))))

    if not stable(root, snap, coverage_tool):
        raise UnstableInputs("corpus inputs changed while processing")
    for rel, text in outputs.items():
        atomic_write(root / "knowledge" / rel, text)
    identity(root)

    next_state = {"schema": SCHEMA, "tools": tool_key, "head": snap.head,
                  "history": history, "history_key": snap.history_key, "pages": pages, "tldr_key": tldr_key,
                  "tldr_output": tldr_output, "behavior": state.get("behavior"),
                  "build_key": build_key, "outputs": outputs}
    rendered = 0
    if mode == "check":
        reader = reader_path(root, reader)
        reader_key = fingerprint([str(reader), digest(reader.read_bytes()),
                                  {k: v for k, v in os.environ.items() if k.startswith(("TAM_", "XDG_", "LC_")) or k in ("LANG", "TERM", "HOME")}, version])
        pending = []
        for entry in entries:
            rel = entry["path"]
            key = fingerprint([reader_key, pages[rel]["hash"], entry])
            if pages[rel].get("render") != key:
                pending.append((rel, key))
        with ThreadPoolExecutor(max_workers=min(8, os.cpu_count() or 1)) as pool:
            list(pool.map(lambda item: render_page(reader, root, item[0]), pending))
        for rel, key in pending:
            pages[rel]["render"] = key
        rendered = len(pending)
        # The remaining guide assertions are cheap global behavior checks. Reuse
        # their result only when *every* authored input, reader and output matches.
        behavior_key = fingerprint([reader_key, snap.hashes, sorted(snap.signatures), outputs,
                                    (root / "knowledge/.build_id").read_text() if (root / "knowledge/.build_id").is_file() else ""])
        if state.get("behavior") != behavior_key:
            script = root / "tests/guide.sh"
            if not script.is_file():
                raise CorpusError("tests/guide.sh is required for corpus rendering checks")
            result = subprocess.run(["sh", str(script), str(reader)], cwd=root,
                                    env={**os.environ, "TAM_GUIDE_SKIP_PAGES": "1"},
                                    capture_output=True, text=True, timeout=30)
            if result.returncode:
                raise CorpusError(result.stdout + result.stderr)
        next_state["behavior"] = behavior_key
    if not stable(root, snap, coverage_tool):
        raise UnstableInputs("corpus inputs changed while checking")
    print(f"Corpus: {len(pages)} pages, {changed} parsed" +
          (f", {rendered} rendered; checks passed." if mode == "check" else "; indexes current."))
    if learning["warnings"]:
        print(f"Learning warnings: {len(learning['warnings'])} level sections need migration; see coverage.json.")
    return next_state


def run(root=None, mode="build", reader=None):
    root = Path(root).resolve() if root else Path(__file__).resolve().parent.parent
    try:
        with cache_lock(root) as path:
            enabled = os.environ.get("CORPUS_CACHE", "1") != "0"
            state = read_cache(path) if enabled else {}
            for attempt in range(2):
                try:
                    coverage = load_module("report-component-coverage")
                    snap = snapshot(root, coverage)
                    previous = state
                    state = process(root, mode, state, snap, reader)
                    if enabled and state is not previous:
                        save_cache(path, state)
                    return 0
                except UnstableInputs:
                    state = {}
                    if attempt:
                        raise CorpusError("inputs kept changing; save your files and retry")
    except (CorpusError, OSError, subprocess.TimeoutExpired, ValueError) as error:
        print(f"corpus: {error}", file=sys.stderr)
        return 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("build", "check", "preview", "identity"))
    parser.add_argument("--root", type=Path)
    parser.add_argument("--reader")
    parser.add_argument("--page", default="")
    args = parser.parse_args()
    root = (args.root or Path(__file__).resolve().parent.parent).resolve()
    if args.mode == "identity":
        with cache_lock(root):
            identity(root)
        return 0
    status = run(root, "build" if args.mode == "preview" else args.mode, args.reader)
    if status or args.mode != "preview":
        return status
    try:
        reader = reader_path(root, args.reader)
        return subprocess.call([str(reader), "show", *([args.page] if args.page else [])],
                               env={**os.environ, "TAM_KNOWLEDGE_DIR": str(root / "knowledge")})
    except (CorpusError, OSError) as error:
        print(f"corpus: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
