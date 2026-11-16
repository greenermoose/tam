"""Host capability markup and native identity, independent of installed state."""
from pathlib import Path
import itertools
import os
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
LIB=Path(os.environ.get('LIBTAM_TEST_PREFIX',str(Path.home()/'.local/dev')))


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
        files=[self.root/'doc/info/bash.info',self.root/'doc/tldr/pages/common/tar.md',self.root/'doc/arch-wiki/html/en/Pacman.html']
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

    def test_native_identity_uses_format_two_and_never_stale_fallback(self):
        native=self.root/'native';native.mkdir()
        (native/'BUNDLE').write_text('format 2\nname tamlinux\nversion 0.6.28\nsource tam abc\n')
        cultivar=native/'share/tamlinux/cultivar';cultivar.parent.mkdir(parents=True);cultivar.write_text('KBN WB\n')
        binary=self.compile('identity',['src/identity.c'],r'''
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
int main(void) { char *v=cultivar_label(),*p=cultivar_path("version");puts(v?v:"unavailable");puts(p?p:"unavailable");free(v);free(p); }
''','-DTAM_BUNDLE_ROOT="'+str(native)+'"')
        env={**self.env,'TAMLINUX_VERSION':'0.6.24','XDG_CONFIG_HOME':str(self.root/'config')}
        stale=self.root/'config/tamlinux';stale.mkdir(parents=True);(stale/'version').write_text('0.6.24\n')
        text=subprocess.check_output([binary],env=env,text=True)
        self.assertEqual(text.splitlines(),['KBN WB 0.6.28',str(native/'BUNDLE')])
        for bad in ('format 99\nname tamlinux\nversion 9\n','format 2\nname tamlinux\nversion 1\nversion 2\n','format 2\nname other\nversion 9\n'):
            (native/'BUNDLE').write_text(bad)
            self.assertEqual(subprocess.check_output([binary],env=env,text=True).splitlines()[0],'unavailable')

if __name__=='__main__':unittest.main()
