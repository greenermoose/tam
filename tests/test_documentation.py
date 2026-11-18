"""Host capability markup and native identity, independent of installed state."""
from pathlib import Path
import itertools
import os
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]


def _libtam_prefix():
    """The libtam the C fixtures link: the development prefix first, then
    wherever pkg-config reports one, so a bare `make test-command` works on a
    packaged host as well."""
    local=Path.home()/'.local'
    if (local/'lib/libtam.a').is_file():
        return local
    found=subprocess.run(['pkg-config','--variable=prefix','libtam'],
                         capture_output=True,text=True)
    if found.returncode==0 and found.stdout.strip():
        return Path(found.stdout.strip())
    return local


LIB=Path(os.environ.get('LIBTAM_TEST_PREFIX') or _libtam_prefix())

IDENTITY_MAIN = r'''
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
int main(void) {
    char *label = cultivar_label();
    char *path = cultivar_path("version");
    puts(label ? label : "unavailable");
    puts(path ? path : "unavailable");
    free(label);
    free(path);
}
'''


class Documentation(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        self.env={**os.environ,'HOME':str(self.root),'XDG_DATA_HOME':str(self.root/'user'),
                  'TAM_DOC_DIR':str(self.root/'doc'),'TAM_TLDR_DIR':str(self.root/'doc/tldr'),
                  'TAM_ARCHWIKI_DIR':str(self.root/'doc/arch-wiki/html/en'),'INFOPATH':''}

    def compile(self,name,sources,main,*flags):
        source=self.root/(name+'.c');source.write_text(main)
        binary=self.root/name
        subprocess.run(['cc','-D_GNU_SOURCE','-std=c11','-I'+str(ROOT/'src'),'-I'+str(LIB/'include'),
                        *flags,*[str(ROOT/s) for s in sources],str(source),str(LIB/'lib/libtam.a'),'-lz','-o',str(binary)],check=True,capture_output=True)
        return binary

    def test_all_eight_documentation_combinations_and_no_dead_links(self):
        binary=self.compile('markup',['src/documentation.c','src/html_scan.c'],r'''
#include "documentation.h"
#include <stdio.h>
#include <stdlib.h>
char *tam_find_tldr_root(const char *unused) {(void)unused; return tam_documentation_root("tldr");}
char *tam_find_archwiki_root(void) {return tam_documentation_root("archwiki");}
int main(void) {
 const char *html="<table><tr><td><a href='info:dir' data-doc-source='info'>Info</a></td><td>Description<span data-doc-missing='info'> MISSING-info</span></td></tr>"
 "<tr><td><a href='tam:tldr' data-doc-source='tldr'>tldr</a></td><td>Description<span data-doc-missing='tldr'> MISSING-tldr</span></td></tr>"
 "<tr><td><a href='tam:archwiki' data-doc-source='archwiki'>Wiki</a></td><td>Description<span data-doc-missing='archwiki'><span> MISSING-archwiki</span></span></td></tr></table>";
 char *out=tam_documentation_markup(html, __builtin_strlen(html));if(!out)return 1;puts(out);free(out);return 0;
}''')
        files=[self.root/'doc/info/bash.info',self.root/'doc/tldr/pages/common/tar.md',self.root/'doc/arch-wiki/tam/Pacman.html']
        for flags in itertools.product((False,True),repeat=3):
            for present,path in zip(flags,files):
                path.parent.mkdir(parents=True,exist_ok=True)
                path.write_text('content' if present else '')
            text=subprocess.check_output([binary],env=self.env,text=True)
            for present,ident,href in zip(flags,['info','tldr','archwiki'],['info:dir','tam:tldr','tam:archwiki']):
                self.assertEqual("href='"+href+"'" in text,present)
                self.assertEqual('MISSING-'+ident in text,not present)
        # An empty directory or Info directory index alone does not count.
        for path in files:path.unlink()
        (self.root/'doc/info/dir').write_text('index only')
        text=subprocess.check_output([binary],env=self.env,text=True)
        self.assertNotIn('<a ',text)
        # Reopening after installing/removing files observes the new state.
        files[0].write_text('manual')
        self.assertIn("href='info:dir'",subprocess.check_output([binary],env=self.env,text=True))
        files[0].unlink()
        self.assertNotIn("href='info:dir'",subprocess.check_output([binary],env=self.env,text=True))

    def test_archwiki_prefers_tam_and_falls_back_to_html_en(self):
        binary=self.compile('awroot',['src/documentation.c','src/html_scan.c'],r'''
#include "documentation.h"
#include <stdio.h>
#include <stdlib.h>
char *tam_find_tldr_root(const char *unused) {(void)unused; return tam_documentation_root("tldr");}
char *tam_find_archwiki_root(void) {return tam_documentation_root("archwiki");}
int main(void) {
    char *p = tam_documentation_root("archwiki");
    puts(p ? p : "");
    free(p);
    return 0;
}''')
        html_dir = self.root/'doc/arch-wiki/html/en'
        tam_dir = self.root/'doc/arch-wiki/tam'
        html_dir.mkdir(parents=True)
        self.assertEqual(subprocess.check_output([binary],env=self.env,text=True).strip(), str(html_dir))
        tam_dir.mkdir(parents=True)
        self.assertEqual(subprocess.check_output([binary],env=self.env,text=True).strip(), str(tam_dir))

        # When TAM_DOC_DIR is unset, prefer $XDG_DATA_HOME/doc/arch-wiki/tam
        # and ~/.local/share/doc/arch-wiki/tam ahead of TAM_DOCDIR.
        user_xdg_tam = self.root/'user/doc/arch-wiki/tam'
        user_xdg_tam.mkdir(parents=True)
        env_no_doc = {k: v for k, v in self.env.items() if k != 'TAM_DOC_DIR'}
        self.assertEqual(subprocess.check_output([binary],env=env_no_doc,text=True).strip(), str(user_xdg_tam))

        home_tam = self.root/'.local/share/doc/arch-wiki/tam'
        home_tam.mkdir(parents=True)
        env_no_xdg = {k: v for k, v in env_no_doc.items() if k != 'XDG_DATA_HOME'}
        self.assertEqual(subprocess.check_output([binary],env=env_no_xdg,text=True).strip(), str(home_tam))

    def test_native_identity_uses_format_two_and_never_stale_fallback(self):
        native=self.root/'native';native.mkdir()
        (native/'BUNDLE').write_text('format 2\nname tamlinux\nversion 0.6.28\nsource tam abc\n')
        cultivar=native/'share/tamlinux/cultivar';cultivar.parent.mkdir(parents=True);cultivar.write_text('KBN WB\n')
        binary=self.compile('identity',['src/identity.c'],IDENTITY_MAIN,'-DTAM_BUNDLE_ROOT="'+str(native)+'"')
        env={**self.env,'TAMLINUX_VERSION':'0.6.24','XDG_CONFIG_HOME':str(self.root/'config')}
        stale=self.root/'config/tamlinux';stale.mkdir(parents=True);(stale/'version').write_text('0.6.24\n')
        text=subprocess.check_output([binary],env=env,text=True)
        self.assertEqual(text.splitlines(),['KBN WB 0.6.28',str(native/'BUNDLE')])
        for bad in ('format 99\nname tamlinux\nversion 9\n','format 2\nname tamlinux\nversion 1\nversion 2\n','format 2\nname other\nversion 9\n'):
            (native/'BUNDLE').write_text(bad)
            self.assertEqual(subprocess.check_output([binary],env=env,text=True).splitlines()[0],'unavailable')

    def test_development_command_follows_active_bundle(self):
        active = self.root / 'current'
        native = self.root / 'version' / 'tamlinux-0.6.30'
        cultivar = native / 'share/tamlinux/cultivar'
        cultivar.parent.mkdir(parents=True)
        cultivar.write_text('KBN WB\n')
        manifest = native / 'BUNDLE'
        manifest.write_text('format 2\nname tamlinux\nversion 0.6.30\nsource tam abc\n')
        active.symlink_to(native, target_is_directory=True)
        binary = self.compile('development', ['src/identity.c'], IDENTITY_MAIN,
                              f'-DTAM_ACTIVE_BUNDLE_ROOT="{active}"')

        config = self.root / 'config' / 'tamlinux'
        config.mkdir(parents=True)
        (config / 'version').write_text('0.6.24\n')
        (config / 'cultivar').write_text('Legacy\n')
        env = {**self.env, 'XDG_CONFIG_HOME': str(config.parent), 'TAMLINUX_VERSION': '0.6.25'}
        env.pop('PKEXEC_UID', None)

        def identity():
            return subprocess.check_output([binary], env=env, text=True).splitlines()

        self.assertEqual(identity(), ['KBN WB 0.6.30', str(active / 'BUNDLE')])
        cultivar.unlink()
        self.assertEqual(identity(), ['0.6.30', str(active / 'BUNDLE')])
        manifest.write_text('format 2\nname other\nversion 0.6.30\n')
        self.assertEqual(identity(), ['Legacy 0.6.24', str(config / 'version')])
        active.unlink()
        self.assertEqual(identity(), ['Legacy 0.6.24', str(config / 'version')])

if __name__=='__main__':unittest.main()
