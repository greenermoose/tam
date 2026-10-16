"""Tests for tools/lint-corpus.py, run on small fixture trees."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

LINT = Path(__file__).resolve().parent.parent / "tools" / "lint-corpus.py"

RENDER_C = """static const struct { const char *name; unsigned cp; } entities[] = {
    {"amp", '&'}, {"lt", '<'}, {"gt", '>'}, {"rarr", 0x2192},
    {NULL, 0}
};
"""
TAM_C = """if (strcmp(arg, "show") == 0) {
if (strcmp(arg, "install") == 0) {
"""
INSTALL_C = """if (strcmp(sub, "list") == 0) {
    return list();
}
/* padding */
/* padding */
/* padding */
/* padding */
if (strcmp(sub, "plan") == 0) {
    fprintf(stderr, "'plan' is not implemented yet\\n");
}
"""


class LintCorpus(unittest.TestCase):
    """Each case writes one page and checks stdout and exit status exactly."""

    def run_lint(self, page, path="t.html", sources=True):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            target = root / "knowledge" / path
            target.parent.mkdir(parents=True)
            target.write_text(page)
            src = root / "src"
            src.mkdir()
            if sources:
                (src / "render.c").write_text(RENDER_C)
            (src / "tam.c").write_text(TAM_C)
            (src / "install.c").write_text(INSTALL_C)
            result = subprocess.run([sys.executable, str(LINT), "--root", str(root)],
                                    capture_output=True, text=True, timeout=30)
        return result

    def check(self, page, expected, status, path="t.html"):
        result = self.run_lint(page, path)
        self.assertEqual(result.stdout, "".join(line + "\n" for line in expected))
        self.assertEqual(result.returncode, status)

    def finding(self, rule, message, line=1, page="knowledge/t.html"):
        return f"{page}:{line}: {rule}: {message}"

    # A: entities
    def test_a1_known_and_numeric_entities_pass(self):
        self.check("<p>Fine &amp; dandy &rarr; ok &#8594;</p>", [], 0)

    def test_a2_unknown_entity(self):
        self.check("<p>a &times; b</p>",
                   [self.finding("ENTITY", "entity &times; is not decoded by the renderer")], 1)

    def test_a3_comment_ignored(self):
        self.check("<!-- &times; -->", [], 0)

    # B: LaTeX
    def test_b1_dollar_math(self):
        self.check(r"<p>$2 \times 3$</p>", [self.finding("LATEX", "LaTeX is not rendered")], 1)

    def test_b2_pre_ignored(self):
        self.check(r'<pre>echo "$A\n$B"</pre>', [], 0)

    def test_b3_prices_pass(self):
        self.check("<p>costs $5 and $6</p>", [], 0)

    def test_b4_bare_command(self):
        self.check(r"<p>x \approx y</p>", [self.finding("LATEX", "LaTeX is not rendered")], 1)

    # C: private references
    def test_c1_generic_homes_pass(self):
        self.check("<p>/home/alex/src and /home/user/x</p>", [], 0)

    def test_c2_other_home(self):
        self.check("<p>/home/bob/src</p>",
                   [self.finding("PRIVATE", "private reference: /home/bob")], 1)

    def test_c3_private_repository(self):
        self.check("<p>see config-fred-tamlinux</p>",
                   [self.finding("PRIVATE", "private reference: config-fred-tamlinux")], 1)

    def test_c4_public_repository_passes(self):
        self.check("<p>ecosystem-fred-tamlinux registry</p>", [], 0)

    def test_c5_uuid(self):
        uuid = "3abd7a9c-1111-2222-3333-444455556666"
        self.check(f"<p>id {uuid}</p>",
                   [self.finding("PRIVATE", f"private reference: {uuid}")], 1)

    # D: internal identifiers
    def test_d1_decision_id(self):
        self.check("<p>Decision D131 says</p>", [self.finding(
            "INTERNAL-ID", "internal identifier D131; state the reasoning instead")], 1)

    def test_d2_virtue_id(self):
        self.check("<p>Virtue V11</p>", [self.finding(
            "INTERNAL-ID", "internal identifier V11; state the reasoning instead")], 1)

    def test_d3_attribute_values_ignored(self):
        self.check('<meta name="keywords" content="v11, D53">', [], 0)

    def test_d4_short_ids_pass(self):
        self.check("<p>ACPI D3 and DDR4</p>", [], 0)

    # E: subcommands
    def test_e1_known_and_hyphenated_pass(self):
        self.check("<p><code>tam show</code> and <code>tam-deploy status</code></p>", [], 0)

    def test_e2_unknown_subcommand(self):
        self.check("<p><code>tam browse</code></p>", [self.finding(
            "SUBCOMMAND", "tam browse is not a tam subcommand")], 1)

    def test_e3_prose_ignored(self):
        self.check("<p>run tam browse in prose</p>", [], 0)

    def test_e4_help_option_passes(self):
        self.check("<pre>tam --help</pre>", [], 0)

    def test_e5_version_output_passes(self):
        self.check("<pre>tam command 0.25.1 (dev abc1234)</pre>", [], 0)

    # F: install verbs
    def test_f1_verb_and_option_pass(self):
        self.check("<p><code>tam install list</code> and <code>tam install --json</code></p>",
                   [], 0)

    def test_f2_unknown_install_verb(self):
        self.check("<p><code>tam install inspect</code></p>", [self.finding(
            "INSTALL-VERB", "tam install inspect is not an install verb")], 1)

    # G: stubs
    def test_g1_stub_written_as_working(self):
        self.check("<p><code>tam install plan</code> computes a plan.</p>", [self.finding(
            "STUB", "tam install plan is not implemented; mark it planned")], 1)

    def test_g2_stub_marked_planned(self):
        self.check("<li><code>tam install plan</code> (planned)</li>", [], 0)

    # H: worktrees
    def test_h1_worktree_elsewhere(self):
        self.check("<p>Use a Git worktree.</p>", [self.finding(
            "WORKTREE", "worktrees are mentioned only in improve/toolchains.html")], 1)

    def test_h2_worktree_on_toolchains_page(self):
        self.check("<p>Use a Git worktree.</p>", [], 0, path="improve/toolchains.html")

    # I: ordering and errors
    def test_i1_findings_sorted_by_line(self):
        self.check("<p>a &times; b</p>\n<p>/home/bob</p>", [
            self.finding("ENTITY", "entity &times; is not decoded by the renderer"),
            self.finding("PRIVATE", "private reference: /home/bob", line=2)], 1)

    def test_i2_missing_source(self):
        result = self.run_lint("<p>x</p>", sources=False)
        self.assertEqual(result.stdout, "")
        self.assertEqual(result.returncode, 2)
        self.assertIn("src/render.c", result.stderr)


if __name__ == "__main__":
    unittest.main()
