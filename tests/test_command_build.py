"""Compile small fixtures to verify actual incremental Make behavior."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "C compiler is unavailable")
class CommandBuild(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        shutil.copyfile(REPO / "Makefile", self.root / "Makefile")
        (self.root / "src").mkdir()
        (self.root / "src/tam.c").write_text('int helper(void); int main(void) { return helper(); }\n')
        (self.root / "src/upgrade.c").write_text('int unused(void) { return 0; }\n')
        (self.root / "src/other.c").write_text('#include "used.h"\nint helper(void) { return VALUE; }\n')
        (self.root / "src/used.h").write_text('#define VALUE 0\n')
        (self.root / "src/unused.h").write_text('#define UNUSED 1\n')
        self.args = ["make", "--no-print-directory", "command", "SRC=src/tam.c src/upgrade.c src/other.c",
                     "LIBTAM_CFLAGS=", "LIBTAM_LIBS=", "LIBTAM_ARCHIVE=", "TERMINFO_CFLAGS=", "TERMINFO_LIBS=", "LDLIBS="]

    def build(self, *options):
        result = subprocess.run([*self.args, *options], cwd=self.root,
                                env={**os.environ, "MAKEFLAGS": "", "MFLAGS": ""}, capture_output=True, text=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return result.stdout

    def test_only_real_dependants_recompile_and_link_flags_only_relink(self):
        self.build()
        self.assertEqual(self.build(), "")
        (self.root / "src/unused.h").write_text('#define UNUSED 2\n')
        self.assertEqual(self.build(), "")
        (self.root / "src/used.h").write_text('#define VALUE 1\n')
        out = self.build()
        self.assertIn('-c src/other.c', out)
        self.assertNotIn('-c src/tam.c', out)
        self.assertNotIn('-c src/upgrade.c', out)
        out = self.build('LDFLAGS=-Wl,--as-needed')
        self.assertNotIn(' -c ', out)
        self.assertIn(' -o build/tam', out)
        out = self.build('CFLAGS=-O0')
        self.assertEqual(out.count(' -c '), 3)

    def test_identity_changes_touch_only_identity_consumers(self):
        self.build('TAM_BUILD_ID=before', 'TAM_SOURCE_REVISION=before')
        out = self.build('TAM_BUILD_ID=after', 'TAM_SOURCE_REVISION=after')
        self.assertIn('-c src/tam.c', out)
        self.assertIn('-c src/upgrade.c', out)
        self.assertNotIn('-c src/other.c', out)
        self.assertEqual(self.build('TAM_BUILD_ID=after', 'TAM_SOURCE_REVISION=after'), '')

    def test_static_archive_changes_relink(self):
        archive = self.root / 'libtam.a'
        archive.write_bytes(b'!<arch>\n')
        arg = f'LIBTAM_ARCHIVE={archive}'
        self.build(arg)
        self.assertEqual(self.build(arg), '')
        archive.write_bytes(b'!<arch>\n')
        out = self.build(arg)
        self.assertNotIn(' -c ', out)
        self.assertIn(' -o build/tam', out)


if __name__ == '__main__':
    unittest.main()
