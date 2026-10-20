#!/usr/bin/env python3
"""build-tldr-index.py: Build knowledge/tldr/index.tsv from knowledge/tldr/pages/.

Each tldr Markdown page under pages/<platform>/<name>.md starts with:
  # command-name
  > One-line summary...
  > More information: <...>.

This script extracts (name, platform, summary) sorted deterministically by
command name and platform (linux before common) and writes index.tsv with a
header row:
  name\tplatform\tsummary
"""

import argparse
from pathlib import Path
import re
import sys

PLATFORM_ORDER = {"linux": 0, "common": 1}


def write_if_changed(path: Path, content: str) -> None:
    data = content.encode("utf-8")
    if not path.exists() or path.read_bytes() != data:
        path.write_bytes(data)


def extract_summary(md_text: str) -> str:
    """Extract the descriptive blockquote lines from a tldr Markdown page."""
    parts = []
    for raw_line in md_text.splitlines():
        line = raw_line.strip()
        if not line.startswith(">"):
            if parts:
                break
            continue
        body = line[1:].strip()
        if not body:
            continue
        low = body.lower()
        if low.startswith("more information:") or low.startswith("see also:"):
            continue
        # Strip inline markdown backticks and collapse whitespace
        body = body.replace("`", "")
        body = re.sub(r"\{\{\[([^|\]]+)\|([^\]]+)\]\}\}", r"[\1|\2]", body)
        body = body.replace("{{", "").replace("}}", "")
        body = re.sub(r"\s+", " ", body).strip()
        if body:
            parts.append(body)
    return " ".join(parts) if parts else "Command examples."


def build_tldr_index(tldr_dir: Path) -> int:
    pages_dir = tldr_dir / "pages"
    if not pages_dir.is_dir():
        return 0

    rows = []
    for plat_dir in sorted(pages_dir.iterdir(), key=lambda p: (PLATFORM_ORDER.get(p.name, 99), p.name)):
        if not plat_dir.is_dir():
            continue
        platform = plat_dir.name
        for md_file in sorted(plat_dir.glob("*.md"), key=lambda p: p.stem):
            name = md_file.stem
            text = md_file.read_text(encoding="utf-8", errors="replace")
            summary = extract_summary(text).replace("\t", " ")
            rows.append((name, platform, summary))

    rows.sort(key=lambda r: (r[0], PLATFORM_ORDER.get(r[1], 99), r[1]))

    lines = ["name\tplatform\tsummary\n"]
    for name, platform, summary in rows:
        lines.append(f"{name}\t{platform}\t{summary}\n")

    write_if_changed(tldr_dir / "index.tsv", "".join(lines))
    return len(rows)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--tldr-dir",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "knowledge" / "tldr",
        help="Path to the tldr directory containing pages/",
    )
    args = parser.parse_args()
    if not (args.tldr_dir / "pages").is_dir():
        print(f"Error: pages directory not found under {args.tldr_dir}", file=sys.stderr)
        return 1
    count = build_tldr_index(args.tldr_dir)
    print(f"Generated {args.tldr_dir / 'index.tsv'} with {count} tldr page entries.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
