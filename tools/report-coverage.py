#!/usr/bin/env python3
"""Print the generated learning coverage/gap report; rebuild with make corpus."""

import argparse
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parent.parent)
    parser.add_argument("--warnings", action="store_true", help="list anatomy warnings")
    args = parser.parse_args()
    path = args.root / "knowledge/coverage.json"
    try:
        report = json.loads(path.read_text())
    except (OSError, ValueError) as error:
        parser.exit(1, f"{path}: {error}; run make corpus first\n")
    print("Skill cell                     Pages Paths  Gaps")
    for cell in report["cells"]:
        gaps = ", ".join(name for name, key in (("pages", "page_gap"), ("paths", "path_gap")) if cell[key]) or "none"
        print(f"{cell['id']:<30} {len(cell['pages']):>5} {len(cell['paths']):>5}  {gaps}")
    print(f"\nUnclassified pages: {len(report['unclassified_pages'])}; "
          f"unmapped pages: {len(report['unmapped_pages'])}; "
          f"anatomy warnings: {len(report['warnings'])}")
    if args.warnings:
        for warning in report["warnings"]:
            print(f"{warning['path']} [{warning['level']}]: missing " + ", ".join(warning["missing"]))


if __name__ == "__main__":
    main()
