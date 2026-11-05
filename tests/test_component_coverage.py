"""Unit tests for tools/report-component-coverage.py (plan 26 W1)."""

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

TOOL = Path(__file__).resolve().parents[1] / "tools" / "report-component-coverage.py"
spec = importlib.util.spec_from_file_location("component_coverage_tool", str(TOOL))
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


class ComponentCoverage(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.tam = self.root / "tam"
        self.product = self.root / "tamlinux"
        write(self.tam / "knowledge" / "index.html", "<html><body>Guide</body></html>\n")
        write(
            self.tam / "tools" / "component-coverage-exemptions.tsv",
            "# id\tkind\treason\tdate\n",
        )
        write(self.product / "commands" / "tam-demo" / "README.md", "demo\n")
        write(self.product / "commands" / "tam-hidden-helper" / "README.md", "helper\n")
        write(
            self.product / "desktop" / "plugins" / "fred.demo" / "manifest.json",
            "{}\n",
        )
        write(
            self.product
            / "desktop"
            / "session"
            / "hypr"
            / "tamlinux"
            / "bindings"
            / "tiling.lua",
            "-- binds\n",
        )

    def tearDown(self):
        self.tmp.cleanup()

    def test_inventory_kinds(self):
        rows = mod.inventory(self.product)
        ids = {r["id"]: r["kind"] for r in rows}
        self.assertEqual(ids["tam-demo"], "command")
        self.assertEqual(ids["fred.demo"], "plugin")
        self.assertEqual(ids["tiling.lua"], "bindings")

    def test_mention_covers_and_gap(self):
        write(
            self.tam / "knowledge" / "use" / "demo.html",
            "<p>Run <code>tam-demo</code> and open fred.demo.</p>\n",
        )
        report = mod.build_report(self.tam, self.product)
        by_id = {r["id"]: r for r in report["components"]}
        self.assertEqual(by_id["tam-demo"]["status"], "covered")
        self.assertIn("use/demo.html", by_id["tam-demo"]["pages"])
        self.assertEqual(by_id["fred.demo"]["status"], "covered")
        self.assertEqual(by_id["tam-hidden-helper"]["status"], "gap")
        self.assertEqual(by_id["tiling.lua"]["status"], "gap")

    def test_exemption_marks_exempt(self):
        write(
            self.tam / "tools" / "component-coverage-exemptions.tsv",
            "tam-hidden-helper\tcommand\tinternal helper\t2026-11-04\n",
        )
        report = mod.build_report(self.tam, self.product)
        by_id = {r["id"]: r for r in report["components"]}
        self.assertEqual(by_id["tam-hidden-helper"]["status"], "exempt")
        self.assertEqual(report["counts"]["exempt"], 1)
        self.assertEqual(report["counts"]["gap"], 3)  # tam-demo, fred.demo, tiling.lua

    def test_check_fails_on_gaps(self):
        code = mod.main([
            "--root", str(self.tam),
            "--tamlinux", str(self.product),
            "--quiet",
            "--check",
        ])
        self.assertEqual(code, 1)

    def test_json_write(self):
        out = self.tam / "knowledge" / "component-coverage.json"
        code = mod.main([
            "--root", str(self.tam),
            "--tamlinux", str(self.product),
            "--quiet",
            "--json", str(out),
        ])
        self.assertEqual(code, 0)
        data = json.loads(out.read_text(encoding="utf-8"))
        self.assertEqual(data["counts"]["components"], 4)

    def test_missing_product_skips_inventory(self):
        report = mod.build_report(self.tam, None)
        self.assertIsNone(report["product_root"])
        self.assertEqual(report["counts"]["components"], 0)


if __name__ == "__main__":
    unittest.main()
