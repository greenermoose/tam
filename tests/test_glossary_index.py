"""Tests for glossary index generation and misplaced-page detection."""
import importlib.util
import io
from pathlib import Path
import re
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

from tests.test_build_index import page

REPO = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("glossary_index", REPO / "tools/glossary_index.py")
glossary_index = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(glossary_index)

PIPE_SPEC = importlib.util.spec_from_file_location("pipeline", REPO / "tools/corpus_pipeline.py")
pipeline = importlib.util.module_from_spec(PIPE_SPEC)
PIPE_SPEC.loader.exec_module(pipeline)


def term_page(title, summary, letter, parent=None):
    parent_path = parent or f"glossary/{letter}/index.html"
    slug = re.sub(r"[^a-z0-9]+", "-", title.lower()).strip("-")
    return (
        "<!DOCTYPE html>\n<html><head>\n"
        f"<title>{title}</title>\n"
        f'<meta name="topic" content="glossary-{slug}">\n'
        f'<meta name="summary" content="{summary}">\n'
        '<meta name="keywords" content="glossary">\n'
        '<meta name="level" content="novice">\n'
        '<meta name="track" content="glossary">\n'
        f'<meta name="parent" content="{parent_path}">\n'
        '<meta name="kind" content="glossary">\n'
        "</head><body>\n"
        f'<nav aria-label="Breadcrumb"><p><a href="index.html">Guide</a> / '
        f'<a href="glossary/index.html">Glossary A\u2013Z</a> / '
        f'<a href="{parent_path}">Terms beginning with {letter.upper()}</a> / {title}</p></nav>\n'
        f"<h1>{title}</h1>\n<p>{summary}</p>\n</body></html>\n"
    )


class GlossaryIndexTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name) / "tam"
        self.root.mkdir()
        self.knowledge = self.root / "knowledge"
        (self.knowledge / "glossary" / "a").mkdir(parents=True)
        (self.knowledge / "glossary" / "b").mkdir(parents=True)
        (self.knowledge / "glossary" / "y").mkdir(parents=True)
        (self.knowledge / "glossary" / "z").mkdir(parents=True)
        (self.knowledge / "glossary" / "s").mkdir(parents=True)

        (self.knowledge / "glossary" / "a" / "awg.html").write_text(
            term_page("AWG (American Wire Gauge)", "AWG: Wire gauge.", "a")
        )
        (self.knowledge / "glossary" / "a" / "aad.html").write_text(
            term_page("AAD (Additional Authenticated Data)", "AAD: Auth data.", "a")
        )
        (self.knowledge / "glossary" / "a" / "ags.html").write_text(
            term_page("AGS (Aylur's GTK Shell)", "AGS: GTK shell.", "a")
        )
        (self.knowledge / "glossary" / "y" / "yuv.html").write_text(
            term_page("YUV", "YUV: Color model.", "y")
        )
        (self.knowledge / "glossary" / "s" / "srcinfo.html").write_text(
            term_page(".SRCINFO", ".SRCINFO: AUR metadata.", "s")
        )

    def test_generate_indexes_format_and_idempotency(self):
        glossary_index.generate_glossary_indexes(self.knowledge)
        top = (self.knowledge / "glossary" / "index.html").read_text()
        self.assertIn(
            '<li><a href="glossary/a/index.html">A : AAD (Additional Authenticated Data) - AWG (American Wire Gauge)</a></li>',
            top,
        )
        self.assertIn('<li><a href="glossary/y/index.html">Y : YUV</a></li>', top)
        self.assertIn('<li><a href="glossary/s/index.html">S : .SRCINFO</a></li>', top)
        # Empty folders b and z are omitted.
        self.assertNotIn("glossary/b/index.html", top)
        self.assertNotIn("glossary/z/index.html", top)

        a_idx = (self.knowledge / "glossary" / "a" / "index.html").read_text()
        self.assertIn("Aylur&#x27;s GTK Shell", a_idx)
        self.assertLess(a_idx.index("AAD"), a_idx.index("AGS"))
        self.assertLess(a_idx.index("AGS"), a_idx.index("AWG"))

        mtime_before = (self.knowledge / "glossary" / "index.html").stat().st_mtime_ns
        glossary_index.generate_glossary_indexes(self.knowledge)
        mtime_after = (self.knowledge / "glossary" / "index.html").stat().st_mtime_ns
        self.assertEqual(mtime_before, mtime_after)

    def test_noop_without_glossary_dir(self):
        empty = Path(self.tmp.name) / "empty_knowledge"
        empty.mkdir()
        self.assertEqual(glossary_index.find_misplaced_glossary_pages(empty), [])
        glossary_index.generate_glossary_indexes(empty)
        self.assertFalse((empty / "glossary").exists())

    def test_misplaced_page_detected_and_fixed(self):
        # Put a Z term inside glossary/a/ and link to it from another page.
        (self.knowledge / "glossary" / "a" / "zebra.html").write_text(
            term_page("Zebra", "Zebra: Striped animal.", "a")
        )
        (self.knowledge / "guide.html").write_text(
            '<p>See <a href="glossary/a/zebra.html#top">Zebra</a>.</p>\n'
        )

        with self.assertRaises(glossary_index.GlossaryPlacementError) as ctx:
            glossary_index.check_glossary_placement(self.knowledge, fix=False, interactive=False)
        self.assertIn("--fix-glossary", str(ctx.exception))

        # Declining interactive prompt also fails without moving.
        with self.assertRaises(glossary_index.GlossaryPlacementError):
            glossary_index.check_glossary_placement(
                self.knowledge, fix=False, interactive=True, input_fn=lambda _: "n"
            )
        self.assertTrue((self.knowledge / "glossary" / "a" / "zebra.html").exists())

        # Accepting interactive prompt moves file, updates metadata/links, and enables index regen.
        moves = glossary_index.check_glossary_placement(
            self.knowledge, fix=False, interactive=True, input_fn=lambda _: "y"
        )
        self.assertEqual(moves, [("glossary/a/zebra.html", "glossary/z/zebra.html")])
        self.assertFalse((self.knowledge / "glossary" / "a" / "zebra.html").exists())
        moved = (self.knowledge / "glossary" / "z" / "zebra.html").read_text()
        self.assertIn('<meta name="parent" content="glossary/z/index.html">', moved)
        self.assertIn('<a href="glossary/z/index.html">Terms beginning with Z</a>', moved)
        self.assertIn('href="glossary/z/zebra.html#top"', (self.knowledge / "guide.html").read_text())

        glossary_index.generate_glossary_indexes(self.knowledge)
        top = (self.knowledge / "glossary" / "index.html").read_text()
        self.assertIn('<li><a href="glossary/z/index.html">Z : Zebra</a></li>', top)

    def test_collision_prevents_overwrite(self):
        (self.knowledge / "glossary" / "a" / "yuv.html").write_text(
            term_page("YUV duplicate", "Duplicate.", "a")
        )
        with self.assertRaises(glossary_index.GlossaryPlacementError) as ctx:
            glossary_index.check_glossary_placement(self.knowledge, fix=True)
        self.assertIn("destination already exists", str(ctx.exception))

    def test_pipeline_check_and_fix_glossary(self):
        shutil.copytree(REPO / "tools", self.root / "tools", ignore=shutil.ignore_patterns("__pycache__"))
        (self.root / "src").mkdir()
        for name in ("render.c", "tam.c", "install.c"):
            shutil.copyfile(REPO / "src" / name, self.root / "src" / name)
        (self.knowledge / "index.html").write_text(page("Guide"))
        (self.knowledge / "version").write_text("0.39.0\n")
        (self.root / "tests").mkdir()
        (self.root / "tests" / "guide.sh").write_text("#!/bin/sh\nexit 0\n")
        reader = self.root / "reader"
        reader.write_text('#!/bin/sh\nprintf "Page\\nRev 1 Last updated 2026-11-11 12:00\\n"\n')
        reader.chmod(0o755)
        (self.knowledge / "glossary" / "a" / "zebra.html").write_text(
            term_page("Zebra", "Zebra: Striped animal.", "a")
        )

        with patch.object(sys, "stdin", io.StringIO("")):
            rc_fail = pipeline.run(self.root, "check", str(reader), fix_glossary=False)
        self.assertEqual(rc_fail, 1)

        rc_fix = pipeline.run(self.root, "check", str(reader), fix_glossary=True)
        self.assertEqual(rc_fix, 0)
        self.assertTrue((self.knowledge / "glossary" / "z" / "zebra.html").exists())
        self.assertIn(
            '<li><a href="glossary/z/index.html">Z : Zebra</a></li>',
            (self.knowledge / "glossary" / "index.html").read_text(),
        )
