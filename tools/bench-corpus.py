#!/usr/bin/env python3
"""Measure actual corpus commands in disposable fixtures; never edit authored pages."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import statistics
import subprocess
import sys
import tempfile
import time

sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("corpus_pipeline", Path(__file__).with_name("corpus_pipeline.py"))
pipeline = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pipeline)


def benchmark(source, reader, repeats=5):
    results = {}
    with tempfile.TemporaryDirectory(prefix="tam-corpus-bench-") as temporary:
        root = Path(temporary) / "tam"
        root.mkdir()
        for directory in ("knowledge", "tools"):
            shutil.copytree(source / directory, root / directory, ignore=shutil.ignore_patterns("__pycache__"))
        (root / "src").mkdir()
        for name in ("render.c", "tam.c", "install.c"):
            shutil.copyfile(source / "src" / name, root / "src" / name)
        (root / "tests").mkdir()
        shutil.copyfile(source / "tests/guide.sh", root / "tests/guide.sh")
        shutil.copyfile(source / "Makefile", root / "Makefile")
        (root / ".gitignore").write_text("build/\n__pycache__/\nknowledge/.build_id\n")
        # A local fixture history permits real page and index follow-up commits.
        env = {**os.environ, "TAM_READER": str(reader), "CORPUS_CACHE": "1",
               "MAKEFLAGS": "", "MFLAGS": "", "SKIP_COVERAGE_CHECK": "1",
               "TAMLINUX_ROOT": str(Path(temporary) / "product"),
               "GIT_CONFIG_NOSYSTEM": "1", "GIT_CONFIG_GLOBAL": os.devnull,
               "GIT_AUTHOR_NAME": "Corpus benchmark", "GIT_AUTHOR_EMAIL": "bench@example.invalid",
               "GIT_COMMITTER_NAME": "Corpus benchmark", "GIT_COMMITTER_EMAIL": "bench@example.invalid"}
        coverage = pipeline.load_module("report-component-coverage")
        product = coverage.resolve_tamlinux(source, None)
        if product:
            for item in coverage.inventory(product):
                p = Path(env["TAMLINUX_ROOT"]) / item["source"]
                if item["kind"] == "bindings":
                    p.parent.mkdir(parents=True, exist_ok=True)
                    p.touch()
                else:
                    p.mkdir(parents=True, exist_ok=True)

        def command(*args):
            start = time.perf_counter()
            res = subprocess.run(args, cwd=root, env=env, capture_output=True, text=True, timeout=60)
            seconds = time.perf_counter() - start
            if res.returncode:
                raise RuntimeError(res.stdout + res.stderr)
            return seconds

        def git(*args):
            return command("git", "-c", "core.hooksPath=/dev/null", "-c", "commit.gpgsign=false", *args)

        def measure(name, fn):
            times = [fn(i) for i in range(repeats)]
            results[name] = {"median": round(statistics.median(times), 4),
                             "slowest": round(max(times), 4), "runs": repeats}
            print(f"{name:36} median {statistics.median(times):.3f}s; slowest {max(times):.3f}s", flush=True)

        git("init", "-q", "-b", "develop")
        git("add", "--", ".gitignore", "Makefile", "tools", "src", "tests", "knowledge")
        git("commit", "-qm", "Fixture baseline")
        pages = sorted((root / "knowledge").rglob("*.html"))
        print(f"Corpus benchmark: {len(pages)} HTML pages; {repeats} repetitions; temporary fixture history.", flush=True)

        def cold(_):
            shutil.rmtree(root / "build/corpus", ignore_errors=True)
            return command("make", "-s", "corpus-check")
        measure("Cold build and content checks", cold)
        measure("Unchanged build", lambda _: command("make", "-s", "corpus"))

        def changed(count, i):
            for p in pages[:count]:
                with p.open("a") as stream:
                    stream.write(f"\n<!-- corpus benchmark {count}:{i} -->\n")
            return command("make", "-s", "corpus-check")
        measure("One edited page and checks", lambda i: changed(1, i))
        measure("100 edited pages and checks", lambda i: changed(min(100, len(pages)), i))
        measure("All edited pages and checks", lambda i: changed(len(pages), i))

        def finish(i):
            for p in pages[:100]:
                with p.open("a") as stream:
                    stream.write(f"\n<!-- finish batch {i} -->\n")
            start = time.perf_counter()
            command("make", "-s", "corpus-check")
            git("add", "--", "knowledge")
            git("commit", "-qm", "Edited corpus batch")
            command("make", "-s", "corpus")
            git("add", "--", "knowledge")
            git("commit", "-qm", "Refresh corpus metadata")
            command("make", "-s", "corpus-identity")
            command("make", "-s", "install-corpus", "DESTDIR=" + str(Path(temporary) / "stage"), "PREFIX=/usr")
            return time.perf_counter() - start
        measure("Batch check/commits/staged install", finish)
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reader")
    parser.add_argument("--repeat", type=int, default=5)
    args = parser.parse_args()
    if args.repeat < 1:
        parser.error("--repeat must be positive")
    root = Path(__file__).resolve().parent.parent
    try:
        results = benchmark(root, pipeline.reader_path(root, args.reader), args.repeat)
    except (OSError, RuntimeError, pipeline.CorpusError, subprocess.TimeoutExpired) as error:
        print(f"bench-corpus: {error}", file=sys.stderr)
        return 1
    print("Finishing measurement excludes interactive prompts, claims, version editing, repository checks and network.")
    targets = {"Cold build and content checks": 3, "Unchanged build": .25,
               "One edited page and checks": 1, "100 edited pages and checks": 2}
    missed = [name for name, target in targets.items() if results[name]["median"] > target]
    print("Performance targets: " + ("OPEN: " + ", ".join(missed) if missed else "all met"))
    return 1 if missed else 0


if __name__ == "__main__":
    sys.exit(main())
