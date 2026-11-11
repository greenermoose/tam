#!/usr/bin/env python3
"""Map Tamlinux product components to corpus page mentions (plan 26 W1).

Inventory (sibling product tree, override with TAMLINUX_ROOT):
  - tamlinux/commands/<name>/          → kind command
  - tamlinux/desktop/plugins/fred.*/   → kind plugin
  - tamlinux/desktop/.../bindings/*.lua → kind bindings (one row per file)

A component is covered when any knowledge/*.html page contains its id as a
whole word. Quality of teaching is out of scope; see corpus-style.md.

Exemptions: tools/component-coverage-exemptions.tsv (id, kind, reason, date).

Default: print a human table. --json writes knowledge/component-coverage.json.
--check exits 1 when any non-exempt component has no page mention (not wired
into make corpus-and-test until a first exemption baseline exists).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path


KINDS = ("command", "plugin", "bindings")


def tam_root(args_root: Path) -> Path:
    return args_root.resolve()


def resolve_tamlinux(tam: Path, explicit: Path | None) -> Path | None:
    if explicit is not None:
        return explicit.resolve() if explicit.is_dir() else None
    env = os.environ.get("TAMLINUX_ROOT")
    if env:
        path = Path(env)
        return path.resolve() if path.is_dir() else None
    sibling = tam.parent / "tamlinux"
    return sibling.resolve() if sibling.is_dir() else None


def inventory(product: Path) -> list[dict]:
    rows: list[dict] = []
    commands = product / "commands"
    if commands.is_dir():
        for path in sorted(commands.iterdir()):
            if path.is_dir() and not path.name.startswith("."):
                rows.append({
                    "id": path.name,
                    "kind": "command",
                    "source": os.path.relpath(path, product),
                })
    plugins = product / "desktop" / "plugins"
    if plugins.is_dir():
        for path in sorted(plugins.glob("fred.*")):
            if path.is_dir():
                rows.append({
                    "id": path.name,
                    "kind": "plugin",
                    "source": os.path.relpath(path, product),
                })
    bindings = (
        product / "desktop" / "session" / "hypr" / "tamlinux" / "bindings"
    )
    if bindings.is_dir():
        for path in sorted(bindings.glob("*.lua")):
            rows.append({
                "id": path.name,
                "kind": "bindings",
                "source": os.path.relpath(path, product),
            })
    return rows


def load_exemptions(path: Path) -> dict[str, dict]:
    if not path.is_file():
        return {}
    out: dict[str, dict] = {}
    for line_no, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) < 4:
            raise SystemExit(
                f"{path}:{line_no}: need id, kind, reason, date (tab-separated)"
            )
        ident, kind, reason, date = parts[0], parts[1], parts[2], parts[3]
        if kind not in KINDS:
            raise SystemExit(f"{path}:{line_no}: kind must be one of {KINDS}")
        out[ident] = {"id": ident, "kind": kind, "reason": reason, "date": date}
    return out


def load_corpus_pages(knowledge: Path) -> list[tuple[str, str]]:
    pages: list[tuple[str, str]] = []
    for path in sorted(knowledge.rglob("*.html")):
        try:
            pages.append(
                (str(path.relative_to(knowledge)), path.read_text(encoding="utf-8", errors="replace"))
            )
        except OSError:
            continue
    return pages


def match_components(pages: list[tuple[str, str]], components: list[dict]) -> dict[str, list[str]]:
    if not components or not pages:
        return {c["id"]: [] for c in components}
    sorted_ids = sorted([c["id"] for c in components], key=lambda x: -len(x))
    pattern = re.compile(
        r"(?<![A-Za-z0-9_.-])(" + "|".join(re.escape(i) for i in sorted_ids) + r")(?![A-Za-z0-9_-])"
    )
    hits: dict[str, list[str]] = {c["id"]: [] for c in components}
    for rel, text in pages:
        matches = set(pattern.findall(text))
        for m in matches:
            if m in hits:
                hits[m].append(rel)
    return hits


def corpus_blob(knowledge: Path) -> str:
    parts: list[str] = []
    for path in sorted(knowledge.rglob("*.html")):
        try:
            parts.append(path.read_text(encoding="utf-8", errors="replace"))
        except OSError:
            continue
    return "\n".join(parts)


def pages_mentioning(knowledge: Path, ident: str, blob: str | None = None) -> list[str]:
    """Return knowledge-relative paths that contain ident as a whole word."""
    pattern = re.compile(r"(?<![A-Za-z0-9_.-])" + re.escape(ident) + r"(?![A-Za-z0-9_-])")
    hits: list[str] = []
    if blob is not None and not pattern.search(blob):
        return hits
    for path in sorted(knowledge.rglob("*.html")):
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        if pattern.search(text):
            hits.append(str(path.relative_to(knowledge)))
    return hits


def build_report(tam: Path, product: Path | None, pages=None, components=None, comp_hits=None) -> dict:
    exemptions_path = tam / "tools" / "component-coverage-exemptions.tsv"
    exemptions = load_exemptions(exemptions_path)
    knowledge = tam / "knowledge"
    if components is None:
        components = inventory(product) if product is not None else []
    if pages is None:
        pages = load_corpus_pages(knowledge) if components else []
    if comp_hits is None:
        comp_hits = match_components(pages, components)
    rows = []
    for item in components:
        matched_pages = comp_hits.get(item["id"], [])
        exempt = exemptions.get(item["id"])
        if matched_pages:
            status = "covered"
        elif exempt:
            status = "exempt"
        else:
            status = "gap"
        row = {
            **item,
            "pages": matched_pages,
            "status": status,
        }
        if exempt:
            row["exemption"] = {
                "reason": exempt["reason"],
                "date": exempt["date"],
            }
        rows.append(row)
    unknown_exemptions = sorted(
        ident for ident in exemptions if ident not in {r["id"] for r in components}
    )
    return {
        "version": 1,
        "product_root": str(product) if product else None,
        "exemptions_file": str(exemptions_path.relative_to(tam)),
        "counts": {
            "components": len(rows),
            "covered": sum(1 for r in rows if r["status"] == "covered"),
            "exempt": sum(1 for r in rows if r["status"] == "exempt"),
            "gap": sum(1 for r in rows if r["status"] == "gap"),
            "unknown_exemptions": len(unknown_exemptions),
        },
        "unknown_exemptions": unknown_exemptions,
        "components": rows,
    }


def print_table(report: dict) -> None:
    if report["product_root"] is None:
        print("No tamlinux product tree found (set TAMLINUX_ROOT or keep ../tamlinux).")
        print("Skipped component inventory.")
        return
    print(f"Product: {report['product_root']}")
    print(
        f"Components: {report['counts']['components']}  "
        f"covered: {report['counts']['covered']}  "
        f"exempt: {report['counts']['exempt']}  "
        f"gap: {report['counts']['gap']}"
    )
    print(f"{'status':<8} {'kind':<9} {'id':<40} pages")
    for row in report["components"]:
        page_note = ", ".join(row["pages"][:3])
        if len(row["pages"]) > 3:
            page_note += f" (+{len(row['pages']) - 3})"
        if row["status"] == "exempt":
            page_note = row.get("exemption", {}).get("reason", "exempt")
        elif row["status"] == "gap":
            page_note = "—"
        print(f"{row['status']:<8} {row['kind']:<9} {row['id']:<40} {page_note}")
    if report["unknown_exemptions"]:
        print("Unknown exemption ids (not in inventory):")
        for ident in report["unknown_exemptions"]:
            print(f"  {ident}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parent.parent,
        help="tam repository root",
    )
    parser.add_argument(
        "--tamlinux",
        type=Path,
        default=None,
        help="tamlinux product root (default: TAMLINUX_ROOT or ../tamlinux)",
    )
    parser.add_argument(
        "--json",
        nargs="?",
        const="knowledge/component-coverage.json",
        metavar="PATH",
        help="write JSON report (default path: knowledge/component-coverage.json)",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="exit 1 if any non-exempt component has no page mention",
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="suppress the human table",
    )
    args = parser.parse_args(argv)
    tam = tam_root(args.root)
    product = resolve_tamlinux(tam, args.tamlinux)
    report = build_report(tam, product)
    if not args.quiet:
        print_table(report)
    if args.json is not None:
        out = Path(args.json)
        if not out.is_absolute():
            out = tam / out
        out.parent.mkdir(parents=True, exist_ok=True)
        text = json.dumps(report, indent=2) + "\n"
        if not out.is_file() or out.read_text(encoding="utf-8") != text:
            out.write_text(text, encoding="utf-8")
            if not args.quiet:
                print(f"Wrote {out.relative_to(tam)}")
    gaps = [r for r in report["components"] if r["status"] == "gap"]
    if args.check and gaps:
        print(f"--check: {len(gaps)} component(s) uncovered", file=sys.stderr)
        return 1
    if args.tamlinux is not None and report["product_root"] is None:
        print(f"--tamlinux: {args.tamlinux} is not a directory", file=sys.stderr)
        return 1
    if args.check and report["product_root"] is None:
        if not args.quiet:
            print("--check: no product tree; skipped check", file=sys.stderr)
        return 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
