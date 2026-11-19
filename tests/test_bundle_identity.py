"""Identity follows the active cultivar; ownership falls back to native tools."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

BIN=Path(os.environ.get('TAM_TEST_BIN','build/tam')).resolve()


class CultivarIdentity(unittest.TestCase):
    def test_cultivar_command_ignores_stale_per_user_version(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'bin').mkdir();shutil.copy2(BIN,root/'bin/tam')
            config=root/'config/tamlinux';config.mkdir(parents=True)
            (config/'version').write_text('0.1.0\n');(config/'cultivar').write_text('Stale\n')
            (root/'VERSION').write_text('1.2.3-dev\n')
            cultivar=root/'share/tamlinux/cultivar';cultivar.parent.mkdir(parents=True);cultivar.write_text('CultivarName\n')
            env={**os.environ,'XDG_CONFIG_HOME':str(root/'config'),'TAMLINUX_VERSION':'0.0.0'}
            output=subprocess.check_output([str(root/'bin/tam'),'--version'],env=env,text=True)
            self.assertIn('Tamlinux cultivar CultivarName 1.2.3-dev',output)
            self.assertNotIn('Stale',output)
            (root/'VERSION').unlink()
            (root/'BUNDLE').write_text('format 1\nname tamlinux\nversion 1.2.4-dev\n')
            output=subprocess.check_output([str(root/'bin/tam'),'--version'],env=env,text=True)
            self.assertIn('Tamlinux cultivar CultivarName 1.2.4-dev',output)
            (root/'BUNDLE').write_text('format 99\nname tamlinux\nversion 9.9.9\n')
            output=subprocess.check_output([str(root/'bin/tam'),'--version'],env=env,text=True)
            self.assertIn('Stale 0.1.0',output)

    def test_where_uses_cultivar_manifest_then_native_and_passes_paths_as_arguments(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);marker=root/'native-called'
            state=root/'state';manifests=state/'manifests';manifests.mkdir(parents=True)
            digest='a'*64
            (state/'active.json').write_text(f'{{"current": "{digest}"}}\n')
            (manifests/f'{digest}.json').write_text('{"files": {"usr/bin/tam-owned": [493, "0"]}}\n')
            native=root/'pacman';native.write_text('#!/bin/sh\nprintf "%s\\n" "$3"\n: > "'+str(marker)+'"\n');native.chmod(0o755)
            env={**os.environ,'PATH':str(root),'TAM_STATE_DIR':str(state)}
            output=subprocess.check_output([str(BIN),'where','/usr/bin/tam-owned'],env=env,text=True)
            self.assertIn('/usr/bin/tam-owned is owned by tamlinux ',output)
            self.assertFalse(marker.exists())
            path='a file; touch should-not-run'
            output=subprocess.check_output([str(BIN),'where',path],env=env,text=True)
            self.assertEqual(output,path+'\n');self.assertTrue(marker.exists())


if __name__=='__main__':unittest.main()
