"""Incremental results must agree with a fresh build, including failures."""
import contextlib
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import sys
import unittest
from unittest.mock import patch

from tests.test_build_index import page, INDEXES

REPO = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("pipeline", REPO / "tools/corpus_pipeline.py")
pipeline = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pipeline)


class CorpusPipeline(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name) / "tam"
        self.root.mkdir()
        shutil.copytree(REPO / "tools", self.root / "tools", ignore=shutil.ignore_patterns("__pycache__"))
        (self.root / "src").mkdir()
        for name in ("render.c", "tam.c", "install.c"):
            shutil.copyfile(REPO / "src" / name, self.root / "src" / name)
        knowledge = self.root / "knowledge"
        (knowledge / "use").mkdir(parents=True)
        (knowledge / "index.html").write_text(page("Guide"))
        (knowledge / "use/index.html").write_text(page("Using", "use", "index.html"))
        (knowledge / "version").write_text("0.39.0\n")
        (self.root / "tests").mkdir()
        (self.root / "tests/guide.sh").write_text('#!/bin/sh\necho check >> build/global-count\n')
        self.reader = self.root / "reader"
        self.reader.write_text('#!/bin/sh\necho render >> "$(dirname "$0")/build/render-count"\nprintf "Page\\nRev 1 Last updated 2026-11-11 12:00\\n"\n')
        self.reader.chmod(0o755)
        self.env = patch.dict(os.environ, {"TAMLINUX_ROOT": str(Path(self.tmp.name) / "none"),
                                          "SKIP_COVERAGE_CHECK": "1", "CORPUS_CACHE": "1"})
        self.env.start()
        self.addCleanup(self.env.stop)

    def run_pipeline(self, mode="build", cached=True):
        out, err = io.StringIO(), io.StringIO()
        with patch.dict(os.environ, {"CORPUS_CACHE": "1" if cached else "0"}), contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = pipeline.run(self.root, mode, str(self.reader))
        return rc, out.getvalue(), err.getvalue()

    def ok(self, mode="build", cached=True):
        rc, out, err = self.run_pipeline(mode, cached)
        self.assertEqual(rc, 0, out + err)
        return out

    def outputs(self):
        return {p: (self.root / "knowledge" / p).read_bytes() for p in (*INDEXES, "component-coverage.json")}

    def edit(self, rel, before="</body>", after="<p>Changed</p></body>"):
        p = self.root / "knowledge" / rel
        p.write_text(p.read_text().replace(before, after))

    def test_cached_uncached_and_reference_outputs_match_and_noop_preserves_mtimes(self):
        self.ok("check")
        baseline = self.outputs()
        times = {p: (self.root / "knowledge" / p).stat().st_mtime_ns for p in baseline}
        self.assertIn("0 parsed, 0 rendered", self.ok("check"))
        self.assertEqual(times, {p: (self.root / "knowledge" / p).stat().st_mtime_ns for p in baseline})
        self.ok("check", cached=False)
        self.assertEqual(baseline, self.outputs())
        builder = pipeline.load_module("build-index")
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(builder.reference_main(self.root), 0)
        self.assertEqual(baseline, self.outputs())
        coverage = pipeline.load_module("report-component-coverage")
        self.assertEqual(json.loads(baseline["component-coverage.json"]), coverage.build_report(self.root, None))

    def test_restored_mtime_does_not_hide_a_content_edit(self):
        self.ok("check")
        p = self.root / "knowledge/use/index.html"
        st = p.stat()
        self.edit("use/index.html", "Using", "Usage")
        os.utime(p, ns=(st.st_atime_ns, st.st_mtime_ns))
        self.assertIn("1 parsed, 1 rendered", self.ok("check"))
        cached = self.outputs()
        self.ok("check", cached=False)
        self.assertEqual(cached, self.outputs())

    def test_invalid_metadata_and_lint_failures_are_not_cached_as_success(self):
        self.ok("check")
        baseline = self.outputs()
        self.edit("use/index.html", 'content="use"', 'content="bogus"')
        for cached in (True, True, False):
            rc, _, err = self.run_pipeline("check", cached)
            self.assertEqual(rc, 1)
            self.assertIn("unknown track", err)
        self.assertEqual(baseline, self.outputs())
        self.edit("use/index.html", 'content="bogus"', 'content="use"')
        self.edit("use/index.html", "</body>", "<p>the SUPER key</p></body>")
        rc, _, err = self.run_pipeline("check")
        self.assertEqual(rc, 1)
        self.assertIn("SUPER", err)

    def test_deletions_renames_and_broken_links_are_detected_globally(self):
        self.edit("index.html", "</body>", '<a href="use/index.html">Using</a></body>')
        self.ok("check")
        p = self.root / "knowledge/use/index.html"
        p.rename(p.with_name("moved.html"))
        rc, _, err = self.run_pipeline("check")
        self.assertEqual(rc, 1)
        self.assertIn("broken internal link", err)
        self.edit("index.html", "use/index.html", "use/moved.html")
        self.ok("check")
        p.with_name("moved.html").unlink()
        self.edit("index.html", '<a href="use/moved.html">Using</a>', "")
        self.ok("check")
        self.assertNotIn("use/moved.html", (self.root / "knowledge/index.json").read_text())

    def test_link_check_preserves_guide_rules_and_checks_optional_generated_targets(self):
        self.edit("index.html", "</body>", '<a href="use/index.html#section">Anchor</a></body>')
        self.ok("check")
        self.edit("index.html", "</body>", '<a href="tldr/index.tsv">Examples</a></body>')
        self.assertEqual(self.run_pipeline("check")[0], 1)
        (self.root / "knowledge/tldr/pages").mkdir(parents=True)
        self.ok("check")

    def test_registry_changes_revalidate_unchanged_pages(self):
        self.ok()
        registry = self.root / "knowledge/learn/skills/skills.json"
        registry.parent.mkdir(parents=True)
        registry.write_text('{"version": 2}')
        for cached in (True, False):
            rc, _, err = self.run_pipeline(cached=cached)
            self.assertEqual(rc, 1)
            self.assertIn("skills.json", err)

    def test_tools_rules_reader_and_inventory_changes_invalidate(self):
        self.ok("check")
        # A rule change must rerun the linter even though no HTML changed.
        rules = self.root / "src/render.c"
        rules.write_text(rules.read_text().replace('{"amp",', '{"amp-changed",'))
        self.assertIn("2 parsed", self.ok("check"))
        self.reader.write_text(self.reader.read_text() + "# different reader\n")
        self.assertIn("2 rendered", self.ok("check"))
        product = Path(self.tmp.name) / "product"
        (product / "commands/new-command").mkdir(parents=True)
        with patch.dict(os.environ, {"TAMLINUX_ROOT": str(product)}):
            self.ok()
        report = json.loads((self.root / "knowledge/component-coverage.json").read_text())
        self.assertEqual(report["counts"]["gap"], 1)
        with patch.dict(os.environ, {"TAMLINUX_ROOT": str(product), "SKIP_COVERAGE_CHECK": ""}):
            self.assertEqual(self.run_pipeline("check")[0], 1)

    def test_tldr_inputs_are_independent_and_missing_outputs_are_restored(self):
        md = self.root / "knowledge/tldr/pages/common/demo.md"
        md.parent.mkdir(parents=True)
        md.write_text("# demo\n> First summary.\n")
        self.ok()
        index = self.root / "knowledge/tldr/index.tsv"
        first = index.read_bytes()
        self.edit("use/index.html")
        self.ok()
        self.assertEqual(first, index.read_bytes())
        md.write_text("# demo\n> Second summary.\n")
        self.ok()
        self.assertIn("Second summary.", index.read_text())
        (self.root / "knowledge/index.tsv").write_text("corrupt\n")
        index.unlink()
        self.ok()
        self.assertIn("topic\tpath", (self.root / "knowledge/index.tsv").read_text())
        self.assertIn("Second summary.", index.read_text())

    def test_corrupt_cache_and_concurrent_processes_recover(self):
        self.ok()
        cache = self.root / "build/corpus/state.json"
        for broken in ("{", "[]", '{"payload":"{}","checksum":"wrong"}'):
            cache.write_text(broken)
            self.assertIn("2 parsed", self.ok())
        cmd = [sys.executable, "-B", str(self.root / "tools/corpus_pipeline.py"), "build", "--root", str(self.root)]
        processes = [subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True) for _ in range(3)]
        for process in processes:
            out, err = process.communicate(timeout=15)
            self.assertEqual(process.returncode, 0, out + err)
        self.ok()

    def test_history_changes_at_same_head_refresh_page_revisions(self):
        def git(*args):
            res = subprocess.run(["git", "-c", "core.hooksPath=/dev/null", "-c", "commit.gpgsign=false",
                                  "-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid", *args],
                                 cwd=self.root, capture_output=True, text=True, timeout=10)
            self.assertEqual(res.returncode, 0, res.stderr)
            return res.stdout.strip()
        git("init", "-q")
        git("add", "knowledge")
        git("commit", "-qm", "Initial pages")
        self.edit("use/index.html")
        git("add", "knowledge/use/index.html")
        git("commit", "-qm", "Page edit")
        self.ok()
        def revision():
            return next(e["rev"] for e in json.loads((self.root / "knowledge/index.json").read_text())
                        if e["path"] == "use/index.html")
        self.assertEqual(revision(), 2)
        head = git("rev-parse", "HEAD")
        (self.root / ".git/info/grafts").write_text(head + "\n")
        self.ok()
        self.assertEqual(git("rev-parse", "HEAD"), head)
        self.assertEqual(revision(), 1)

    def test_input_change_during_processing_retries_then_fails_plainly(self):
        coverage = pipeline.load_module("report-component-coverage")
        snap = pipeline.snapshot(self.root, coverage)
        self.edit("use/index.html")
        self.assertFalse(pipeline.stable(self.root, snap, coverage))
        with patch.object(pipeline, "stable", side_effect=[False, False]):
            rc, _, err = self.run_pipeline()
        self.assertEqual(rc, 1)
        self.assertIn("kept changing", err)

    def test_make_corpus_does_not_require_compiler_or_pkg_config(self):
        shutil.copyfile(REPO / "Makefile", self.root / "Makefile")
        result = subprocess.run(["make", "-s", "corpus", "CC=/missing-compiler", "PKG_CONFIG=/missing-pkg-config"],
                                cwd=self.root, capture_output=True, text=True, timeout=15,
                                env={**os.environ, "MAKEFLAGS": "", "MFLAGS": ""})
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn("missing-pkg-config", result.stderr)
        self.assertFalse((self.root / "build/tam").exists())

    def test_missing_rule_sources_fail_without_traceback(self):
        self.ok("check")
        (self.root / "src/render.c").unlink()
        rc, _, err = self.run_pipeline("check")
        self.assertEqual(rc, 1)
        self.assertIn("src/render.c", err)

    def test_empty_tldr_directory_is_an_input(self):
        self.ok()
        (self.root / "knowledge/tldr/pages").mkdir(parents=True)
        self.ok()
        self.assertEqual((self.root / "knowledge/tldr/index.tsv").read_text(), "name\tplatform\tsummary\n")

    def test_missing_and_failing_readers_are_actionable(self):
        with patch.dict(os.environ, {"TAM_READER": str(self.root / "missing")}):
            with self.assertRaisesRegex(pipeline.CorpusError, "reader"):
                pipeline.reader_path(self.root)
        self.reader.write_text("#!/bin/sh\nexit 1\n")
        rc, _, err = self.run_pipeline("check")
        self.assertEqual(rc, 1)
        self.assertIn("reader produced no text", err)


if __name__ == "__main__":
    unittest.main()
