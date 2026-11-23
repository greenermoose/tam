"""Behavioral tests for the Makefile's check-clean-tree install guard."""

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

MAKEFILE = Path(__file__).resolve().parents[1] / "Makefile"


def git(root, *args):
    subprocess.run(["git", "-C", str(root), *args], check=True,
                   capture_output=True, text=True)


class InstallGuardTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name) / "tam"
        (self.root / "knowledge").mkdir(parents=True)
        (self.root / "src").mkdir()
        shutil.copy(MAKEFILE, self.root / "Makefile")
        (self.root / "src" / "tam.c").write_text("int main(void) { return 0; }\n")
        (self.root / "knowledge" / "page.html").write_text("<h1>Page</h1>\n")
        (self.root / "knowledge" / "index.tsv").write_text("topic\tpath\n")
        git(self.root, "init", "-q")
        git(self.root, "-c", "user.name=t", "-c", "user.email=t@example.invalid",
            "add", ".")
        git(self.root, "-c", "user.name=t", "-c", "user.email=t@example.invalid",
            "commit", "-q", "-m", "init")

    def tearDown(self):
        self.tmp.cleanup()

    def guard(self, *args, root=None):
        return subprocess.run(
            ["make", "--no-print-directory", "-C", str(root or self.root),
             "check-clean-tree", *args],
            capture_output=True, text=True)

    def test_clean_tree_passes(self):
        self.assertEqual(self.guard().returncode, 0)

    def test_page_edit_is_refused(self):
        (self.root / "knowledge" / "page.html").write_text("<h1>Edited</h1>\n")
        res = self.guard()
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("refusing to install from a dirty tree", res.stderr)
        self.assertIn("knowledge/page.html", res.stderr)
        self.assertIn("Commit or discard", res.stderr)

    def test_untracked_command_source_is_refused(self):
        (self.root / "src" / "new.c").write_text("\n")
        res = self.guard()
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("src/new.c", res.stderr)

    def test_stale_index_names_the_follow_up_commit(self):
        (self.root / "knowledge" / "index.tsv").write_text("topic\tpath\trev\n")
        res = self.guard()
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("Only the regenerated corpus index changed", res.stderr)
        self.assertIn("Refresh corpus index revisions after", res.stderr)

    def test_coverage_report_is_a_generated_index_output(self):
        (self.root / "knowledge" / "coverage.json").write_text('{}\n')
        res = self.guard()
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("Only the regenerated corpus index changed", res.stderr)
        self.assertIn("knowledge/coverage.json", res.stderr)

    def test_component_coverage_report_is_a_generated_index_output(self):
        (self.root / "knowledge" / "component-coverage.json").write_text('{}\n')
        res = self.guard()
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("Only the regenerated corpus index changed", res.stderr)
        self.assertIn("knowledge/component-coverage.json", res.stderr)

    def test_paths_outside_the_install_are_ignored(self):
        (self.root / "notes.md").write_text("scratch\n")
        self.assertEqual(self.guard().returncode, 0)

    def test_staged_install_is_not_checked(self):
        (self.root / "knowledge" / "page.html").write_text("<h1>Edited</h1>\n")
        self.assertEqual(self.guard("DESTDIR=/stage").returncode, 0)

    def test_source_without_git_is_not_checked(self):
        plain = Path(self.tmp.name) / "plain"
        shutil.copytree(self.root, plain, ignore=shutil.ignore_patterns(".git"))
        self.assertEqual(self.guard(root=plain).returncode, 0)

    def test_default_prefix_with_nix(self):
        home = str(Path(self.tmp.name) / "home")
        res = subprocess.run(
            ["make", "--no-print-directory", "-C", str(self.root),
             "-f", "Makefile", "-f", "-", "print-prefix"],
            input="print-prefix: ; @echo $(PREFIX)\n",
            capture_output=True, text=True, check=True,
            env={"HOME": home, "NIX_PROFILES": "/nix/var/nix/profiles/default"})
        self.assertEqual(res.stdout.strip(), f"{home}/.local")

    def test_default_prefix_without_nix(self):
        home = str(Path(self.tmp.name) / "home")
        res = subprocess.run(
            ["make", "--no-print-directory", "-C", str(self.root),
             "-f", "Makefile", "-f", "-", "print-prefix"],
            input="print-prefix: ; @echo $(PREFIX)\n",
            capture_output=True, text=True, check=True,
            env={"HOME": home, "PATH": "/bin:/usr/bin"})
        self.assertEqual(res.stdout.strip(), f"{home}/.local")

    def test_explicit_prefix_override(self):
        res = subprocess.run(
            ["make", "--no-print-directory", "-C", str(self.root),
             "PREFIX=/custom/opt",
             "-f", "Makefile", "-f", "-", "print-prefix"],
            input="print-prefix: ; @echo $(PREFIX)\n",
            capture_output=True, text=True, check=True)
        self.assertEqual(res.stdout.strip(), "/custom/opt")

    def test_install_corpus_retains_tldr_when_destdir_empty_and_strips_when_staged(self):
        tldr_page = self.root / "knowledge" / "tldr" / "pages" / "common" / "tar.md"
        tldr_page.parent.mkdir(parents=True)
        tldr_page.write_text("# tar\n")
        git(self.root, "-c", "user.name=t", "-c", "user.email=t@example.invalid",
            "add", ".")
        git(self.root, "-c", "user.name=t", "-c", "user.email=t@example.invalid",
            "commit", "-q", "-m", "add tldr")

        local_prefix = Path(self.tmp.name) / "local-prefix"
        subprocess.run(
            ["make", "--no-print-directory", "-C", str(self.root),
             "install-corpus", f"PREFIX={local_prefix}"],
            check=True, capture_output=True, text=True)
        self.assertTrue((local_prefix / "share/tamlinux/knowledge/tldr/pages/common/tar.md").is_file())

        staged_dest = Path(self.tmp.name) / "staged"
        subprocess.run(
            ["make", "--no-print-directory", "-C", str(self.root),
             "install-corpus", "PREFIX=/usr/lib/tamlinux/current",
             f"DESTDIR={staged_dest}"],
            check=True, capture_output=True, text=True)
        self.assertFalse((staged_dest / "usr/lib/tamlinux/current/share/tamlinux/knowledge/tldr").exists())


if __name__ == "__main__":
    unittest.main()
