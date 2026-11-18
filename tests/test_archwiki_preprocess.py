"""Tests for tools/build-archwiki-tam.py pre-processor."""

import gzip
import importlib.util
import io
import json
from pathlib import Path
import tarfile
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "build_archwiki_tam", ROOT / "tools/build-archwiki-tam.py"
)
MOD = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MOD)


class ArchWikiPreprocessTest(unittest.TestCase):
    def _make_sample_docs_and_lite(self, base: Path):
        docs = base / "docs/usr/share/doc/arch-wiki/html/en"
        (docs / "Pacman").mkdir(parents=True)
        (docs / "Pacman.html").write_text(
            """<!DOCTYPE html><html><head><title>pacman - ArchWiki</title></head><body>
<div id="toc"><ul><li>Sidebar TOC</li></ul></div>
<div class="mw-content-ltr mw-parser-output">
  <div class="mw-heading mw-heading2"><h2 id="Usage">Usage</h2><span class="mw-editsection">[edit]</span></div>
  <p>See <a href="../en/Pacman/Package_signing.html">signing</a>,
  <a href="../en/systemd.html">case link</a>,
  <a href="../en/Systemdd.html">fuzzy link</a>,
  <a href="../en/Does_Not_Exist.html">missing</a>, and
  <a href="https://man.archlinux.org/man/pacman.8">pacman(8)</a>.</p>
  <div class="archwiki-template-box archwiki-template-box-tip"><strong>Tip</strong> Use <code>-Syu</code>.</div>
  <div class="archwiki-template-box archwiki-template-box-warning"><strong>Warning</strong> Avoid <code>-Sy</code>.</div>
  <script>alert(1)</script><style>.x{}</style><svg><path d="M0"/></svg><button>btn</button>
</div>
<div id="catlinks">Categories</div>
</body></html>""",
            encoding="utf-8",
        )
        (docs / "Pacman/Package_signing.html").write_text(
            """<!DOCTYPE html><html><head><title>Pacman/Package signing - ArchWiki</title></head><body>
<div class="mw-content-ltr mw-parser-output">
  <p>Back to <a href="../../en/Pacman.html#Usage">Pacman</a>.</p>
</div></body></html>""",
            encoding="utf-8",
        )
        (docs / "Systemd.html").write_text(
            """<!DOCTYPE html><html><head><title>systemd - ArchWiki</title></head><body>
<div class="mw-content-ltr mw-parser-output"><p>Init system.</p></div></body></html>
""",
            encoding="utf-8",
        )

        lite_dir = base / "lite"
        lite_dir.mkdir(parents=True)
        (lite_dir / "index").write_text(
            "en/Pacman 00000001\nen/Pacman/Package_signing 00000002\nen/Systemd 00000003\n",
            encoding="utf-8",
        )
        with gzip.open(lite_dir / "arch-wiki.txt.gz", "wt", encoding="utf-8") as gz:
            gz.write("00000001:Usage\n00000002:Back to Pacman\n00000003:Init system\n")
        return docs, lite_dir

    def test_sanitize_link_mending_broken_and_patches(self):
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            docs, lite = self._make_sample_docs_and_lite(base)
            patches = base / "patches"
            patches.mkdir()
            (patches / "0001-mend.patch").write_text(
                "--- a/Systemd.html\n+++ b/Systemd.html\n"
                "@@ -1,2 +1,2 @@\n"
                " <!DOCTYPE html><html><head><title>systemd - ArchWiki</title></head><body>\n"
                "-<div class=\"mw-content-ltr mw-parser-output\"><p>Init system.</p></div></body></html>\n"
                "+<div class=\"mw-content-ltr mw-parser-output\"><p>Patched init system.</p></div></body></html>\n",
                encoding="utf-8",
            )

            out = base / "tam"
            summary = MOD.process_archwiki(docs, lite, out, patches_dir=patches)
            self.assertEqual(summary["pages"], 3)
            self.assertEqual(summary["lite_guided_pages"], 3)
            self.assertEqual(summary["applied_patches"], ["0001-mend.patch"])
            self.assertEqual(summary["broken_count"], 1)
            self.assertGreaterEqual(summary["repaired_count"], 2)

            pacman_html = (out / "Pacman.html").read_text(encoding="utf-8")
            self.assertIn('href="archwiki:Pacman/Package_signing"', pacman_html)
            self.assertIn('href="archwiki:Systemd"', pacman_html)
            self.assertIn('href="tam:archwiki:broken?target=..%2Fen%2FDoes_Not_Exist.html&amp;from=Pacman"', pacman_html)
            self.assertIn('href="https://man.archlinux.org/man/pacman.8"', pacman_html)
            self.assertIn('<aside class="tip">', pacman_html)
            self.assertIn('<aside class="caution">', pacman_html)
            for forbidden in ("<script", "<style", "<svg", "<button", "<div", "<span", "Sidebar TOC", "Categories", "[edit]"):
                self.assertNotIn(forbidden, pacman_html)

            sub_html = (out / "Pacman/Package_signing.html").read_text(encoding="utf-8")
            self.assertIn('href="archwiki:Pacman#Usage"', sub_html)

            sys_html = (out / "Systemd.html").read_text(encoding="utf-8")
            self.assertIn("Patched init system.", sys_html)

            broken = json.loads((out / "broken-links.json").read_text(encoding="utf-8"))
            self.assertEqual(len(broken["broken"]), 1)
            self.assertEqual(broken["broken"][0]["from"], "Pacman")


if __name__ == "__main__":
    unittest.main()
