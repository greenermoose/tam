#!/usr/bin/env python3
"""timer.py: Measure and display execution time for build chain commands."""

import subprocess
import sys
import time
from pathlib import Path

TIMING_FILE = Path("build/.timings")


def run_command(label, cmd):
    cmd_str = " ".join(cmd)
    print(f"\n==> [start] {label}: {cmd_str}")
    sys.stdout.flush()
    t0 = time.perf_counter()
    res = subprocess.run(cmd)
    dt = time.perf_counter() - t0
    status = "done" if res.returncode == 0 else f"failed ({res.returncode})"
    print(f"==> [{status}]  {label} ({dt:.2f}s)\n")
    sys.stdout.flush()
    try:
        TIMING_FILE.parent.mkdir(parents=True, exist_ok=True)
        with TIMING_FILE.open("a", encoding="utf-8") as f:
            f.write(f"{label}\t{dt:.2f}\t{cmd_str}\n")
    except OSError:
        pass
    sys.exit(res.returncode)


def print_summary():
    if not TIMING_FILE.is_file():
        return
    try:
        lines = TIMING_FILE.read_text(encoding="utf-8").splitlines()
    except OSError:
        return
    if not lines:
        return
    rows = []
    total = 0.0
    for line in lines:
        parts = line.split("\t")
        if len(parts) >= 2:
            label = parts[0]
            try:
                sec = float(parts[1])
                total += sec
                rows.append((label, sec))
            except ValueError:
                pass
    if not rows:
        return
    max_label_len = max(len(r[0]) for r in rows)
    max_label_len = max(max_label_len, 35)
    width = max_label_len + 12
    print("\n" + "=" * width)
    header = "Build & Test Timing Summary"
    print(f"{header:^{width}}")
    print("=" * width)
    for label, sec in rows:
        print(f"  {label:<{max_label_len}}  {sec:>6.2f}s")
    print("-" * width)
    print(f"  {'Total':<{max_label_len}}  {total:>6.2f}s")
    print("=" * width + "\n")
    sys.stdout.flush()


def reset_timer():
    if TIMING_FILE.is_file():
        try:
            TIMING_FILE.unlink()
        except OSError:
            pass


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} run <label> <cmd...> | summary | reset")
        sys.exit(1)
    action = sys.argv[1]
    if action == "run":
        if len(sys.argv) < 4:
            print(f"Usage: {sys.argv[0]} run <label> <cmd...>")
            sys.exit(1)
        run_command(sys.argv[2], sys.argv[3:])
    elif action == "summary":
        print_summary()
    elif action == "reset":
        reset_timer()
    else:
        print(f"Unknown action: {action}")
        sys.exit(1)


if __name__ == "__main__":
    main()
